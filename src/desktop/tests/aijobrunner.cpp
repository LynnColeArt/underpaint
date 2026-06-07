// SPDX-License-Identifier: GPL-3.0-or-later
#include "desktop/ai/aijobrunner.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QJsonObject>
#include <QPainter>
#include <QTemporaryDir>
#include <QtTest/QtTest>

class AiJobRunnerTest final : public QObject {
	Q_OBJECT
private slots:
	void workerStubProducesCandidatesAndProgress();
	void workerStubClampsCandidateCount();
	void workerStubProducesObjectDecompositionMetadata();
	void forcedWorkerFailureIsReported();

private:
	static QString workerPath();
	static ai::JobRequest makeRequest(
		QTemporaryDir &dir, ai::Operation operation = ai::Operation::Inpaint);
};

QString AiJobRunnerTest::workerPath()
{
	QString executable = QStringLiteral("underpaint-ai-worker-stub");
#ifdef Q_OS_WIN
	executable += QStringLiteral(".exe");
#endif
	const QString path =
		QDir(QCoreApplication::applicationDirPath()).filePath(executable);
	return path;
}

ai::JobRequest AiJobRunnerTest::makeRequest(QTemporaryDir &dir, ai::Operation operation)
{
	Q_ASSERT(dir.isValid());
	const QString sourcePath = QDir(dir.path()).filePath(QStringLiteral("source.png"));
	const QString maskPath = QDir(dir.path()).filePath(QStringLiteral("mask.png"));

	QImage source(32, 24, QImage::Format_ARGB32_Premultiplied);
	source.fill(QColor(24, 28, 32));
	QPainter sourcePainter(&source);
	sourcePainter.fillRect(QRect(0, 0, 16, 24), QColor(80, 120, 180));
	sourcePainter.fillRect(QRect(16, 0, 16, 24), QColor(190, 140, 80));
	sourcePainter.end();
	Q_ASSERT(source.save(sourcePath, "PNG"));

	QImage mask(32, 24, QImage::Format_Grayscale8);
	mask.fill(0);
	QPainter maskPainter(&mask);
	maskPainter.fillRect(QRect(8, 6, 16, 12), QColor(255, 255, 255));
	maskPainter.end();
	Q_ASSERT(mask.save(maskPath, "PNG"));

	ai::JobRequest request = ai::JobRequest::create(operation);
	request.id = QStringLiteral("test-job");
	request.region = QJsonObject{
		{QStringLiteral("x"), 0},
		{QStringLiteral("y"), 0},
		{QStringLiteral("width"), source.width()},
		{QStringLiteral("height"), source.height()},
	};
	request.inputs.append(ai::JobAsset{
		QStringLiteral("source-image"),
		sourcePath,
		QStringLiteral("image/png"),
		QJsonObject{},
	});
	request.inputs.append(ai::JobAsset{
		QStringLiteral("mask"),
		maskPath,
		QStringLiteral("image/png"),
		QJsonObject{{QStringLiteral("channel"), QStringLiteral("alpha")}},
	});
	request.provenance = QJsonObject{
		{QStringLiteral("createdBy"), QStringLiteral("aijobrunner-test")},
	};
	return request;
}

void AiJobRunnerTest::workerStubProducesCandidatesAndProgress()
{
	QTemporaryDir dir;
	ai::JobRequest request = makeRequest(dir);
	request.parameters = QJsonObject{
		{QStringLiteral("candidateCount"), 3},
		{QStringLiteral("seed"), 123},
	};

	QVector<QJsonObject> progressEvents;
	const QString stubPath = workerPath();
	QVERIFY2(QFileInfo::exists(stubPath), qPrintable(QStringLiteral("Missing worker stub at %1").arg(stubPath)));
	const ai::JobRunResult result = ai::JobRunner::run(
		request, stubPath, 10000,
		[&progressEvents](const QJsonObject &event) {
			progressEvents.append(event);
		});

	QVERIFY2(result.ok, qPrintable(result.errorMessage + QString::fromUtf8(result.standardError)));
	QCOMPARE(result.response.status, ai::JobStatus::Succeeded);
	QCOMPARE(result.response.candidates.size(), 3);
	QCOMPARE(result.response.provenance.value(QStringLiteral("backend")).toString(), QStringLiteral("worker-stub"));
	QCOMPARE(progressEvents.size(), 3);

	for(int i = 0; i < result.response.candidates.size(); ++i) {
		const ai::JobCandidate &candidate = result.response.candidates.at(i);
		QVERIFY2(QFileInfo::exists(candidate.imagePath), qPrintable(candidate.imagePath));
		QCOMPARE(candidate.maskPath, request.inputs.at(1).path);
		QCOMPARE(candidate.metadata.value(QStringLiteral("seed")).toInt(), 123 + i);
		QImage image(candidate.imagePath);
		QVERIFY(!image.isNull());
		QCOMPARE(image.size(), QSize(32, 24));
		QCOMPARE(progressEvents.at(i).value(QStringLiteral("type")).toString(), QStringLiteral("candidate"));
		QCOMPARE(progressEvents.at(i).value(QStringLiteral("candidate")).toInt(), i + 1);
	}
}

void AiJobRunnerTest::workerStubClampsCandidateCount()
{
	QTemporaryDir dir;
	ai::JobRequest request = makeRequest(dir);
	request.parameters = QJsonObject{
		{QStringLiteral("candidateCount"), 99},
		{QStringLiteral("seed"), 5},
	};

	const QString stubPath = workerPath();
	QVERIFY2(QFileInfo::exists(stubPath), qPrintable(QStringLiteral("Missing worker stub at %1").arg(stubPath)));
	const ai::JobRunResult result = ai::JobRunner::run(request, stubPath, 10000);
	QVERIFY2(result.ok, qPrintable(result.errorMessage));
	QCOMPARE(result.response.candidates.size(), 4);
	QCOMPARE(result.response.candidates.last().metadata.value(QStringLiteral("seed")).toInt(), 8);
}

void AiJobRunnerTest::workerStubProducesObjectDecompositionMetadata()
{
	QTemporaryDir dir;
	ai::JobRequest request = makeRequest(dir, ai::Operation::ObjectDecomposition);
	request.parameters = QJsonObject{
		{QStringLiteral("maxMasks"), 5},
		{QStringLiteral("decompositionDepth"), QStringLiteral("detailed")},
		{QStringLiteral("groupRepeatedRegions"), true},
	};

	QVector<QJsonObject> progressEvents;
	const QString stubPath = workerPath();
	QVERIFY2(QFileInfo::exists(stubPath), qPrintable(QStringLiteral("Missing worker stub at %1").arg(stubPath)));
	const ai::JobRunResult result = ai::JobRunner::run(
		request, stubPath, 10000,
		[&progressEvents](const QJsonObject &event) {
			progressEvents.append(event);
		});
	QVERIFY2(result.ok, qPrintable(result.errorMessage));
	QCOMPARE(result.response.candidates.size(), 5);
	QCOMPARE(progressEvents.size(), 5);
	QCOMPARE(result.response.diagnostics.value(QStringLiteral("decompositionDepth")).toString(), QStringLiteral("detailed"));

	const ai::JobCandidate &candidate = result.response.candidates.first();
	QVERIFY(QFileInfo::exists(candidate.imagePath));
	QVERIFY(QFileInfo::exists(candidate.maskPath));
	QCOMPARE(candidate.metadata.value(QStringLiteral("operation")).toString(), QStringLiteral("object-decomposition"));
	QCOMPARE(candidate.metadata.value(QStringLiteral("modelRole")).toString(), QStringLiteral("object-decomposition"));
	QCOMPARE(candidate.metadata.value(QStringLiteral("regionSetLabel")).toString(), QStringLiteral("Region Set - Detailed"));
	QCOMPARE(candidate.metadata.value(QStringLiteral("maskRole")).toString(), QStringLiteral("extracted-region"));
}

void AiJobRunnerTest::forcedWorkerFailureIsReported()
{
	QTemporaryDir dir;
	ai::JobRequest request = makeRequest(dir);
	request.parameters = QJsonObject{
		{QStringLiteral("forceFailure"), true},
	};

	const QString stubPath = workerPath();
	QVERIFY2(QFileInfo::exists(stubPath), qPrintable(QStringLiteral("Missing worker stub at %1").arg(stubPath)));
	const ai::JobRunResult result = ai::JobRunner::run(request, stubPath, 10000);
	QVERIFY(!result.ok);
	QCOMPARE(result.exitCode, 1);
	QCOMPARE(result.response.id, request.id);
	QCOMPARE(result.response.status, ai::JobStatus::Failed);
	QVERIFY(result.errorMessage.contains(QStringLiteral("Forced failure")));
	QCOMPARE(result.response.provenance.value(QStringLiteral("backend")).toString(), QStringLiteral("worker-stub"));
}

QTEST_GUILESS_MAIN(AiJobRunnerTest)
#include "aijobrunner.moc"
