#pragma once

#include <QSettings>

#include "logger.hpp"

namespace Logger_p {
class DualFileLoggerPrivate;

/**
 * @brief 同时将日志消息写入两个日志文件
 * @see FileLogger 详细日志描述
 */
class LOGGER_P_EXPORT DualFileLogger final : public Logger {
	Q_OBJECT
	Q_DISABLE_COPY(DualFileLogger)
	Q_DECLARE_PRIVATE(DualFileLogger)

public:
	/**
	 * @note 在运行期间不能更改，所及建议提供单独的QSettings实例，该实例不被程序的其他部分使用
	 * FileLogger 不接管QSettings实例的所有权，因此调用者应该在关机时销毁它
	 * @param firstSettings 第一个日志文件配置
	 * @param secondSettings 第二个日志文件配置
	 * @param refreshInterval 刷新间隔，单位毫秒, 0 = 禁用
	 * @param parent 父对象
	 */
	explicit DualFileLogger(QSettings* firstSettings, QSettings* secondSettings, int refreshInterval = 10000, QObject* parent = nullptr);
	~DualFileLogger() override;

	/**
	 * @note 如果 type >= minLevel, 装饰并记录消息
	 * 这个方法是线程安全的
	 * @param type 消息类型
	 * @param message 消息内容
	 * @param file 文件名
	 * @param function 函数名
	 * @param line 行号
	 * @see LogMessage 详细日志描述
	 */
	void log(QtMsgType type, const QString& message, const QString& file, const QString& function, int line) override;
	/**
	 * @note 清除日志文件,这个方法是线程安全的
	 * @param buffer 是否清除回溯缓冲区
	 * @param variables 是否清除日志变量
	 */
	void clear(bool buffer, bool variables) override;

protected:
	const QScopedPointer<DualFileLoggerPrivate> d_ptr{ nullptr };
};
}
