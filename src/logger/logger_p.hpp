#pragma once

#include <QtGlobal>
#include <QThreadStorage>

#include "logmessage.hpp"

namespace Logger_p {
class Logger;

class LoggerPrivate {
	Q_DISABLE_COPY(LoggerPrivate)
	Q_DECLARE_PUBLIC(Logger)

public:
	explicit LoggerPrivate(Logger* q);
	~LoggerPrivate();

	Logger* const q_ptr{ nullptr };

	static Logger* defaultLogger; // Ĭ��ʹ�� msgHandler
	/**
	 * @note ��ʼ���̱߳������ݣ��˷������̰߳�ȫ��
	 */
	static void initializeThreadLocalData();
	/**
	 * @note ȫ�־�̬��־��������Ϣ�������, ת����Ĭ�ϼ�¼���ĵ���, �˷������̰߳�ȫ��
	 * �������������Ϣ��������ֹ����Ϣ�еı����������ǵ�ֵ�滻��
	 * @param type ��Ϣ����
	 * @param message ��Ϣ�ı�
	 * @param file �ļ���
	 * @param function ������
	 * @param line �к�
	 */
	static void msgHandler(QtMsgType type, const QString& message, const QString& file = "", const QString& function = "", int line = 0);

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
	/**
	 * @note Qt5�İ�װ��
	 * @param type ��Ϣ����
	 * @param context ��Ϣ������
	 * @param message ��Ϣ�ı�
	 * @see msgHandler()
	 */
	static void msgHandler5(QtMsgType type, const QMessageLogContext& context, const QString& message);
#else
	/**
	 * @note Qt4�İ�װ��
	 * @param type ��Ϣ����
	 * @param message ��Ϣ�ı�
	 * @see msgHandler()
	 */
	static void msgHandler4(QtMsgType type, const char* message);
#endif
	static QThreadStorage<QHash<QString, QString>*> logVars; // ����־��Ϣ��ʹ�õ��ֲ߳̾�����
	QThreadStorage<QList<LogMessage*>*> buffers;             // �̱߳��ػ��ݻ�����
};
}
