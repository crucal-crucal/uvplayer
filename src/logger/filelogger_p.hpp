#pragma once

#include <QBasicTimer>
#include <QFile>
#include <QSettings>

namespace Logger_p {
class FileLogger;

class FileLoggerPrivate {
	Q_DISABLE_COPY(FileLoggerPrivate)
	Q_DECLARE_PUBLIC(FileLogger)

public:
	explicit FileLoggerPrivate(FileLogger* q);
	~FileLoggerPrivate();

	FileLogger* const q_ptr{ nullptr };
	QString m_fileName{};         // 日志文件名
	qlonglong m_maxSize{};        // 文件的最大大小(以字节为单位), 0 =无限
	int m_maxBackups{};           // 备份文件的最大数量, 0 = 无限
	QSettings* m_settings{};      // 配置设置
	QFile* m_file{};              // 日志文件, 0 = 禁用
	QBasicTimer m_refreshTimer{}; // 刷新配置设置的定时器
	QBasicTimer m_flushTimer{};   // 刷新文件I/O缓冲区的定时器
	/**
	 * @note: 打开输出文件
	 */
	void open();
	/**
	 * @note: 关闭输出文件
	 */
	void close();
	/**
	 * @note: 转换文件名, 限制数量在 maxBackups 内
	 */
	void rotate() const;
	/**
	 * @note: 刷新配置设置。
	 * 这个方法是线程安全的。
	 */
	void refreshSettings();
};
}
