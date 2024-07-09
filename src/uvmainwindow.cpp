#include "uvmainwindow.hpp"

#include <QKeyEvent>
#include <QMenuBar>
#include <QSignalMapper>
#include <QStatusBar>
#include <uvmessagebox.hpp>

#include "uvcenterwidget.hpp"
#include "uvdef.hpp"
#include "uvmainwindow_p.hpp"
#include "uvopenmediadlg.hpp"

extern "C" {
#include <libavutil/avutil.h>
}

/*!
 *  \CUVMainWindowPrivate
 *  \internal
 */
CUVMainWindowPrivate::CUVMainWindowPrivate(CUVMainWindow* q): q_ptr(q) {
}

CUVMainWindowPrivate::~CUVMainWindowPrivate() = default;

void CUVMainWindowPrivate::init() {
	Q_Q(CUVMainWindow);

	q->setBaseSize(MAIN_WINDOW_WIDTH, MAIN_WINDOW_HEIGHT);

	// q->setCentralWidget(q);

	centerWidget(q);

	m_pCenterWidget = new CUVCenterWidget(q);
	q->setCentralWidget(m_pCenterWidget);

	initMenu();

	q->statusBar()->showMessage(tr("No Message!"));
}

void CUVMainWindowPrivate::initConnect() {
}

void CUVMainWindowPrivate::initMenu() {
	Q_Q(CUVMainWindow);

	// Media
	QMenu* mediaMenu = q->menuBar()->addMenu(tr("&Media"));
	QToolBar* mediaToolbar = q->addToolBar(tr("&Media"));
	m_toolBars.push_back(mediaToolbar);

	auto actOpenFile = new QAction(QIcon(":/image/file.png"), tr(" Open File"));
	actOpenFile->setShortcut(QKeySequence("Ctrl+F"));
	connect(actOpenFile, &QAction::triggered, this, [=]() {
		q->openMediaDlg(MEDIA_TYPE_FILE);
	});
	mediaMenu->addAction(actOpenFile);
	mediaToolbar->addAction(actOpenFile);

	auto actOpenNetwork = new QAction(QIcon(":/image/network.png"), tr(" Open Network"));
	actOpenNetwork->setShortcut(QKeySequence("Ctrl+N"));
	connect(actOpenNetwork, &QAction::triggered, this, [=]() {
		q->openMediaDlg(MEDIA_TYPE_NETWORK);
	});
	mediaMenu->addAction(actOpenNetwork);
	mediaToolbar->addAction(actOpenNetwork);

	auto actOpenCapture = new QAction(QIcon(":/image/capture.png"), tr(" Open Capture"));
	actOpenCapture->setShortcut(QKeySequence("Ctrl+C"));
	connect(actOpenCapture, &QAction::triggered, this, [=]() {
		q->openMediaDlg(MEDIA_TYPE_CAPTURE);
	});
	mediaMenu->addAction(actOpenCapture);
	mediaToolbar->addAction(actOpenCapture);

	// View
	QMenu* viewMenu = q->menuBar()->addMenu(tr("&View"));

#if WITH_MV_STYLE
	QToolBar* viewToolbar = q->addToolBar(tr("&View"));
	m_toolBars.push_back(viewToolbar);

	QAction* actMVS;
	auto smMVS = new QSignalMapper(this);
#define VISUAL_MV_STYLE(id, row, col, label, image) \
    actMVS = new QAction(QIcon(image), tr(label), this);\
    actMVS->setToolTip(tr(label)); \
    smMVS->setMapping(actMVS, id); \
    connect( actMVS, SIGNAL(triggered(bool)), smMVS, SLOT(map()) ); \
    viewMenu->addAction(actMVS); \
    if (row * col <= 16){        \
        viewToolbar->addAction(actMVS); \
    }

	FOREACH_MV_STYLE(VISUAL_MV_STYLE)
#undef VISUAL_MV_STYLE

	connect(smMVS, SIGNAL(mapped(int)), q, SLOT(onMVStyleSelected(int)));
#endif

	m_pActMvFullScreen = new QAction(tr(" MV Fullscreen F12"));
	m_pActMvFullScreen->setCheckable(true);
	m_pActMvFullScreen->setChecked(false);
	connect(m_pActMvFullScreen, &QAction::triggered, q, &CUVMainWindow::mv_fullScreen);
	viewMenu->addAction(m_pActMvFullScreen);
	viewMenu->addSeparator();

	m_pActFullScreen = new QAction(tr(" Fullscreen F11"));
	m_pActFullScreen->setCheckable(true);
	m_pActFullScreen->setChecked(false);
	connect(m_pActFullScreen, &QAction::triggered, q, &CUVMainWindow::fullScreen);
	viewMenu->addAction(m_pActFullScreen);

	m_pActMenuBar = new QAction(tr(" Menubar F10"));
	m_pActMenuBar->setCheckable(true);
	// bool menubar_visible = g_confile->Get<bool>("menubar_visible", "ui", true);
	bool menubar_visible = true;
	m_pActMenuBar->setChecked(menubar_visible);
	q->menuBar()->setVisible(menubar_visible);
	connect(m_pActMenuBar, &QAction::triggered, [=](bool check) {
		q->menuBar()->setVisible(check);
		// g_confile->Set<bool>("menubar_visible", check, "ui");
	});
	viewMenu->addAction(m_pActMenuBar);

	auto actToolbar = new QAction(tr(" Toolbar"));
	actToolbar->setCheckable(true);
	// bool toolbar_visible = g_confile->Get<bool>("toolbar_visible", "ui", true);
	bool toolbar_visible = true;
	actToolbar->setChecked(toolbar_visible);
	foreach(auto toolbar, m_toolBars) {
		toolbar->setVisible(toolbar_visible);
	}
	connect(actToolbar, &QAction::triggered, [=](bool check) {
		foreach(auto toolbar, m_toolBars) {
			toolbar->setVisible(check);
		}
		// g_confile->Set<bool>("toolbar_visible", check, "ui");
	});
	viewMenu->addAction(actToolbar);

	auto actStatusbar = new QAction(tr(" Statusbar"));
	actStatusbar->setCheckable(true);
	// bool statusbar_visible = g_confile->Get<bool>("statusbar_visible", "ui", false);
	bool statusbar_visible = true;
	actStatusbar->setChecked(statusbar_visible);
	q->statusBar()->setVisible(statusbar_visible);
	connect(actStatusbar, &QAction::triggered, [=](bool check) {
		q->statusBar()->setVisible(check);
		// g_confile->Set<bool>("statusbar_visible", check, "ui");
	});
	viewMenu->addAction(actStatusbar);

	auto actLside = new QAction(tr(" Leftside"));
	actLside->setCheckable(true);
	// bool lside_visible = g_confile->Get<bool>("lside_visible", "ui", false);
	bool lside_visible = false;
	actLside->setChecked(lside_visible);
	m_pCenterWidget->lside->setVisible(lside_visible);
	connect(actLside, &QAction::triggered, [=](bool check) {
		m_pCenterWidget->lside->setVisible(check);
		// g_confile->Set<bool>("lside_visible", check, "ui");
	});
	viewMenu->addAction(actLside);

	auto actRside = new QAction(tr(" Rightside"));
	actRside->setCheckable(true);
	// bool rside_visible = g_confile->Get<bool>("rside_visible", "ui", false);
	bool rside_visible = false;

	actRside->setChecked(rside_visible);
	m_pCenterWidget->rside->setVisible(rside_visible);
	connect(actRside, &QAction::triggered, [=](bool check) {
		m_pCenterWidget->rside->setVisible(check);
		// g_confile->Set<bool>("rside_visible", check, "ui");
	});
	viewMenu->addAction(actRside);

	// Help
	QMenu* helpMenu = q->menuBar()->addMenu(tr("&Help"));
	helpMenu->addAction(tr(" &About"), q, &CUVMainWindow::about);
}

/*!
 *  \CUVMainWindow
 */
CUVMainWindow::CUVMainWindow(QWidget* parent) : QMainWindow(parent), d_ptr(new CUVMainWindowPrivate(this)) {
	d_func()->init();
}

CUVMainWindow::~CUVMainWindow() = default;

void CUVMainWindow::about() {
	QString strAbout = APP_NAME " " APP_VERSION "\n\n";

	strAbout += "Build on ";
	strAbout += QString::asprintf("%s %s\n\n", __DATE__, __TIME__);

	strAbout += "Qt version: ";
	strAbout += qVersion();
	strAbout += "\nFFmpeg version: ";
	strAbout += av_version_info();
	strAbout += "\n\n";

	strAbout += "Copyright 2018-2028 " COMPANY_NAME " Company.\n";
	strAbout += "All rights reserved.\n";

	UVMessageBox::CUVMessageBox::information(this, tr("About Application"), strAbout);
}

void CUVMainWindow::fullScreen() {
	Q_D(CUVMainWindow);

	static QRect rcOld;
	if (isFullScreen()) {
		menuBar()->setVisible(true);
		showNormal();
		setGeometry(rcOld);
	} else {
		rcOld = geometry();
		menuBar()->setVisible(false);
		showFullScreen();
	}
	d->m_pActFullScreen->setChecked(isFullScreen());
	d->m_pActMenuBar->setChecked(menuBar()->isVisible());
}

void CUVMainWindow::onMVStyleSelected(int id) {
	Q_D(CUVMainWindow);

	int r, c;
	switch (id) {
#define CASE_MV_STYLE(id, row, col, lable, image) \
	case id: \
		r = row; c = col; \
		break;

		FOREACH_MV_STYLE(CASE_MV_STYLE)
#undef  CASE_MV_STYLE
		default:
			r = 1;
			c = 1;
			break;
	}
	d->m_pCenterWidget->mv->setLayout(r, c);
}

void CUVMainWindow::mv_fullScreen() {
	Q_D(CUVMainWindow);

	CUVMultiView* mv = d->m_pCenterWidget->mv;
	bool is_mv_fullscreen = false;
	if (mv->windowType() & Qt::Window) {
		mv->setWindowFlags(Qt::SubWindow);
		this->show();
	} else {
		mv->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
		this->hide();
		mv->showFullScreen();
		mv->raise();
		is_mv_fullscreen = true;
	}
	this->grabKeyboard();
	d->m_pActMvFullScreen->setChecked(is_mv_fullscreen);
}

void CUVMainWindow::openMediaDlg(int index) {
	Q_D(CUVMainWindow);

	CUVOpenMediaDlg dlg(this);
	dlg.tab()->setCurrentIndex(index);
	if (dlg.exec() == QDialog::Accepted) {
		d->m_pCenterWidget->mv->play(dlg.media());
	}
}

void CUVMainWindow::keyPressEvent(QKeyEvent* event) {
	Q_D(CUVMainWindow);

	if (d->m_pCenterWidget->mv->windowType() & Qt::Window) {
		if (event->key() == Qt::Key_F12 || event->key() == Qt::Key_Escape) {
			mv_fullScreen();
		}
	} else {
		switch (event->key()) {
			case Qt::Key_F10:
				menuBar()->setVisible(!menuBar()->isVisible());
				d->m_pActMenuBar->setChecked(menuBar()->isVisible());
				return;
			case Qt::Key_F11:
			case Qt::Key_Escape:
				fullScreen();
				return;
			case Qt::Key_F12:
				mv_fullScreen();
				return;
			default:
				return QMainWindow::keyPressEvent(event);
		}
	}
}

void CUVMainWindow::changeEvent(QEvent* event) {
	QMainWindow::changeEvent(event);
	if (event->type() == QEvent::ActivationChange) {
		update();
	} else if (event->type() == QEvent::WindowStateChange) {
		if (isFullScreen()) {
			window_state = FULLSCREEN;
		} else if (isMaximized()) {
			window_state = MAXIMIZED;
		} else if (isMinimized()) {
			window_state = MINIMIZED;
		} else {
			window_state = NORMAL;
		}
		qInfo("window_state=%d", (int) window_state);
	}
}
