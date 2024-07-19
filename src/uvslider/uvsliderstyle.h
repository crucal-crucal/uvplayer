#pragma once

#include <QProxyStyle>

class CUVSliderStyle : public QProxyStyle {
	Q_OBJECT

public:
	explicit CUVSliderStyle(QStyle* style = nullptr);
	~CUVSliderStyle() override;
	void drawComplexControl(ComplexControl control, const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget) const override;
	int pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget) const override;
	int styleHint(StyleHint hint, const QStyleOption* option, const QWidget* widget, QStyleHintReturn* returnData) const override;

private:
	mutable QStyle::State _lastState;
	mutable qreal _circleRadius{ 0 };
	void _startRadiusAnimation(qreal startRadius, qreal endRadius, QWidget* widget) const;
};
