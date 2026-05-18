// SPDX-License-Identifier: GPL-3.0-or-later
#include "desktop/dialogs/aimodelmanagerdialog.h"
#include "desktop/utils/widgetutils.h"
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLabel>
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

	m_table = new QTableWidget(0, 5);
	m_table->setAlternatingRowColors(true);
	m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_table->verticalHeader()->hide();
	m_table->setHorizontalHeaderLabels(
		{tr("Role"), tr("Purpose"), tr("Target"), tr("Status"), tr("Notes")});
	m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
	m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
	m_table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
	utils::bindKineticScrolling(m_table);
	layout->addWidget(m_table, 1);

	addModelRole(
		tr("Segmentation"), tr("Mask-backed layer separation"),
		tr("SAM-family interactive/image segmentation"), tr("Not installed"),
		tr("Used for scene separation, object masks, and restoration regions."));
	addModelRole(
		tr("Background removal"), tr("Foreground/background matte extraction"),
		tr("Compact matting or RMBG-class model"), tr("Not installed"),
		tr("Separate subject masks before repair or replacement workflows."));
	addModelRole(
		tr("Generative fill"), tr("Selection fill and repair candidates"),
		tr("XL-class inpainting model"), tr("Not installed"),
		tr("Primary local diffusion role; optimized by crop, padding, tiling, "
		   "and sequential candidates."));
	addModelRole(
		tr("Outpaint"), tr("Intentional canvas extension"),
		tr("XL-class inpainting model"), tr("Not installed"),
		tr("Shares the region-operation pipeline with generative fill."));
	addModelRole(
		tr("Detail upscaler"), tr("Enhancement after restoration"),
		tr("Compact restoration/upscale model"), tr("Not installed"),
		tr("Used after repair passes, not as a replacement for manual edits."));
	addModelRole(
		tr("Depth guide"), tr("Depth map extraction"),
		tr("Small depth estimation model"), tr("Not installed"),
		tr("Produces guide layers for control and relighting workflows."));
	addModelRole(
		tr("Normal guide"), tr("Surface normal extraction"),
		tr("Normal estimation model"), tr("Not installed"),
		tr("Optional guide layer for controlled restoration or stylization."));
	addModelRole(
		tr("Pose guide"), tr("Human pose estimation"),
		tr("Pose/keypoint model"), tr("Not installed"),
		tr("Optional guide layer for body-aware repair operations."));
	addModelRole(
		tr("Style adapters"), tr("Controlled style transfer"),
		tr("LoRA/adapter collection"), tr("Not installed"),
		tr("Loaded only for the active operation to protect VRAM."));
	addModelRole(
		tr("Face restoration"), tr("Face-aware repair and enhancement"),
		tr("Compact face restoration model"), tr("Not installed"),
		tr("Scoped to restoration workflows with visible layer output."));

	auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close);
	layout->addWidget(buttons);
	connect(
		buttons, &QDialogButtonBox::accepted, this,
		&AiModelManagerDialog::accept);
	connect(
		buttons, &QDialogButtonBox::rejected, this,
		&AiModelManagerDialog::reject);
}

void AiModelManagerDialog::addModelRole(
	const QString &role, const QString &purpose, const QString &target,
	const QString &status, const QString &notes)
{
	const int row = m_table->rowCount();
	m_table->insertRow(row);
	m_table->setItem(row, 0, makeItem(role));
	m_table->setItem(row, 1, makeItem(purpose));
	m_table->setItem(row, 2, makeItem(target));
	m_table->setItem(row, 3, makeItem(status));
	m_table->setItem(row, 4, makeItem(notes));
}

}
