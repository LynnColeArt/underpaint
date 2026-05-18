// SPDX-License-Identifier: GPL-3.0-or-later
#include "desktop/ai/aijob.h"
#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QImage>
#include <QJsonParseError>
#include <QPainter>
#include <QSaveFile>
#include <QTextStream>

namespace {

void printUsage(const QString &program)
{
	QTextStream err(stderr);
	err << "Usage: " << program
		<< " <request.json> <response.json> <job-directory>\n";
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

bool writePlaceholderImage(
	const ai::JobRequest &request, const QString &path, QString &outError)
{
	Q_UNUSED(request);

	QImage image(512, 512, QImage::Format_ARGB32_Premultiplied);
	image.fill(QColor(31, 34, 38));

	QPainter painter(&image);
	painter.setRenderHint(QPainter::Antialiasing);
	QLinearGradient gradient(0, 0, 512, 512);
	gradient.setColorAt(0.0, QColor(45, 91, 140));
	gradient.setColorAt(0.55, QColor(104, 76, 142));
	gradient.setColorAt(1.0, QColor(38, 44, 50));
	painter.fillRect(image.rect(), gradient);
	painter.setPen(QPen(QColor(255, 255, 255, 48), 2));
	for(int i = 0; i < 512; i += 32) {
		painter.drawLine(i, 0, i, 512);
		painter.drawLine(0, i, 512, i);
	}
	painter.setBrush(QColor(255, 255, 255, 180));
	painter.setPen(Qt::NoPen);
	painter.drawRoundedRect(QRect(88, 188, 336, 48), 8, 8);
	painter.setBrush(QColor(255, 255, 255, 96));
	painter.drawRoundedRect(QRect(128, 260, 256, 24), 6, 6);
	painter.drawRoundedRect(QRect(164, 304, 184, 24), 6, 6);
	painter.end();

	if(!image.save(path, "PNG")) {
		outError = QStringLiteral("Could not save placeholder image.");
		return false;
	}
	return true;
}

int writeFailureResponse(
	const QString &responsePath, const QString &id, const QString &message)
{
	ai::JobResponse response;
	response.id = id;
	response.status = ai::JobStatus::Failed;
	response.message = message;
	response.provenance = QJsonObject{
		{QStringLiteral("backend"), QStringLiteral("worker-stub")},
	};
	QString error;
	if(!writeJsonFile(responsePath, response.toJsonDocument(), error)) {
		QTextStream(stderr) << "Could not write failure response: " << error
							<< "\n";
		return 2;
	}
	return 1;
}

}

int main(int argc, char **argv)
{
	QCoreApplication app(argc, argv);
	const QStringList args = app.arguments();
	if(args.size() != 4) {
		printUsage(args.value(0));
		return 2;
	}

	QElapsedTimer timer;
	timer.start();

	const QString requestPath = args.at(1);
	const QString responsePath = args.at(2);
	const QString jobDirectory = args.at(3);

	QString error;
	QJsonDocument requestDocument = readJsonFile(requestPath, error);
	if(requestDocument.isNull()) {
		return writeFailureResponse(
			responsePath, QString(), QStringLiteral("Invalid request: %1").arg(error));
	}

	ai::JobRequest request = ai::JobRequest::fromJsonDocument(requestDocument);
	if(request.id.isEmpty()) {
		return writeFailureResponse(
			responsePath, request.id, QStringLiteral("Request id is missing."));
	}
	if(request.parameters.value(QStringLiteral("forceFailure")).toBool()) {
		return writeFailureResponse(
			responsePath, request.id,
			QStringLiteral("Forced failure requested by job parameters."));
	}

	QDir dir(jobDirectory);
	if(!dir.exists() && !dir.mkpath(QStringLiteral("."))) {
		return writeFailureResponse(
			responsePath, request.id,
			QStringLiteral("Could not create job directory."));
	}

	const QString imagePath = dir.filePath(QStringLiteral("candidate-1.png"));
	if(!writePlaceholderImage(request, imagePath, error)) {
		return writeFailureResponse(responsePath, request.id, error);
	}

	ai::JobCandidate candidate;
	candidate.id = QStringLiteral("candidate-1");
	candidate.label = QStringLiteral("Worker Stub Candidate");
	candidate.imagePath = imagePath;
	candidate.metadata = QJsonObject{
		{QStringLiteral("operation"), ai::operationKey(request.operation)},
		{QStringLiteral("placeholder"), true},
	};

	ai::JobResponse response;
	response.id = request.id;
	response.status = ai::JobStatus::Succeeded;
	response.message = QStringLiteral("Generated one placeholder candidate.");
	response.candidates.append(candidate);
	response.diagnostics = QJsonObject{
		{QStringLiteral("elapsedMsec"), int(timer.elapsed())},
	};
	response.provenance = QJsonObject{
		{QStringLiteral("backend"), QStringLiteral("worker-stub")},
		{QStringLiteral("schema"), ai::schemaVersion()},
	};

	if(!writeJsonFile(responsePath, response.toJsonDocument(), error)) {
		QTextStream(stderr) << "Could not write response: " << error << "\n";
		return 2;
	}

	return 0;
}
