// SPDX-License-Identifier: GPL-3.0-or-later
#include "desktop/ai/aijob.h"
#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QImage>
#include <QJsonParseError>
#include <QPainter>
#include <QRandomGenerator>
#include <QSaveFile>
#include <QTextStream>
#include <climits>

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

void writeProgressEvent(const QJsonObject &event)
{
	QTextStream out(stdout);
	out << QString::fromUtf8(
			   QJsonDocument(event).toJson(QJsonDocument::Compact))
		<< '\n';
	out.flush();
}

int writeFailureResponse(
	const QString &responsePath, const QString &id, const QString &message);

QString inputPathForRole(const ai::JobRequest &request, const QString &role)
{
	for(const ai::JobAsset &asset : request.inputs) {
		if(asset.role == role) {
			return asset.path;
		}
	}
	return QString();
}

QSize requestedRegionSize(const ai::JobRequest &request)
{
	const int width = request.region.value(QStringLiteral("width")).toInt();
	const int height = request.region.value(QStringLiteral("height")).toInt();
	return width > 0 && height > 0 ? QSize(width, height) : QSize(512, 512);
}

QColor candidateColor(int index)
{
	static const QColor colors[] = {
		QColor(48, 112, 178, 92),
		QColor(137, 80, 168, 92),
		QColor(37, 139, 112, 92),
		QColor(191, 112, 48, 92),
	};
	return colors[index % int(sizeof(colors) / sizeof(colors[0]))];
}

QString regionLabel(int index, int count)
{
	static const QString fiveRegionLabels[] = {
		QStringLiteral("Shadows"),
		QStringLiteral("Darks"),
		QStringLiteral("Midtones"),
		QStringLiteral("Lights"),
		QStringLiteral("Highlights"),
	};
	if(count == 5) {
		return fiveRegionLabels[index];
	}
	return QStringLiteral("Region %1").arg(index + 1);
}

void applyAlphaMask(QImage &image, const QString &maskPath)
{
	QImage mask(maskPath);
	if(mask.isNull()) {
		return;
	}
	if(mask.size() != image.size()) {
		mask = mask.scaled(image.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	}
	QImage alpha(image.size(), QImage::Format_ARGB32_Premultiplied);
	alpha.fill(Qt::transparent);
	for(int y = 0; y < alpha.height(); ++y) {
		QRgb *out = reinterpret_cast<QRgb *>(alpha.scanLine(y));
		for(int x = 0; x < alpha.width(); ++x) {
			const int a = qGray(mask.pixel(x, y));
			out[x] = qRgba(255, 255, 255, a);
		}
	}
	QPainter painter(&image);
	painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
	painter.drawImage(0, 0, alpha);
}

bool writePlaceholderImage(
	const ai::JobRequest &request, int candidateIndex, const QString &path,
	QString &outError)
{
	QImage image(inputPathForRole(request, QStringLiteral("source-image")));
	if(image.isNull()) {
		image = QImage(
			requestedRegionSize(request), QImage::Format_ARGB32_Premultiplied);
		image.fill(QColor(31, 34, 38));
	} else {
		image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
	}

	QPainter painter(&image);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.fillRect(image.rect(), candidateColor(candidateIndex));
	painter.setPen(QPen(QColor(255, 255, 255, 54), 2));
	const int grid = qMax(24, qMin(image.width(), image.height()) / 16);
	for(int i = 0; i < image.width(); i += grid) {
		painter.drawLine(i, 0, i, image.height());
	}
	for(int i = 0; i < image.height(); i += grid) {
		painter.drawLine(0, i, image.width(), i);
	}
	const QRect badge(
		image.width() / 10, image.height() / 10, image.width() / 3,
		qMax(32, image.height() / 12));
	painter.setBrush(QColor(255, 255, 255, 178));
	painter.setPen(Qt::NoPen);
	painter.drawRoundedRect(badge, 8, 8);
	painter.setBrush(candidateColor(candidateIndex).darker(170));
	const int dotSize = qMax(8, badge.height() / 3);
	for(int i = 0; i <= candidateIndex; ++i) {
		painter.drawEllipse(
			badge.left() + dotSize + i * dotSize * 2,
			badge.center().y() - dotSize / 2, dotSize, dotSize);
	}
	painter.end();

	applyAlphaMask(image, inputPathForRole(request, QStringLiteral("mask")));

	if(!image.save(path, "PNG")) {
		outError = QStringLiteral("Could not save placeholder image.");
		return false;
	}
	return true;
}

bool writeRegionLayerImages(
	const ai::JobRequest &request, int regionIndex, int regionCount,
	const QString &imagePath, const QString &maskPath, QString &outError)
{
	QImage source(inputPathForRole(request, QStringLiteral("source-image")));
	if(source.isNull()) {
		source = QImage(
			requestedRegionSize(request), QImage::Format_ARGB32_Premultiplied);
		source.fill(QColor(31, 34, 38));
	} else {
		source = source.convertToFormat(QImage::Format_ARGB32_Premultiplied);
	}

	QImage layer(source.size(), QImage::Format_ARGB32_Premultiplied);
	layer.fill(Qt::transparent);
	QImage mask(source.size(), QImage::Format_Grayscale8);
	mask.fill(0);

	const int low = (regionIndex * 256) / regionCount;
	const int high =
		regionIndex == regionCount - 1 ? 256 : ((regionIndex + 1) * 256) / regionCount;
	for(int y = 0; y < source.height(); ++y) {
		const QRgb *in = reinterpret_cast<const QRgb *>(source.constScanLine(y));
		QRgb *out = reinterpret_cast<QRgb *>(layer.scanLine(y));
		uchar *maskOut = mask.scanLine(y);
		for(int x = 0; x < source.width(); ++x) {
			const int luma = qGray(in[x]);
			if(luma >= low && luma < high) {
				out[x] = in[x];
				maskOut[x] = uchar(qAlpha(in[x]));
			}
		}
	}

	if(!layer.save(imagePath, "PNG")) {
		outError = QStringLiteral("Could not save placeholder region layer.");
		return false;
	}
	if(!mask.save(maskPath, "PNG")) {
		outError = QStringLiteral("Could not save placeholder region mask.");
		return false;
	}
	return true;
}

int writeSceneSeparationResponse(
	const ai::JobRequest &request, const QString &responsePath,
	const QString &jobDirectory, qint64 elapsedMsec)
{
	QDir dir(jobDirectory);
	QString error;
	const int requestedRegionCount =
		request.parameters.value(QStringLiteral("maxRegions")).toInt(5);
	const int regionCount = qBound(2, requestedRegionCount, 8);
	const int minRegionAreaPct =
		qBound(1, request.parameters.value(QStringLiteral("minRegionAreaPct")).toInt(3), 20);

	ai::JobResponse response;
	response.id = request.id;
	response.status = ai::JobStatus::Succeeded;
	response.message =
		QStringLiteral("Generated %1 placeholder decomposition layer(s).")
			.arg(regionCount);

	for(int i = 0; i < regionCount; ++i) {
		const QString candidateId = QStringLiteral("region-%1").arg(i + 1);
		const QString label = regionLabel(i, regionCount);
		const QString imagePath =
			dir.filePath(QStringLiteral("%1.png").arg(candidateId));
		const QString maskPath =
			dir.filePath(QStringLiteral("%1-mask.png").arg(candidateId));
		if(!writeRegionLayerImages(
			   request, i, regionCount, imagePath, maskPath, error)) {
			return writeFailureResponse(responsePath, request.id, error);
		}

		ai::JobCandidate candidate;
		candidate.id = candidateId;
		candidate.label = label;
		candidate.imagePath = imagePath;
		candidate.maskPath = maskPath;
		candidate.metadata = QJsonObject{
			{QStringLiteral("operation"), ai::operationKey(request.operation)},
			{QStringLiteral("placeholder"), true},
			{QStringLiteral("modelRole"), QStringLiteral("photo-decomposition")},
			{QStringLiteral("regionIndex"), i},
			{QStringLiteral("regionCount"), regionCount},
			{QStringLiteral("minRegionAreaPct"), minRegionAreaPct},
		};
		writeProgressEvent(QJsonObject{
			{QStringLiteral("schema"), ai::schemaVersion()},
			{QStringLiteral("type"), QStringLiteral("candidate")},
			{QStringLiteral("id"), candidateId},
			{QStringLiteral("candidate"), i + 1},
			{QStringLiteral("label"), label},
			{QStringLiteral("imagePath"), imagePath},
		});
		response.candidates.append(candidate);
	}

	response.diagnostics = QJsonObject{
		{QStringLiteral("elapsedMsec"), int(elapsedMsec)},
		{QStringLiteral("regionCount"), regionCount},
	};
	response.provenance = QJsonObject{
		{QStringLiteral("backend"), QStringLiteral("worker-stub")},
		{QStringLiteral("schema"), ai::schemaVersion()},
		{QStringLiteral("model"), QStringLiteral("placeholder-luma-regions")},
	};

	if(!writeJsonFile(responsePath, response.toJsonDocument(), error)) {
		QTextStream(stderr) << "Could not write response: " << error << "\n";
		return 2;
	}
	return 0;
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
	if(request.operation == ai::Operation::SceneSeparation) {
		return writeSceneSeparationResponse(
			request, responsePath, jobDirectory, timer.elapsed());
	}

	const int requestedCandidateCount =
		request.parameters.value(QStringLiteral("candidateCount")).toInt(1);
	const int candidateCount = qBound(1, requestedCandidateCount, 4);

	ai::JobResponse response;
	response.id = request.id;
	response.status = ai::JobStatus::Succeeded;
	response.message =
		QStringLiteral("Generated %1 placeholder candidate(s).")
			.arg(candidateCount);
	const QString maskPath = inputPathForRole(request, QStringLiteral("mask"));
	int baseSeed = request.parameters.value(QStringLiteral("seed")).toInt(-1);
	const int maxBaseSeed = INT_MAX - candidateCount;
	if(baseSeed < 0) {
		baseSeed = QRandomGenerator::global()->bounded(maxBaseSeed + 1);
	} else {
		baseSeed = qMin(baseSeed, maxBaseSeed);
	}
	for(int i = 0; i < candidateCount; ++i) {
		const QString candidateId =
			QStringLiteral("candidate-%1").arg(i + 1);
		const QString imagePath =
			dir.filePath(QStringLiteral("%1.png").arg(candidateId));
		if(!writePlaceholderImage(request, i, imagePath, error)) {
			return writeFailureResponse(responsePath, request.id, error);
		}

		ai::JobCandidate candidate;
		candidate.id = candidateId;
		candidate.label =
			QStringLiteral("Worker Stub Candidate %1").arg(i + 1);
		candidate.imagePath = imagePath;
		candidate.maskPath = maskPath;
		candidate.metadata = QJsonObject{
			{QStringLiteral("operation"), ai::operationKey(request.operation)},
			{QStringLiteral("placeholder"), true},
			{QStringLiteral("seed"), baseSeed + i},
			{QStringLiteral("variantIndex"), i},
		};
		writeProgressEvent(QJsonObject{
			{QStringLiteral("schema"), ai::schemaVersion()},
			{QStringLiteral("type"), QStringLiteral("candidate")},
			{QStringLiteral("id"), candidateId},
			{QStringLiteral("candidate"), i + 1},
			{QStringLiteral("seed"), baseSeed + i},
			{QStringLiteral("imagePath"), imagePath},
		});
		response.candidates.append(candidate);
	}
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
