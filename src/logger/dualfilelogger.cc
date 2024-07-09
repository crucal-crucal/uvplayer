﻿#include "dualfilelogger.hpp"

#include "dualfilelogger_p.hpp"

/*!
 *  \DualFileLoggerPrivate
 *  \internal
 */
Logger_p::DualFileLoggerPrivate::DualFileLoggerPrivate(DualFileLogger* q): q_ptr(q) {
}

Logger_p::DualFileLoggerPrivate::~DualFileLoggerPrivate() = default;

void Logger_p::DualFileLoggerPrivate::init(QSettings*& firstSettings, QSettings*& secondSettings, const int refreshInterval) {
	Q_Q(DualFileLogger);

	m_firstLogger = new FileLogger(firstSettings, refreshInterval, q);
	m_secondLogger = new FileLogger(secondSettings, refreshInterval, q);
}

/*!
 *  \DualFileLogger
 */
Logger_p::DualFileLogger::DualFileLogger(QSettings* firstSettings, QSettings* secondSettings, const int refreshInterval, QObject* parent)
: Logger(parent), d_ptr(new DualFileLoggerPrivate(this)) {
	d_ptr->init(firstSettings, secondSettings, refreshInterval);
}

Logger_p::DualFileLogger::~DualFileLogger() = default;

void Logger_p::DualFileLogger::log(const QtMsgType type, const QString& message, const QString& file, const QString& function, const int line) {
	Q_D(DualFileLogger);

	d->m_firstLogger->log(type, message, file, function, line);
	d->m_secondLogger->log(type, message, file, function, line);
}

void Logger_p::DualFileLogger::clear(const bool buffer, const bool variables) {
	Q_D(DualFileLogger);

	d->m_firstLogger->clear(buffer, variables);
	d->m_secondLogger->clear(buffer, variables);
}