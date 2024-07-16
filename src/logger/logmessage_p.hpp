#pragma once

#include <QDateTime>
#include <QHash>

namespace Logger {
class LogMessage;

class LogMessagePrivate {
	Q_DISABLE_COPY(LogMessagePrivate)
	Q_DECLARE_PUBLIC(LogMessage)

public:
	explicit LogMessagePrivate(LogMessage* q);
	~LogMessagePrivate();

	void init(const QtMsgType& type, const QString& message, const QHash<QString, QString>*& logVars,
	          const QString& file, const QString& function, const int& line);

	LogMessage* const q_ptr{ nullptr };

	QHash<QString, QString> m_logVars{}; // 用户定义的记录器变量
	QDateTime m_timestamp{};             // 创建日期和时间
	QtMsgType m_type{};                  // 消息类型
	Qt::HANDLE m_threadId{};             // 线程ID号
	QString m_message{};                 // 消息文本
	QString m_file{};                    // 文件名
	QString m_function{};                // 函数名
	int m_line{};                        // 行号
};
}
