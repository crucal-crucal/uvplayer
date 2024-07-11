#include "uvfiledialog.hpp"

#include <QHeaderView>
#include <QLineEdit>
#include <QListView>
#include <QSettings>
#include <QStandardItemModel>
#include <QTreeView>

#include "uvfiledialog_p.hpp"

/*!
 *  \CUVFileBasePrivate
 *  \internal
 */
CUVFileBasePrivate::CUVFileBasePrivate(CUVFileBase* q): q_ptr(q) {
}

CUVFileBasePrivate::~CUVFileBasePrivate() = default;

void CUVFileBasePrivate::init(const QString& strRegisterName, const QString& caption, const QString& directory, const QString& filter) {
	Q_Q(CUVFileBase);

	m_pFileDialog = new QFileDialog(q, caption, directory, filter);
	m_strRegisterName = strRegisterName;

	q->setTitleBtnRole(CUVBaseDialog::CloseRole);
	m_pFileDialog->setWindowFlags(Qt::FramelessWindowHint | Qt::Widget);
	m_pFileDialog->setOption(QFileDialog::DontUseNativeDialog);
	m_pFileDialog->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
	m_pFileDialog->setLabelText(QFileDialog::Accept, tr("IDS_SELECT"));
	m_pFileDialog->setLabelText(QFileDialog::Reject, tr("IDS_CANCEL"));
	m_pFileDialog->setLabelText(QFileDialog::LookIn, tr("IDS_DIRECTORY"));
	m_pFileDialog->setLabelText(QFileDialog::FileName, tr("IDS_NAME"));
	m_pFileDialog->setLabelText(QFileDialog::FileType, tr("IDS_TYPE"));
	m_pFileDialog->setViewMode(QFileDialog::List);
	// 去掉右下角的拖动
	m_pFileDialog->setSizeGripEnabled(false);
	// 去掉右下角的拖动
	if (const auto pListView = m_pFileDialog->findChild<QListView*>("listView")) {
		pListView->setContextMenuPolicy(Qt::NoContextMenu);
	}
	if (const auto pTreeView = m_pFileDialog->findChild<QTreeView*>("treeView")) {
		pTreeView->setContextMenuPolicy(Qt::NoContextMenu);
	}
	if (const auto pHeaderView = m_pFileDialog->findChild<QHeaderView*>()) {
		const auto pHeaderModel = new QStandardItemModel(pHeaderView);
		pHeaderView->setModel(pHeaderModel);
		pHeaderView->setContextMenuPolicy(Qt::NoContextMenu);
		QStringList headers;
		headers << tr("Name") << tr("Size") << tr("Type") << tr("Date Modified");
		pHeaderModel->setHorizontalHeaderLabels(headers);
	}
	if (const auto pLineEdit = m_pFileDialog->findChild<QLineEdit*>("fileNameEdit")) {
		pLineEdit->setContextMenuPolicy(Qt::NoContextMenu);
	}
	if (const auto fileTypeCombo = m_pFileDialog->findChild<QComboBox*>("fileTypeCombo")) {
		fileTypeCombo->setView(new QListView());
	}
	if (const auto lookInCombo = m_pFileDialog->findChild<QComboBox*>("lookInCombo")) {
		lookInCombo->setView(new QListView());
	}

	connect(m_pFileDialog, &QFileDialog::accepted, q, &CUVFileBase::accept);
	connect(m_pFileDialog, &QFileDialog::rejected, q, &CUVFileBase::reject);
	q->setContent(m_pFileDialog);

	// 恢复之前的状态
	const QSettings fileDialogState("crucal", "uvplayer");
	const QByteArray byHistoryState = QByteArray::fromBase64(
		fileDialogState.value(QString("client/%1").arg(m_strRegisterName)).toByteArray()
	);
	m_pFileDialog->restoreState(byHistoryState);
}

/*!
 *  \CUVFileBase
 */
CUVFileBase::CUVFileBase(const QString& strRegisterName, QWidget* parent, const QString& caption,
                         const QString& directory, const QString& filter)
: CUVBaseDialog(parent), d_ptr(new CUVFileBasePrivate(this)) {
	setResizeable(true);
	setTitle(caption);
	d_func()->init(strRegisterName, caption, directory, filter);
	setObjectName("CUVFileBase");
}

CUVFileBase::~CUVFileBase() {
	Q_D(CUVFileBase);

	if (!d->m_strRegisterName.isEmpty()) {
		QSettings fileDialogState("crucal", "uvplayer");
		const QByteArray byState = d->m_pFileDialog->saveState().toBase64();
		fileDialogState.setValue(QString("client/%1").arg(d->m_strRegisterName), byState);
	}
}

// 设置文件对话框中的指定标签的文本内容
void CUVFileBase::setLabelText(const QFileDialog::DialogLabel label, const QString& strText) const {
	Q_D(const CUVFileBase);

	d->m_pFileDialog->setLabelText(label, strText);
}

// 设置文件对话框过滤器，限制显示的文件类型
void CUVFileBase::setFilter(const QDir::Filters filters) const {
	Q_D(const CUVFileBase);

	d->m_pFileDialog->setFilter(filters);
}

// 设置对话框的工作模式
void CUVFileBase::setFileMode(const QFileDialog::FileMode mode) const {
	Q_D(const CUVFileBase);

	d->m_pFileDialog->setFileMode(mode);
}

// 设置对话框的接受模式
void CUVFileBase::setAcceptMode(const QFileDialog::AcceptMode mode) const {
	Q_D(const CUVFileBase);

	d->m_pFileDialog->setAcceptMode(mode);
}

QFileDialog::AcceptMode CUVFileBase::acceptMode() const {
	Q_D(const CUVFileBase);

	return d->m_pFileDialog->acceptMode();
}

// 设置默认文件后缀
void CUVFileBase::setDefaultSuffix(const QString& suffix) const {
	Q_D(const CUVFileBase);

	d->m_pFileDialog->setDefaultSuffix(suffix);
}

QString CUVFileBase::defaultSuffix() const {
	Q_D(const CUVFileBase);

	return d->m_pFileDialog->defaultSuffix();
}

void CUVFileBase::setDirectory(const QString& directory) const {
	Q_D(const CUVFileBase);

	d->m_pFileDialog->setDirectory(directory);
}

inline void CUVFileBase::setDirectory(const QDir& directory) const {
	Q_D(const CUVFileBase);

	d->m_pFileDialog->setDirectory(directory);
}

QDir CUVFileBase::directory() const {
	Q_D(const CUVFileBase);

	return d->m_pFileDialog->directory();
}

void CUVFileBase::setDirectoryUrl(const QUrl& directory) const {
	Q_D(const CUVFileBase);

	d->m_pFileDialog->setDirectoryUrl(directory);
}

void CUVFileBase::setViewMode(const QFileDialog::ViewMode mode) const {
	Q_D(const CUVFileBase);

	d->m_pFileDialog->setViewMode(mode);
}

QUrl CUVFileBase::directoryUrl() const {
	Q_D(const CUVFileBase);

	return d->m_pFileDialog->directoryUrl();
}

// 指定选中默认文件
void CUVFileBase::selectFile(const QString& filename) const {
	Q_D(const CUVFileBase);

	d->m_pFileDialog->selectFile(filename);
}

QStringList CUVFileBase::selectedFiles() const {
	Q_D(const CUVFileBase);

	return d->m_pFileDialog->selectedFiles();
}

QString CUVFileBase::getOpenFileName(QString* selectedFilter, const QFileDialog::Options options) {
	Q_D(CUVFileBase);

	d->m_pFileDialog->setOptions(options | d->m_pFileDialog->options());
	d->m_pFileDialog->setFileMode(QFileDialog::ExistingFiles);
	if (selectedFilter) {
		d->m_pFileDialog->selectNameFilter(*selectedFilter);
	}
	if (this->exec()) {
		if (selectedFilter) {
			*selectedFilter = d->m_pFileDialog->selectedNameFilter();
		}
		return this->selectedFiles().first();
	}

	return {};
}

QUrl CUVFileBase::getOpenFileUrl(const QUrl& dir, QString* selectedFilter, const QFileDialog::Options options,
                                 const QStringList& supportedSchemes) {
	Q_D(CUVFileBase);

	this->setDirectoryUrl(dir);
	d->m_pFileDialog->setOptions(options | d->m_pFileDialog->options());
	d->m_pFileDialog->setSupportedSchemes(supportedSchemes);
	if (selectedFilter) {
		d->m_pFileDialog->selectNameFilter(*selectedFilter);
	}
	if (this->exec()) {
		if (selectedFilter) {
			*selectedFilter = d->m_pFileDialog->selectedNameFilter();
		}
		return d->m_pFileDialog->selectedUrls().first();
	}
	return {};
}

QString CUVFileBase::getSaveFileName(QString* selectedFilter, const QFileDialog::Options options) {
	Q_D(CUVFileBase);

	d->m_pFileDialog->setAcceptMode(QFileDialog::AcceptSave);
	d->m_pFileDialog->setOptions(options | d->m_pFileDialog->options());
	if (selectedFilter) {
		d->m_pFileDialog->selectNameFilter(*selectedFilter);
	}
	if (this->exec()) {
		if (selectedFilter) {
			*selectedFilter = d->m_pFileDialog->selectedNameFilter();
		}
		return this->selectedFiles().first();
	}

	return {};
}

QUrl CUVFileBase::getSaveFileUrl(const QUrl& dir, QString* selectedFilter, const QFileDialog::Options options,
                                 const QStringList& supportedSchemes) {
	Q_D(CUVFileBase);

	this->setDirectoryUrl(dir);
	d->m_pFileDialog->setAcceptMode(QFileDialog::AcceptSave);
	d->m_pFileDialog->setOptions(options | d->m_pFileDialog->options());
	d->m_pFileDialog->setSupportedSchemes(supportedSchemes);
	if (selectedFilter) {
		d->m_pFileDialog->selectNameFilter(*selectedFilter);
	}
	if (this->exec()) {
		if (selectedFilter) {
			*selectedFilter = d->m_pFileDialog->selectedNameFilter();
		}
		return this->selectedFiles().first();
	}

	return {};
}

QString CUVFileBase::getExistingDirectory(const QFileDialog::Options options) {
	Q_D(CUVFileBase);

	/*
	 * @note:
	 * 设置目录模式必须在前，因为默认设置 QFileDialog::ShowDirsOnly 为 Options时，默认同时显示文件和目录
	 * 仅显示目录仅在目录模式下有效
	 */
	this->setFileMode(QFileDialog::Directory);
	d->m_pFileDialog->setOptions(options | d->m_pFileDialog->options());
	if (this->exec()) {
		return this->selectedFiles().first();
	}
	return {};
}

QString CUVFileBase::getExistingDirectoryUrl(const QUrl& dir, const QFileDialog::Options options,
                                             const QStringList& supportedSchemes) {
	Q_D(CUVFileBase);

	this->setFileMode(QFileDialog::Directory);
	d->m_pFileDialog->setOptions(options | d->m_pFileDialog->options());
	d->m_pFileDialog->setSupportedSchemes(supportedSchemes);
	this->setDirectoryUrl(dir);
	if (this->exec()) {
		return this->selectedFiles().first();
	}
	return {};
}

QStringList CUVFileBase::getOpenFileNames(QString* selectedFilter, const QFileDialog::Options options) {
	Q_D(CUVFileBase);

	d->m_pFileDialog->setOptions(options | d->m_pFileDialog->options());
	if (selectedFilter) {
		d->m_pFileDialog->selectNameFilter(*selectedFilter);
	}
	if (this->exec()) {
		if (selectedFilter) {
			*selectedFilter = d->m_pFileDialog->selectedNameFilter();
		}
		return this->selectedFiles();
	}

	return {};
}

QList<QUrl> CUVFileBase::getOpenFileUrls(const QUrl& dir, QString* selectedFilter, const QFileDialog::Options options,
                                         const QStringList& supportedSchemes) {
	Q_D(CUVFileBase);

	this->setDirectoryUrl(dir);
	d->m_pFileDialog->setOptions(options | d->m_pFileDialog->options());
	d->m_pFileDialog->setSupportedSchemes(supportedSchemes);
	if (selectedFilter) {
		d->m_pFileDialog->selectNameFilter(*selectedFilter);
	}
	if (this->exec()) {
		if (selectedFilter) {
			*selectedFilter = d->m_pFileDialog->selectedNameFilter();
		}
		return d->m_pFileDialog->selectedUrls();
	}
	return {};
}

void CUVFileBase::getOpenFileContent(const QString& nameFilter,
                                     const std::function<void(const QString&, const QByteArray&)>& fileContentsReady) {
	Q_D(CUVFileBase);

	this->setFileMode(QFileDialog::ExistingFile);
	d->m_pFileDialog->setNameFilter(nameFilter);

	connect(d->m_pFileDialog, &QFileDialog::fileSelected, [=](const QString& filePath) {
		        if (QFile file(filePath); file.open(QIODevice::ReadOnly)) {
			        const QByteArray content = file.readAll();
			        file.close();
			        fileContentsReady(filePath, content);
		        } else {
			        fileContentsReady(filePath, QByteArray()); // Pass an empty QByteArray to indicate failure
		        }
	        }
	);
	this->exec();
}

void CUVFileBase::saveFileContent(const QByteArray& fileContent, const QString& fileNameHint) {
	Q_D(CUVFileBase);

	d->m_pFileDialog->setWindowTitle(tr("Save File"));
	this->setAcceptMode(QFileDialog::AcceptSave);
	d->m_pFileDialog->selectFile(fileNameHint);
	if (this->exec() == QDialog::Accepted) {
		const QString fileName = d->m_pFileDialog->selectedFiles().first();
		if (QFile file(fileName); file.open(QIODevice::WriteOnly)) {
			file.write(fileContent);
			file.close();
		}
	}
}

// CUVFileDialog
CUVFileDialog::CUVFileDialog(QWidget* parent) : CUVBaseDialog(parent) {
}

CUVFileDialog::~CUVFileDialog() = default;

QString CUVFileDialog::getOpenFileName(QWidget* parent, const QString& caption, const QString& dir,
                                       const QString& filter, QString* selectedFilter,
                                       const QFileDialog::Options options) {
	CUVFileBase file_base(tr("uvplayer"), parent, caption, dir, filter);
	return file_base.getOpenFileName(selectedFilter, options);
}

QUrl CUVFileDialog::getOpenFileUrl(QWidget* parent, const QString& caption, const QUrl& dir, const QString& filter,
                                   QString* selectedFilter, const QFileDialog::Options options,
                                   const QStringList& supportedSchemes) {
	CUVFileBase file_base(tr("uvplayer"), parent, caption, "", filter);
	return file_base.getOpenFileUrl(dir, selectedFilter, options, supportedSchemes);
}

QString CUVFileDialog::getSaveFileName(QWidget* parent, const QString& caption, const QString& dir, const QString& filter,
                                       QString* selectedFilter, const QFileDialog::Options options) {
	CUVFileBase file_base(tr("uvplayer"), parent, caption, dir, filter);
	return file_base.getSaveFileName(selectedFilter, options);
}

QUrl CUVFileDialog::getSaveFileUrl(QWidget* parent, const QString& caption, const QUrl& dir, const QString& filter,
                                   QString* selectedFilter, const QFileDialog::Options options,
                                   const QStringList& supportedSchemes) {
	CUVFileBase file_base(tr("uvplayer"), parent, caption, "", filter);
	return file_base.getSaveFileUrl(dir, selectedFilter, options, supportedSchemes);
}

QString CUVFileDialog::getExistingDirectory(QWidget* parent, const QString& caption, const QString& dir,
                                            const QFileDialog::Options options) {
	CUVFileBase file_base(tr("uvplayer"), parent, caption, dir);
	return file_base.getExistingDirectory(options);
}

QUrl CUVFileDialog::getExistingDirectoryUrl(QWidget* parent, const QString& caption, const QUrl& dir,
                                            const QFileDialog::Options options, const QStringList& supportedSchemes) {
	CUVFileBase file_base(tr("uvplayer"), parent, caption);
	return file_base.getExistingDirectoryUrl(dir, options, supportedSchemes);
}

QStringList CUVFileDialog::getOpenFileNames(QWidget* parent, const QString& caption, const QString& dir, const QString& filter,
                                            QString* selectedFilter, const QFileDialog::Options options) {
	CUVFileBase file_base(tr("uvplayer"), parent, caption, dir, filter);
	return file_base.getOpenFileNames(selectedFilter, options);
}

QList<QUrl> CUVFileDialog::getOpenFileUrls(QWidget* parent, const QString& caption, const QUrl& dir, const QString& filter,
                                           QString* selectedFilter, const QFileDialog::Options options,
                                           const QStringList& supportedSchemes) {
	CUVFileBase file_base(tr("uvplayer"), parent, caption, "", filter);
	return file_base.getOpenFileUrls(dir, selectedFilter, options, supportedSchemes);
}

void CUVFileDialog::getOpenFileContent(const QString& nameFilter,
                                       const std::function<void(const QString&, const QByteArray&)>&
                                       fileContentsReady) {
	CUVFileBase file_base(tr("uvplayer"));
	return file_base.getOpenFileContent(nameFilter, fileContentsReady);
}

void CUVFileDialog::saveFileContent(const QByteArray& fileContent, const QString& fileNameHint) {
	CUVFileBase file_base(tr("uvplayer"));
	return file_base.saveFileContent(fileContent, fileNameHint);
}
