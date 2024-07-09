#include "logmessage.hpp"

#include <QThread>
#include <utility>

#include "logmessage_p.hpp"

/*!
 *  \LogMessagePrivate
 *  \internal
 */
Logger_p::LogMessagePrivate::LogMessagePrivate(LogMessage* q): q_ptr(q) {
}

Logger_p::LogMessagePrivate::~LogMessagePrivate() = default;

void Logger_p::LogMessagePrivate::init(const QtMsgType& type, const QString& message, const QHash<QString, QString>*& logVars, const QString& file,
                                       const QString& function, const int& line) {
	m_timestamp = QDateTime::currentDateTime();
	m_threadId = QThread::currentThreadId();
	m_type = type;
	m_message = message;
	m_file = file;
	m_function = function;
	m_line = line;
	if (logVars) {
		m_logVars = *logVars;
	}
}

/*!
 *  \LogMessage
 */
Logger_p::LogMessage::LogMessage(const QtMsgType& type, const QString& message, const QHash<QString, QString>* logVars, const QString& file,
                                 const QString& function, const int line): d_ptr(new LogMessagePrivate(this)) {
	d_func()->init(type, message, logVars, file, function, line);
}

Logger_p::LogMessage::~LogMessage() = default;

QString Logger_p::LogMessage::toString(const QString& msgFormat, const QString& timestampFormat) const {
	Q_D(const LogMessage);

	QString decorated = msgFormat + "\n";
	decorated.replace("{message}", d->m_message);
	if (decorated.contains("{timestamp}")) {
		decorated.replace("{timestamp}", d->m_timestamp.toString(timestampFormat));
	}
	QString typeNr{};
	typeNr.setNum(d->m_type);
	decorated.replace("{typeNr}", typeNr);

	switch (d->m_type) {
		case QtDebugMsg: {
			decorated.replace("{type}", "DEBUG	 ");
			break;
		}
		case QtWarningMsg: {
			decorated.replace("{type}", "WARNING ");
			break;
		}
		case QtCriticalMsg: {
			decorated.replace("{type}", "CRITICAL");
			break;
		}
		case QtFatalMsg: {
			decorated.replace("{type}", "FATAL   ");
			break;
		}
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
		case QtInfoMsg: {
			decorated.replace("{type}", "INFO    ");
			break;
		}
#endif
		default: break;
	}

	decorated.replace("{file}", d->m_file);
	decorated.replace("{function}", d->m_function);
	decorated.replace("{line}", QString::number(d->m_line));

	const QString threadId = QString("0x%1").arg(reinterpret_cast<qulonglong>(QThread::currentThread()), 8, 16, QLatin1Char('0'));
	decorated.replace("{thread}", threadId);

	if (decorated.contains("{") && !d->m_logVars.isEmpty()) {
		QList<QString> keys = d->m_logVars.keys();
		foreach(auto key, keys) {
			decorated.replace("{" + key + "}", d->m_logVars.value(key));
		}
	}

	return decorated;
}

QtMsgType Logger_p::LogMessage::getType() const {
	Q_D(const LogMessage);

	return d->m_type;
}
