#pragma once

#include <QDateTime>
#include <QHash>

namespace Logger_p {
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

	QHash<QString, QString> m_logVars{}; // �û�����ļ�¼������
	QDateTime m_timestamp{};             // �������ں�ʱ��
	QtMsgType m_type{};                  // ��Ϣ����
	Qt::HANDLE m_threadId{};             // �߳�ID��
	QString m_message{};                 // ��Ϣ�ı�
	QString m_file{};                    // �ļ���
	QString m_function{};                // ������
	int m_line{};                        // �к�
};
}
