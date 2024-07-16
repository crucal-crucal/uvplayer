#pragma once

#include <QBasicTimer>
#include <QFile>
#include <QSettings>

namespace Logger {
class FileLogger;

class FileLoggerPrivate {
	Q_DISABLE_COPY(FileLoggerPrivate)
	Q_DECLARE_PUBLIC(FileLogger)

public:
	explicit FileLoggerPrivate(FileLogger* q);
	~FileLoggerPrivate();

	FileLogger* const q_ptr{ nullptr };
	QString m_fileName{};             // ��־�ļ���
	qlonglong m_maxSize{};            // �ļ�������С(���ֽ�Ϊ��λ), 0 =����
	int m_maxBackups{};               // �����ļ����������, 0 = ����
	QSettings* m_settings{ nullptr }; // ��������
	QFile* m_file{ nullptr };         // ��־�ļ�, 0 = ����
	QBasicTimer m_refreshTimer{};     // ˢ���������õĶ�ʱ��
	QBasicTimer m_flushTimer{};       // ˢ���ļ�I/O�������Ķ�ʱ��
	/**
	 * @note: ������ļ�
	 */
	void open();
	/**
	 * @note: �ر�����ļ�
	 */
	void close();
	/**
	 * @note: ת���ļ���, ���������� maxBackups ��
	 */
	void rotate() const;
	/**
	 * @note: ˢ���������á�
	 * ����������̰߳�ȫ�ġ�
	 */
	void refreshSettings();
};
}
