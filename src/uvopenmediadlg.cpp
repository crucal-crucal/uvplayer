#include "uvopenmediadlg.hpp"

#include <QLabel>
#include <QVBoxLayout>

#include "framelessMessageBox/uvfiledialog.hpp"
#include "framelessMessageBox/uvmessagebox.hpp"
/**
 * class CUVFileTab
 * @param parent
 */
CUVFileTab::CUVFileTab(QWidget* parent) : QWidget(parent) {
	auto vbox = new QVBoxLayout;
	vbox->addStretch();
	vbox->addWidget(new QLabel(tr("File:")));

	auto hbox = new QHBoxLayout;
	m_pLeFileName = new QLineEdit(this);
	hbox->addWidget(m_pLeFileName);

	m_pBtnBrowse = new QPushButton("...");
	connect(m_pBtnBrowse, &QPushButton::clicked, this, [=]() {
		QString file = CUVFileDialog::getOpenFileName(
			this, tr("Open Media File"), "",
			"Video Files(*.3gp *.amv *.asf *.avi *.flv *.m2v *.m4v *.mkv *.mp2 *.mp4 *.mpg *.swf *.ts *.rmvb *.wmv)\n"
			"All Files(*)"
		);
		if (!file.isEmpty()) {
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
	auto vbox = new QVBoxLayout;

	vbox->addStretch();
	vbox->addWidget(new QLabel(tr("Url:")));

	m_pLeURL = new QLineEdit(this);

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
	auto vbox = new QVBoxLayout;

	vbox->addStretch();
	vbox->addWidget(new QLabel(tr("Device:")));

	m_pCbCaptureDevice = new QComboBox(this);

	// std::vector<HDevice> devs = getVideoDevices();
	// for (int i = 0; i < devs.size(); ++i){
	// 	m_pCbCaptureDevice->addItem(devs[i].name);
	// }

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
	switch (m_pTab->currentIndex()) {
		case MEDIA_TYPE_FILE: {
			auto filetab = qobject_cast<CUVFileTab*>(m_pTab->currentWidget());
			if (filetab) {
				m_media.type = MEDIA_TYPE_FILE;
				m_media.src = filetab->edit()->text().toUtf8().data();
				// g_confile->SetValue("last_file_source", m_media.src.c_str(), "m_media");
				// g_confile->Save();
			}
		}
		break;
		case MEDIA_TYPE_NETWORK: {
			auto nettab = qobject_cast<CUVNetWorkTab*>(m_pTab->currentWidget());
			if (nettab) {
				m_media.type = MEDIA_TYPE_NETWORK;
				m_media.src = nettab->edit()->text().toUtf8().data();
				// g_confile->SetValue("last_network_source", m_media.src.c_str(), "m_media");
				// g_confile->Save();
			}
		}
		break;
		case MEDIA_TYPE_CAPTURE: {
			auto captab = qobject_cast<CUVCaptureTab*>(m_pTab->currentWidget());
			if (captab) {
				m_media.type = MEDIA_TYPE_CAPTURE;
				m_media.src = qPrintable(captab->cmb()->currentText());
				m_media.index = captab->cmb()->currentIndex();
			}
		}
		break;
		default:
			break;
	}

	if (m_media.type == MEDIA_TYPE_NONE ||
	    (m_media.src.empty() && m_media.index < 0)) {
		UVMessageBox::CUVMessageBox::information(this, tr("Info"), tr("Invalid m_media source!"));
		return;
	}

	// g_confile->Set<int>("last_tab", tab->currentIndex(), "m_media");
	// g_confile->Save();

	QDialog::accept();
}

void CUVOpenMediaDlg::init() {
	setWindowTitle(tr("Open Media"));
	setFixedSize(600, 300);

	auto btns = new QDialogButtonBox(QDialogButtonBox::Open | QDialogButtonBox::Cancel);
	connect(btns, &QDialogButtonBox::accepted, this, &CUVOpenMediaDlg::accept);
	connect(btns, &QDialogButtonBox::rejected, this, &CUVOpenMediaDlg::reject);

	m_pTab = new QTabWidget(this);
	m_pTab->addTab(new CUVFileTab(this), tr("File"));
	m_pTab->addTab(new CUVNetWorkTab(this), tr("Network"));
	m_pTab->addTab(new CUVCaptureTab(this), tr("Capture"));

	m_pTab->setCurrentIndex(DEFAULT_MEDIA_TYPE);

	auto vbox = new QVBoxLayout;
	vbox->setContentsMargins(1, 1, 1, 1);
	vbox->setSpacing(1);
	vbox->addWidget(m_pTab);
	vbox->addWidget(btns);

	setLayout(vbox);
}
