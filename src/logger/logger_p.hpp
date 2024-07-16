#pragma once

#include <QtGlobal>
#include <QThreadStorage>

#include "logmessage.hpp"

namespace Logger {
class Logger;

class LoggerPrivate {
	Q_DISABLE_COPY(LoggerPrivate)
	Q_DECLARE_PUBLIC(Logger)

public:
	explicit LoggerPrivate(Logger* q);
	~LoggerPrivate();

	Logger* const q_ptr{ nullptr };

	static Logger* defaultLogger; // 默认使用 msgHandler
	/**
	 * @note 初始化线程本地数据，此方法是线程安全的
	 */
	static void initializeThreadLocalData();
	/**
	 * @note 全局静态日志函数的消息处理程序, 转发对默认记录器的调用, 此方法是线程安全的
	 * 如果出现致命消息，程序将中止。消息中的变量将被它们的值替换。
	 * @param type 消息类型
	 * @param message 消息文本
	 * @param file 文件名
	 * @param function 函数名
	 * @param line 行号
	 */
	static void msgHandler(QtMsgType type, const QString& message, const QString& file = "", const QString& function = "", int line = 0);

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
	/**
	 * @note Qt5的包装器
	 * @param type 消息类型
	 * @param context 消息上下文
	 * @param message 消息文本
	 * @see msgHandler()
	 */
	static void msgHandler5(QtMsgType type, const QMessageLogContext& context, const QString& message);
#else
	/**
	 * @note Qt4的包装器
	 * @param type 消息类型
	 * @param message 消息文本
	 * @see msgHandler()
	 */
	static void msgHandler4(QtMsgType type, const char* message);
#endif
	static QThreadStorage<QHash<QString, QString>*> logVars; // 在日志消息中使用的线程局部变量
	QThreadStorage<QList<LogMessage*>*> buffers;             // 线程本地回溯缓冲区
};
}
