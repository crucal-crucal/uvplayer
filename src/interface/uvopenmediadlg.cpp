#include "uvopenmediadlg.hpp"

#include <QLabel>
#include <QVBoxLayout>

#include "conf/uvconf.hpp"
#include "framelessMessageBox/uvfiledialog.hpp"
#include "framelessMessageBox/uvmessagebox.hpp"
#include "global/uvdevice.hpp"

/**
 * class CUVFileTab
 * @param parent
 */
CUVFileTab::CUVFileTab(QWidget* parent) : QWidget(parent) {
	const auto vbox = new QVBoxLayout;
	vbox->addStretch();
	vbox->addWidget(new QLabel(tr("File:")));

	const auto hbox = new QHBoxLayout;
	m_pLeFileName = new QLineEdit(this);
	if (const std::string str = g_confile->getValue("last_file_source", "media"); !str.empty()) {
		m_pLeFileName->setText(QString::fromStdString(str));
	}

	hbox->addWidget(m_pLeFileName);

	m_pBtnBrowse = new QPushButton("...");
	connect(m_pBtnBrowse, &QPushButton::clicked, this, [=]() {
		if (const QString file = QFileDialog::getOpenFileName(
			this, tr("Open Media File"), "",
			"Video Files(*.3gp *.amv *.asf *.avi *.flv *.m2v *.m4v *.mkv *.mp2 *.mp4 *.mpg *.swf *.ts *.rmvb *.wmv *.dav)\n"
			"All Files(*)"
		); !file.isEmpty()) {
			m_pLeFileName->setText(file);
		}
	});
	hbox->addWidget(m_pBtnBrowse);

	vbox->addLayout(hbox);
	vbox->addStretch();

	setLayout(vbox);
}

CUVFileTab::~CUVFileTab() = default;

/**
 * class CUVNetWorkTab
 * @param parent
 */
CUVNetWorkTab::CUVNetWorkTab(QWidget* parent) : QWidget(parent) {
	const auto vbox = new QVBoxLayout;

	vbox->addStretch();
	vbox->addWidget(new QLabel(tr("Url:")));

	m_pLeURL = new QLineEdit(this);
	if (const std::string str = g_confile->getValue("last_network_source", "media"); !str.empty()) {
		m_pLeURL->setText(QString::fromStdString(str));
	}

	vbox->addWidget(m_pLeURL);
	vbox->addStretch();

	setLayout(vbox);
}

CUVNetWorkTab::~CUVNetWorkTab() = default;

/**
 * class CUVCaptureTab
 * @param parent
 */
CUVCaptureTab::CUVCaptureTab(QWidget* parent) : QWidget(parent) {
	const auto vbox = new QVBoxLayout;

	vbox->addStretch();
	vbox->addWidget(new QLabel(tr("Device:")));

	m_pCbCaptureDevice = new QComboBox(this);

	const auto devs = CUVDevice::GetCameraList().toVector();
	for (const auto& dev: devs) {
		m_pCbCaptureDevice->addItem(dev.description());
	}

	vbox->addWidget(m_pCbCaptureDevice);
	vbox->addStretch();

	setLayout(vbox);
}

CUVCaptureTab::~CUVCaptureTab() = default;

/**
 * class CUVOpenMediaDlg
 * @param parent
 */
CUVOpenMediaDlg::CUVOpenMediaDlg(QWidget* parent) : QDialog(parent) {
	init();
}

CUVOpenMediaDlg::~CUVOpenMediaDlg() = default;

void CUVOpenMediaDlg::accept() {
	switch (tab->currentIndex()) {
		case MEDIA_TYPE_FILE: {
			if (const auto filetab = qobject_cast<CUVFileTab*>(tab->currentWidget())) {
				media.type = MEDIA_TYPE_FILE;
				media.src = filetab->edit()->text().toUtf8().data();
				g_confile->setValue("last_file_source", media.src, "media");
				g_confile->save();
			}
			break;
		}
		case MEDIA_TYPE_NETWORK: {
			if (const auto nettab = qobject_cast<CUVNetWorkTab*>(tab->currentWidget())) {
				media.type = MEDIA_TYPE_NETWORK;
				media.src = nettab->edit()->text().toUtf8().data();
				g_confile->setValue("last_network_source", media.src, "media");
				g_confile->save();
			}
			break;
		}
		case MEDIA_TYPE_CAPTURE: {
			if (const auto captab = qobject_cast<CUVCaptureTab*>(tab->currentWidget())) {
				media.type = MEDIA_TYPE_CAPTURE;
				media.src = qPrintable(captab->cmb()->currentText());
				media.index = captab->cmb()->currentIndex();
			}
			break;
		}
		default: break;
	}

	if (media.type == MEDIA_TYPE_NONE || (media.src.empty() && media.index < 0)) {
		UVMessageBox::CUVMessageBox::information(this, tr("Info"), tr("Invalid media source!"));
		return;
	}

	g_confile->set<int>("last_tab", tab->currentIndex(), "media");
	g_confile->save();

	QDialog::accept();
}

void CUVOpenMediaDlg::init() {
	setWindowTitle(tr("Open Media"));
	setFixedSize(600, 300);

	const auto btns = new QDialogButtonBox(QDialogButtonBox::Open | QDialogButtonBox::Cancel);
	connect(btns, &QDialogButtonBox::accepted, this, &CUVOpenMediaDlg::accept);
	connect(btns, &QDialogButtonBox::rejected, this, &CUVOpenMediaDlg::reject);
	btns->button(QDialogButtonBox::Open)->setText(tr("Open"));
	btns->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));

	tab = new QTabWidget(this);
	tab->addTab(new CUVFileTab(this), QIcon(":/image/file.png"), tr("File"));
	tab->addTab(new CUVNetWorkTab(this), QIcon(":/image/network.png"), tr("Network"));
	tab->addTab(new CUVCaptureTab(this), QIcon(":/image/capture.png"), tr("Capture"));

	tab->setCurrentIndex(g_confile->get<int>("last_tab", "media", DEFAULT_MEDIA_TYPE));

	const auto vbox = new QVBoxLayout;
	vbox->setContentsMargins(1, 1, 1, 1);
	vbox->setSpacing(1);
	vbox->addWidget(tab);
	vbox->addWidget(btns);

	setLayout(vbox);
}
