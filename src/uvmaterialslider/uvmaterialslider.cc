#include "uvmaterialslider.hpp"

#include <QMouseEvent>
#include <QtWidgets/QApplication>

#include "uvmaterialslider_internal.hpp"
#include "uvmaterialslider_p.hpp"
#include "uvshared/uvmaterialstatetransitionevent.hpp"
#include "uvshared/uvmaterialstyle.hpp"

/*!
 *  \class CUVMaterialSliderPrivate
 *  \internal
 */
CUVMaterialSliderPrivate::CUVMaterialSliderPrivate(CUVMaterialSlider* q): q_ptr(q) {
}

CUVMaterialSliderPrivate::~CUVMaterialSliderPrivate() = default;

void CUVMaterialSliderPrivate::init() {
	Q_Q(CUVMaterialSlider);

	thumb = new CUVMaterialSliderThumb(q);
	track = new CUVMaterialSliderTrack(thumb, q);
	stateMachine = new CUVMaterialSliderStateMachine(q, thumb, track);
	stepTo = 0;
	oldValue = q->value();
	trackWidth = 2;
	hoverTrack = false;
	hoverThumb = false;
	hover = false;
	step = false;
	pageStepMode = true;
	useThemeColors = true;

	q->setMouseTracking(true);
	q->setFocusPolicy(Qt::StrongFocus);
	q->setPageStep(1);

	QSizePolicy sp(QSizePolicy::Expanding, QSizePolicy::Fixed);

	if (q->orientation() == Qt::Vertical) {
		sp.transpose();
	}

	q->setSizePolicy(sp);
	q->setAttribute(Qt::WA_WState_OwnSizePolicy, false);

	stateMachine->start();
	QCoreApplication::processEvents();
}

QRectF CUVMaterialSliderPrivate::trackBoundingRect() const {
	Q_Q(const CUVMaterialSlider);

	const qreal hw = static_cast<qreal>(trackWidth) / 2;

	return Qt::Horizontal == q->orientation()
		       ? QRectF(CUV_MATERIAL_SLIDER_MARGIN, q->height() / 2 - hw, // NOLINT
		                q->width() - CUV_MATERIAL_SLIDER_MARGIN * 2, hw * 2)
		       : QRectF(q->width() / 2 - hw, CUV_MATERIAL_SLIDER_MARGIN, hw * 2, // NOLINT
		                q->height() - CUV_MATERIAL_SLIDER_MARGIN * 2);
}

QRectF CUVMaterialSliderPrivate::thumbBoundingRect() const {
	Q_Q(const CUVMaterialSlider);

	return Qt::Horizontal == q->orientation()
		       ? QRectF(thumb->offset(), q->height() / 2 - CUV_MATERIAL_SLIDER_MARGIN, // NOLINT
		                CUV_MATERIAL_SLIDER_MARGIN * 2, CUV_MATERIAL_SLIDER_MARGIN * 2)
		       : QRectF(q->width() / 2 - CUV_MATERIAL_SLIDER_MARGIN, thumb->offset(), // NOLINT
		                CUV_MATERIAL_SLIDER_MARGIN * 2, CUV_MATERIAL_SLIDER_MARGIN * 2);
}

int CUVMaterialSliderPrivate::valueFromPosition(const QPoint& pos) const {
	Q_Q(const CUVMaterialSlider);

	const int position = Qt::Horizontal == q->orientation() ? pos.x() : pos.y();

	const int span = Qt::Horizontal == q->orientation() ? q->width() - CUV_MATERIAL_SLIDER_MARGIN * 2 : q->height() - CUV_MATERIAL_SLIDER_MARGIN * 2;

	return CUVMaterialStyle::sliderValueFromPosition(q->minimum(), q->maximum(), position - CUV_MATERIAL_SLIDER_MARGIN, span,
	                                                 q->invertedAppearance());
}

void CUVMaterialSliderPrivate::setHovered(const bool status) {
	Q_Q(CUVMaterialSlider);

	if (hover == status) {
		return;
	}

	hover = status;

	if (!q->hasFocus()) {
		if (status) {
			stateMachine->postEvent(new CUVMaterialStateTransitionEvent(SliderNoFocusMouseEnter));
		} else {
			stateMachine->postEvent(new CUVMaterialStateTransitionEvent(SliderNoFocusMouseLeave));
		}
	}

	q->update();
}

/*!
 *  \class CUVMaterialSlider
 */
CUVMaterialSlider::CUVMaterialSlider(QWidget* parent)
: QAbstractSlider(parent), d_ptr(new CUVMaterialSliderPrivate(this)) {
	d_func()->init();
}

CUVMaterialSlider::~CUVMaterialSlider() = default;

void CUVMaterialSlider::setUseThemeColors(const bool value) {
	Q_D(CUVMaterialSlider);

	if (d->useThemeColors == value) {
		return;
	}

	d->useThemeColors = value;
	d->stateMachine->setupProperties();
}

bool CUVMaterialSlider::useThemeColors() const {
	Q_D(const CUVMaterialSlider);

	return d->useThemeColors;
}

void CUVMaterialSlider::setThumbColor(const QColor& color) {
	Q_D(CUVMaterialSlider);

	d->thumbColor = color;

	MATERIAL_DISABLE_THEME_COLORS
	d->stateMachine->setupProperties();
	update();
}

QColor CUVMaterialSlider::thumbColor() const {
	Q_D(const CUVMaterialSlider);

	if (d->useThemeColors || !d->thumbColor.isValid()) {
		return CUVMaterialStyle::instance().themeColor("primary1");
	} else {
		return d->thumbColor;
	}
}

void CUVMaterialSlider::setThumbDiameter(const qreal diameter) {
	Q_D(CUVMaterialSlider);

	d->thumb->setDiameter(diameter);
}

qreal CUVMaterialSlider::thumbDiameter() const {
	Q_D(const CUVMaterialSlider);

	return d->thumb->diameter();
}

void CUVMaterialSlider::setThumbHaloSize(const qreal size) {
	Q_D(CUVMaterialSlider);

	d->thumb->setHaloSize(size);
}

qreal CUVMaterialSlider::thumbHaloSize() const {
	Q_D(const CUVMaterialSlider);

	return d->thumb->haloSize();
}

void CUVMaterialSlider::setTrackColor(const QColor& color) {
	Q_D(CUVMaterialSlider);

	d->trackColor = color;

	MATERIAL_DISABLE_THEME_COLORS
	d->stateMachine->setupProperties();
	update();
}

QColor CUVMaterialSlider::trackColor() const {
	Q_D(const CUVMaterialSlider);

	if (d->useThemeColors || !d->trackColor.isValid()) {
		return CUVMaterialStyle::instance().themeColor("accent3");
	} else {
		return d->trackColor;
	}
}

void CUVMaterialSlider::setTrackHeight(const int height) {
	Q_D(CUVMaterialSlider);

	d->track->setTrackWidth(height);
}

int CUVMaterialSlider::trackHeight() const {
	Q_D(const CUVMaterialSlider);

	return d->track->trackWidth();
}

void CUVMaterialSlider::setDisabledColor(const QColor& color) {
	Q_D(CUVMaterialSlider);

	d->disabledColor = color;

	MATERIAL_DISABLE_THEME_COLORS
	d->stateMachine->setupProperties();
	update();
}

QColor CUVMaterialSlider::disabledColor() const {
	Q_D(const CUVMaterialSlider);

	if (d->useThemeColors || !d->disabledColor.isValid()) {
		return CUVMaterialStyle::instance().themeColor("disabled");
	} else {
		return d->disabledColor;
	}
}

void CUVMaterialSlider::setPageStepMode(const bool pageStep) {
	Q_D(CUVMaterialSlider);

	d->pageStepMode = pageStep;
}

bool CUVMaterialSlider::pageStepMode() const {
	Q_D(const CUVMaterialSlider);

	return d->pageStepMode;
}

QSize CUVMaterialSlider::minimumSizeHint() const {
	return Qt::Horizontal == orientation() ? QSize(130, 34) : QSize(34, 130);
}

void CUVMaterialSlider::setInvertedAppearance(const bool value) {
	QAbstractSlider::setInvertedAppearance(value);

	updateThumbOffset();
}

void CUVMaterialSlider::custom_hide() {
	Q_D(CUVMaterialSlider);

	setvisible(false);
}

void CUVMaterialSlider::custom_show() {
	Q_D(CUVMaterialSlider);

	setvisible(true);
}

void CUVMaterialSlider::setvisible(const bool visible) {
	Q_D(CUVMaterialSlider);

	d->thumb->setVisible(visible);
	d->track->setVisible(visible);
	this->setVisible(visible);
}

void CUVMaterialSlider::sliderChange(const SliderChange change) {
	Q_D(CUVMaterialSlider);

	if (SliderOrientationChange == change) {
		QSizePolicy sp(QSizePolicy::Expanding, QSizePolicy::Fixed);
		if (orientation() == Qt::Vertical) {
			sp.transpose();
		}
		setSizePolicy(sp);
	} else if (SliderValueChange == change) {
		if (minimum() == value()) {
			triggerAction(SliderToMinimum);
			d->stateMachine->postEvent(new CUVMaterialStateTransitionEvent(SliderChangedToMinimum));
		} else if (maximum() == value()) {
			triggerAction(SliderToMaximum);
		}
		if (minimum() == d->oldValue) {
			d->stateMachine->postEvent(new CUVMaterialStateTransitionEvent(SliderChangedFromMinimum));
		}
		d->oldValue = value();
	}

	updateThumbOffset();

	QAbstractSlider::sliderChange(change);
}

void CUVMaterialSlider::mouseMoveEvent(QMouseEvent* event) {
	Q_D(CUVMaterialSlider);

	if (isSliderDown()) {
		setSliderPosition(d->valueFromPosition(event->pos()));
	} else {
		if (const QRectF track(d->trackBoundingRect().adjusted(-2, -2, 2, 2)); track.contains(event->pos()) != d->hoverTrack) {
			d->hoverTrack = !d->hoverTrack;
			update();
		}

		QRectF thumb(0, 0, 16, 16);
		thumb.moveCenter(d->thumbBoundingRect().center());

		if (thumb.contains(event->pos()) != d->hoverThumb) {
			d->hoverThumb = !d->hoverThumb;
			update();
		}

		d->setHovered(d->hoverTrack || d->hoverThumb);
	}

	QAbstractSlider::mouseMoveEvent(event);
}

void CUVMaterialSlider::mousePressEvent(QMouseEvent* event) {
	Q_D(CUVMaterialSlider);

	const QPoint pos = event->pos();

	QRectF thumb(0, 0, 16, 16);
	thumb.moveCenter(d->thumbBoundingRect().center());

	if (thumb.contains(pos)) {
		setSliderDown(true);
		return;
	}

	if (!d->pageStepMode) {
		setSliderPosition(d->valueFromPosition(event->pos()));
		d->thumb->setHaloSize(0);
		setSliderDown(true);
		return;
	}

	d->step = true;
	d->stepTo = d->valueFromPosition(pos);

	const SliderAction action = d->stepTo > sliderPosition() ? SliderPageStepAdd : SliderPageStepSub;

	triggerAction(action);
	setRepeatAction(action, 400, 8);
}

void CUVMaterialSlider::mouseReleaseEvent(QMouseEvent* event) {
	Q_D(CUVMaterialSlider);

	if (isSliderDown()) {
		setSliderDown(false);
	} else if (d->step) {
		d->step = false;
		setRepeatAction(SliderNoAction, 0);
	}

	QAbstractSlider::mouseReleaseEvent(event);
}

void CUVMaterialSlider::leaveEvent(QEvent* event) {
	Q_D(CUVMaterialSlider);

	if (d->hoverTrack) {
		d->hoverTrack = false;
		update();
	}
	if (d->hoverThumb) {
		d->hoverThumb = false;
		update();
	}

	d->setHovered(false);

	QAbstractSlider::leaveEvent(event);
}

void CUVMaterialSlider::updateThumbOffset() {
	Q_D(CUVMaterialSlider);

	const int offset = CUVMaterialStyle::sliderPositionFromValue(
		minimum(),
		maximum(),
		sliderPosition(),
		Qt::Horizontal == orientation() ? width() - CUV_MATERIAL_SLIDER_MARGIN * 2 : height() - CUV_MATERIAL_SLIDER_MARGIN * 2,
		invertedAppearance());

	d->thumb->setOffset(offset);
}
