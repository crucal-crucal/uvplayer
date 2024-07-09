#pragma once

#include "avdef.hpp"
#include "uvglwnd.hpp"
#include "uvmedia.hpp"
#include "uvvideoplayer.hpp"
#include "uvvideotitlebar.hpp"
#include "uvvideotoolbar.hpp"

class CUVVideoWidget : public QFrame {
	Q_OBJECT

public:
	enum Status {
		STOP,
		PAUSE,
		PLAY,
	};

	explicit CUVVideoWidget(QWidget* parent = nullptr);
	~CUVVideoWidget() override;

public slots:
	void open(CUVMedia& media);
	void close(); // NOLINT

	void start();
	void stop();
	void pause();
	void resume();
	void restart();
	void retry();

	void onTimerUpdate();
	void onOpenSucceed();
	void onOpenFailed();
	void onPlayerEOF();
	void onPlayerError();

	void setAspectRatio(aspect_ratio_t aspect_ratio);

protected:
	void init();
	void initConnect();
	void updateUI() const;

	void resizeEvent(QResizeEvent* e) override;
	void enterEvent(QEvent* e) override;
	void leaveEvent(QEvent* e) override;
	void mousePressEvent(QMouseEvent* e) override;
	void mouseReleaseEvent(QMouseEvent* e) override;
	void mouseMoveEvent(QMouseEvent* e) override;
	void customEvent(QEvent* e) override;

public:
	int playerid;
	int status;
	QString title;
	int fps;
	aspect_ratio_t aspect_ratio;

	CUVGLWnd* videownd;
	CUVVideoTitlebar* titlebar;
	CUVVideoToolbar* toolbar;
	QPushButton* btnMedia;

private:
	QPoint ptMousePress;
	QTimer* timer;

	CUVMedia media;
	CUVVideoPlayer* pImpl_player;
	// for retry when SIGNAL_END_OF_FILE
	int retry_interval;
	int retry_maxcnt;
	int64_t last_retry_time;
	int retry_cnt;
};
