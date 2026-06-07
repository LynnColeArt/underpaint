// SPDX-License-Identifier: GPL-3.0-or-later

#include "libclient/utils/images.h"

#include <QBuffer>
#include <QColor>
#include <QImage>
#include <QImageWriter>
#include <QMimeData>
#include <QtTest/QtTest>

class TestImageUtils final : public QObject {
	Q_OBJECT
private:
	static bool hasImageWriterFormat(const char *format)
	{
		for(const QByteArray &supported : QImageWriter::supportedImageFormats()) {
			if(supported.compare(format, Qt::CaseInsensitive) == 0) {
				return true;
			}
		}
		return false;
	}

	static bool encodeImage(
		const char *format, QByteArray &bytes, QString &outError)
	{
		QImage img(2, 2, QImage::Format_ARGB32);
		img.fill(QColor(200, 100, 50));

		bytes.clear();
		QBuffer buffer(&bytes);
		if(!buffer.open(QIODevice::WriteOnly)) {
			outError = QStringLiteral("Could not open output buffer.");
			return false;
		}
		QImageWriter writer(&buffer, format);
		if(writer.write(img)) {
			outError.clear();
			return true;
		} else {
			outError = writer.errorString();
			return false;
		}
	}

private slots:
	void testLoadJpegMimeData()
	{
		if(!hasImageWriterFormat("jpg") && !hasImageWriterFormat("jpeg")) {
			QSKIP("Qt JPEG writer plugin is unavailable");
		}

		QByteArray bytes;
		QString error;
		QVERIFY2(encodeImage("JPEG", bytes, error), qPrintable(error));

		QMimeData mimeData;
		mimeData.setData(QStringLiteral("image/jpeg"), bytes);
		QVERIFY(utils::mimeDataHasLoadableImage(&mimeData));

		QImage img = utils::loadImageFromMimeData(&mimeData, &error);
		QVERIFY2(!img.isNull(), qPrintable(error));
		QCOMPARE(img.size(), QSize(2, 2));
	}

	void testLoadPngMimeData()
	{
		if(!hasImageWriterFormat("png")) {
			QSKIP("Qt PNG writer plugin is unavailable");
		}

		QByteArray bytes;
		QString error;
		QVERIFY2(encodeImage("PNG", bytes, error), qPrintable(error));

		QMimeData mimeData;
		mimeData.setData(QStringLiteral("image/png"), bytes);
		QVERIFY(utils::mimeDataHasLoadableImage(&mimeData));

		QImage img = utils::loadImageFromMimeData(&mimeData, &error);
		QVERIFY2(!img.isNull(), qPrintable(error));
		QCOMPARE(img.size(), QSize(2, 2));
	}
};

QTEST_MAIN(TestImageUtils)
#include "images.moc"
