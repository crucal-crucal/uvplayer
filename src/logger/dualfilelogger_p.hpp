#pragma once

#include "filelogger.hpp"

namespace Logger_p {
class DualFileLogger;

class DualFileLoggerPrivate {
	Q_DISABLE_COPY(DualFileLoggerPrivate)
	Q_DECLARE_PUBLIC(DualFileLogger)

public:
	explicit DualFileLoggerPrivate(DualFileLogger* q);
	~DualFileLoggerPrivate();

	void init(QSettings*& firstSettings, QSettings*& secondSettings, int refreshInterval);

	DualFileLogger* const q_ptr{ nullptr };

	FileLogger* m_firstLogger{ nullptr };
	FileLogger* m_secondLogger{ nullptr };
};
} // namespace Logger_p
