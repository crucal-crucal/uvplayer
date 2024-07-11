#include "uvmessagebox.hpp"

#include <QRegularExpression>

#include "uvmessagebox_p.hpp"

/*!
 *  \CUVFileBasePrivate
 *  \internal
 */
CUVMessageBoxPrivate::CUVMessageBoxPrivate(UVMessageBox::CUVMessageBox* q): q_ptr(q) {
	m_pMessageBox = new QMessageBox(q);
}

CUVMessageBoxPrivate::~CUVMessageBoxPrivate() = default;

QMessageBox::ButtonRole CUVMessageBoxPrivate::standardConvertToRole(const QMessageBox::StandardButton button) {
	switch (button) {
		case QMessageBox::Cancel:
		case QMessageBox::Abort:
		case QMessageBox::Close: return QMessageBox::RejectRole;
		case QMessageBox::No:
		case QMessageBox::NoToAll: return QMessageBox::NoRole;
		case QMessageBox::Ok:
		case QMessageBox::Yes:
		case QMessageBox::Open: return QMessageBox::AcceptRole;
		default: break;
	}
	return QMessageBox::NRoles;
}

namespace UVMessageBox {
/*!
 *  \CUVMessageBox
 */
CUVMessageBox::CUVMessageBox(const QMessageBox::Icon icon, const QString& title, const QString& text,
                             const QMessageBox::StandardButtons buttons, QWidget* parent, const Qt::WindowFlags flags)
: CUVBaseDialog(parent), d_ptr(new CUVMessageBoxPrivate(this)) {
	setTitle(title);
	setTitleBtnRole(CloseRole);
	d_func()->m_pMessageBox->setWindowFlags(flags);
	d_func()->m_pMessageBox->setStyleSheet("");
	d_func()->m_pMessageBox->setIcon(icon);
	d_func()->m_pMessageBox->setText(text);
	setContent(d_func()->m_pMessageBox);

	QFont curFont = font();
	curFont.setPixelSize(16);
	const QFontMetrics fontMetrics(curFont);
	int nMinWidth [[maybe_unused]] = fontMetrics.horizontalAdvance(title);
	connect(d_func()->m_pMessageBox, &QMessageBox::buttonClicked, this, &CUVMessageBox::onMsgBoxButtonClicked);
}

CUVMessageBox::~CUVMessageBox() = default;

void CUVMessageBox::setDefaultButton(QPushButton* button) const {
	Q_D(const CUVMessageBox);

	d->m_pMessageBox->setDefaultButton(button);
}

void CUVMessageBox::setDefaultButton(const QMessageBox::StandardButton button) const {
	Q_D(const CUVMessageBox);

	d->m_pMessageBox->setDefaultButton(button);
}

void CUVMessageBox::addButton(QAbstractButton* button, const QMessageBox::ButtonRole role) const {
	Q_D(const CUVMessageBox);

	d->m_pMessageBox->addButton(button, role);
}

QPushButton* CUVMessageBox::addButton(const QString& text, const QMessageBox::ButtonRole role) const {
	Q_D(const CUVMessageBox);

	return d->m_pMessageBox->addButton(text, role);
}

QPushButton* CUVMessageBox::addButton(const QMessageBox::StandardButton button) const {
	Q_D(const CUVMessageBox);

	return d->m_pMessageBox->addButton(button);
}

void CUVMessageBox::removeButton(QAbstractButton* button) const {
	Q_D(const CUVMessageBox);

	d->m_pMessageBox->removeButton(button);
}

QMessageBox::ButtonRole CUVMessageBox::information(QWidget* parent, const QString& title, const QString& text,
                                                   const QMessageBox::StandardButtons buttons,
                                                   [[maybe_unused]] const QMessageBox::StandardButton defaultButton) {
	CUVMessageBox messageBox(QMessageBox::Information, title, text, buttons, parent);
	return static_cast<QMessageBox::ButtonRole>(messageBox.exec());
}

QMessageBox::ButtonRole CUVMessageBox::question(QWidget* parent, const QString& title, const QString& text,
                                                const QMessageBox::StandardButtons buttons,
                                                [[maybe_unused]] const QMessageBox::StandardButton defaultButton) {
	CUVMessageBox messageBox(QMessageBox::Question, title, text, buttons, parent);
	return static_cast<QMessageBox::ButtonRole>(messageBox.exec());
}

QMessageBox::ButtonRole CUVMessageBox::warning(QWidget* parent, const QString& title, const QString& text,
                                               const QMessageBox::StandardButtons buttons,
                                               [[maybe_unused]] const QMessageBox::StandardButton defaultButton) {
	CUVMessageBox messageBox(QMessageBox::Warning, title, text, buttons, parent);
	return static_cast<QMessageBox::ButtonRole>(messageBox.exec());
}

QMessageBox::ButtonRole CUVMessageBox::critical(QWidget* parent, const QString& title, const QString& text,
                                                const QMessageBox::StandardButtons buttons,
                                                [[maybe_unused]] const QMessageBox::StandardButton defaultButton) {
	CUVMessageBox messageBox(QMessageBox::Critical, title, text, buttons, parent);
	return static_cast<QMessageBox::ButtonRole>(messageBox.exec());
}

void CUVMessageBox::about(QWidget* parent, const QString& title, const QString& text) {
	CUVMessageBox messageBox(QMessageBox::NoIcon, title, text, QMessageBox::NoButton, parent);
	messageBox.exec();
}

int CUVMessageBox::exec() {
	Q_D(CUVMessageBox);

	int nRet [[maybe_unused]] = CUVBaseDialog::exec();
	int button = d->m_pMessageBox->standardButton(d->m_pMessageBox->clickedButton());
	if (QMessageBox::NoButton == button) {
		button = d->m_pMessageBox->buttonRole(d->m_pMessageBox->clickedButton());
	} else {
		button = CUVMessageBoxPrivate::standardConvertToRole(static_cast<QMessageBox::StandardButton>(button));
	}
	return button;
}

void CUVMessageBox::apply() {
	Q_D(CUVMessageBox);

	d->m_pMessageBox->accept();
	close();
}

void CUVMessageBox::closeDialog() {
	Q_D(CUVMessageBox);

	d->m_pMessageBox->reject();
	close();
}

void CUVMessageBox::onMsgBoxButtonClicked([[maybe_unused]] const QAbstractButton* button) {
	close();
}

/*!
 *  \CUVCountdownMessageBox
 */
const QRegularExpression CUVCountdownMessageBox::regex("\\d+");

CUVCountdownMessageBox::CUVCountdownMessageBox(const QMessageBox::Icon icon, const QString& title, const QString& text, QWidget* parent)
: CUVMessageBox(icon, title, text, QMessageBox::Yes | QMessageBox::No, parent, Qt::Widget | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint) {
	setModal(true);
	setWindowModality(Qt::WindowModal);
	setWindowFlags(Qt::Widget | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
	QFont curFont = font();
	curFont.setPixelSize(16);
	const QFontMetrics fontMetrics(curFont);
	const int nMinWidth = fontMetrics.horizontalAdvance(title);
	resize(nMinWidth + 100, 100);
}

CUVCountdownMessageBox::~CUVCountdownMessageBox() = default;

int CUVCountdownMessageBox::exec(const int nSecond) {
	nCountdown = nSecond;
	d_ptr->m_pMessageBox->button(QMessageBox::Yes)->setText(
		QString("%1(%2)").arg(d_ptr->m_pMessageBox->button(QMessageBox::Yes)->text()).arg(nCountdown)
	);
	startTimer(1000);
	return CUVMessageBox::exec();
}

void CUVCountdownMessageBox::timerEvent(QTimerEvent* event) {
	nCountdown--;
	if (nCountdown <= 0) {
		killTimer(event->timerId());
		d_ptr->m_pMessageBox->button(QMessageBox::Yes)->clicked();
	}
	QString strText = d_ptr->m_pMessageBox->button(QMessageBox::Yes)->text();
	strText.replace(regex, QString::number(nCountdown));
	d_ptr->m_pMessageBox->button(QMessageBox::Yes)->setText(strText);
}
}
