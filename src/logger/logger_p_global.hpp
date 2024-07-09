#pragma once

#include <qcompilerdetection.h>
#include <qsystemdetection.h>

/*
 * @note: 导出宏定义
 * 在Windows平台下，动态库中的符号默认情况下是不可见的，这意味着在编译动态库时，其内部的函数和类不会被自动导出到外部使用的程序中。这样做的目的是为了避免动态库中的内部实现细节被外部程序访问到，
 * 从而提高了程序的封装性和安全性。为了让其他程序能够使用动态库中的函数和类，需要在导出时显示地使用__declspec(dllexport)来指定符号的可见性。
 * 而在Linux平台下，默认情况下符号是可见的，这种默认情况是由链接器决定的。
 * Linux系统采用的是一种"就近原则"来解析符号，即在链接时会优先选择最先找到的符号。因此，如果在同一程序或者库中存在多个同名的符号，默认情况下，链接器会选择最先找到的符号，这些符号都是可见的。
 * 这样设计简化了程序的开发和链接过程，并符合了Unix/Linux系统的设计哲学。
 */
#ifdef Q_OS_WIN
#ifdef LOGGER_LIB
#define LOGGER_P_EXPORT Q_DECL_EXPORT
#else
#define LOGGER_P_EXPORT Q_DECL_IMPORT
#endif
#elif defined(Q_OS_UNIX)
#define LOGGER_P_EXPORT
#endif

using i64 = long long;
using ui64 = unsigned long long;
/*
 * @breif: 配置文件默认配置
 */
struct LoggerConfigData {
	std::wstring group{};
	std::wstring fileName{};
	ui64 maxSize{};
	int maxBackups{};
	ui64 bufferSize{};
	std::wstring minLevel{};
	std::string msgFormat{};
	std::string timestampFormat{};

	LoggerConfigData();
	LoggerConfigData& operator=(const LoggerConfigData& other);
	void reset();
};

inline LoggerConfigData::LoggerConfigData() {
	reset();
}

inline LoggerConfigData& LoggerConfigData::operator=(const LoggerConfigData& other) {
	if (this == &other) {
		return *this;
	}
	this->group = other.group;
	this->fileName = other.fileName;
	this->maxSize = other.maxSize;
	this->maxBackups = other.maxBackups;
	this->bufferSize = other.bufferSize;
	this->minLevel = other.minLevel;
	this->msgFormat = other.msgFormat;
	this->timestampFormat = other.timestampFormat;

	return *this;
}

inline void LoggerConfigData::reset() {
	group = L"logging";
	fileName = L"../log/stdout.log";
	maxSize = 1000000;
	maxBackups = 10;
	bufferSize = 100;
	minLevel = L"DEBUG";
	msgFormat = "{timestamp} {typeNr} {type} {thread} {message}\\n  in {file} line {line} function {function}";
	timestampFormat = "dd.MM.yyyy hh:mm:ss.zzz";
}
