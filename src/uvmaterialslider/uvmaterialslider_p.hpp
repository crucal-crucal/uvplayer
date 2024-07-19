#pragma once

#include <QColor>
#include <QRectF>
#include <QtGlobal>

class CUVMaterialSlider;
class CUVMaterialSliderThumb;
class CUVMaterialSliderTrack;
class CUVMaterialSliderStateMachine;

class CUVMaterialSliderPrivate {
	Q_DISABLE_COPY(CUVMaterialSliderPrivate)
	Q_DECLARE_PUBLIC(CUVMaterialSlider)

public:
	explicit CUVMaterialSliderPrivate(CUVMaterialSlider* q);
	~CUVMaterialSliderPrivate();

	void init();

	[[nodiscard]] QRectF trackBoundingRect() const;
	[[nodiscard]] QRectF thumbBoundingRect() const;

	[[nodiscard]] int valueFromPosition(const QPoint& pos) const;

	void setHovered(bool status);

	CUVMaterialSlider* const q_ptr{ nullptr };
	CUVMaterialSliderThumb* thumb{ nullptr };
	CUVMaterialSliderTrack* track{ nullptr };
	CUVMaterialSliderStateMachine* stateMachine{ nullptr };
	QColor thumbColor{};
	QColor trackColor{};
	QColor disabledColor{};
	int stepTo{};
	int oldValue{};
	int trackWidth{};
	bool hoverTrack{};
	bool hoverThumb{};
	bool hover{};
	bool step{};
	bool pageStepMode{};
	bool useThemeColors{};
};
