#include "uvvideowidget.hpp"

#include <QTimer>
#include <QVBoxLayout>

#include "uvconf.hpp"
#include "uvcustomeventtype.hpp"
#include "uvffplayer.hpp"
#include "uvfunctions.hpp"
#include "uvopenmediadlg.hpp"
#include "framelessMessageBox/uvmessagebox.hpp"

#define DEFAULT_RETRY_INTERVAL  10000  // ms
#define DEFAULT_RETRY_MAXCNT    6

char* duration_fmt(const int sec, char* buf) {
	int m = sec / 60;
	const int s = sec % 60;
	const int h = m / 60;
	m = m % 60;
	sprintf(buf, "%02d:%02d:%02d", h, m, s); // NOLINT
	return buf;
}

static int64_t gettimeofday_ms() {
	auto now = std::chrono::system_clock::now();
	auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
	auto value = now_ms.time_since_epoch();
	return value.count();
}

static int uvplayer_event_callback(uvplayer_event_e e, void* userdata) {
	auto wdg = static_cast<CUVVideoWidget*>(userdata);
	int custom_event_type;
	switch (e) {
		case HPLAYER_OPENED:
			custom_event_type = CUVCustomEvent::OpenMediaSucceed;
			break;
		case HPLAYER_OPEN_FAILED:
			custom_event_type = CUVCustomEvent::OpenMediaFailed;
			break;
		case HPLAYER_EOF:
			custom_event_type = CUVCustomEvent::PlayerEOF;
			break;
		case HPLAYER_ERROR:
			custom_event_type = CUVCustomEvent::PlayerError;
			break;
		default:
			custom_event_type = CUVCustomEvent::User;
			break;
	}

	QApplication::postEvent(wdg, new QEvent(static_cast<QEvent::Type>(custom_event_type)));
	return 0;
}

CUVVideoWidget::CUVVideoWidget(QWidget* parent) : QFrame(parent) {
	playerid = 0;
	status = STOP;
	pImpl_player = nullptr;
	fps = g_confile->get<int>("fps", "video");

	std::string str = g_confile->getValue("aspect_ratio", "video");
	initAspectRatio(str);

	// retry
	retry_interval = g_confile->get<int>("retry_interval", "video", DEFAULT_RETRY_INTERVAL);
	retry_maxcnt = g_confile->get<int>("retry_maxcnt", "video", DEFAULT_RETRY_MAXCNT);
	last_retry_time = 0;
	retry_cnt = 0;
	init();
	initConnect();
}

CUVVideoWidget::~CUVVideoWidget() {
	close();
}

void CUVVideoWidget::open(const CUVMedia& media) {
	this->media = media;
	start();
}

void CUVVideoWidget::close() {
	stop();
	this->media.type = MEDIA_TYPE_NONE;
	title = "";
	updateUI();
}

void CUVVideoWidget::start() { // NOLINT
	if (media.type == MEDIA_TYPE_NONE) {
		UVMessageBox::CUVMessageBox::information(this, tr("Info"), tr("Please first set media source, then start."));
		updateUI();
		return;
	}

	if (!pImpl_player) {
		pImpl_player = new CUVFFPlayer;
		pImpl_player->set_media(media);
		pImpl_player->set_event_callback(uvplayer_event_callback, this);
		title = media.src.c_str();
		int ret = pImpl_player->start();
		if (ret != 0) {
			onOpenFailed();
		} else {
			onOpenSucceed();
		}
		updateUI();
	} else {
		if (status == PAUSE) {
			resume();
		}
	}
}

void CUVVideoWidget::stop() {
	timer->stop();

	if (pImpl_player) {
		pImpl_player->stop();
		delete pImpl_player;
		pImpl_player = nullptr;
	}

	videownd->last_frame.buf.cleanup();
	videownd->Update();
	status = STOP;

	last_retry_time = 0;
	retry_cnt = 0;

	updateUI();
}

void CUVVideoWidget::pause() {
	if (pImpl_player) {
		pImpl_player->pause();
	}
	timer->stop();
	status = PAUSE;

	updateUI();
}

void CUVVideoWidget::resume() {
	if (status == PAUSE && pImpl_player) {
		pImpl_player->resume();
		timer->start(1000 / (fps ? fps : pImpl_player->fps));
		status = PLAY;

		updateUI();
	}
}

void CUVVideoWidget::restart() { // NOLINT
	std::cout << "restart..." << std::endl;
	if (pImpl_player) {
		pImpl_player->stop();
		pImpl_player->start();
	} else {
		start();
	}
}

void CUVVideoWidget::retry() { // NOLINT
	if (retry_maxcnt < 0 || retry_cnt < retry_maxcnt) {
		++retry_cnt;
		int64_t cur_time = gettimeofday_ms();
		int64_t timespan = cur_time - last_retry_time;
		if (timespan >= retry_interval) {
			last_retry_time = cur_time;
			restart();
		} else {
			last_retry_time += retry_interval;
			if (pImpl_player) {
				pImpl_player->stop();
			}
			int retry_after = retry_interval - timespan; // NOLINT
			QTimer::singleShot(retry_after, this, &CUVVideoWidget::restart);
		}
	} else {
		stop();
	}
}

void CUVVideoWidget::onTimerUpdate() const {
	if (!pImpl_player) return;

	if (pImpl_player->pop_frame(&videownd->last_frame) == 0) {
		// update progress bar
		if (toolbar->sldProgress()->isVisible()) {
			int progress = (videownd->last_frame.ts - pImpl_player->start_time) / 1000; // NOLINT
			if (toolbar->sldProgress()->value() != progress && !toolbar->sldProgress()->isSliderDown()) {
				toolbar->sldProgress()->setValue(progress);
			}
		}
		// update video frame
		videownd->Update();
	}
}

void CUVVideoWidget::onOpenSucceed() {
	timer->start(1000 / (fps ? fps : pImpl_player->fps));
	status = PLAY;
	setAspectRatio(aspect_ratio);
	if (pImpl_player->duration > 0) {
		int duration_sec = pImpl_player->duration / 1000; // NOLINT
		char szTime[16];
		duration_fmt(duration_sec, szTime);
		toolbar->lblDuration()->setText(szTime);
		toolbar->sldProgress()->setRange(0, duration_sec);
		toolbar->lblDuration()->show();
		toolbar->sldProgress()->show();
	}

	if (retry_cnt != 0) {
		// hlogi("retry succeed: cnt=%d media.src=%s", retry_cnt, media.src.c_str());
	}
}

void CUVVideoWidget::onOpenFailed() { // NOLINT
	if (retry_cnt == 0) {
		UVMessageBox::CUVMessageBox::critical(this, tr("ERROR"), tr("Could not open media: \n") +
		                                                         media.src.c_str() +
		                                                         QString::asprintf("\nerrcode=%d", pImpl_player->error));
		stop();
	} else {
		// hlogw("retry failed: cnt=%d media.src=%s", retry_cnt, media.src.c_str());
		retry();
	}
}

void CUVVideoWidget::onPlayerEOF() {
	switch (media.type) {
		case MEDIA_TYPE_NETWORK:
			retry();
			break;
		case MEDIA_TYPE_FILE:
			if (g_confile->get<bool>("loop_playback", "video")) {
				restart();
			} else {
				stop();
			}
			break;
		default:
			stop();
			break;
	}
}

void CUVVideoWidget::onPlayerError() {
	switch (media.type) {
		case MEDIA_TYPE_NETWORK:
			retry();
			break;
		default:
			stop();
			break;
	}
}

void CUVVideoWidget::setAspectRatio(const aspect_ratio_t& aspect_ratio) {
	this->aspect_ratio = aspect_ratio;
	int border = 1;
	int scr_w = width() - border * 2;
	int scr_h = height() - border * 2;
	if (scr_w <= 0 || scr_h <= 0) return;
	int pic_w = 0;
	int pic_h = 0;
	if (pImpl_player && status != STOP) {
		pic_w = pImpl_player->width;
		pic_h = pImpl_player->height;
	}
	if (pic_w == 0) pic_w = scr_w;
	if (pic_h == 0) pic_h = scr_h;
	// calc videownd rect
	int dst_w, dst_h;
	switch (aspect_ratio.type) {
		case ASPECT_FULL:
			dst_w = scr_w;
			dst_h = scr_h;
			break;
		case ASPECT_PERCENT:
			dst_w = pic_w * aspect_ratio.w / 100;
			dst_h = pic_h * aspect_ratio.h / 100;
			break;
		case ASPECT_ORIGINAL_SIZE:
			dst_w = pic_w;
			dst_h = pic_h;
			break;
		case ASPECT_CUSTOM_SIZE:
			dst_w = aspect_ratio.w;
			dst_h = aspect_ratio.h;
			break;
		case ASPECT_ORIGINAL_RATIO:
		case ASPECT_CUSTOM_RATIO: {
			double scr_ratio = static_cast<double>(scr_w) / static_cast<double>(scr_h);
			double dst_ratio;
			if (aspect_ratio.type == ASPECT_CUSTOM_RATIO) {
				dst_ratio = static_cast<double>(aspect_ratio.w) / static_cast<double>(aspect_ratio.h);
			} else {
				dst_ratio = static_cast<double>(pic_w) / static_cast<double>(pic_h);
			}
			if (dst_ratio > scr_ratio) {
				dst_w = scr_w;
				dst_h = scr_w / dst_ratio; // NOLINT
			} else {
				dst_h = scr_h;
				dst_w = scr_h * dst_ratio; // NOLINT
			}
		}
		break;
	}
	dst_w = MIN(dst_w, scr_w);
	dst_h = MIN(dst_h, scr_h);
	// align 4
	dst_w = dst_w >> 2 << 2;
	dst_h = dst_h >> 2 << 2;

	int x = border + (scr_w - dst_w) / 2;
	int y = border + (scr_h - dst_h) / 2;
	videownd->setgeometry(QRect(x, y, dst_w, dst_h));
}

void CUVVideoWidget::init() {
	setFocusPolicy(Qt::ClickFocus);

	videownd = new CUVGLWnd(this);
	titlebar = new CUVVideoTitlebar(this);
	toolbar = new CUVVideoToolbar(this);
	btnMedia = getPushButton(QPixmap(":/image/media_bk.png"), tr("Open media"), {}, this);

	auto vbox = new QVBoxLayout;
	vbox->setContentsMargins(1, 1, 1, 1);
	vbox->setSpacing(1);

	vbox->addWidget(titlebar, 0, Qt::AlignTop);
	vbox->addWidget(btnMedia, 0, Qt::AlignCenter);
	vbox->addWidget(toolbar, 0, Qt::AlignBottom);

	setLayout(vbox);

	titlebar->hide();
	toolbar->hide();
}

void CUVVideoWidget::initConnect() {
	connect(btnMedia, &QPushButton::clicked, [this] {
		CUVOpenMediaDlg dlg(this);
		if (dlg.exec() == QDialog::Accepted) {
			open(dlg.media);
		}
	});
	connect(titlebar->btnClose, &QPushButton::clicked, this, &CUVVideoWidget::close);

	connect(toolbar, &CUVVideoToolbar::sigStart, this, &CUVVideoWidget::start);
	connect(toolbar, &CUVVideoToolbar::sigPause, this, &CUVVideoWidget::pause);
	connect(toolbar, &CUVVideoToolbar::sigStop, this, &CUVVideoWidget::stop);
	connect(toolbar->sldProgress(), &QSlider::sliderReleased, [this]() {
		if (pImpl_player) {
			pImpl_player->seek(toolbar->sldProgress()->value() * 1000);
		}
	});

	timer = new QTimer(this);
	timer->setTimerType(Qt::PreciseTimer);
	connect(timer, &QTimer::timeout, this, &CUVVideoWidget::onTimerUpdate);
}

void CUVVideoWidget::updateUI() const {
	titlebar->labTitle->setText(QString::asprintf("%02d ", playerid) + title);

	toolbar->btnStart()->setVisible(status != PLAY);
	toolbar->btnPause()->setVisible(status == PLAY);

	btnMedia->setVisible(status == STOP);

	if (status == STOP) {
		toolbar->sldProgress()->hide();
		toolbar->lblDuration()->hide();
	}
}

void CUVVideoWidget::initAspectRatio(const std::string& str) {
	aspect_ratio.type = ASPECT_FULL; // Default type
	const auto c_str = str.c_str();

	if (str.empty() || strcmp(c_str, "100%") == 0) { // NOLINT
		aspect_ratio.type = ASPECT_FULL;
	} else if (_stricmp(c_str, "w:h") == 0) {
		aspect_ratio.type = ASPECT_ORIGINAL_RATIO;
	} else if (_stricmp(c_str, "wxh") == 0 || _stricmp(c_str, "w*h") == 0) {
		aspect_ratio.type = ASPECT_ORIGINAL_SIZE;
	} else if (strchr(c_str, '%')) {
		int percent = 0;
		sscanf_s(c_str, "%d%%", &percent);
		if (percent) {
			aspect_ratio.type = ASPECT_PERCENT;
			aspect_ratio.w = percent;
			aspect_ratio.h = percent;
		}
	} else if (strchr(c_str, ':')) {
		int w = 0;
		int h = 0;
		sscanf_s(c_str, "%d:%d", &w, &h);
		if (w && h) {
			aspect_ratio.type = ASPECT_CUSTOM_RATIO;
			aspect_ratio.w = w;
			aspect_ratio.h = h;
		}
	} else if (strchr(c_str, 'x')) {
		int w = 0;
		int h = 0;
		sscanf_s(c_str, "%dx%d", &w, &h);
		if (w && h) {
			aspect_ratio.type = ASPECT_CUSTOM_SIZE;
			aspect_ratio.w = w;
			aspect_ratio.h = h;
		}
	} else if (strchr(c_str, 'X')) {
		int w = 0;
		int h = 0;
		sscanf_s(c_str, "%dX%d", &w, &h);
		if (w && h) {
			aspect_ratio.type = ASPECT_CUSTOM_SIZE;
			aspect_ratio.w = w;
			aspect_ratio.h = h;
		}
	} else if (strchr(c_str, '*')) {
		int w = 0;
		int h = 0;
		sscanf_s(c_str, "%d*%d", &w, &h);
		if (w && h) {
			aspect_ratio.type = ASPECT_CUSTOM_SIZE;
			aspect_ratio.w = w;
			aspect_ratio.h = h;
		}
	} else {
		aspect_ratio.type = ASPECT_FULL;
	}
}

void CUVVideoWidget::resizeEvent(QResizeEvent* e) {
	setAspectRatio(aspect_ratio);
}

void CUVVideoWidget::enterEvent(QEvent* e) {
	updateUI();

	titlebar->show();
	toolbar->show();
}

void CUVVideoWidget::leaveEvent(QEvent* e) {
	titlebar->hide();
	toolbar->hide();
}

void CUVVideoWidget::mousePressEvent(QMouseEvent* e) {
	ptMousePress = e->pos();
	e->ignore();
}

void CUVVideoWidget::mouseReleaseEvent(QMouseEvent* e) {
	e->ignore();
}

void CUVVideoWidget::mouseMoveEvent(QMouseEvent* e) {
	e->ignore();
}

void CUVVideoWidget::customEvent(QEvent* e) {
	switch (e->type()) {
		case CUVCustomEvent::OpenMediaSucceed:
			onOpenSucceed();
			break;
		case CUVCustomEvent::OpenMediaFailed:
			onOpenFailed();
			break;
		case CUVCustomEvent::PlayerEOF:
			onPlayerEOF();
			break;
		case CUVCustomEvent::PlayerError:
			onPlayerError();
			break;
		default:
			break;
	}
}
