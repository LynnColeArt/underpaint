// SPDX-License-Identifier: GPL-3.0-or-later
#include "desktop/dialogs/aimodelmanagerdialog.h"
#include "desktop/utils/widgetutils.h"
#include <QApplication>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QProcessEnvironment>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

namespace dialogs {

namespace {

QTableWidgetItem *makeItem(const QString &text)
{
	auto *item = new QTableWidgetItem(text);
	item->setFlags(item->flags() & ~Qt::ItemIsEditable);
	return item;
}

QString registryPath()
{
	const QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
	const QString configured =
		environment.value(QStringLiteral("UNDERPAINT_MODEL_REGISTRY"));
	if(!configured.isEmpty()) {
		return configured;
	}
	const QString relativePath =
		QStringLiteral("tools/ai/model-registry.json");
	const QString root =
		environment.value(QStringLiteral("UNDERPAINT_TOOL_ROOT"));
	if(!root.isEmpty()) {
		const QString path = QDir(root).filePath(relativePath);
		if(QFile::exists(path)) {
			return path;
		}
	}
	const QString currentPath = QDir::current().filePath(relativePath);
	if(QFile::exists(currentPath)) {
		return currentPath;
	}
	const QString appPath =
		QDir(QApplication::applicationDirPath()).filePath(
			QStringLiteral("../../%1").arg(relativePath));
	return QFile::exists(appPath) ? appPath : currentPath;
}

}

AiModelManagerDialog::AiModelManagerDialog(QWidget *parent)
	: QDialog(parent)
{
	setWindowTitle(tr("Model Manager"));
	setWindowModality(Qt::NonModal);
	resize(900, 520);

	auto *layout = new QVBoxLayout(this);

	auto *summary = new QLabel(
		tr("Local model inventory for Underpaint restoration roles. Download, "
		   "validation, and runtime loading will be connected through the AI "
		   "job boundary."));
	summary->setWordWrap(true);
	layout->addWidget(summary);

	m_table = new QTableWidget(0, 6);
	m_table->setAlternatingRowColors(true);
	m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_table->verticalHeader()->hide();
	m_table->setHorizontalHeaderLabels(
		{tr("Role"), tr("Purpose"), tr("Target"), tr("Backend"), tr("Status"),
		 tr("Notes")});
	m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
	m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
	m_table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
	m_table->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
	utils::bindKineticScrolling(m_table);
	layout->addWidget(m_table, 1);

	addModelRole(
		tr("Segmentation"), tr("Mask-backed layer separation"),
		tr("SAM-family interactive/image segmentation"), tr("ONNX / Python"),
		tr("Not installed"),
		tr("Used for scene separation, object masks, and restoration regions."));
	addModelRole(
		tr("Background removal"), tr("Foreground/background matte extraction"),
		tr("Compact matting or RMBG-class model"), tr("ONNX / Python"),
		tr("Not installed"),
		tr("Separate subject masks before repair or replacement workflows."));
	addModelRole(
		tr("Inpaint"), tr("Selection repair and fill candidates"),
		tr("XL-class inpainting model"), tr("Diffusers / GGUF"),
		tr("Not installed"),
		tr("Primary local diffusion role; optimized by crop, padding, tiling, "
		   "and sequential candidates."));
	addModelRole(
		tr("Outpaint"), tr("Intentional canvas extension"),
		tr("XL-class inpainting model"), tr("Diffusers / GGUF"),
		tr("Not installed"),
		tr("Shares the region-operation pipeline with inpaint."));
	addModelRole(
		tr("Refiner"), tr("Late-stage candidate polish"),
		tr("SDXL refiner or compatible img2img model"), tr("Diffusers / GGUF"),
		tr("Experimental"),
		tr("Runs after base generation and before targeted detail passes."));
	addModelRole(
		tr("Detail upscaler"), tr("Enhancement after restoration"),
		tr("Compact restoration/upscale model"), tr("ONNX / Python"),
		tr("Not installed"),
		tr("Used after repair passes, not as a replacement for manual edits."));
	addModelRole(
		tr("Depth guide"), tr("Depth map extraction"),
		tr("Small depth estimation model"), tr("ONNX / Diffusers"),
		tr("Not installed"),
		tr("Produces guide layers for control and relighting workflows."));
	addModelRole(
		tr("Normal guide"), tr("Surface normal extraction"),
		tr("Normal estimation model"), tr("ONNX / Diffusers"),
		tr("Not installed"),
		tr("Optional guide layer for controlled restoration or stylization."));
	addModelRole(
		tr("Pose guide"), tr("Human pose estimation"),
		tr("Pose/keypoint model"), tr("ONNX"),
		tr("Not installed"),
		tr("Optional guide layer for body-aware repair operations."));
	addModelRole(
		tr("Style adapters"), tr("Controlled style transfer"),
		tr("LoRA/adapter collection"), tr("Diffusers"),
		tr("Not installed"),
		tr("Loaded only for the active operation to protect VRAM."));
	addModelRole(
		tr("Face restoration"), tr("Face-aware repair and enhancement"),
		tr("Compact face restoration model"), tr("YOLO + Diffusers"),
		tr("Not installed"),
		tr("Scoped to restoration workflows with visible layer output."));

	addRegistryModels();

	auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close);
	layout->addWidget(buttons);
	connect(
		buttons, &QDialogButtonBox::accepted, this,
		&AiModelManagerDialog::accept);
	connect(
		buttons, &QDialogButtonBox::rejected, this,
		&AiModelManagerDialog::reject);
}

void AiModelManagerDialog::addRegistryModels()
{
	QFile file(registryPath());
	if(!file.open(QIODevice::ReadOnly)) {
		return;
	}
	const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
	const QJsonArray models =
		document.object().value(QStringLiteral("models")).toArray();
	for(const QJsonValue &value : models) {
		const QJsonObject object = value.toObject();
		QStringList capabilities;
		const QJsonArray capabilityValues =
			object.value(QStringLiteral("capabilities")).toArray();
		for(const QJsonValue &capability : capabilityValues) {
			if(!capability.toString().isEmpty()) {
				capabilities.append(capability.toString());
			}
		}
		const QString displayName =
			object.value(QStringLiteral("displayName")).toString(
				object.value(QStringLiteral("id")).toString());
		const QString backend = object.value(QStringLiteral("backend")).toString();
		const QString format = object.value(QStringLiteral("format")).toString();
		const QString status =
			object.value(QStringLiteral("installedState")).toString(
				tr("unknown"));
		QString notes = object.value(QStringLiteral("notes")).toString();
		if(object.value(QStringLiteral("experimental")).toBool()) {
			notes = tr("Experimental. %1").arg(notes);
		}
		addModelRole(
			capabilities.join(QStringLiteral(", ")), displayName,
			object.value(QStringLiteral("model")).toString(),
			format.isEmpty() ? backend : tr("%1 / %2").arg(backend, format),
			status, notes);
	}
}

void AiModelManagerDialog::addModelRole(
	const QString &role, const QString &purpose, const QString &target,
	const QString &backend, const QString &status, const QString &notes)
{
	const int row = m_table->rowCount();
	m_table->insertRow(row);
	m_table->setItem(row, 0, makeItem(role));
	m_table->setItem(row, 1, makeItem(purpose));
	m_table->setItem(row, 2, makeItem(target));
	m_table->setItem(row, 3, makeItem(backend));
	m_table->setItem(row, 4, makeItem(status));
	m_table->setItem(row, 5, makeItem(notes));
}

}
