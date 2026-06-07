// SPDX-License-Identifier: GPL-3.0-or-later
#include "desktop/ai/aijob.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QtTest/QtTest>

class AiJobTest final : public QObject {
	Q_OBJECT
private slots:
	void operationKeysRoundTrip();
	void operationAliasesAndUnknowns();
	void statusKeysRoundTrip();
	void requestRoundTripPreservesContractFields();
	void responseRoundTripPreservesCandidatesAndDiagnostics();
};

void AiJobTest::operationKeysRoundTrip()
{
	const QVector<ai::Operation> operations = {
		ai::Operation::SceneSeparation,
		ai::Operation::ObjectDecomposition,
		ai::Operation::Inpaint,
		ai::Operation::Outpaint,
		ai::Operation::BackgroundRemoval,
		ai::Operation::Upscale,
		ai::Operation::DepthGuide,
		ai::Operation::NormalGuide,
		ai::Operation::PoseGuide,
		ai::Operation::StyleTransfer,
		ai::Operation::FaceRestore,
	};

	for(ai::Operation operation : operations) {
		bool ok = false;
		const QString key = ai::operationKey(operation);
		QVERIFY2(!key.isEmpty(), qPrintable(QStringLiteral("empty key")));
		QCOMPARE(ai::operationFromKey(key, &ok), operation);
		QVERIFY2(ok, qPrintable(key));
		QVERIFY2(!ai::operationDisplayName(operation).isEmpty(), qPrintable(key));
	}
}

void AiJobTest::operationAliasesAndUnknowns()
{
	bool ok = false;
	QCOMPARE(
		ai::operationFromKey(QStringLiteral("generative-fill"), &ok),
		ai::Operation::Inpaint);
	QVERIFY(ok);

	QCOMPARE(
		ai::operationFromKey(QStringLiteral("not-a-real-operation"), &ok),
		ai::Operation::Inpaint);
	QVERIFY(!ok);
}

void AiJobTest::statusKeysRoundTrip()
{
	const QVector<ai::JobStatus> statuses = {
		ai::JobStatus::Queued,
		ai::JobStatus::Running,
		ai::JobStatus::Succeeded,
		ai::JobStatus::Failed,
		ai::JobStatus::Canceled,
	};

	for(ai::JobStatus status : statuses) {
		bool ok = false;
		const QString key = ai::jobStatusKey(status);
		QVERIFY2(!key.isEmpty(), qPrintable(QStringLiteral("empty key")));
		QCOMPARE(ai::jobStatusFromKey(key, &ok), status);
		QVERIFY2(ok, qPrintable(key));
	}

	bool ok = true;
	QCOMPARE(
		ai::jobStatusFromKey(QStringLiteral("mystery-status"), &ok),
		ai::JobStatus::Failed);
	QVERIFY(!ok);
}

void AiJobTest::requestRoundTripPreservesContractFields()
{
	ai::JobRequest request = ai::JobRequest::create(ai::Operation::Outpaint);
	QVERIFY(!request.id.isEmpty());
	request.region = QJsonObject{
		{QStringLiteral("x"), 12},
		{QStringLiteral("y"), 34},
		{QStringLiteral("width"), 256},
		{QStringLiteral("height"), 128},
	};
	request.parameters = QJsonObject{
		{QStringLiteral("prompt"), QStringLiteral("extend the old wall")},
		{QStringLiteral("candidateCount"), 3},
		{QStringLiteral("seed"), 1234},
	};
	request.preferences = QJsonObject{
		{QStringLiteral("safe4070Mode"), true},
	};
	request.source = QJsonObject{
		{QStringLiteral("documentName"), QStringLiteral("test-photo")},
	};
	request.provenance = QJsonObject{
		{QStringLiteral("createdBy"), QStringLiteral("unit-test")},
	};
	request.inputs.append(ai::JobAsset{
		QStringLiteral("source-image"),
		QStringLiteral("/tmp/source.png"),
		QStringLiteral("image/png"),
		QJsonObject{{QStringLiteral("role"), QStringLiteral("visible-canvas")}},
	});
	request.inputs.append(ai::JobAsset{
		QStringLiteral("mask"),
		QStringLiteral("/tmp/mask.png"),
		QStringLiteral("image/png"),
		QJsonObject{{QStringLiteral("channel"), QStringLiteral("alpha")}},
	});

	const QJsonObject json = request.toJsonObject();
	QCOMPARE(json.value(QStringLiteral("schema")).toString(), ai::schemaVersion());
	QCOMPARE(json.value(QStringLiteral("operation")).toString(), QStringLiteral("outpaint"));
	QCOMPARE(json.value(QStringLiteral("inputs")).toArray().size(), 2);

	const ai::JobRequest roundTrip =
		ai::JobRequest::fromJsonDocument(request.toJsonDocument());
	QCOMPARE(roundTrip.id, request.id);
	QCOMPARE(roundTrip.operation, ai::Operation::Outpaint);
	QCOMPARE(roundTrip.inputs.size(), 2);
	QCOMPARE(roundTrip.inputs.at(0).role, QStringLiteral("source-image"));
	QCOMPARE(roundTrip.inputs.at(1).metadata.value(QStringLiteral("channel")).toString(), QStringLiteral("alpha"));
	QCOMPARE(roundTrip.region.value(QStringLiteral("width")).toInt(), 256);
	QCOMPARE(roundTrip.parameters.value(QStringLiteral("candidateCount")).toInt(), 3);
	QCOMPARE(roundTrip.preferences.value(QStringLiteral("safe4070Mode")).toBool(), true);
	QCOMPARE(roundTrip.source.value(QStringLiteral("documentName")).toString(), QStringLiteral("test-photo"));
	QCOMPARE(roundTrip.provenance.value(QStringLiteral("createdBy")).toString(), QStringLiteral("unit-test"));
}

void AiJobTest::responseRoundTripPreservesCandidatesAndDiagnostics()
{
	ai::JobResponse response;
	response.id = QStringLiteral("job-1");
	response.status = ai::JobStatus::Succeeded;
	response.message = QStringLiteral("Generated 2 candidates.");
	response.diagnostics = QJsonObject{
		{QStringLiteral("backend"), QStringLiteral("diffusers")},
		{QStringLiteral("elapsedMsec"), 42},
	};
	response.provenance = QJsonObject{
		{QStringLiteral("schema"), ai::schemaVersion()},
	};
	response.candidates.append(ai::JobCandidate{
		QStringLiteral("candidate-1"),
		QStringLiteral("Candidate 1"),
		QStringLiteral("/tmp/candidate-1.png"),
		QStringLiteral("/tmp/mask.png"),
		QJsonObject{{QStringLiteral("seed"), 100}},
	});
	response.candidates.append(ai::JobCandidate{
		QStringLiteral("candidate-2"),
		QStringLiteral("Candidate 2"),
		QStringLiteral("/tmp/candidate-2.png"),
		QString(),
		QJsonObject{{QStringLiteral("seed"), 101}},
	});

	const ai::JobResponse roundTrip =
		ai::JobResponse::fromJsonDocument(response.toJsonDocument());
	QCOMPARE(roundTrip.id, QStringLiteral("job-1"));
	QCOMPARE(roundTrip.status, ai::JobStatus::Succeeded);
	QCOMPARE(roundTrip.message, response.message);
	QCOMPARE(roundTrip.candidates.size(), 2);
	QCOMPARE(roundTrip.candidates.at(0).id, QStringLiteral("candidate-1"));
	QCOMPARE(roundTrip.candidates.at(1).metadata.value(QStringLiteral("seed")).toInt(), 101);
	QCOMPARE(roundTrip.diagnostics.value(QStringLiteral("elapsedMsec")).toInt(), 42);
	QCOMPARE(roundTrip.provenance.value(QStringLiteral("schema")).toString(), ai::schemaVersion());
}

QTEST_APPLESS_MAIN(AiJobTest)
#include "aijob.moc"
