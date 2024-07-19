#include "uvvideotoolbar.hpp"

#include <QHBoxLayout>

#include "def/uvdef.hpp"
#include "global/uvfunctions.hpp"

CUVVideoToolbar::CUVVideoToolbar(QWidget* parent) : QFrame(parent) {
	init();
	initConnect();
}

CUVVideoToolbar::~CUVVideoToolbar() {
	SAFE_DELETE(m_pBtnStart);
	SAFE_DELETE(m_pBtnPause);
	SAFE_DELETE(m_pBtnPrev);
	SAFE_DELETE(m_pBtnStop);
	SAFE_DELETE(m_pBtnNext);
	SAFE_DELETE(sldProgress);
	SAFE_DELETE(m_pLbDuration);
}

void CUVVideoToolbar::init() {
	setFixedHeight(50);

	m_pBtnStart = getPushButton(QPixmap(":/image/start.png"), tr("start"), {}, this);
	m_pBtnPause = getPushButton(QPixmap(":/image/pause.png"), tr("pause"), {}, this);

	m_pBtnStart->setShortcut(Qt::Key_Space);
	m_pBtnPause->setShortcut(Qt::Key_Space);

	m_pBtnPrev = getPushButton(QPixmap(":/image/prev.png"), tr("prev"), {}, this);
	m_pBtnStop = getPushButton(QPixmap(":/image/stop.png"), tr("stop"), {}, this);
	m_pBtnStop->setAutoDefault(true);
	m_pBtnNext = getPushButton(QPixmap(":/image/next.png"), tr("next"), {}, this);

	m_pLbCurDuration = new QLabel("00:00:00");
	m_pLbCurDuration->hide();
	sldProgress = new CUVMaterialSlider;
	sldProgress->setOrientation(Qt::Horizontal);
	sldProgress->setThumbColor(QColor("#f4843c"));
	sldProgress->custom_hide();
	sldProgress->setTrackHeight(10);
	m_pLbDuration = new QLabel("00:00:00");
	m_pLbDuration->hide();

	const auto hbox = new QHBoxLayout;
	hbox->setContentsMargins(10,1,10,1);
	hbox->setSpacing(5);
	hbox->addWidget(m_pBtnStart, 0, Qt::AlignLeft);
	hbox->addWidget(m_pBtnPause, 0, Qt::AlignLeft);
	m_pBtnPause->hide();

	hbox->addSpacing(5);
	hbox->addWidget(m_pBtnPrev, 0, Qt::AlignLeft);
	m_pBtnPrev->hide();
	hbox->addWidget(m_pBtnStop, 0, Qt::AlignLeft);
	m_pBtnStop->hide();
	hbox->addWidget(m_pBtnNext, 0, Qt::AlignLeft);
	m_pBtnNext->hide();

	hbox->addSpacing(10);
	hbox->addWidget(m_pLbCurDuration);
	hbox->addSpacing(10);
	hbox->addWidget(sldProgress);
	hbox->addSpacing(10);
	hbox->addWidget(m_pLbDuration);
	hbox->addSpacing(10);

	setLayout(hbox);
}

void CUVVideoToolbar::initConnect() {
	connect(m_pBtnStart, &QPushButton::clicked, m_pBtnStart, &QPushButton::hide);
	connect(m_pBtnStart, &QPushButton::clicked, m_pBtnPause, &QPushButton::show);

	connect(m_pBtnPause, &QPushButton::clicked, m_pBtnPause, &QPushButton::hide);
	connect(m_pBtnPause, &QPushButton::clicked, m_pBtnStart, &QPushButton::show);

	connect(m_pBtnStart, &QPushButton::clicked, this, &CUVVideoToolbar::sigStart);
	connect(m_pBtnPause, &QPushButton::clicked, this, &CUVVideoToolbar::sigPause);
	connect(m_pBtnStop, &QPushButton::clicked, this, &CUVVideoToolbar::sigStop);

	connect(m_pBtnStop, &QPushButton::clicked, m_pBtnStart, &QPushButton::show);
	connect(m_pBtnStop, &QPushButton::clicked, m_pBtnPause, &QPushButton::hide);
}
