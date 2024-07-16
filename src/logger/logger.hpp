#pragma once

#include <QMutex>
#include <QObject>

#include "logmessage.hpp"

namespace Logger {
class LoggerPrivate;

/*
 * 装饰日志消息并将其写入控制台, stderr
 * 装饰器使用预定义的 msgFormat 字符串来丰富日志消息
 * 附加信息(如时间戳)。
 * msgFormat字符串和消息文本可能包含额外的, 格式为<i>{name}</i>的变量名由值填充, 取自静态线程本地字典。
 * 日志记录器可以在线程本地收集可配置数量的消息, FIFO缓冲区。严重性 >= minLevel的日志消息会刷新缓冲区。
 * 这样信息就被写出来了。有一个例外: INFO 消息被视为DEBUG消息(级别0)。
 * 示例:如果启用缓冲区并使用 minLevel = 2，则应用程序等待，直到发生错误。然后连同错误信息一起写出来
 * 使用同一线程的所有缓冲的低级消息。但只要不错误发生了，什么也不会写出。
 * 如果缓冲区被禁用，那么只有严重性 >= minLevel的消息会写出。
 * @see 参见 LogMessage 获取消息装饰的描述。
 */
class LOGGER_EXPORT Logger : public QObject {
	Q_OBJECT
	Q_DISABLE_COPY(Logger)
	Q_DECLARE_PRIVATE(Logger)

public:
	explicit Logger(QObject* parent = nullptr);
	/**
	 * log levels: 0 = DEBUG, 1 = WARNING, 2 = CRITICAL, 3 = FATAL, 4 = INFO
	 * @param msgFormat 消息格式字符串
	 * @param timestampFormat 时间戳格式字符串
	 * @param minLevel 最低日志级别, 如果bufferSize = 0, 较低级别的消息将被丢弃, 如果bufferSize > 0, 较低级别的消极被缓冲，相等或更高级别的消息触发写入缓冲内容
	 * @param bufferSize 缓冲区大小, 0 = 禁用缓冲区, 否则启用
	 * @param parent 父对象
	 * @see 参见 LogMessage 获取消息装饰的描述。
	 */
	explicit Logger(QString msgFormat, QString timestampFormat, QtMsgType minLevel, int bufferSize, QObject* parent = nullptr);

	~Logger() override;
	/**
	 * @note 如果type >= minLevel, 装饰并记录消息，此方法是线程安全的
	 * @param type 消息类型
	 * @param message 消息文本
	 * @param file 文件名
	 * @param function 函数名
	 * @param line 行号
	 * @see 参见 LogMessage 获取消息装饰的描述。
	 */
	virtual void log(QtMsgType type, const QString& message, const QString& file, const QString& function, int line);
	/*
	 * @note 将此日志记录器安装为默认消息处理程序，安装过后就可使用 qDebug() 等宏来记录日志
	 */
	void installMsgHandler();
	/**
	 * @note 设置可用于修饰日志消息的线程局部变量，此方法是线程安全的
	 * @param name 变量名
	 * @param value 变量值
	 */
	static void setLogVar(const QString& name, const QString& value);
	/*
	 *
	 * @param: name
	 */
	/**
	 * @note 获取可用于修饰日志消息的线程局部变量，此方法是线程安全的
	 * @param name 变量名
	 * @return 变量参数
	 */
	static QString getLogVar(const QString& name);
	/**
	 * @note 清除当前线程的线程本地数据，此方法是线程安全的
	 * @param buffer 是否清除回溯缓冲区
	 * @param variables 是否清除日志变量
	 */
	virtual void clear(bool buffer, bool variables);

protected:
	QString msgFormat{};       // 消息格式字符串
	QString timestampFormat{}; // 时间戳格式字符串
	QtMsgType minLevel{};      // 最低日志级别
	int bufferSize{};          // 缓冲区大小
	static QMutex mutex;       // 用于同步并发线程的访问
	/*
	 * @note 装饰并编写一个日志消息给 stderr
	 */
	virtual void write(const LogMessage* logMessage);

	const QScopedPointer<LoggerPrivate> d_ptr{ nullptr };
};
}
