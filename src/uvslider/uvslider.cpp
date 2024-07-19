#include "uvslider.h"

#include <QEvent>

#include "uvsliderstyle.h"

CUVSlider::CUVSlider(QWidget* parent): QSlider(parent) {
	setOrientation(Qt::Horizontal);
	setStyle(new CUVSliderStyle(style()));
}

CUVSlider::CUVSlider(const Qt::Orientation orientation, QWidget* parent): QSlider(orientation, parent) {
	setStyle(new CUVSliderStyle(style()));
}

CUVSlider::~CUVSlider() = default;
