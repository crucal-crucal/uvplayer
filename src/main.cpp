#include <fstream>
#include <iostream>
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QResource>
#include <QTranslator>

#include "uvdef.hpp"
#include "conf/uvconf.hpp"
#include "global/uvsingletonwindow.h"
#include "interface/uvmainwindow.hpp"
#include "logger/filelogger.hpp"
#include "uvstring/uvstring.hpp"

CUVIniParser* g_confile = nullptr;   // Configuration parser
QTranslator* g_translator = nullptr; // translator

QString getExecutablePath(); // 获取可执行文件路径
QString getExecutableDir();  // 获取可执行文件所在目录
QString getRunDir();         // 获取运行目录

/**
 * @note 加载配置文件, 内部复制默认配置文件，并且加载日志系统
 * @param app 应用程序对象
 * @return 0 成功，-1 失败
 */
int load_config(QApplication& app);
bool loadResources(const QString& strPath);
bool unloadResources(const QString& strPath);
bool loadStyle(QApplication& app, const QString& strPath);
bool loadTranslate(QApplication& app, const QString& strPath);
bool unloadTranslate();

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	QApplication::setApplicationName(APP_NAME);

	if (load_config(app) != 0) {
		std::cerr << "Failed to load config" << std::endl;
		return -1;
	}

	qInfo("-------------------app start----------------------------------");
	const QFileInfo appFile = getExecutablePath();
	const QDir dir(appFile.absolutePath());
	const QString appParpath = dir.absolutePath();
	const QString rcc_path = appParpath + "/" + QString::fromLatin1(APP_NAME) + ".rcc";
	const QString style_path = ":/skin/" + QString::fromLatin1(APP_NAME) + ".qss";
	const auto translateFilePath = ":/translation/" + QString::fromStdString(g_confile->getValue("language", "ui")) + ".qm";
	// 加载 rcc
	qInfo(loadResources(rcc_path) ? "load resource success!" : "load resource failed!");
	// 加载 style
	qInfo(loadStyle(app, style_path) ? "load style success!" : "load style failed!");
	// translate
	qInfo(loadTranslate(app, translateFilePath) ? "load translate success!" : "load translate failed!");

	CUVMainWindow window;
	// 释放资源
	QObject::connect(&window, &CUVMainWindow::destroyed, [&]() {
		unloadResources(rcc_path);
		unloadTranslate();
		g_confile->set<int>("main_window_state", static_cast<int>(window.window_state), "ui");
		const auto rect = uvstring::asprintf("rect(%d,%d,%d,%d)", window.x(), window.y(), window.width(), window.height());
		g_confile->setValue("main_window_rect", rect, "ui");
		qInfo("-------------------app end----------------------------------");
		g_confile->save();
		SAFE_DELETE(g_confile);
	});

	window.window_state = static_cast<CUVMainWindow::window_state_e>(g_confile->get<int>("main_window_state", "ui"));
	switch (window.window_state) {
		case CUVMainWindow::FULLSCREEN: {
			window.showFullScreen();
			break;
		}
		case CUVMainWindow::MAXIMIZED: {
			window.showMaximized();
			break;
		}
		case CUVMainWindow::MINIMIZED: {
			window.showMinimized();
			break;
		}
		default: {
			const auto rect = g_confile->getValue("main_window_rect", "ui");
			if (!rect.empty()) {
				int x, y, w, h;
				sscanf_s(rect.c_str(), "rect(%d,%d,%d,%d)", &x, &y, &w, &h);
				if (w && h) {
					window.setGeometry(x, y, w, h);
				}
			}
			window.show();
			break;
		}
	}

	if (g_confile->get<bool>("mv_fullscreen", "ui")) {
		window.mv_fullScreen();
	}


	return QApplication::exec();
}

QString getExecutablePath() {
	return QCoreApplication::applicationFilePath();
}

// 获取可执行文件所在目录
QString getExecutableDir() {
	return QCoreApplication::applicationDirPath();
}

// 获取运行目录
QString getRunDir() {
	return QDir::currentPath();
}

int load_config(QApplication& app) {
	QString execDir = getExecutableDir();

	g_confile = new CUVIniParser;
	const QString confFilePath = QString("%1/conf/%2.conf").arg(execDir, APP_NAME);

	if (!QFile::exists(confFilePath)) {
		if (!QFile::copy(execDir + "/conf/" APP_NAME ".conf.default", confFilePath)) {
			qCritical() << "Failed to copy default config file";
			return -1;
		}
	}

	if (g_confile->loadFromFile(confFilePath.toStdString().c_str()) != 0) {
		qCritical() << "Failed to load config file";
		return -1;
	}

	// fontSize & fontFamily
	const auto fontfamily = g_confile->getValue("fontfamily", "ui");
	const auto fontsize = g_confile->get<int>("fontsize", "ui", DEFAULT_FONT_SIZE);
	QFont appFont;
	appFont.setFamily(!fontfamily.empty() ? QString::fromStdString(fontfamily) : "");
	appFont.setPointSize(fontsize);
	QApplication::setFont(appFont);

	// log
	if (QString logFile = QString::fromStdString(g_confile->getValue("logfile")); logFile.isEmpty()) {
		logFile = QString("%1/logs").arg(execDir);
		if (!QDir().mkpath(logFile)) {
			qCritical() << "Failed to create log directory";
			return -1;
		}
	}

	const LoggerConfigData logger_config_data{};
	const auto logSettings = new QSettings(confFilePath, QSettings::IniFormat, &app);
	logSettings->beginGroup(QString::fromStdWString(logger_config_data.group));
	const auto logger = new Logger::FileLogger(logSettings, 10000, &app);
	logger->installMsgHandler();

	return 0; // 返回0表示成功
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

bool loadStyle(QApplication& app, const QString& strPath) {
	if (QFile file(strPath); file.open(QFile::ReadOnly)) {
		const QString styleSheet = QLatin1String(file.readAll());
		app.setStyleSheet(styleSheet);
		file.close();
		return true;
	}
	return false;
}

bool loadTranslate(QApplication& app, const QString& strPath) {
	qDebug() << "Translation filePath:\t" << strPath;
	g_translator = new QTranslator(&app);
	if (!g_translator->load(strPath)) {
		return false;
	}
	QApplication::installTranslator(g_translator);
	return true;
}

bool unloadTranslate() {
	auto bRet = QApplication::removeTranslator(g_translator);
	if (g_translator) {
		delete g_translator;
		g_translator = nullptr;
		qDebug() << "unLoadTranslations, g_translator delete success";
	} else {
		qDebug() << "unLoadTranslations, g_translator == nullptr";
	}

	return bRet;
}
