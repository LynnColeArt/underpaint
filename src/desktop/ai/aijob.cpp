// SPDX-License-Identifier: GPL-3.0-or-later
#include "desktop/ai/aijob.h"
#include <QJsonArray>
#include <QUuid>

namespace ai {

namespace {

constexpr char SCHEMA_VERSION[] = "underpaint.ai-job.v1";

QJsonArray assetsToJson(const QVector<JobAsset> &assets)
{
	QJsonArray array;
	for(const JobAsset &asset : assets) {
		array.append(asset.toJsonObject());
	}
	return array;
}

QVector<JobAsset> assetsFromJson(const QJsonArray &array)
{
	QVector<JobAsset> assets;
	assets.reserve(array.size());
	for(const QJsonValue &value : array) {
		if(value.isObject()) {
			assets.append(JobAsset::fromJsonObject(value.toObject()));
		}
	}
	return assets;
}

QJsonArray candidatesToJson(const QVector<JobCandidate> &candidates)
{
	QJsonArray array;
	for(const JobCandidate &candidate : candidates) {
		array.append(candidate.toJsonObject());
	}
	return array;
}

QVector<JobCandidate> candidatesFromJson(const QJsonArray &array)
{
	QVector<JobCandidate> candidates;
	candidates.reserve(array.size());
	for(const QJsonValue &value : array) {
		if(value.isObject()) {
			candidates.append(JobCandidate::fromJsonObject(value.toObject()));
		}
	}
	return candidates;
}

QString makeJobId()
{
	return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

}

QString schemaVersion()
{
	return QString::fromLatin1(SCHEMA_VERSION);
}

QString operationKey(Operation operation)
{
	switch(operation) {
	case Operation::SceneSeparation:
		return QStringLiteral("scene-separation");
	case Operation::ObjectDecomposition:
		return QStringLiteral("object-decomposition");
	case Operation::Inpaint:
		return QStringLiteral("inpaint");
	case Operation::Outpaint:
		return QStringLiteral("outpaint");
	case Operation::BackgroundRemoval:
		return QStringLiteral("background-removal");
	case Operation::Upscale:
		return QStringLiteral("upscale");
	case Operation::DepthGuide:
		return QStringLiteral("depth-guide");
	case Operation::NormalGuide:
		return QStringLiteral("normal-guide");
	case Operation::PoseGuide:
		return QStringLiteral("pose-guide");
	case Operation::StyleTransfer:
		return QStringLiteral("style-transfer");
	case Operation::FaceRestore:
		return QStringLiteral("face-restore");
	}
	return QStringLiteral("inpaint");
}

QString operationDisplayName(Operation operation)
{
	switch(operation) {
	case Operation::SceneSeparation:
		return QStringLiteral("Scene Separation");
	case Operation::ObjectDecomposition:
		return QStringLiteral("Object Decomposition");
	case Operation::Inpaint:
		return QStringLiteral("Inpaint");
	case Operation::Outpaint:
		return QStringLiteral("Outpaint");
	case Operation::BackgroundRemoval:
		return QStringLiteral("Background Removal");
	case Operation::Upscale:
		return QStringLiteral("Upscale");
	case Operation::DepthGuide:
		return QStringLiteral("Depth Guide");
	case Operation::NormalGuide:
		return QStringLiteral("Normal Guide");
	case Operation::PoseGuide:
		return QStringLiteral("Pose Guide");
	case Operation::StyleTransfer:
		return QStringLiteral("Style Transfer");
	case Operation::FaceRestore:
		return QStringLiteral("Face Restore");
	}
	return QStringLiteral("Inpaint");
}

Operation operationFromKey(const QString &key, bool *ok)
{
	const struct {
		const char *key;
		Operation operation;
	} operations[] = {
		{"scene-separation", Operation::SceneSeparation},
		{"object-decomposition", Operation::ObjectDecomposition},
		{"inpaint", Operation::Inpaint},
		{"generative-fill", Operation::Inpaint},
		{"outpaint", Operation::Outpaint},
		{"background-removal", Operation::BackgroundRemoval},
		{"upscale", Operation::Upscale},
		{"depth-guide", Operation::DepthGuide},
		{"normal-guide", Operation::NormalGuide},
		{"pose-guide", Operation::PoseGuide},
		{"style-transfer", Operation::StyleTransfer},
		{"face-restore", Operation::FaceRestore},
	};
	for(const auto &candidate : operations) {
		if(key == QString::fromLatin1(candidate.key)) {
			if(ok) {
				*ok = true;
			}
			return candidate.operation;
		}
	}
	if(ok) {
		*ok = false;
	}
	return Operation::Inpaint;
}

QString jobStatusKey(JobStatus status)
{
	switch(status) {
	case JobStatus::Queued:
		return QStringLiteral("queued");
	case JobStatus::Running:
		return QStringLiteral("running");
	case JobStatus::Succeeded:
		return QStringLiteral("succeeded");
	case JobStatus::Failed:
		return QStringLiteral("failed");
	case JobStatus::Canceled:
		return QStringLiteral("canceled");
	}
	return QStringLiteral("queued");
}

JobStatus jobStatusFromKey(const QString &key, bool *ok)
{
	const struct {
		const char *key;
		JobStatus status;
	} statuses[] = {
		{"queued", JobStatus::Queued},
		{"running", JobStatus::Running},
		{"succeeded", JobStatus::Succeeded},
		{"failed", JobStatus::Failed},
		{"canceled", JobStatus::Canceled},
	};
	for(const auto &candidate : statuses) {
		if(key == QString::fromLatin1(candidate.key)) {
			if(ok) {
				*ok = true;
			}
			return candidate.status;
		}
	}
	if(ok) {
		*ok = false;
	}
	return JobStatus::Failed;
}

QJsonObject JobAsset::toJsonObject() const
{
	return QJsonObject{
		{QStringLiteral("role"), role},
		{QStringLiteral("path"), path},
		{QStringLiteral("mimeType"), mimeType},
		{QStringLiteral("metadata"), metadata},
	};
}

JobAsset JobAsset::fromJsonObject(const QJsonObject &json)
{
	return JobAsset{
		json.value(QStringLiteral("role")).toString(),
		json.value(QStringLiteral("path")).toString(),
		json.value(QStringLiteral("mimeType")).toString(),
		json.value(QStringLiteral("metadata")).toObject(),
	};
}

QJsonObject JobCandidate::toJsonObject() const
{
	return QJsonObject{
		{QStringLiteral("id"), id},
		{QStringLiteral("label"), label},
		{QStringLiteral("imagePath"), imagePath},
		{QStringLiteral("maskPath"), maskPath},
		{QStringLiteral("metadata"), metadata},
	};
}

JobCandidate JobCandidate::fromJsonObject(const QJsonObject &json)
{
	return JobCandidate{
		json.value(QStringLiteral("id")).toString(),
		json.value(QStringLiteral("label")).toString(),
		json.value(QStringLiteral("imagePath")).toString(),
		json.value(QStringLiteral("maskPath")).toString(),
		json.value(QStringLiteral("metadata")).toObject(),
	};
}

JobRequest JobRequest::create(Operation operation)
{
	JobRequest request;
	request.id = makeJobId();
	request.operation = operation;
	return request;
}

QJsonObject JobRequest::toJsonObject() const
{
	return QJsonObject{
		{QStringLiteral("schema"), schemaVersion()},
		{QStringLiteral("id"), id},
		{QStringLiteral("operation"), operationKey(operation)},
		{QStringLiteral("inputs"), assetsToJson(inputs)},
		{QStringLiteral("region"), region},
		{QStringLiteral("parameters"), parameters},
		{QStringLiteral("preferences"), preferences},
		{QStringLiteral("source"), source},
		{QStringLiteral("provenance"), provenance},
	};
}

QJsonDocument JobRequest::toJsonDocument() const
{
	return QJsonDocument(toJsonObject());
}

JobRequest JobRequest::fromJsonObject(const QJsonObject &json)
{
	bool operationOk = false;
	JobRequest request;
	request.id = json.value(QStringLiteral("id")).toString();
	request.operation = operationFromKey(
		json.value(QStringLiteral("operation")).toString(), &operationOk);
	if(!operationOk) {
		request.operation = Operation::Inpaint;
	}
	request.inputs =
		assetsFromJson(json.value(QStringLiteral("inputs")).toArray());
	request.region = json.value(QStringLiteral("region")).toObject();
	request.parameters = json.value(QStringLiteral("parameters")).toObject();
	request.preferences = json.value(QStringLiteral("preferences")).toObject();
	request.source = json.value(QStringLiteral("source")).toObject();
	request.provenance = json.value(QStringLiteral("provenance")).toObject();
	return request;
}

JobRequest JobRequest::fromJsonDocument(const QJsonDocument &document)
{
	return fromJsonObject(document.object());
}

QJsonObject JobResponse::toJsonObject() const
{
	return QJsonObject{
		{QStringLiteral("schema"), schemaVersion()},
		{QStringLiteral("id"), id},
		{QStringLiteral("status"), jobStatusKey(status)},
		{QStringLiteral("message"), message},
		{QStringLiteral("candidates"), candidatesToJson(candidates)},
		{QStringLiteral("diagnostics"), diagnostics},
		{QStringLiteral("provenance"), provenance},
	};
}

QJsonDocument JobResponse::toJsonDocument() const
{
	return QJsonDocument(toJsonObject());
}

JobResponse JobResponse::fromJsonObject(const QJsonObject &json)
{
	bool statusOk = false;
	JobResponse response;
	response.id = json.value(QStringLiteral("id")).toString();
	response.status = jobStatusFromKey(
		json.value(QStringLiteral("status")).toString(), &statusOk);
	if(!statusOk) {
		response.status = JobStatus::Failed;
	}
	response.message = json.value(QStringLiteral("message")).toString();
	response.candidates =
		candidatesFromJson(json.value(QStringLiteral("candidates")).toArray());
	response.diagnostics =
		json.value(QStringLiteral("diagnostics")).toObject();
	response.provenance = json.value(QStringLiteral("provenance")).toObject();
	return response;
}

JobResponse JobResponse::fromJsonDocument(const QJsonDocument &document)
{
	return fromJsonObject(document.object());
}

}
