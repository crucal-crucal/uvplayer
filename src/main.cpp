#include <fstream>
#include <iostream>
#include <iostream>
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QResource>

#include "uvbase.hpp"
#include "uvconf.hpp"
#include "uvmainwindow.hpp"
/**
 * @note config
 */
CUVIniParser* g_confile = nullptr;

QString getExecutablePath(); // 获取可执行文件路径
QString getExecutableDir();  // 获取可执行文件所在目录
QString getRunDir();         // 获取运行目录
void logToFile(const std::string& message, const std::string& logFile);
int load_config();
bool loadResources(const QString& strPath);
bool unloadResources(const QString& strPath);
bool loadStyle(QApplication& app, const QString& strPath);

int main(int argc, char* argv[]) {
	qInfo("-------------------app start----------------------------------");
	QApplication app(argc, argv);
	QApplication::setApplicationName(APP_NAME);

	if (load_config() != 0) {
		std::cerr << "Failed to load config" << std::endl;
		return -1;
	}

	const QFileInfo appFile = getExecutablePath();;
	const QDir dir(appFile.absolutePath());
	const QString appParpath = dir.absolutePath();
	const QString rcc_path = appParpath + "/" + QString::fromLatin1(APP_NAME) + ".rcc";
	const QString style_path = ":/skin/uvplayer.qss";
	// 加载 rcc
	qInfo((loadResources(rcc_path) ? "Load Resource Success!" : "Load Resource Failed!"));
	// 加载 style
	qInfo((loadStyle(app, style_path) ? "Load Style Success!" : "Load Style Failed!"));

	CUVMainWindow w;

	// 释放资源
	QObject::connect(&w, &CUVMainWindow::destroyed, [&]() {
		unloadResources(rcc_path);
		qInfo("-------------------app end----------------------------------");
		g_confile->save();
	});

	w.show();

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

void logToFile(const std::string& message, const std::string& logFile) {
	if (std::ofstream logStream(logFile, std::ios_base::app); logStream.is_open()) {
		logStream << message << std::endl;
	}
}

int load_config() {
	QString execDir = getExecutableDir();

	g_confile = new CUVIniParser;
	const QString confFilePath = QString("%1/conf/%2.conf").arg(execDir, APP_NAME);

	if (!QFile::exists(confFilePath)) {
		if (!QFile::copy(execDir + "/conf/" APP_NAME ".conf.default", confFilePath)) {
			std::cerr << "Failed to copy default config file" << std::endl;
			return -1; // 返回负值表示失败
		}
	}

	if (g_confile->loadFromFile(confFilePath.toStdString().c_str()) != 0) {
		std::cerr << "Failed to load config file" << std::endl;
		return -1; // 返回负值表示失败
	}

	// 日志文件
	if (QString logFile = QString::fromStdString(g_confile->getValue("logfile")); logFile.isEmpty()) {
		logFile = QString("%1/logs").arg(execDir);
		if (!QDir().mkpath(logFile)) {
			std::cerr << "Failed to create log directory" << std::endl;
			return -1; // 返回负值表示失败
		}
	}

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
	QFile file(strPath);
	if (file.open(QFile::ReadOnly)) {
		const QString styleSheet = QLatin1String(file.readAll());
		app.setStyleSheet(styleSheet);
		file.close();
		return true;
	}
	return false;
}
