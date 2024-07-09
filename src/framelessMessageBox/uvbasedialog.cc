#include "uvbasedialog.hpp"

#include <QStyleOption>

#ifdef Q_OS_WIN
#include <Windowsx.h>
#endif

#include "uvbasedialog_p.hpp"

/*!
 *  \CUVBaseDialogPrivate
 *  \internal
 */
CUVBaseDialogPrivate::CUVBaseDialogPrivate(CUVBaseDialog* q): q_ptr(q) {
}

CUVBaseDialogPrivate::~CUVBaseDialogPrivate() = default;

void CUVBaseDialogPrivate::switchSize() {
	Q_Q(CUVBaseDialog);

	if (q->isMaximized()) {
		q->showNormal();
	} else {
		q->showMaximized();
	}
}

void CUVBaseDialogPrivate::init() {
	Q_Q(CUVBaseDialog);

	m_pDialogBtnBox = new QDialogButtonBox(q);
	connect(m_pDialogBtnBox, &QDialogButtonBox::accepted, q, &CUVBaseDialog::accept);
	connect(m_pDialogBtnBox, &QDialogButtonBox::rejected, q, &CUVBaseDialog::reject);
	m_pDialogBtnBox->setContentsMargins(0, 6, 6, 6);
	m_pDialogBtnBox->hide();

	m_plyVTotal = new QVBoxLayout(q);
	m_plyHTitle = new QHBoxLayout;
	m_plyHContent = new QHBoxLayout;

	auto* spacerItem = new QSpacerItem(0, TITLE_HEIGHT, QSizePolicy::Expanding, QSizePolicy::Fixed);
	m_plyHTitle->addSpacerItem(spacerItem);
	m_plyHTitle->setContentsMargins(0, 0, 6, 0);
	m_plyHTitle->setSpacing(6);
	m_plyHContent->setContentsMargins(1, 0, 1, 1);

	m_plyVTotal->setContentsMargins(BORDER_SHADOW_WIDTH, BORDER_SHADOW_WIDTH, BORDER_SHADOW_WIDTH, BORDER_SHADOW_WIDTH);

	m_plyVTotal->addLayout(m_plyHTitle);
	m_plyVTotal->addLayout(m_plyHContent);
	m_plyVTotal->addWidget(m_pDialogBtnBox);
	m_plyVTotal->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	m_plyVTotal->setSpacing(0);
	q->setLayout(m_plyVTotal);
}

/*!
 *  \CUVBaseDialog
 */
constexpr int NATIVE_DETECT_WIDTH = 10;

CUVBaseDialog::CUVBaseDialog(QWidget* parent) : QDialog(parent), d_ptr(new CUVBaseDialogPrivate(this)) {
	setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinMaxButtonsHint | Qt::Dialog);
	setAttribute(Qt::WA_TranslucentBackground);
	d_func()->init();
}

CUVBaseDialog::~CUVBaseDialog() = default;

void CUVBaseDialog::setTitle(QString strTitle) {
	Q_D(CUVBaseDialog);

	d->m_strTitle = std::move(strTitle);
}

QRect CUVBaseDialog::getTilteRect() const {
	Q_D(const CUVBaseDialog);

	return d->m_rtTitle;
}

QList<QAbstractButton*> CUVBaseDialog::getTitleButton() const {
	Q_D(const CUVBaseDialog);

	return d->m_hashTitleButtons.values();
}

void CUVBaseDialog::setIcon(const QString& strPath, const bool bScale, const QSize scaleSize) {
	Q_D(CUVBaseDialog);

	d->m_strIconPath = strPath;
	d->m_bIconScaled = bScale;
	d->m_icon.load(strPath);
	if (!d->m_icon.isNull() && d->m_bIconScaled) {
		d->m_icon = d->m_icon.scaled(scaleSize, Qt::KeepAspectRatio);
	}
}

void CUVBaseDialog::setResizeable(const bool bResizeable) {
	Q_D(CUVBaseDialog);

	d->m_bResizeable = bResizeable;
}

void CUVBaseDialog::setEscEnable(const bool bEnable) {
	Q_D(CUVBaseDialog);

	d->m_bEscEnable = bEnable;
}

void CUVBaseDialog::setMoveEnable(const bool bEnable) {
	Q_D(CUVBaseDialog);

	d->m_bMoveEnable = bEnable;
}

void CUVBaseDialog::setTitleVisible(const bool bVisible) {
	Q_D(CUVBaseDialog);

	d->m_bTitleVisible = bVisible;
	for (const auto i : d->m_hashTitleButtons) {
		i->setVisible(d->m_bTitleVisible);
	}
}

void CUVBaseDialog::setEnterEnable(const bool bEnable) {
	Q_D(CUVBaseDialog);

	d->m_bEnterEnable = bEnable;
}

void CUVBaseDialog::setShadowVisible(const bool bVisible) {
	Q_D(CUVBaseDialog);

	d->m_bShadowVisible = bVisible;
	if (!d->m_bShadowVisible) {
		setAttribute(Qt::WA_TranslucentBackground, false);
		d->m_plyHContent->setContentsMargins(1, 0, 0, 1);
		d->m_plyVTotal->setContentsMargins(0, 0, 0, 0);
	}
}

void CUVBaseDialog::setTitleBtnRole(const TitleButtonRoles emTitleButtonRoles) {
	Q_D(CUVBaseDialog);

	if (emTitleButtonRoles == NoButtonRole) {
		qDeleteAll(d->m_hashTitleButtons.values());
		d->m_hashTitleButtons.clear();
		return;
	}
	if (emTitleButtonRoles & MinRole) {
		const QPushButton* pBtn = addTitleButton(MinRole);
		connect(pBtn, &QPushButton::clicked, this, &CUVBaseDialog::showMinimized);
	}
	if (emTitleButtonRoles & MaxRole) {
		QPushButton* pBtn = addTitleButton(MaxRole);
		pBtn->setCheckable(true);
		connect(pBtn, &QPushButton::clicked, d, &CUVBaseDialogPrivate::switchSize);
	}
	if (emTitleButtonRoles & CloseRole) {
		const QPushButton* pBtn = addTitleButton(CloseRole);
		connect(pBtn, &QPushButton::clicked, this, &CUVBaseDialog::closeDialog);
	}
}

void CUVBaseDialog::setDialogBtnRole(const QDialogButtonBox::StandardButtons emBtns) {
	Q_D(CUVBaseDialog);

	if (QDialogButtonBox::NoButton == emBtns) {
		d->m_pDialogBtnBox->hide();
	} else {
		d->m_pDialogBtnBox->setStandardButtons(emBtns);
		if (emBtns & QDialogButtonBox::Ok) {
			d->m_pDialogBtnBox->button(QDialogButtonBox::Ok)->setText(QObject::tr("IDS_OK"));
		}
		if (emBtns & QDialogButtonBox::Cancel) {
			d->m_pDialogBtnBox->button(QDialogButtonBox::Cancel)->setText(QObject::tr("IDS_CANCEL"));
		}
		if (emBtns & QDialogButtonBox::Apply) {
			d->m_pDialogBtnBox->button(QDialogButtonBox::Apply)->setText(QObject::tr("IDS_APPLY"));
			connect(d->m_pDialogBtnBox->button(QDialogButtonBox::Apply),
			        &QPushButton::clicked,
			        this,
			        &CUVBaseDialog::apply
			);
		}
		d->m_pDialogBtnBox->show();
	}
}

void CUVBaseDialog::setContent(QWidget* pContentWidget) const {
	Q_D(const CUVBaseDialog);

	if (pContentWidget) {
		if (!d->m_plyHContent->isEmpty()) {
			d->m_plyHContent->removeItem(d->m_plyHContent->itemAt(0));
		}
		d->m_plyHContent->addWidget(pContentWidget);
	}
}

void CUVBaseDialog::setContent(QLayout* pLayout) const {
	Q_D(const CUVBaseDialog);

	if (pLayout) {
		if (!d->m_plyHContent->isEmpty()) {
			d->m_plyHContent->removeItem(d->m_plyHContent->itemAt(0));
		}
		d->m_plyHContent->addLayout(pLayout);
	}
}

QPushButton* CUVBaseDialog::button(const QDialogButtonBox::StandardButton emBtn) const {
	Q_D(const CUVBaseDialog);

	return d->m_pDialogBtnBox->button(emBtn);
}

void CUVBaseDialog::paintEvent(QPaintEvent* event) {
	Q_D(CUVBaseDialog);

	if (d->m_bTitleVisible) {
		QPainter painter(this);
		QStyleOption opt;
		opt.init(this);
		if (d->m_bShadowVisible) {
			opt.rect.adjust(BORDER_SHADOW_WIDTH, BORDER_SHADOW_WIDTH, -BORDER_SHADOW_WIDTH, -BORDER_SHADOW_WIDTH);
		} else {
			opt.rect.adjust(0, 0, -1, 0);
		}
		style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

		painter.setPen(Qt::NoPen);

		painter.fillRect(d->m_rtTitle, QColor(70, 69, 69));

		if (d->m_bShadowVisible) {
			painter.setBrush(Qt::NoBrush);
			for (int i = BORDER_SHADOW_WIDTH - 1; i >= 0; --i) {
				const int nRGB = 80 - i * 10;
				painter.setPen(QColor(nRGB, nRGB, nRGB, i * 40));
				painter.drawRoundedRect(rect().adjusted(i, i, -i - 1, -i - 1), 3, 3, Qt::AbsoluteSize);
			}
		}
		QFont font;
		font.setPixelSize(16);
		painter.setFont(font);
		painter.setRenderHint(QPainter::TextAntialiasing);

		QRect rtUseable = d->m_rtTitle;
		if (!d->m_plyHTitle->isEmpty()) {
			int nLeftSide = d->m_plyHTitle->geometry().right();
			for (int i = 0; i < d->m_plyHTitle->count(); ++i) {
				if (QLayoutItem* item = d->m_plyHTitle->itemAt(i); item->widget()) {
					nLeftSide = qMin(item->widget()->geometry().left(), nLeftSide);
				}
			}
			rtUseable.setWidth(nLeftSide);
		}
		QRect rtIcon = rtUseable;
		if (!d->m_icon.isNull()) {
			if (d->m_bIconScaled) {
				rtIcon.setLeft(rtUseable.left() + 10);
				rtIcon.setTop(rtUseable.top() + (rtUseable.height() - d->m_icon.height()) / 2);
				rtIcon.setBottom(rtIcon.top() + d->m_icon.height());
				rtIcon.setRight(rtIcon.left() + d->m_icon.width());
				painter.drawPixmap(rtIcon, d->m_icon);
			} else {
				rtIcon.setLeft(rtUseable.left() + 2);
				rtIcon.setTop(rtUseable.top() + 2);
				rtIcon.setBottom(rtUseable.bottom() - 1);
				rtIcon.setRight(rtIcon.left() + d->m_icon.width());
				painter.drawPixmap(rtIcon, d->m_icon);
			}
		}
		if (!d->m_strTitle.isEmpty()) {
			painter.setPen(QColor(255, 255, 255));
			QRect rtItem = rtUseable;
			rtItem.setLeft(23);
			const QFontMetrics fontMetrics(painter.font());
			if (!d->m_icon.isNull()) {
				rtItem.setLeft(rtIcon.right() + 23);
			}
			QString strTitle = d->m_strTitle;
			if (fontMetrics.horizontalAdvance(d->m_strTitle) > rtItem.width()) {
				strTitle = fontMetrics.elidedText(d->m_strTitle, Qt::ElideRight, rtItem.width());
			}
			painter.drawText(rtItem, strTitle, QTextOption(Qt::AlignLeft | Qt::AlignVCenter));
		}
	}
}

bool CUVBaseDialog::nativeEvent(const QByteArray& eventType, void* message, long* result) {
#ifdef Q_OS_WIN
	Q_D(CUVBaseDialog);

	if (!d->m_bResizeable) {
		return false;
	}
	switch (const MSG* msg = static_cast<MSG*>(message); msg->message) {
		case WM_NCHITTEST: {
			if (!isMaximized()) {
				const double dpr = this->devicePixelRatioF();
				long xPos = GET_X_LPARAM(msg->lParam) / dpr;
				long yPos = GET_Y_LPARAM(msg->lParam) / dpr;
				const QPoint pos = mapFromGlobal(QPoint(xPos/* / dpr*/, yPos/* / dpr*/));
				xPos = pos.x();
				yPos = pos.y();
				if (xPos < NATIVE_DETECT_WIDTH && yPos < NATIVE_DETECT_WIDTH) // 左上角
					*result = HTTOPLEFT;
				else if (xPos >= width() - NATIVE_DETECT_WIDTH && yPos < NATIVE_DETECT_WIDTH) // 右上角
					*result = HTTOPRIGHT;
				else if (xPos < NATIVE_DETECT_WIDTH && yPos >= height() - NATIVE_DETECT_WIDTH) // 左下角
					*result = HTBOTTOMLEFT;
				else if (xPos >= width() - NATIVE_DETECT_WIDTH && yPos >= height() - NATIVE_DETECT_WIDTH) // 右下角
					*result = HTBOTTOMRIGHT;
				else if (xPos < NATIVE_DETECT_WIDTH) //左边
					*result = HTLEFT;
				else if (xPos >= width() - NATIVE_DETECT_WIDTH) //右边
					*result = HTRIGHT;
				else if (yPos < NATIVE_DETECT_WIDTH) //上边
					*result = HTTOP;
				else if (yPos >= height() - NATIVE_DETECT_WIDTH) //下边
					*result = HTBOTTOM;
				else //其他部分不做处理，返回false，留给其他事件处理器处理
					return false;
				return true;
			}
		}
		break;
		case WM_GETMINMAXINFO: {
			if (::IsZoomed(msg->hwnd)) {
				RECT frame = { 0, 0, 0, 0 };
				AdjustWindowRectEx(&frame, WS_OVERLAPPEDWINDOW, FALSE, 0);

				//record frame area data
				const double dpr = this->devicePixelRatioF();

				QMargins frames;
				constexpr QMargins margins(0, 0, 0, 0);
				frames.setLeft(lround(abs(frame.left) / dpr + 0.5));
				frames.setTop(lround(abs(frame.bottom) / dpr + 0.5));
				frames.setRight(lround(abs(frame.right) / dpr + 0.5));
				frames.setBottom(lround(abs(frame.bottom) / dpr + 0.5));

				setContentsMargins(frames.left() + margins.left(),
				                   frames.top() + margins.top(),
				                   frames.right() + margins.right(),
				                   frames.bottom() + margins.bottom()
				);

				if (d->m_hashTitleButtons[MaxRole]) {
					if (!d->m_hashTitleButtons[MaxRole]->isChecked()) {
						d->m_hashTitleButtons[MaxRole]->setChecked(true);
					}
				}
			} else {
				setContentsMargins(QMargins());
				if (d->m_hashTitleButtons[MaxRole]) {
					if (d->m_hashTitleButtons[MaxRole]->isChecked()) {
						d->m_hashTitleButtons[MaxRole]->setChecked(false);
					}
				}
			}
			return false;
		}
		default: break;
	}

#endif
	return false;
}

void CUVBaseDialog::mousePressEvent(QMouseEvent* event) {
	Q_D(CUVBaseDialog);

	if (d->m_bMoveEnable && event->button() == Qt::LeftButton && d->m_rtTitle.contains(event->pos())) {
		d->m_PressTitlePoint = event->pos();
		d->m_bPressTitle = true;
	}
	QDialog::mousePressEvent(event);
}

void CUVBaseDialog::mouseReleaseEvent(QMouseEvent* event) {
	Q_D(CUVBaseDialog);

	if (d->m_bPressTitle) {
		d->m_bPressTitle = false;
	}
	QDialog::mouseReleaseEvent(event);
}

void CUVBaseDialog::mouseMoveEvent(QMouseEvent* event) {
	Q_D(CUVBaseDialog);

	if (d->m_bPressTitle) {
		move(event->globalPos() - d->m_PressTitlePoint);
	}
	QDialog::mouseMoveEvent(event);
}

void CUVBaseDialog::keyPressEvent(QKeyEvent* event) {
	Q_D(CUVBaseDialog);

	if (!d->m_bEscEnable && Qt::Key_Escape == event->key()) {
		return;
	} else if (!d->m_bEnterEnable && (Qt::Key_Return == event->key() || Qt::Key_Enter == event->key())) {
		const auto key = static_cast<Qt::Key>(event->key());
		Q_UNUSED(key)
		return;
	} else {
		QDialog::keyPressEvent(event);
	}
}

void CUVBaseDialog::resizeEvent(QResizeEvent* event) {
	Q_D(CUVBaseDialog);

	if (d->m_bShadowVisible) {
		d->m_rtTitle = rect().adjusted(BORDER_SHADOW_WIDTH + 1, BORDER_SHADOW_WIDTH + 1, -BORDER_SHADOW_WIDTH - 1, 0);
	} else {
		if (d->m_hashTitleButtons[MaxRole] && d->m_hashTitleButtons[MaxRole]->isChecked()) {
			d->m_rtTitle = rect().adjusted(1, 10, -2, 0);
		} else {
			d->m_rtTitle = rect().adjusted(1, 2, -2, 0);
		}
	}
	d->m_rtTitle.setBottom(d->m_rtTitle.top() + TITLE_HEIGHT);
	if (auto* spacerItem = dynamic_cast<QSpacerItem*>(d->m_plyHTitle->itemAt(0)))
		spacerItem->changeSize(0, d->m_rtTitle.height() + 2, QSizePolicy::Expanding, QSizePolicy::Fixed);

	QDialog::resizeEvent(event);
}

void CUVBaseDialog::apply() {
}

void CUVBaseDialog::closeDialog() {
	reject();
}

[[maybe_unused]] void CUVBaseDialog::addTitleButton(QAbstractButton* pButton, const TitleButtonRole emButtonRole) {
	Q_D(CUVBaseDialog);

	insertTitleButton(pButton, d->m_plyHTitle->count(), emButtonRole);
}

QPushButton* CUVBaseDialog::addTitleButton(const TitleButtonRole emButtonRole) {
	Q_D(CUVBaseDialog);

	return insertTitleButton(d->m_plyHTitle->count(), emButtonRole);
}

QPushButton* CUVBaseDialog::insertTitleButton(const int nIndex, const TitleButtonRole emButtonRole) {
	QStyle::StandardPixmap icon = QStyle::SP_CustomBase;
	QString strObjectName;
	QString strToolTip;
	switch (emButtonRole) {
		case CUVBaseDialog::HelpRole: icon = QStyle::SP_TitleBarContextHelpButton;
			strObjectName = "CUVBaseDialog_Dialog_Btn_Help";
			strToolTip = tr("CUVBaseDialog_HELP");
			break;
		case CUVBaseDialog::MinRole: icon = QStyle::SP_TitleBarMinButton;
			strObjectName = "CUVBaseDialog_Dialog_Btn_Min";
			strToolTip = tr("CUVBaseDialog_MIN");
			break;
		case CUVBaseDialog::MaxRole: icon = QStyle::SP_TitleBarMaxButton;
			strObjectName = "CUVBaseDialog_Dialog_Btn_Max";
			strToolTip = tr("CUVBaseDialog_MAX");
			break;
		case CUVBaseDialog::CloseRole: icon = QStyle::SP_TitleBarCloseButton;
			strObjectName = "CUVBaseDialog_Dialog_Btn_Close";
			strToolTip = tr("CUVBaseDialog_CLOSE");
			break;
		default: break;
	}
	auto* pBtn = new QPushButton(this);
	pBtn->setObjectName(strObjectName);
	pBtn->setToolTip(strToolTip);
	if (!pBtn->testAttribute(Qt::WA_StyleSheet)) {
		const QStyle* style = pBtn->style();
		pBtn->setIcon(style->standardIcon(icon));
	}
	insertTitleButton(pBtn, nIndex, emButtonRole);
	return pBtn;
}

void CUVBaseDialog::insertTitleButton(QAbstractButton* pButton, const int nIndex, const TitleButtonRole emButtonRole) {
	Q_D(CUVBaseDialog);

	d->m_hashTitleButtons.insert(emButtonRole, pButton);
	d->m_plyHTitle->insertWidget(nIndex + 1, pButton);
}
