// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef DESKTOP_DIALOGS_AIMODELMANAGERDIALOG_H
#define DESKTOP_DIALOGS_AIMODELMANAGERDIALOG_H
#include <QDialog>

class QTableWidget;

namespace dialogs {

class AiModelManagerDialog final : public QDialog {
	Q_OBJECT
public:
	explicit AiModelManagerDialog(QWidget *parent = nullptr);

private:
	void addModelRole(
		const QString &role, const QString &purpose,
		const QString &target, const QString &status, const QString &notes);

	QTableWidget *m_table;
};

}

#endif
