#include <iostream>
#include <QApplication>
#include <QDebug>
#include <QFileInfo>
#include <QResource>
#include <QDir>

#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/avfilter.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
#include <libswscale/swscale.h>
#ifdef __cplusplus
}
#endif

#include "uvmainwindow.hpp"

bool loadResources(const QString& strPath);
bool unloadResources(const QString& strPath);

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	QApplication::setApplicationName(APP_NAME);

	const QFileInfo appFile(QApplication::applicationFilePath());
	QDir dir(appFile.absolutePath());
	const QString appParpath = dir.absolutePath();
	const QString rcc_path = appParpath + QString::fromLatin1("/uvplayer.rcc");
	// 加载 rcc
	qInfo((loadResources(rcc_path) ? "Load Resource Success!" : "Load Resource Failed!"));

	CUVMainWindow w;

	// 释放资源
	QObject::connect(&w, &CUVMainWindow::destroyed, [&]() {
		unloadResources(rcc_path);
	});

	w.show();

	return QApplication::exec();
}

bool loadResources(const QString& strPath) {
	qDebug() << ("Resource filePath:\t" + strPath);
	return QResource::registerResource(strPath);
}

bool unloadResources(const QString& strPath) {
	if (QResource::unregisterResource(strPath)) {
		qDebug() << "unregister resource success";
		return true;
	}

	return false;
}
