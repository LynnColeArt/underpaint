// SPDX-License-Identifier: GPL-3.0-or-later
#include "desktop/ai/aijobrunner.h"
#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QJsonParseError>
#include <QProcess>
#include <QProcessEnvironment>
#include <QSaveFile>
#include <QTemporaryDir>

namespace ai {

namespace {

bool writeJsonFile(
	const QString &path, const QJsonDocument &document, QString &outError)
{
	QSaveFile file(path);
	if(!file.open(QIODevice::WriteOnly)) {
		outError = file.errorString();
		return false;
	}
	file.write(document.toJson(QJsonDocument::Indented));
	if(!file.commit()) {
		outError = file.errorString();
		return false;
	}
	return true;
}

QJsonDocument readJsonFile(const QString &path, QString &outError)
{
	QFile file(path);
	if(!file.open(QIODevice::ReadOnly)) {
		outError = file.errorString();
		return QJsonDocument();
	}

	QJsonParseError parseError;
	QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
	if(parseError.error != QJsonParseError::NoError) {
		outError = parseError.errorString();
		return QJsonDocument();
	}
	if(!document.isObject()) {
		outError = QStringLiteral("Expected a JSON object.");
		return QJsonDocument();
	}
	return document;
}

void processProgressLines(
	QByteArray &buffer, const QByteArray &chunk,
	const JobRunner::ProgressCallback &progressCallback)
{
	if(!progressCallback || chunk.isEmpty()) {
		buffer += chunk;
		return;
	}

	buffer += chunk;
	int lineEnd = buffer.indexOf('\n');
	while(lineEnd >= 0) {
		const QByteArray line = buffer.left(lineEnd).trimmed();
		buffer.remove(0, lineEnd + 1);
		if(line.startsWith('{')) {
			QJsonParseError parseError;
			const QJsonDocument document = QJsonDocument::fromJson(line, &parseError);
			if(parseError.error == QJsonParseError::NoError && document.isObject()) {
				const QJsonObject object = document.object();
				if(object.contains(QStringLiteral("type"))) {
					progressCallback(object);
				}
			}
		}
		lineEnd = buffer.indexOf('\n');
	}
}

}

QString JobRunner::defaultWorkerPath()
{
	const QString configuredWorker =
		QProcessEnvironment::systemEnvironment().value(
			QStringLiteral("UNDERPAINT_AI_WORKER"));
	if(!configuredWorker.isEmpty()) {
		return configuredWorker;
	}

	QString executable = QStringLiteral("underpaint-ai-worker-stub");
#ifdef Q_OS_WIN
	executable += QStringLiteral(".exe");
#endif
	return QDir(QCoreApplication::applicationDirPath()).filePath(executable);
}

JobRunResult JobRunner::run(
	const JobRequest &request, const QString &workerPath, int timeoutMsec,
	const ProgressCallback &progressCallback,
	const std::atomic_bool *cancelRequested)
{
	JobRunResult result;

	QTemporaryDir jobDir(
		QDir::temp().filePath(QStringLiteral("underpaint-ai-job-XXXXXX")));
	if(!jobDir.isValid()) {
		result.errorMessage = QStringLiteral("Could not create AI job directory.");
		return result;
	}
	jobDir.setAutoRemove(false);
	result.jobDirectoryPath = jobDir.path();
	result.requestPath =
		QDir(jobDir.path()).filePath(QStringLiteral("request.json"));
	result.responsePath =
		QDir(jobDir.path()).filePath(QStringLiteral("response.json"));

	QString error;
	if(!writeJsonFile(result.requestPath, request.toJsonDocument(), error)) {
		result.errorMessage =
			QStringLiteral("Could not write AI request: %1").arg(error);
		return result;
	}

	const QString resolvedWorkerPath =
		workerPath.isEmpty() ? defaultWorkerPath() : workerPath;
	result.resolvedWorkerPath = resolvedWorkerPath;
	QProcess process;
	process.setProgram(resolvedWorkerPath);
	process.setArguments(
		{result.requestPath, result.responsePath, result.jobDirectoryPath});
	process.setProcessChannelMode(QProcess::SeparateChannels);
	process.start();
	if(!process.waitForStarted()) {
		result.errorMessage = QStringLiteral("Could not start AI worker: %1")
								  .arg(process.errorString());
		return result;
	}

	QElapsedTimer timer;
	timer.start();
	QByteArray standardOutputBuffer;
	bool timedOut = false;
	bool canceled = false;
	while(!process.waitForFinished(100)) {
		const QByteArray stdoutChunk = process.readAllStandardOutput();
		result.standardOutput += stdoutChunk;
		processProgressLines(
			standardOutputBuffer, stdoutChunk, progressCallback);
		result.standardError += process.readAllStandardError();
		if(cancelRequested && cancelRequested->load()) {
			canceled = true;
			break;
		}
		if(timeoutMsec >= 0 && timer.elapsed() >= timeoutMsec) {
			timedOut = true;
			break;
		}
	}
	if(canceled) {
		process.kill();
		process.waitForFinished(3000);
		result.canceled = true;
		result.errorMessage = QStringLiteral("AI worker was canceled.");
		const QByteArray stdoutChunk = process.readAllStandardOutput();
		result.standardOutput += stdoutChunk;
		processProgressLines(
			standardOutputBuffer, stdoutChunk, progressCallback);
		result.standardError += process.readAllStandardError();
		return result;
	}
	if(timedOut) {
		process.kill();
		process.waitForFinished(3000);
		result.errorMessage =
			QStringLiteral("AI worker timed out after %1 ms.").arg(timeoutMsec);
		const QByteArray stdoutChunk = process.readAllStandardOutput();
		result.standardOutput += stdoutChunk;
		processProgressLines(
			standardOutputBuffer, stdoutChunk, progressCallback);
		result.standardError += process.readAllStandardError();
		return result;
	}

	result.exitCode = process.exitCode();
	const QByteArray stdoutChunk = process.readAllStandardOutput();
	result.standardOutput += stdoutChunk;
	processProgressLines(standardOutputBuffer, stdoutChunk, progressCallback);
	result.standardError += process.readAllStandardError();

	if(QFile::exists(result.responsePath)) {
		QJsonDocument responseDocument = readJsonFile(result.responsePath, error);
		if(!responseDocument.isNull()) {
			result.response = JobResponse::fromJsonDocument(responseDocument);
		}
	}

	const bool processFailed =
		process.exitStatus() != QProcess::NormalExit || result.exitCode != 0;
	if(processFailed) {
		result.errorMessage = result.response.message.isEmpty()
								  ? QStringLiteral(
										"AI worker failed with exit code %1.")
										.arg(result.exitCode)
								  : result.response.message;
		return result;
	}

	if(result.response.id.isEmpty()) {
		result.errorMessage =
			QStringLiteral("Could not read AI response: %1").arg(error);
		return result;
	}

	result.ok = true;
	return result;
}

}
