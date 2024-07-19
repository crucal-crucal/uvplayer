#include "uvmaterialrippleoverlay.hpp"

#include <QPainter>

#include "uvmaterialripple.hpp"

/*!
 *  \class CUVMaterialRippleOverlay
 *  \internal
 */
CUVMaterialRippleOverlay::CUVMaterialRippleOverlay(QWidget* parent)
: CUVMaterialOverlayWidget(parent), m_useClip(false) {
	setAttribute(Qt::WA_TransparentForMouseEvents);
	setAttribute(Qt::WA_NoSystemBackground);
}

CUVMaterialRippleOverlay::~CUVMaterialRippleOverlay() = default;

void CUVMaterialRippleOverlay::addRipple(CUVMaterialRipple* ripple) {
	ripple->setOverlay(this);
	m_ripples.push_back(ripple);
	ripple->start();

	connect(this, &CUVMaterialRippleOverlay::destroyed, ripple, &CUVMaterialRipple::stop);
	connect(this, &CUVMaterialRippleOverlay::destroyed, ripple, &CUVMaterialRipple::deleteLater);
}

void CUVMaterialRippleOverlay::addRipple(const QPoint& position, const qreal radius) {
	const auto ripple = new CUVMaterialRipple(position);
	ripple->setRadiusEndValue(radius);
	addRipple(ripple);
}

void CUVMaterialRippleOverlay::removeRipple(CUVMaterialRipple* ripple) {
	if (m_ripples.removeOne(ripple)) {
		delete ripple;
		update();
	}
}

void CUVMaterialRippleOverlay::paintEvent(QPaintEvent* event) {
	Q_UNUSED(event)

	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setPen(Qt::NoPen);

	if (m_useClip) {
		painter.setClipPath(m_clipPath);
	}

	for (const auto& ripple : m_ripples) {
		paintRipple(&painter, ripple);
	}
}

void CUVMaterialRippleOverlay::paintRipple(QPainter* painter, const CUVMaterialRipple* ripple) {
	const qreal radius = ripple->radius();
	const QPointF center = ripple->center();
	painter->setOpacity(ripple->opacity());
	painter->setBrush(ripple->brush());
	painter->drawEllipse(center, radius, radius);
}
