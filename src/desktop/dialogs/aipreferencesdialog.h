// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef DESKTOP_DIALOGS_AIPREFERENCESDIALOG_H
#define DESKTOP_DIALOGS_AIPREFERENCESDIALOG_H
#include <QDialog>

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QSpinBox;

namespace dialogs {

class AiPreferencesDialog final : public QDialog {
	Q_OBJECT
public:
	explicit AiPreferencesDialog(QWidget *parent = nullptr);

private:
	void restoreDefaults();

	QSpinBox *m_candidateCount;
	QSpinBox *m_maxRenderEdge;
	QSpinBox *m_contextPadding;
	QDoubleSpinBox *m_cfgScale;
	QDoubleSpinBox *m_denoiseStrength;
	QSpinBox *m_seed;
	QComboBox *m_variantMode;
	QComboBox *m_unloadPolicy;
	QCheckBox *m_safeMode;
	QCheckBox *m_vaeTiling;
	QCheckBox *m_cacheGuides;
};

}

#endif
