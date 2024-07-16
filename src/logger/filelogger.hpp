#pragma once

#include <QBasicTimer>
#include <QFile>
#include <QSettings>

#include "logger.hpp"

namespace Logger {
class FileLoggerPrivate;

/*
 * @brief 使用文本文件作为输出的记录器。设置从使用QSettings对象创建配置文件。可以在运行时更改配置设置
 * <p>
 * 配置示例:
 * <code><pre>
 * fileName = ../logs/QtWebApp.log
 * maxSize = 1000000
 * maxBackups = 10
 * bufferSize = 0
 * minLevel = WARNING
 * msgformat = {timestamp} {typeNr} {type}
 * timestampFormat=dd.MM.  yyyy hh:mm:ss.zzz
 * </pre></code>
 * - 可能的日志级别为:ALL/DEBUG=0, WARN/WARNING=1, ERROR/CRITICAL=2, FATAL=3, INFO=4
 * - "fileName"为日志文件相对于设置文件所在目录的名称。对于windows，如果设置在注册表中，则路径相对于当前工作目录
 * - maxSize为该文件的最大大小(以字节为单位)。文件将被备份, 如果该文件大于此限制，则由新文件替换。请注意实际文件大小可能会比这个限制稍大一些。默认值为0=无限制
 * - maxBackups定义要保留的备份文件数量。默认值为0=无限制
 * - bufferSize定义环形缓冲区的大小。默认为0=禁用
 * - minLevel如果bufferSize=0，表示丢弃低级别的消息
 * - msgFormat定义日志消息的装饰，参见LogMessage类。默认为"{timestamp} {type} {message}"
 *
 * @see set()描述了如何设置日志记录器变量
 * @see LogMessage获取消息装饰的描述
 * @see Logger获取缓冲区的描述
 */
class LOGGER_EXPORT FileLogger final : public Logger {
	Q_OBJECT
	Q_DISABLE_COPY(FileLogger)
	Q_DECLARE_PRIVATE(FileLogger)

public:
	/**
	 * @note 配置设置，通常存储在INI文件中。不能是0。设置是从当前组读取的，因此调用者必须调用Settings ->beginGroup()。
	 * 因为组在运行时期间不能更改，所以建议提供单独的QSettings实例，该实例不被程序的其他部分使用。
	 * FileLogger不接管QSettings实例的所有权，因此调用者应该在关机时销毁它。
	 * @param settings 配置
	 * @param refreshInterval 刷新配置的时间间隔(以毫秒为单位)。默认值为10000, 0 = 禁用
	 * @param parent 父对象
	 */
	explicit FileLogger(QSettings* settings, int refreshInterval = 10000, QObject* parent = nullptr);
	~FileLogger() override;
	/**
	 * @note 写入日志文件
	 * @param logMessage 一条日志消息对象
	 */
	void write(const LogMessage* logMessage) override;

protected:
	/*
	 * @note 根据事件刷新配置设置或同步I/O缓冲区。
	 * 这个方法是线程安全的。
	 */
	void timerEvent(QTimerEvent* event) override;

	const QScopedPointer<FileLoggerPrivate> d_ptr{ nullptr };
};
}
