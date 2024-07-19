#include "uvmaterialripple.hpp"

#include "uvmaterialrippleoverlay.hpp"

/*!
 *  \class CUVMaterialRipple
 */
CUVMaterialRipple::CUVMaterialRipple(const QPoint& center, QObject* parent)
: QParallelAnimationGroup(parent), m_overlay(nullptr), m_radiusAnimation(animate("radius")), m_opacityAnimation(animate("opacity")),
  m_radius(0), m_opacity(0), m_center(center) {
	init();
}

CUVMaterialRipple::CUVMaterialRipple(const QPoint& center, CUVMaterialRippleOverlay* overlay, QObject* parent)
: QParallelAnimationGroup(parent), m_overlay(overlay), m_radiusAnimation(animate("radius")), m_opacityAnimation(animate("opacity")),
  m_radius(0), m_opacity(0), m_center(center) {
	init();
}

CUVMaterialRipple::~CUVMaterialRipple() = default;

void CUVMaterialRipple::setRadius(const qreal radius) {
	Q_ASSERT(m_overlay);

	if (m_radius == radius) {
		return;
	}
	m_radius = radius;
	m_overlay->update();
}

void CUVMaterialRipple::setOpacity(const qreal opacity) {
	Q_ASSERT(m_overlay);

	if (m_opacity == opacity) {
		return;
	}
	m_opacity = opacity;
	m_overlay->update();
}

void CUVMaterialRipple::setColor(const QColor& color) {
	if (m_brush.color() == color) {
		return;
	}
	m_brush.setColor(color);

	if (m_overlay) {
		m_overlay->update();
	}
}

void CUVMaterialRipple::setBrush(const QBrush& brush) {
	m_brush = brush;

	if (m_overlay) {
		m_overlay->update();
	}
}

QPropertyAnimation* CUVMaterialRipple::radiusAnimation() const {
	return m_radiusAnimation;
}

QPropertyAnimation* CUVMaterialRipple::opacityAnimation() const {
	return m_opacityAnimation;
}

void CUVMaterialRipple::setOpacityStartValue(const qreal value) const {
	m_opacityAnimation->setStartValue(value);
}

void CUVMaterialRipple::setOpacityEndValue(const qreal value) const {
	m_opacityAnimation->setEndValue(value);
}

void CUVMaterialRipple::setRadiusStartValue(const qreal value) const {
	m_radiusAnimation->setStartValue(value);
}

void CUVMaterialRipple::setRadiusEndValue(const qreal value) const {
	m_radiusAnimation->setEndValue(value);
}

void CUVMaterialRipple::setDuration(const int msecs) const {
	m_radiusAnimation->setDuration(msecs);
	m_opacityAnimation->setDuration(msecs);
}

void CUVMaterialRipple::destroy() {
	Q_ASSERT(m_overlay);

	m_overlay->removeRipple(this);
}

QPropertyAnimation* CUVMaterialRipple::animate(const QByteArray& property, const QEasingCurve& easing, const int duration) {
	const auto animation = new QPropertyAnimation;
	animation->setTargetObject(this);
	animation->setPropertyName(property);
	animation->setEasingCurve(easing);
	animation->setDuration(duration);
	addAnimation(animation);
	return animation;
}

void CUVMaterialRipple::init() {
	setOpacityStartValue(0.5);
	setOpacityEndValue(0);
	setRadiusStartValue(0);
	setRadiusEndValue(300);

	m_brush.setColor(Qt::black);
	m_brush.setStyle(Qt::SolidPattern);

	connect(this, &CUVMaterialRipple::finished, this, &CUVMaterialRipple::destroy);
}
