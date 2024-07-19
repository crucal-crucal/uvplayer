#pragma once
#include <atomic>
#include <QApplication>
#include <QDebug>
#include <QMutex>
#include <QRunnable>
#include <QThread>
#include <QUrl>
#include <QWaitCondition>

#include "def/avdef.hpp"
#include "def/uvdef.hpp"
#include "util/uvffmpeg_util.hpp"

class CLXCodecThread;
class CLXVideoThread;
class CLXAudioThread;
class CLXVideoPlayThread;
class CLXAudioPlayThread;
class CLXEncodeVideoThread;
class CLXEncodeAudioThread;
class CLXEncodeMuteAudioThread;
class CLXPushThread;
struct AVFormatContext;
struct AVCodecContext;
struct AVFrame;
struct SwsContext;
struct AVPacket;
struct AVRational;
struct AVCodecParameters;

// ѭ������
template<typename T>
struct CircularQueue {
public:
	// ����ָ����С��ѭ������
	explicit CircularQueue(quint64 size);
	~CircularQueue();
	// �ж�ѭ�������Ƿ�����
	bool isFull() const;
	// �ж�ѭ�������Ƿ�Ϊ��
	bool isEmpty() const;
	// ��Ԫ������ѭ������ĩβ
	bool push(T);
	// ��������
	T pop();
	// ��ȡ����
	T first();
	// ��ն���
	void clear();
	int size() const;

private:
	std::atomic_int m_nSize{ 0 };  // ѭ�������ܴ�С
	std::atomic_int m_nFront{ 0 }; // ѭ������ͷ������
	std::atomic_int m_nRear{ 0 };  // ѭ������β������
	T* m_pData{ nullptr };         // �洢ѭ������Ԫ�ص�����
};

template<typename T>
inline CircularQueue<T>::CircularQueue(quint64 size): m_nSize(size) { // NOLINT
	m_pData = new T[size];
}

template<typename T>
inline CircularQueue<T>::~CircularQueue() {
	delete[]m_pData;
}

template<typename T>
inline bool CircularQueue<T>::isFull() const {
	return m_nFront == (m_nRear + 1) % m_nSize;
}

template<typename T>
inline bool CircularQueue<T>::isEmpty() const {
	return m_nFront == m_nRear;
}

template<typename T>
inline bool CircularQueue<T>::push(T data) {
	if (isFull()) {
		return false;
	}
	m_pData[m_nRear] = data;
	m_nRear = (m_nRear + 1) % m_nSize;
	return true;
}

template<typename T>
inline T CircularQueue<T>::pop() {
	if (isEmpty()) {
		return nullptr;
	}
	T data = m_pData[m_nFront];
	m_nFront = (m_nFront + 1) % m_nSize;
	return data;
}

template<typename T>
inline T CircularQueue<T>::first() {
	if (isEmpty()) {
		return nullptr;
	}
	T data = m_pData[m_nFront];
	return data;
}

template<typename T>
inline void CircularQueue<T>::clear() {
	m_nFront = 0;
	m_nRear = 0;
}

template<typename T>
inline int CircularQueue<T>::size() const {
	return (m_nRear - m_nFront + m_nSize) % m_nSize;
}

class CCalcPtsDur {
public:
	CCalcPtsDur();
	~CCalcPtsDur();

	// ����ʱ���׼��֡�ʣ�������֡�ĳ���ʱ��
	void SetTimeBase(int iTimeBase, int iFpsNum, int iTimeBen);
	// ���þ��Ի�׼ʱ��
	void SetAbsBaseTime(const int64_t& llAbsBaseTime);
	// ����֡����������Ƶ֡��ʱ���
	[[nodiscard]] int64_t GetVideoPts(int64_t lFrameIndex) const;
	// ����֡����������Ƶ֡�ĳ���ʱ��
	[[nodiscard]] int GetVideoDur(int64_t lFrameIndex) const;
	// ���ݰ���������Ƶ������������Ƶ֡��ʱ���
	[[nodiscard]] int64_t GetAudioPts(int64_t lPaketIndex, int iAudioSample) const;
	// ���ݰ���������Ƶ������������Ƶ֡�ĳ���ʱ��
	[[nodiscard]] int GetAudioDur(int64_t lPaketIndex, int iAudioSample) const;

	double m_dTimeBase; // ʱ���׼
	double m_dFpsBen;   // ֡�ʷ���
	double m_dFpsNum;   // ֡�ʷ�ĸ
	double m_dFrameDur; // ֡�ĳ���ʱ��

private:
	int64_t m_llAbsBaseTime; // ���Ի�׼ʱ��
};

class CLXCodecThread final : public QThread, public QRunnable {
	Q_OBJECT

public:
	// �̴߳�ģʽ
	enum OpenMode {
		OpenMode_Play = 0x1,
		OpenMode_Push = 0x2
	};
	enum eLXDecodeMode {
		eLXDecodeMode_CPU = 0, //cpu����
		eLXDecodeMode_GPU      //gpu����
	};

	explicit CLXCodecThread(QString strFile, LXPushStreamInfo stStreamInfo, QSize szPlay,
	                        QPair<AVCodecParameters*, AVCodecParameters*> pairRecvCodecPara, OpenMode mode = OpenMode::OpenMode_Push,
	                        bool bLoop = false, bool bPicture = false, QObject* parent = nullptr);
	explicit CLXCodecThread(QObject* parent = nullptr);
	~CLXCodecThread() override;

public:
	void open(QString strFile, QSize szPlay, LXPushStreamInfo stStreamInfo = LXPushStreamInfo(), OpenMode mode = OpenMode_Play, bool bLoop = false,
	          bool bPicture = false);
	void seek(const quint64& nDuration);
	void pause();
	void resume();
	void stop();
signals:
	void notifyClipInfo(const quint64&);
	void notifyCountDown(const quint64&);
	void notifyImage(const QPixmap&);
	void notifyAudio(const QByteArray&);
	void notifyAudioPara(const quint64&, const quint64&);

protected:
	void run() override;

private:
	static void LogCallBack(void*, int, const char*, va_list);
	void clearMemory();

private:
	QPair<AVCodecParameters*, AVCodecParameters*> m_pairRecvCodecPara;
	AVFormatContext* m_pFormatCtx{ nullptr };

private:
	QString m_strFile;                   // Ҫ������ļ�·��
	QSize m_szPlay;                      // ���Ŵ��ڴ�С
	LXPushStreamInfo m_stPushStreamInfo; // ��������Ϣ
	bool m_bRunning{ true };
	bool m_bPause{ false };
	int m_nVideoIndex{ -1 };
	int m_nAudioIndex{ -1 };
	CircularQueue<AVPacket*> m_videoPacketQueue{ 1000 };      // �洢��Ƶ���ݵ� AVPacket ָ�����
	CircularQueue<AVPacket*> m_audioPacketQueue{ 10000 };     // �洢��Ƶ���ݵ� AVPacket ָ��
	CircularQueue<AVPacket*> m_videoPushPacketQueue{ 1000 };  // �洢������Ƶ���ݵ� AVPacket ָ��
	CircularQueue<AVPacket*> m_audioPushPacketQueue{ 10000 }; // �洢������Ƶ���ݵ� AVPacket ָ��
	QWaitCondition m_videoWaitCondition;
	QMutex m_videoMutex; // ��Ƶ�̵߳ĵȴ������ͻ�����
	QWaitCondition m_audioWaitCondition;
	QMutex m_audioMutex; // ��Ƶ�̵߳ĵȴ������ͻ�����
	QWaitCondition m_pushVideoWaitCondition;
	QMutex m_pushVideoMutex; // ������Ƶ�����̵߳ĵȴ������ͻ�����
	QWaitCondition m_pushAudioWaitCondition;
	QMutex m_pushAudioMutex; // ������Ƶ�����̵߳ĵȴ������ͻ�����
	QWaitCondition m_AVSyncWaitCondition;
	QMutex m_AVSyncMutex; // ����Ƶͬ���̵߳ĵĵȴ������ͻ�����

	CLXVideoThread* m_pVideoThread{ nullptr };                // ��Ƶ�߳�
	CLXAudioThread* m_pAudioThread{ nullptr };                // ��Ƶ�߳�
	CLXEncodeMuteAudioThread* m_pEncodeMuteThread{ nullptr }; // ���뾲����Ƶ�߳�
	CLXPushThread* m_pPushThread{ nullptr };                  // �����߳�
	CLXCodecThread::OpenMode m_eMode{ CLXCodecThread::OpenMode::OpenMode_Play };
	bool m_bLoop{ false };
	bool m_bPicture{ false };
	eLXDecodeMode m_eDecodeMode{ eLXDecodeMode::eLXDecodeMode_CPU };
	static FILE* m_pLogFile;
};

class CLXVideoThread final : public QThread {
	Q_OBJECT

public:
	/* @Parameter
	packetQueue �洢����ǰ�����ݰ�����
	pushPacketQueue �洢���������ݰ�����
	waitCondition �߳�ͬ������
	mutex ������
	videoWaitCondition ��Ƶͬ������
	videoMutex ��Ƶ������
	pFormatCtx ������Ƶ��ʽ������
	nStreamIndex ������Ƶ������
	nEncodeStreamIndex �������Ƶ������
	mapOutputCtx �����ʽ�����ĺͱ���������Ϣ
	eMode �򿪸�ʽ
	bSendCountDown �Ƿ��͵���ʱ�ź�
	bPush �Ƿ�����
	szPlay ��Ƶ���ŵĳߴ�
	eDecodeMode ����ģʽ
	*/
	explicit CLXVideoThread(CircularQueue<AVPacket*>& packetQueue, CircularQueue<AVPacket*>& pushPacketQueue, QWaitCondition& waitCondition,
	                        QMutex& mutex,
	                        QWaitCondition& videoWaitCondition, QMutex& videoMutex, AVFormatContext* pFormatCtx,
	                        int nStreamIndex, int nEncodeStreamIndex, QPair<AVFormatContext*, std::tuple<CCalcPtsDur, AVCodecContext*>> mapOutputCtx,
	                        CLXCodecThread::OpenMode eMode,
	                        bool bSendCountDown, bool bPush, QSize szPlay, CLXCodecThread::eLXDecodeMode eDecodeMode, QObject* parent = nullptr);
	~CLXVideoThread() override;

public:
	void pause();
	void resume();
	void stop();

public:
	// ���õ�ǰ��ʱ���
	void setCurrentPts(int64_t nPts);

	static enum AVPixelFormat hw_pix_fmt;
	static enum AVPixelFormat get_hw_format(AVCodecContext* ctx,
	                                        const enum AVPixelFormat* pix_fmts);
signals:
	// ֪ͨ����ʱ
	void notifyCountDown(const quint64&);
	// ֪ͨͼ��
	void notifyImage(const QPixmap&);

protected:
	void run() override;

private:
	AVFormatContext* m_pFormatCtx{ nullptr };                                          // ������Ƶ����������
	AVCodecContext* m_pCodecCtx{ nullptr };                                            // ��Ƶ���������������
	QSize m_szPlay;                                                                    // ��Ƶ���ųߴ�
	QPair<AVFormatContext*, std::tuple<CCalcPtsDur, AVCodecContext*>> m_pairOutputCtx; // �����ʽ�����ĺͱ���������Ϣ
	int m_nStreamIndex{ -1 };                                                          // ������Ƶ������
	int m_nEncodeStreamIndex{ -1 };                                                    // �������Ƶ������
	bool m_bRunning{ true };                                                           // �Ƿ�������
	bool m_bPause{ false };                                                            // �Ƿ���ͣ
	bool m_bSendCountDown{ false };                                                    // �Ƿ��͵���ʱ�ź�
	bool m_bPush{ false };                                                             // �Ƿ�����
	bool m_decodeType{ false };                                                        // ��������

	CLXEncodeVideoThread* m_pEncodeThread{ nullptr };  // ��Ƶ�����߳�
	CLXVideoPlayThread* m_pPlayThread{ nullptr };      // ��Ƶ�����߳�
	CircularQueue<AVPacket*>& m_packetQueue;           // �洢����ǰ�����ݰ�ѭ������
	CircularQueue<AVPacket*>& m_pushPacketQueue;       // �洢���������ݰ�ѭ������
	CircularQueue<AVFrame*> m_decodeFrameQueue{ 100 }; // �洢�������Ƶ֡��ѭ������
	CircularQueue<AVFrame*> m_playFrameQueue{ 100 };   // �洢���ŵ���Ƶ֡��ѭ������
	QWaitCondition& m_decodeWaitCondition;             // �����߳����������ͻ�����
	QMutex& m_decodeMutex;
	QWaitCondition m_encodeWaitCondition; // �����߳����������ͻ�����
	QMutex m_encodeMutex;
	QWaitCondition m_playWaitCondition; // �����߳����������ͻ�����
	QMutex m_playMutex;
	QWaitCondition& m_pushWaitCondition; // �����߳����������ͻ�����
	QMutex& m_pushMutex;
	CLXCodecThread::OpenMode m_eMode{ CLXCodecThread::OpenMode::OpenMode_Play }; // ��ģʽ���ǲ��Ż�������
	const CLXCodecThread::eLXDecodeMode& m_eDecodeMode;                          // ����ģʽ
};

class CLXAudioThread final : public QThread {
	Q_OBJECT

public:
	/* @Parameter
	packetQueue �洢����ǰ�����ݰ�����
	pushPacketQueue �洢���������ݰ�����
	waitCondition �߳�ͬ������
	mutex ������
	audioWaitCondition ��Ƶͬ������
	audioMutex ��Ƶ������
	pFormatCtx ������Ƶ��ʽ������
	nStreamIndex ������Ƶ������
	nEncodeStreamIndex �������Ƶ������
	pairOutputCtx �����ʽ�����ĺͱ���������Ϣ
	eMode �򿪸�ʽ
	bPush �Ƿ�����
	decodePara �������
	*/
	explicit CLXAudioThread(CircularQueue<AVPacket*>& packetQueue, CircularQueue<AVPacket*>& pushPacketQueue, QWaitCondition& waitCondition,
	                        QMutex& mutex,
	                        QWaitCondition& audioWaitCondition, QMutex& audioMutex, AVFormatContext* pFormatCtx,
	                        int nStreamIndex, int nEncodeStreamIndex, QPair<AVFormatContext*, std::tuple<CCalcPtsDur, AVCodecContext*>> pairOutputCtx,
	                        CLXCodecThread::OpenMode eMode,
	                        bool bPush, const AVCodecParameters& decodePara, QObject* parent = nullptr);
	~CLXAudioThread() override;

public:
	void pause();
	void resume();
	void stop();

public:
	void setCurrentPts(int64_t nPts);
signals:
	// ֪ͨ����ʱ��Ϣ
	void notifyCountDown(const quint64&);
	// ֪ͨ��Ƶ����
	void notifyAudio(const QByteArray&);
	// ֪ͨ��Ƶ����
	void notifyAudioPara(const quint64&, const quint64&);

protected:
	void run() override;

private:
	CLXEncodeAudioThread* m_pEncodeThread{ nullptr };                                  // ������Ƶ�߳�
	CLXAudioPlayThread* m_pPlayThread{ nullptr };                                      // ������Ƶ�߳�
	AVFormatContext* m_pFormatCtx{ nullptr };                                          // ������Ƶ���ĸ�ʽ������
	AVCodecContext* m_pCodecCtx{ nullptr };                                            // ��Ƶ������������
	const AVCodecParameters& m_decodePara;                                             // �������
	QPair<AVFormatContext*, std::tuple<CCalcPtsDur, AVCodecContext*>> m_pairOutputCtx; // �����Ƶ�������������
	int m_nStreamIndex{ -1 };                                                          // ������Ƶ������
	int m_nEncodeStreamIndex{ -1 };                                                    // ������Ƶ������
	bool m_bRunning{ true };                                                           // �߳��Ƿ�����
	bool m_bPause{ false };                                                            // �߳��Ƿ���ͣ
	bool m_bPush{ false };                                                             // �߳��Ƿ�����

	CircularQueue<AVPacket*>& m_packetQueue;            // ��Ƶ������
	CircularQueue<AVPacket*>& m_pushPacketQueue;        // ��Ƶ���Ͱ�����
	CircularQueue<AVFrame*> m_decodeFrameQueue{ 1000 }; // ����֡����
	CircularQueue<AVFrame*> m_playFrameQueue{ 1000 };   // ����֡����
	QWaitCondition& m_decodeWaitCondition;              // ����ȴ������Լ����뻥����
	QMutex& m_decodeMutex;
	QWaitCondition m_encodeWaitCondition; // ����ȴ������Լ����뻥����
	QMutex m_encodeMutex;
	QWaitCondition m_playWaitCondition; // ���ŵȴ������Լ����Ż�����
	QMutex m_playMutex;
	QWaitCondition& m_pushWaitCondition; // �����ȴ������Լ�����������
	QMutex& m_pushMutex;
	CLXCodecThread::OpenMode m_eMode{ CLXCodecThread::OpenMode::OpenMode_Play }; // ��Ƶ����ʽ
};

class CLXVideoPlayThread final : public QThread {
	Q_OBJECT

public:
	/* @Parameter
	decodeFrameQueue �ȴ�������Ƶ֡����
	waitCondition �߳�ͬ������
	mutex ������
	pCodecCtx ��Ӧ����Ƶ������
	timeBase ��Ƶ֡ʱ�����
	bSendCountDown �Ƿ��͵���ʱ
	videoThread ��Ƶ�߳�
	szPlay ���Ŵ��ڴ�С
	eDecodeMode ��Ƶ����ģʽ
	*/
	explicit CLXVideoPlayThread(CircularQueue<AVFrame*>& decodeFrameQueue, QWaitCondition& waitCondition, QMutex& mutex,
	                            AVCodecContext* pCodecCtx, const AVRational& timeBase, bool bSendCountDown, CLXVideoThread* videoThread, QSize szPlay,
	                            CLXCodecThread::eLXDecodeMode eDecodeMode, QObject* parent = nullptr);
	~CLXVideoPlayThread() override;

public:
	void pause();
	void resume();
	void stop();

public:
	void setCurrentPts(int64_t nPts);

protected:
	void run() override;

private:
	AVCodecContext* m_pCodecCtx{ nullptr };    // ��Ƶ������
	bool m_bRunning{ true };                   // �߳��Ƿ�����
	bool m_bPause{ false };                    // �Ƿ���ͣ
	int64_t m_nLastTime{ -1 };                 // ��һ�β���ʱ��
	int64_t m_nLastPts{ -1 };                  // ��һ�β���ʱ���
	bool m_bSendCountDown{ false };            // �Ƿ��͵���ʱ֪ͨ
	const AVRational& m_timeBase;              // ��Ƶ֡ʱ�����
	CLXVideoThread* m_videoThread{ nullptr };  // ��Ƶ�߳�
	CircularQueue<AVFrame*>& m_playFrameQueue; // ��Ƶ����֡����
	QWaitCondition& m_playWaitCondition;       // ��Ƶ���ŵȴ������Լ�������
	QMutex& m_playMutex;
	QSize m_szPlay;                                     // ���Ŵ��ڳߴ�
	const CLXCodecThread::eLXDecodeMode& m_eDecodeMode; // ��Ƶ����ģʽ
};

class CLXAudioPlayThread final : public QThread {
	Q_OBJECT

public:
	/* @Parameter
	decodeFrameQueue �ȴ�������Ƶ֡����
	waitCondition �߳�ͬ������
	mutex ������
	pCodecCtx ��Ӧ����Ƶ������
	timeBase ��Ƶ֡ʱ�����
	audioThread ��Ƶ�߳�
	*/
	explicit CLXAudioPlayThread(CircularQueue<AVFrame*>& decodeFrameQueue, QWaitCondition& waitCondition, QMutex& mutex,
	                            AVCodecContext* pCodecCtx, const AVRational& timeBase, CLXAudioThread* audioThread, QObject* parent = nullptr);
	~CLXAudioPlayThread() override;

public:
	void pause();
	void resume();
	void stop();

public:
	void setCurrentPts(int64_t nPts);

protected:
	void run() override;

private:
	AVCodecContext* m_pCodecCtx{ nullptr };    // ��Ƶ������
	bool m_bRunning{ true };                   // �Ƿ�����
	bool m_bPause{ false };                    // �Ƿ���ͣ
	int64_t m_nLastTime{ -1 };                 // ��һ�β���ʱ��
	int64_t m_nLastPts{ -1 };                  // ��һ�β���ʱ���
	const AVRational& m_timeBase;              // ��Ƶʱ�����
	CLXAudioThread* m_audioThread{ nullptr };  // ��Ƶ�߳�
	CircularQueue<AVFrame*>& m_playFrameQueue; // ��������Ƶ֡����
	QWaitCondition& m_playWaitCondition;       // ��Ƶ���ŵȴ������Լ�������
	QMutex& m_playMutex;
};

class CLXEncodeVideoThread final : public QThread {
	Q_OBJECT

public:
	/* @Parameter
	encodeFrameQueue �ȴ��������Ƶ֡����
	pushPacketQueue �ȴ����͵���Ƶ������
	encodeWaitCondition �����߳�ͬ������
	encodeMutex ���뻥����
	pushWaitCondition �����߳�ͬ������
	pushMutex ���ͻ�����
	nEncodeStreamIndex ��Ƶ������
	pairEncodeCtx �����������ĺ������Ϣ�����
	eDecodeMode ����ģʽ
	*/
	explicit CLXEncodeVideoThread(CircularQueue<AVFrame*>& encodeFrameQueue, CircularQueue<AVPacket*>& pushPacketQueue,
	                              QWaitCondition& encodeWaitCondition,
	                              QMutex& encodeMutex, QWaitCondition& pushWaitCondition, QMutex& pushMutex, int nEncodeStreamIndex,
	                              QPair<AVCodecContext*, std::tuple<CCalcPtsDur, AVCodecContext*>> pairEncodeCtx,
	                              CLXCodecThread::eLXDecodeMode eDecodeMode, QObject* parent = nullptr);
	~CLXEncodeVideoThread() override;

public:
	void pause();
	void resume();
	void stop();

protected:
	void run() override;

private:
	CircularQueue<AVFrame*>& m_encodeFrameQueue;                                      // �ȴ��������Ƶ֡����
	CircularQueue<AVPacket*>& m_pushPacketQueue;                                      // �ȴ���������Ƶ������
	int m_nEncodeStreamIndex{ -1 };                                                   // ��Ƶ������
	QWaitCondition& m_encodeWaitCondition;                                            // �����߳�ͬ������
	QMutex& m_encodeMutex;                                                            // ���뻥����
	QWaitCondition& m_pushWaitCondition;                                              // �����̻߳�������
	QMutex& m_pushMutex;                                                              // ����������
	QPair<AVCodecContext*, std::tuple<CCalcPtsDur, AVCodecContext*>> m_pairEncodeCtx; // �������Լ���ص���Ϣ
	bool m_bRunning{ true };                                                          // �Ƿ�����
	bool m_bPause{ false };                                                           // �Ƿ���ͣ
	const CLXCodecThread::eLXDecodeMode& m_eEncodeMode;                               // ����ģʽ
};

class CLXEncodeAudioThread final : public QThread {
	Q_OBJECT

public:
	/* @Parameter
	encodeFrameQueue �ȴ��������Ƶ֡����
	pushPacketQueue �ȴ����͵���Ƶ������
	encodeWaitCondition �����߳�ͬ������
	encodeMutex ���뻥����
	pushWaitCondition �����߳�ͬ������
	pushMutex ���ͻ�����
	nEncodeStreamIndex ��Ƶ������
	pairEncodeCtx �����������ĺ������Ϣ�����
	*/
	explicit CLXEncodeAudioThread(CircularQueue<AVFrame*>& encodeFrameQueue, CircularQueue<AVPacket*>& pushPacketQueue,
	                              QWaitCondition& encodeWaitCondition,
	                              QMutex& encodeMutex, QWaitCondition& pushWaitCondition, QMutex& pushMutex, int nEncodeStreamIndex,
	                              QPair<AVCodecContext*, std::tuple<CCalcPtsDur, AVCodecContext*>> pairEncodeCtx, QObject* parent = nullptr);
	~CLXEncodeAudioThread() override;

public:
	void pause();
	void resume();
	void stop();

protected:
	void run() override;

private:
	CircularQueue<AVFrame*>& m_encodeFrameQueue;                                      // �ȴ��������Ƶ֡����
	CircularQueue<AVPacket*>& m_pushPacketQueue;                                      // �ȴ����͵���Ƶ������
	int m_nEncodeStreamIndex{ -1 };                                                   // ��Ƶ������
	QWaitCondition& m_encodeWaitCondition;                                            // �����߳�ͬ������
	QMutex& m_encodeMutex;                                                            // �����̻߳�����
	QWaitCondition& m_pushWaitCondition;                                              // �����߳�ͬ������
	QMutex& m_pushMutex;                                                              // �����̻߳�����
	QPair<AVCodecContext*, std::tuple<CCalcPtsDur, AVCodecContext*>> m_pairEncodeCtx; // ���������������Ϣ
	bool m_bRunning{ true };                                                          // �Ƿ�����
	bool m_bPause{ false };                                                           // �Ƿ���ͣ
};

class CLXEncodeMuteAudioThread final : public QThread {
	Q_OBJECT

public:
	/* @Parameter
	pushPacketQueue �ȴ����͵���Ƶ����������
	encodeWaitCondition �����߳�ͬ����������
	encodeMutex ���뻥��������
	syncWaitCondition �����߳�ͬ����������
	syncMutex ���ͻ���������
	nEncodeStreamIndex ��Ƶ������
	pairEncodeCtx �����������ĺ������Ϣ�����
	*/
	explicit CLXEncodeMuteAudioThread(CircularQueue<AVPacket*>& pushPacketQueue, QWaitCondition& encodeWaitCondition,
	                                  QMutex& encodeMutex, QWaitCondition& syncWaitCondition, QMutex& syncMutex, int nEncodeStreamIndex,
	                                  std::tuple<CCalcPtsDur, AVCodecContext*> pairEncodeCtx, QObject* parent = nullptr);
	~CLXEncodeMuteAudioThread() override;

public:
	void pause();
	void resume();
	void stop();

protected:
	void run() override;

private:
	CircularQueue<AVPacket*>& m_encodePacketQueue;            // �ȴ����͵���Ƶ����������
	int m_nEncodeStreamIndex{ -1 };                           // ��Ƶ������
	QWaitCondition& m_encodeWaitCondition;                    // �����߳�ͬ����������
	QMutex& m_encodeMutex;                                    // ���뻥��������
	QWaitCondition& m_syncWaitCondition;                      // �����߳�ͬ����������
	QMutex& m_syncMutex;                                      // ���ͻ���������
	std::tuple<CCalcPtsDur, AVCodecContext*> m_pairEncodeCtx; // �����������ĺ������Ϣ
	bool m_bRunning{ true };                                  // �Ƿ�����
	bool m_bPause{ false };                                   // �Ƿ���ͣ
};

class CLXPushThread final : public QThread {
	Q_OBJECT

public:
	explicit CLXPushThread(AVFormatContext* pOutputFormatCtx, CircularQueue<AVPacket*>& videoPacketQueue,
	                       CircularQueue<AVPacket*>& audioPacketQueue, QWaitCondition& videoWaitCondition, QMutex& videoMutex,
	                       QWaitCondition& audioWaitCondition, QMutex& audioMutex, bool bPushVideo, bool bPushAudio, QObject* parent = nullptr);
	~CLXPushThread() override;

public:
	// ��ͣ����
	void pause();
	// �ָ�����
	void resume();
	// ֹͣ����
	void stop();

protected:
	void run() override;

private:
	CircularQueue<AVPacket*>& m_videoPacketQueue;   // ��Ƶ���ݶ���
	CircularQueue<AVPacket*>& m_audioPacketQueue;   // ��Ƶ���ݶ���
	bool m_bRunning{ true };                        // �߳��Ƿ���ִ��
	bool m_bPause{ false };                         // �߳��Ƿ���ͣ
	bool m_bPushVideo{ true };                      // �Ƿ�������Ƶ
	bool m_bPushAudio{ false };                     // �Ƿ�������Ƶ
	QWaitCondition& m_videoWaitCondition;           // ��Ƶ�ȴ�����
	QMutex& m_videoMutex;                           // ��Ƶ������
	QWaitCondition& m_audioWaitCondition;           // ��Ƶ�ȴ�����
	QMutex& m_audioMutex;                           // ��Ƶ������
	AVFormatContext* m_pOutputFormatCtx{ nullptr }; // �����ʽ������
};

class CLXRecvThread final : public QThread, public QRunnable {
	Q_OBJECT

public:
	explicit CLXRecvThread(QString strPath, QObject* parent = nullptr);
	~CLXRecvThread() override;

public:
	void stop();
	[[nodiscard]] QString path() const;
	[[nodiscard]] QPair<AVCodecParameters*, AVCodecParameters*> getContext() const;

protected:
	void run() override;

private:
	AVFormatContext* m_pFormatCtx{ nullptr };
	QString m_strPath{ "" };
	bool m_bRunning{ false };
	int64_t m_nVideoPts{ -1 };
	int64_t m_nAudioPts{ -1 };
	AVCodecParameters* m_pVideoCodecPara{ nullptr };
	AVCodecParameters* m_pAudioCodecPara{ nullptr };
};
