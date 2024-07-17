#pragma once

#include "uvvideoplayer.hpp"
#include "uvvideotitlebar.hpp"
#include "uvvideotoolbar.hpp"
#include "uvvideowndfactory.hpp"
#include "def/avdef.hpp"
#include "global/uvmedia.hpp"

class CUVVideoWidget final : public QFrame {
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
	void open(const CUVMedia& media);
	void Close();

	void start();
	void stop();
	void pause();
	void resume();
	void restart();
	void retry();

	void onTimerUpdate() const;
	void onOpenSucceed();
	void onOpenFailed();
	void onPlayerEOF();
	void onPlayerError();

	void setAspectRatio(const aspect_ratio_t& aspect_ratio);

protected:
	void init();
	void initConnect();
	void updateUI() const;
	void initAspectRatio(const std::string& str);

	void resizeEvent(QResizeEvent* e) override;
	void enterEvent(QEvent* e) override;
	void leaveEvent(QEvent* e) override;
	void mousePressEvent(QMouseEvent* e) override;
	void mouseReleaseEvent(QMouseEvent* e) override;
	void mouseMoveEvent(QMouseEvent* e) override;
	void customEvent(QEvent* e) override;

public:
	int playerid{};
	int status{};
	QString title{};
	int fps{};
	aspect_ratio_t aspect_ratio{};
	renderer_type_e renderer_type{};

	CUVVideoWnd* videownd{ nullptr };
	CUVVideoTitlebar* titlebar{ nullptr };
	CUVVideoToolbar* toolbar{ nullptr };
	QPushButton* btnMedia{ nullptr };

private:
	QPoint ptMousePress{};
	QTimer* timer{ nullptr };

	CUVMedia media{};
	CUVVideoPlayer* pImpl_player{ nullptr };
	// for retry when SIGNAL_END_OF_FILE
	int retry_interval{};
	int retry_maxcnt{};
	int64_t last_retry_time{};
	int retry_cnt{};
};
