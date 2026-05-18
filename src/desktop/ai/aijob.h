// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef DESKTOP_AI_AIJOB_H
#define DESKTOP_AI_AIJOB_H
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QVector>

namespace ai {

enum class Operation {
	SceneSeparation,
	GenerativeFill,
	Outpaint,
	BackgroundRemoval,
	Upscale,
	DepthGuide,
	NormalGuide,
	PoseGuide,
	StyleTransfer,
	FaceRestore,
};

enum class JobStatus {
	Queued,
	Running,
	Succeeded,
	Failed,
	Canceled,
};

QString schemaVersion();
QString operationKey(Operation operation);
QString operationDisplayName(Operation operation);
Operation operationFromKey(const QString &key, bool *ok = nullptr);
QString jobStatusKey(JobStatus status);
JobStatus jobStatusFromKey(const QString &key, bool *ok = nullptr);

struct JobAsset {
	QString role;
	QString path;
	QString mimeType;
	QJsonObject metadata;

	QJsonObject toJsonObject() const;
	static JobAsset fromJsonObject(const QJsonObject &json);
};

struct JobCandidate {
	QString id;
	QString label;
	QString imagePath;
	QString maskPath;
	QJsonObject metadata;

	QJsonObject toJsonObject() const;
	static JobCandidate fromJsonObject(const QJsonObject &json);
};

struct JobRequest {
	QString id;
	Operation operation = Operation::GenerativeFill;
	QVector<JobAsset> inputs;
	QJsonObject region;
	QJsonObject parameters;
	QJsonObject preferences;
	QJsonObject source;
	QJsonObject provenance;

	static JobRequest create(Operation operation);
	QJsonObject toJsonObject() const;
	QJsonDocument toJsonDocument() const;
};

struct JobResponse {
	QString id;
	JobStatus status = JobStatus::Queued;
	QString message;
	QVector<JobCandidate> candidates;
	QJsonObject diagnostics;
	QJsonObject provenance;

	QJsonObject toJsonObject() const;
	QJsonDocument toJsonDocument() const;
	static JobResponse fromJsonObject(const QJsonObject &json);
	static JobResponse fromJsonDocument(const QJsonDocument &document);
};

}

#endif
