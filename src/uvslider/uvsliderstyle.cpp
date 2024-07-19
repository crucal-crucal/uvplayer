#include "uvsliderstyle.h"

#include <QPainter>
#include <QPropertyAnimation>
#include <QStyleOptionSlider>

CUVSliderStyle::CUVSliderStyle(QStyle* style): QProxyStyle(style) {
	_lastState = nullptr;
	setProperty("circleRadius", 0.01);
}

CUVSliderStyle::~CUVSliderStyle() = default;

void CUVSliderStyle::drawComplexControl(const ComplexControl control, const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget) const {
	switch (control) {
		case QStyle::CC_Slider: {
			const auto sopt = qstyleoption_cast<const QStyleOptionSlider*>(option);
			if (!sopt) {
				break;
			}

			painter->save();
			painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
			const QRect sliderRect = sopt->rect;
			const QRect sliderHandleRect = subControlRect(control, sopt, SC_SliderHandle, widget);
			// 滑槽
			painter->setPen(Qt::NoPen);
			painter->setBrush(QColor(0x8A, 0x8A, 0x8A));
			if (sopt->orientation == Qt::Horizontal) {
				// 未滑过
				painter->drawRoundedRect(QRect(sliderRect.x() + sliderRect.height() / 8, sliderRect.y() + sliderRect.height() * 0.375, sliderRect.width() - sliderRect.height() / 4, sliderRect.height() / 4), sliderRect.height() / 8, sliderRect.height() / 8); // NOLINT
				// 已滑过
				painter->setBrush(QColor(0x00, 0x66, 0xB4));
				painter->drawRoundedRect(QRect(sliderRect.x() + sliderRect.height() / 8, sliderRect.y() + sliderRect.height() * 0.375, sliderHandleRect.x(), sliderRect.height() / 4), sliderRect.height() / 8, sliderRect.height() / 8); // NOLINT
			} else {
				// 未滑过
				painter->drawRoundedRect(QRect(sliderRect.x() + sliderRect.width() * 0.375, sliderRect.y() + sliderRect.width() / 8, sliderRect.width() / 4, sliderRect.height() - sliderRect.width() / 4), sliderRect.width() / 8, sliderRect.width() / 8); // NOLINT
				// 已滑过
				painter->setBrush(QColor(0x00, 0x66, 0xB4));
				painter->drawRoundedRect(QRect(sliderRect.x() + sliderRect.width() * 0.375, sliderHandleRect.y(), sliderRect.width() / 4, sliderRect.height() - sliderRect.width() / 8 - sliderHandleRect.y()), sliderRect.width() / 8, sliderRect.width() / 8); // NOLINT
			}
			// 滑块
			// 外圆形
			painter->setBrush(QColor(0x00, 0x66, 0xB4));
			painter->drawEllipse(QPointF(sliderHandleRect.center().x() + 1, sliderHandleRect.center().y() + 1), sliderHandleRect.width() / 2, sliderHandleRect.width() / 2); // NOLINT
			// 内圆形
			painter->setPen(Qt::NoPen);
			painter->setBrush(QColor(0xEA, 0xEA, 0xEB));
			if (_lastState == 0) {
				_lastState = sopt->state;
			}
			if (_circleRadius == 0) {
				_circleRadius = sliderHandleRect.width() / 3.8;
			}
			if (sopt->activeSubControls == SC_SliderHandle) {
				if (sopt->state & QStyle::State_Sunken) {
					if (sopt->state & QStyle::State_MouseOver) {
						if (!_lastState.testFlag(QStyle::State_Sunken)) {
							_startRadiusAnimation(sliderHandleRect.width() / 2.8, sliderHandleRect.width() / 4.5, const_cast<QWidget*>(widget));
							_lastState = sopt->state;
						}
						painter->drawEllipse(QPointF(sliderHandleRect.center().x() + 1, sliderHandleRect.center().y() + 1), _circleRadius, _circleRadius);
					}
				} else {
					if (sopt->state & QStyle::State_MouseOver) {
						if (!_lastState.testFlag(QStyle::State_MouseOver)) {
							_startRadiusAnimation(_circleRadius, sliderHandleRect.width() / 2.8, const_cast<QWidget*>(widget));
							_lastState = sopt->state;
						}
						if (_lastState.testFlag(QStyle::State_Sunken)) {
							_startRadiusAnimation(_circleRadius, sliderHandleRect.width() / 2.8, const_cast<QWidget*>(widget));
							_lastState = sopt->state;
						}
						painter->drawEllipse(QPointF(sliderHandleRect.center().x() + 1, sliderHandleRect.center().y() + 1), _circleRadius, _circleRadius);
					}
				}
			} else {
				if (_lastState.testFlag(QStyle::State_MouseOver) || _lastState.testFlag(QStyle::State_Sunken)) {
					_startRadiusAnimation(_circleRadius, sliderHandleRect.width() / 3.8, const_cast<QWidget*>(widget));
					_lastState &= ~QStyle::State_MouseOver;
					_lastState &= ~QStyle::State_Sunken;
				}
				painter->drawEllipse(QPointF(sliderHandleRect.center().x() + 1, sliderHandleRect.center().y() + 1), _circleRadius, _circleRadius);
			}
			painter->restore();
			return;
		}
		default: {
			break;
		}
	}
	QProxyStyle::drawComplexControl(control, option, painter, widget);
}

int CUVSliderStyle::pixelMetric(const PixelMetric metric, const QStyleOption* option, const QWidget* widget) const {
	switch (metric) {
		case QStyle::PM_SliderLength:
		case QStyle::PM_SliderThickness: {
			return 20;
		}
		default: {
			break;
		}
	}
	return QProxyStyle::pixelMetric(metric, option, widget);
}

int CUVSliderStyle::styleHint(const StyleHint hint, const QStyleOption* option, const QWidget* widget, QStyleHintReturn* returnData) const {
	if (hint == QStyle::SH_Slider_AbsoluteSetButtons) {
		return Qt::LeftButton;
	}
	return QProxyStyle::styleHint(hint, option, widget, returnData);
}

void CUVSliderStyle::_startRadiusAnimation(const qreal startRadius, const qreal endRadius, QWidget* widget) const {
	const auto style = const_cast<CUVSliderStyle*>(this);
	const auto circleRadiusAnimation = new QPropertyAnimation(style, "circleRadius");
	connect(circleRadiusAnimation, &QPropertyAnimation::valueChanged, style, [=](const QVariant& value) {
		this->_circleRadius = value.toReal();
		widget->update();
	});
	circleRadiusAnimation->setEasingCurve(QEasingCurve::InOutSine);
	circleRadiusAnimation->setStartValue(startRadius);
	circleRadiusAnimation->setEndValue(endRadius);
	circleRadiusAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}
