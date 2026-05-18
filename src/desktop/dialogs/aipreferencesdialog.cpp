// SPDX-License-Identifier: GPL-3.0-or-later
#include "desktop/dialogs/aipreferencesdialog.h"
#include "desktop/utils/widgetutils.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QVBoxLayout>

namespace dialogs {

AiPreferencesDialog::AiPreferencesDialog(QWidget *parent)
	: QDialog(parent)
{
	setWindowTitle(tr("AI Preferences"));
	setWindowModality(Qt::NonModal);
	resize(620, 560);

	auto *layout = new QVBoxLayout(this);

	auto *summary = new QLabel(
		tr("Default controls for intentional AI restoration operations. These "
		   "values define the UI contract; runtime persistence and worker "
		   "integration will land with the AI job boundary."));
	summary->setWordWrap(true);
	layout->addWidget(summary);

	auto *scroll = new QScrollArea;
	scroll->setWidgetResizable(true);
	scroll->setFrameStyle(QFrame::NoFrame);
	utils::bindKineticScrolling(scroll);
	layout->addWidget(scroll, 1);

	auto *contents = new QWidget;
	auto *contentsLayout = new QVBoxLayout(contents);
	contentsLayout->setAlignment(Qt::AlignTop);
	scroll->setWidget(contents);

	auto *generationGroup = new QGroupBox(tr("Generation Defaults"));
	auto *generationForm = new QFormLayout(generationGroup);

	m_candidateCount = new QSpinBox;
	m_candidateCount->setRange(1, 4);
	generationForm->addRow(tr("Candidate count:"), m_candidateCount);

	m_maxRenderEdge = new QSpinBox;
	m_maxRenderEdge->setRange(512, 2048);
	m_maxRenderEdge->setSingleStep(64);
	m_maxRenderEdge->setSuffix(tr(" px"));
	generationForm->addRow(tr("Max render edge:"), m_maxRenderEdge);

	m_contextPadding = new QSpinBox;
	m_contextPadding->setRange(0, 512);
	m_contextPadding->setSingleStep(16);
	m_contextPadding->setSuffix(tr(" px"));
	generationForm->addRow(tr("Context padding:"), m_contextPadding);

	m_cfgScale = new QDoubleSpinBox;
	m_cfgScale->setRange(1.0, 15.0);
	m_cfgScale->setSingleStep(0.5);
	m_cfgScale->setDecimals(1);
	generationForm->addRow(tr("CFG scale:"), m_cfgScale);

	m_denoiseStrength = new QDoubleSpinBox;
	m_denoiseStrength->setRange(0.0, 1.0);
	m_denoiseStrength->setSingleStep(0.05);
	m_denoiseStrength->setDecimals(2);
	generationForm->addRow(tr("Denoise strength:"), m_denoiseStrength);

	m_seed = new QSpinBox;
	m_seed->setRange(-1, 2147483647);
	m_seed->setSpecialValueText(tr("Random"));
	generationForm->addRow(tr("Seed:"), m_seed);

	contentsLayout->addWidget(generationGroup);

	auto *performanceGroup = new QGroupBox(tr("4070-Class Performance"));
	auto *performanceForm = new QFormLayout(performanceGroup);

	m_variantMode = new QComboBox;
	m_variantMode->addItems(
		{tr("Sequential candidates"), tr("Batch when memory allows")});
	performanceForm->addRow(tr("Variant rendering:"), m_variantMode);

	m_unloadPolicy = new QComboBox;
	m_unloadPolicy->addItems(
		{tr("Unload after each job"), tr("Unload when idle"),
		 tr("Keep active model warm")});
	performanceForm->addRow(tr("Model unload policy:"), m_unloadPolicy);

	m_safeMode = new QCheckBox(tr("Prefer 4070-safe limits"));
	performanceForm->addRow(QString(), m_safeMode);

	m_vaeTiling = new QCheckBox(tr("Use VAE tiling/slicing where available"));
	performanceForm->addRow(QString(), m_vaeTiling);

	m_cacheGuides = new QCheckBox(tr("Cache segmentation and guide maps"));
	performanceForm->addRow(QString(), m_cacheGuides);

	contentsLayout->addWidget(performanceGroup);
	contentsLayout->addStretch(1);

	auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close);
	QPushButton *defaultsButton =
		buttons->addButton(tr("Restore Defaults"), QDialogButtonBox::ResetRole);
	layout->addWidget(buttons);
	connect(
		defaultsButton, &QPushButton::clicked, this,
		&AiPreferencesDialog::restoreDefaults);
	connect(
		buttons, &QDialogButtonBox::accepted, this,
		&AiPreferencesDialog::accept);
	connect(
		buttons, &QDialogButtonBox::rejected, this,
		&AiPreferencesDialog::reject);

	restoreDefaults();
}

void AiPreferencesDialog::restoreDefaults()
{
	m_candidateCount->setValue(3);
	m_maxRenderEdge->setValue(1024);
	m_contextPadding->setValue(128);
	m_cfgScale->setValue(5.0);
	m_denoiseStrength->setValue(0.75);
	m_seed->setValue(-1);
	m_variantMode->setCurrentIndex(0);
	m_unloadPolicy->setCurrentIndex(1);
	m_safeMode->setChecked(true);
	m_vaeTiling->setChecked(true);
	m_cacheGuides->setChecked(true);
}

}
