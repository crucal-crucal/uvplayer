#include "uvmaterialcheckable.hpp"

#include <QColor>
#include <QCoreApplication>
#include <QEvent>
#include <QEventTransition>
#include <QPainter>
#include <QSignalTransition>
#include <QStateMachine>

#include "uvmaterialcheckable_internal.hpp"
#include "uvmaterialcheckable_p.hpp"
#include "uvmaterialripple.hpp"
#include "uvmaterialrippleoverlay.hpp"
#include "uvmaterialstyle.hpp"

/*!
 *  \class CUVMaterialCheckablePrivate
 *  \internal
 */
CUVMaterialCheckablePrivate::CUVMaterialCheckablePrivate(CUVMaterialCheckable* q): q_ptr(q) {
}

CUVMaterialCheckablePrivate::~CUVMaterialCheckablePrivate() = default;

void CUVMaterialCheckablePrivate::init() {
	Q_Q(CUVMaterialCheckable);

	rippleOverlay = new CUVMaterialRippleOverlay;
	checkedIcon = new CUVMaterialCheckableIcon(QIcon(":/icons/icons/toggle/svg/production/ic_check_box_24px.svg"), q);
	uncheckedIcon = new CUVMaterialCheckableIcon(QIcon(":/icons/icons/toggle/svg/production/ic_check_box_outline_blank_24px.svg"), q);
	stateMachine = new QStateMachine(q);
	uncheckedState = new QState;
	checkedState = new QState;
	disabledUncheckedState = new QState;
	disabledCheckedState = new QState;
	uncheckedTransition = new QSignalTransition(q, &CUVMaterialCheckable::toggled);
	checkedTransition = new QSignalTransition(q, &CUVMaterialCheckable::toggled);
	labelPosition = CUVMaterialCheckable::LabelPositionRight;
	useThemeColors = true;

	rippleOverlay->setParent(q->parentWidget());
	rippleOverlay->installEventFilter(q);

	q->setCheckable(true);
	q->setStyle(&CUVMaterialStyle::instance());
	q->setFont(QFont("Roboto", 11, QFont::Normal));

	stateMachine->addState(uncheckedState);
	stateMachine->addState(checkedState);
	stateMachine->addState(disabledUncheckedState);
	stateMachine->addState(disabledCheckedState);
	stateMachine->setInitialState(uncheckedState);

	// Transition to checked
	uncheckedTransition->setTargetState(checkedState);
	uncheckedState->addTransition(uncheckedTransition);

	// Transition to unchecked
	checkedTransition->setTargetState(uncheckedState);
	checkedState->addTransition(checkedTransition);

	// Transitions enabled <==> disabled
	QAbstractTransition* transition = new QEventTransition(q, QEvent::EnabledChange);
	transition->setTargetState(disabledUncheckedState);
	uncheckedState->addTransition(transition);

	transition = new QEventTransition(q, QEvent::EnabledChange);
	transition->setTargetState(uncheckedState);
	disabledUncheckedState->addTransition(transition);

	transition = new QEventTransition(q, QEvent::EnabledChange);
	transition->setTargetState(disabledCheckedState);
	checkedState->addTransition(transition);

	transition = new QEventTransition(q, QEvent::EnabledChange);
	transition->setTargetState(checkedState);
	disabledCheckedState->addTransition(transition);

	transition = new QSignalTransition(q, &CUVMaterialCheckable::toggled);
	transition->setTargetState(disabledCheckedState);
	disabledUncheckedState->addTransition(transition);

	transition = new QSignalTransition(q, &CUVMaterialCheckable::toggled);
	transition->setTargetState(disabledUncheckedState);
	disabledCheckedState->addTransition(transition);

	checkedState->assignProperty(checkedIcon, "opacity", 1);
	checkedState->assignProperty(uncheckedIcon, "opacity", 0);

	uncheckedState->assignProperty(checkedIcon, "opacity", 0);
	uncheckedState->assignProperty(uncheckedIcon, "opacity", 1);

	disabledCheckedState->assignProperty(checkedIcon, "opacity", 1);
	disabledCheckedState->assignProperty(uncheckedIcon, "opacity", 0);

	disabledUncheckedState->assignProperty(checkedIcon, "opacity", 0);
	disabledUncheckedState->assignProperty(uncheckedIcon, "opacity", 1);

	checkedState->assignProperty(checkedIcon, "color", q->checkedColor());
	checkedState->assignProperty(uncheckedIcon, "color", q->checkedColor());

	uncheckedState->assignProperty(uncheckedIcon, "color", q->uncheckedColor());
	uncheckedState->assignProperty(uncheckedIcon, "color", q->uncheckedColor());

	disabledUncheckedState->assignProperty(uncheckedIcon, "color", q->disabledColor());
	disabledCheckedState->assignProperty(checkedIcon, "color", q->disabledColor());

	stateMachine->start();
	QCoreApplication::processEvents();
}

/*!
 *  \class CUVMaterialCheckable
 */
CUVMaterialCheckable::CUVMaterialCheckable(QWidget* parent)
: QAbstractButton(parent), d_ptr(new CUVMaterialCheckablePrivate(this)) {
	d_func()->init();
}

CUVMaterialCheckable::~CUVMaterialCheckable() = default;

void CUVMaterialCheckable::setLabelPosition(const LabelPosition placement) {
	Q_D(CUVMaterialCheckable);

	d->labelPosition = placement;
	update();
}

CUVMaterialCheckable::LabelPosition CUVMaterialCheckable::labelPosition() const {
	Q_D(const CUVMaterialCheckable);

	return d->labelPosition;
}

void CUVMaterialCheckable::setUseThemeColors(const bool value) {
	Q_D(CUVMaterialCheckable);

	if (d->useThemeColors == value) {
		return;
	}

	d->useThemeColors = value;
	setupProperties();
}

bool CUVMaterialCheckable::useThemeColors() const {
	Q_D(const CUVMaterialCheckable);

	return d->useThemeColors;
}

void CUVMaterialCheckable::setCheckedColor(const QColor& color) {
	Q_D(CUVMaterialCheckable);

	d->checkedColor = color;

	MATERIAL_DISABLE_THEME_COLORS
	setupProperties();
}

QColor CUVMaterialCheckable::checkedColor() const {
	Q_D(const CUVMaterialCheckable);

	if (d->useThemeColors || !d->checkedColor.isValid()) {
		return CUVMaterialStyle::instance().themeColor("primary1");
	} else {
		return d->checkedColor;
	}
}

void CUVMaterialCheckable::setUncheckedColor(const QColor& color) {
	Q_D(CUVMaterialCheckable);

	d->uncheckedColor = color;

	MATERIAL_DISABLE_THEME_COLORS
	setupProperties();
}

QColor CUVMaterialCheckable::uncheckedColor() const {
	Q_D(const CUVMaterialCheckable);

	if (d->useThemeColors || !d->uncheckedColor.isValid()) {
		return CUVMaterialStyle::instance().themeColor("text");
	} else {
		return d->uncheckedColor;
	}
}

void CUVMaterialCheckable::setTextColor(const QColor& color) {
	Q_D(CUVMaterialCheckable);

	d->textColor = color;

	MATERIAL_DISABLE_THEME_COLORS
	setupProperties();
}

QColor CUVMaterialCheckable::textColor() const {
	Q_D(const CUVMaterialCheckable);

	if (d->useThemeColors || !d->textColor.isValid()) {
		return CUVMaterialStyle::instance().themeColor("text");
	} else {
		return d->textColor;
	}
}

void CUVMaterialCheckable::setDisabledColor(const QColor& color) {
	Q_D(CUVMaterialCheckable);

	d->disabledColor = color;

	MATERIAL_DISABLE_THEME_COLORS
	setupProperties();
}

QColor CUVMaterialCheckable::disabledColor() const {
	Q_D(const CUVMaterialCheckable);

	if (d->useThemeColors || !d->disabledColor.isValid()) {
		return CUVMaterialStyle::instance().themeColor("accent3");
	} else {
		return d->disabledColor;
	}
}

void CUVMaterialCheckable::setCheckedIcon(const QIcon& icon) {
	Q_D(CUVMaterialCheckable);

	d->checkedIcon->setIcon(icon);
	update();
}

QIcon CUVMaterialCheckable::checkedIcon() const {
	Q_D(const CUVMaterialCheckable);

	return d->checkedIcon->icon();
}

void CUVMaterialCheckable::setUncheckedIcon(const QIcon& icon) {
	Q_D(CUVMaterialCheckable);

	d->uncheckedIcon->setIcon(icon);
	update();
}

QIcon CUVMaterialCheckable::uncheckedIcon() const {
	Q_D(const CUVMaterialCheckable);

	return d->uncheckedIcon->icon();
}

QSize CUVMaterialCheckable::sizeHint() const {
	if (text().isEmpty()) {
		return { 40, 40 };
	}
	return { fontMetrics().size(Qt::TextShowMnemonic, text()).width() + 52, 40 };
}

CUVMaterialCheckable::CUVMaterialCheckable(CUVMaterialCheckablePrivate& d, QWidget* parent)
: QAbstractButton(parent), d_ptr(&d) {
	d_func()->init();
}

bool CUVMaterialCheckable::event(QEvent* event) {
	Q_D(CUVMaterialCheckable);

	switch (event->type()) {
		case QEvent::Resize:
		case QEvent::Move:
			d->checkedIcon->setGeometry(rect());
			d->uncheckedIcon->setGeometry(rect());
			d->rippleOverlay->setGeometry(geometry().adjusted(-8, -8, 8, 8));
			break;
		case QEvent::ParentChange:
			QWidget* widget;
			if ((widget = parentWidget())) {
				d->rippleOverlay->setParent(widget);
			}
			break;
		default:
			break;
	}
	return QAbstractButton::event(event);
}

bool CUVMaterialCheckable::eventFilter(QObject* watched, QEvent* event) {
	if (QEvent::Resize == event->type()) {
		Q_D(CUVMaterialCheckable);

		d->rippleOverlay->setGeometry(geometry().adjusted(-8, -8, 8, 8));
	}
	return QAbstractButton::eventFilter(watched, event);
}

void CUVMaterialCheckable::mousePressEvent(QMouseEvent* event) {
	Q_UNUSED(event)

	Q_D(CUVMaterialCheckable);

	if (!isEnabled()) {
		return;
	}

	CUVMaterialRipple* ripple;
	if (CUVMaterialCheckable::LabelPositionLeft == d->labelPosition) {
		ripple = new CUVMaterialRipple(QPoint(width() - 14, 28));
	} else {
		ripple = new CUVMaterialRipple(QPoint(28, 28));
	}
	ripple->setRadiusEndValue(22);
	ripple->setColor(isChecked() ? checkedColor() : uncheckedColor());
	if (isChecked()) {
		ripple->setOpacityStartValue(1);
	}
	d->rippleOverlay->addRipple(ripple);

	setChecked(!isChecked());
}

void CUVMaterialCheckable::paintEvent(QPaintEvent* event) {
	Q_UNUSED(event)

	Q_D(CUVMaterialCheckable);

	QPainter painter(this);

	QPen pen;
	pen.setColor(isEnabled() ? textColor() : disabledColor());
	painter.setPen(pen);

	if (CUVMaterialCheckable::LabelPositionLeft == d->labelPosition) {
		painter.drawText(4, 26, text());
	} else {
		painter.drawText(48, 26, text());
	}
}

void CUVMaterialCheckable::setupProperties() {
	Q_D(CUVMaterialCheckable);

	d->checkedState->assignProperty(d->checkedIcon, "color", checkedColor());
	d->checkedState->assignProperty(d->uncheckedIcon, "color", checkedColor());
	d->uncheckedState->assignProperty(d->uncheckedIcon, "color", uncheckedColor());
	d->disabledUncheckedState->assignProperty(d->uncheckedIcon, "color", disabledColor());
	d->disabledCheckedState->assignProperty(d->checkedIcon, "color", disabledColor());

	if (isEnabled()) {
		if (isChecked()) {
			d->checkedIcon->setColor(checkedColor());
		} else {
			d->uncheckedIcon->setColor(uncheckedColor());
		}
	} else {
		d->checkedIcon->setColor(disabledColor());
		d->uncheckedIcon->setColor(disabledColor());
	}

	update();
}
