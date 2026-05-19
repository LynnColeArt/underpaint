// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef DESKTOP_AI_AIJOBRUNNER_H
#define DESKTOP_AI_AIJOBRUNNER_H
#include "desktop/ai/aijob.h"
#include <QByteArray>
#include <QJsonObject>
#include <QString>
#include <functional>

namespace ai {

struct JobRunResult {
	bool ok = false;
	QString errorMessage;
	QString jobDirectoryPath;
	QString requestPath;
	QString responsePath;
	QString resolvedWorkerPath;
	JobResponse response;
	int exitCode = -1;
	QByteArray standardOutput;
	QByteArray standardError;
};

class JobRunner {
public:
	using ProgressCallback = std::function<void(const QJsonObject &)>;

	static QString defaultWorkerPath();
	static JobRunResult run(
		const JobRequest &request, const QString &workerPath = QString(),
		int timeoutMsec = 60000,
		const ProgressCallback &progressCallback = ProgressCallback());
};

}

#endif
