#pragma once

#include <QDateTime>
#include <QScopedPointer>

#include "logger_p_global.hpp"

namespace Logger {
class LogMessagePrivate;

/*
 * @brief 一条日志消息
 * 以下变量可以在消息和 msgFormat 中使用:
 * - {timestamp} 创建日期和时间
 * - {typeNr} 数字格式的消息类型(0-3)
 * - {type} 字符串格式的消息类型(DEBUG, WARNING, CRITICAL, FATAL)
 * - {thread} 线程ID号
 * - {message} 消息文本
 * - {xxx} 用于任何用户定义的记录器变量
 * - {file} 文件名
 * - {function} 函数名
 * - {line} 行号
 */
class LOGGER_EXPORT LogMessage {
	Q_DISABLE_COPY(LogMessage)
	Q_DECLARE_PRIVATE(LogMessage)

public:
	/**
	 *
	 * @param type 消息类型
	 * @param message 消息文本
	 * @param logVars 用户定义的记录器变量，允许为0
	 * @param file 文件名
	 * @param function 函数名
	 * @param line 行号
	 */
	LogMessage(const QtMsgType& type, const QString& message, const QHash<QString, QString>* logVars,
	           const QString& file, const QString& function, int line);
	~LogMessage();
	/**
	 * @note 将消息转换为字符串
	 * @param msgFormat msgFormat装饰格式，可能包含变量和静态文本
	 * @param timestampFormat 时间戳格式
	 * @return 格式化后的字符串
	 */
	[[nodiscard]] QString toString(const QString& msgFormat, const QString& timestampFormat) const;
	/**
	 * @note 获取消息类型
	 * @return 消息类型
	 */
	[[nodiscard]] QtMsgType getType() const;

protected:
	const QScopedPointer<LogMessagePrivate> d_ptr{ nullptr };
};
}
