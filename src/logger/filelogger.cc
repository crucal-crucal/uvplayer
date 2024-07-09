#include "filelogger.hpp"

#include <QDebug>
#include <QDir>
#include <QTimerEvent>

#include "filelogger_p.hpp"

/*!
 *  \FileLoggerPrivate
 *  \internal
 */
Logger_p::FileLoggerPrivate::FileLoggerPrivate(FileLogger* q): q_ptr(q) {
}

Logger_p::FileLoggerPrivate::~FileLoggerPrivate() = default;

void Logger_p::FileLoggerPrivate::open() {
	if (m_fileName.isEmpty()) {
		qWarning("Name of logFile is empty");
	} else {
		m_file = new QFile(m_fileName);
		qDebug() << "Opening log file:\t" << m_fileName;
		if (!m_file->open(QFile::WriteOnly | QFile::Append | QFile::Text)) {
			qWarning("Cannot open log file %s: %s",qPrintable(m_fileName),qPrintable(m_file->errorString()));
			m_file = nullptr;
		}
	}
}

void Logger_p::FileLoggerPrivate::close() {
	if (m_file) {
		m_file->close();
		delete m_file;
		m_file = nullptr;
	}
}

void Logger_p::FileLoggerPrivate::rotate() const {
	int count{};
	forever {
		if (QFile bakFile(QString("%1.%2").arg(m_fileName).arg(count + 1)); bakFile.exists()) {
			++count;
		} else {
			break;
		}
	}

	while (m_maxBackups > 0 && count >= m_maxBackups) {
		QFile::remove(QString("%1.%2").arg(m_fileName, count));
		--count;
	}

	for (int i = count; i > 0; i--) {
		QFile::rename(QString("%1.%2").arg(m_fileName, i), QString("%1.%2").arg(m_fileName, i + 1));
	}

	QFile::rename(m_fileName, m_fileName + ".1");
}

void Logger_p::FileLoggerPrivate::refreshSettings() {
	Q_Q(FileLogger);

	Logger_p::FileLogger::mutex.lock();
	const QString oldFileName = m_fileName;
	m_settings->sync();
	m_fileName = m_settings->value("fileName").toString();
#ifdef Q_OS_WIN32
	if (QDir::isRelativePath(m_fileName) && m_settings->format() != QSettings::NativeFormat)
#else
	if (QDir::isRelativePath(m_fileName))
#endif
	{
		const QFileInfo configFile(m_settings->fileName());
		m_fileName = QFileInfo(configFile.absolutePath(), m_fileName).absoluteFilePath();
	}
	m_maxSize = m_settings->value("maxSize", 0).toLongLong();
	m_maxBackups = m_settings->value("maxBackups", 0).toInt();
	q->msgFormat = m_settings->value("msgFormat", "{timestamp} {type} {message}").toString();
	q->timestampFormat = m_settings->value("timestampFormat", "yyyy-MM-dd hh:mm:ss.zzz").toString();
	q->bufferSize = m_settings->value("bufferSize", 0).toInt();

	if (const QByteArray minLevelStr = m_settings->value("minLevel", "ALL").toByteArray(); minLevelStr == "ALL" || minLevelStr == "DEBUG" ||
		minLevelStr == "0") {
		q->minLevel = QtDebugMsg;
	} else if (minLevelStr == "WARNING" || minLevelStr == "WARN" || minLevelStr == "1") {
		q->minLevel = QtWarningMsg;
	} else if (minLevelStr == "CRITICAL" || minLevelStr == "ERROR" || minLevelStr == "2") {
		q->minLevel = QtCriticalMsg;
	} else if (minLevelStr == "FATAL" || minLevelStr == "3") {
		q->minLevel = QtFatalMsg;
	} else if (minLevelStr == "INFO" || minLevelStr == "4") {
		q->minLevel = QtInfoMsg;
	}

	if (oldFileName != m_fileName) {
		fprintf(stderr, "Logging to %s\n", qPrintable(m_fileName));
		close();
		open();
	}
	Logger_p::FileLogger::mutex.unlock();
}

/*!
 *  \FileLogger
 */
Logger_p::FileLogger::FileLogger(QSettings* settings, const int refreshInterval, QObject* parent)
: Logger(parent), d_ptr(new FileLoggerPrivate(this)) {
	Q_ASSERT(settings != nullptr);
	Q_ASSERT(refreshInterval >= 0);
	d_ptr->m_settings = settings;
	if (refreshInterval > 0) {
		d_ptr->m_refreshTimer.start(refreshInterval, this);
	}
	d_ptr->m_flushTimer.start(1000, this);
	d_ptr->refreshSettings();
}
Logger_p::FileLogger::~FileLogger() {
	Q_D(FileLogger);

	d->close();
}

void Logger_p::FileLogger::write(const LogMessage* logMessage) {
	Q_D(FileLogger);

	if (d->m_file) {
		d->m_file->write(qPrintable(logMessage->toString(msgFormat, timestampFormat)));
		if (logMessage->getType() >= QtCriticalMsg) {
			d->m_file->flush();
		}
		if (d->m_file->error()) {
			qWarning("Cannot write to log file %s: %s",qPrintable(d->m_fileName),qPrintable(d->m_file->errorString()));
			d->close();
		}
	}
	Logger::write(logMessage);
}

void Logger_p::FileLogger::timerEvent(QTimerEvent* event) {
	Q_D(FileLogger);

	if (!event) {
		return;
	}

	if (event->timerId() == d->m_refreshTimer.timerId()) {
		d->refreshSettings();
	} else if (event->timerId() == d->m_flushTimer.timerId() && d->m_file) {
		mutex.lock();
		d->m_file->flush();
		if (d->m_maxSize > 0 && d->m_file->size() >= d->m_maxSize) {
			d->close();
			d->rotate();
			d->open();
		}
		mutex.unlock();
	}
}
