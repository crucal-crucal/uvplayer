#include "uvvideotoolbar.hpp"

#include <QHBoxLayout>

CUVVideoToolbar::CUVVideoToolbar(QWidget* parent) : QFrame(parent) {
	init();
	initConnect();
}

CUVVideoToolbar::~CUVVideoToolbar() = default;

void CUVVideoToolbar::init() {
	setFixedHeight(50);

	QSize sz(48, 48);
	m_pBtnStart = new QPushButton(QPixmap(":/image/start.png"), tr("start"));
	m_pBtnPause = new QPushButton(QPixmap(":/image/pause.png"), tr("pause"));

	m_pBtnStart->setShortcut(Qt::Key_Space);
	m_pBtnPause->setShortcut(Qt::Key_Space);


	m_pBtnPrev = new QPushButton(QPixmap(":/image/prev.png"), tr("prev"));
	m_pBtnStop = new QPushButton(QPixmap(":/image/stop.png"), tr("stop"));
	m_pBtnStop->setAutoDefault(true);
	m_pBtnNext = new QPushButton(QPixmap(":/image/next.png"), tr("next"));

	m_pSldProgress = new QSlider;
	m_pSldProgress->setOrientation(Qt::Horizontal);
	m_pLbDuration = new QLabel("00:00:00");

	auto hbox = new QHBoxLayout;
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

	hbox->addSpacing(5);
	hbox->addWidget(m_pSldProgress);
	m_pSldProgress->hide();
	hbox->addWidget(m_pLbDuration);
	m_pLbDuration->hide();

	setLayout(hbox);
}

void CUVVideoToolbar::initConnect() {
	QObject::connect(m_pBtnStart, SIGNAL(clicked(bool)), m_pBtnStart, SLOT(hide()));
	QObject::connect(m_pBtnStart, SIGNAL(clicked(bool)), m_pBtnPause, SLOT(show()));

	QObject::connect(m_pBtnPause, SIGNAL(clicked(bool)), m_pBtnPause, SLOT(hide()));
	QObject::connect(m_pBtnPause, SIGNAL(clicked(bool)), m_pBtnStart, SLOT(show()));

	connect(m_pBtnStart, SIGNAL(clicked(bool)), this, SIGNAL(sigStart()));
	connect(m_pBtnPause, SIGNAL(clicked(bool)), this, SIGNAL(sigPause()));
	connect(m_pBtnStop, SIGNAL(clicked(bool)), this, SIGNAL(sigStop()));

	connect(m_pBtnStop, SIGNAL(clicked(bool)), m_pBtnStart, SLOT(show()));
	connect(m_pBtnStop, SIGNAL(clicked(bool)), m_pBtnPause, SLOT(hide()));
}
