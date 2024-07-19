#pragma once

#include <QtGlobal>

#include "uvmaterialcheckable.hpp"

class QStateMachine;
class QState;
class QSignalTransition;
class CUVMaterialRippleOverlay;
class CUVMaterialCheckableIcon;

class CUVMaterialCheckablePrivate {
	Q_DISABLE_COPY(CUVMaterialCheckablePrivate)
	Q_DECLARE_PUBLIC(CUVMaterialCheckable)

public:
	explicit CUVMaterialCheckablePrivate(CUVMaterialCheckable* q);
	~CUVMaterialCheckablePrivate();

	void init();

	CUVMaterialCheckable* const q_ptr{ nullptr };
	CUVMaterialRippleOverlay* rippleOverlay{ nullptr };
	CUVMaterialCheckableIcon* checkedIcon{ nullptr };
	CUVMaterialCheckableIcon* uncheckedIcon{ nullptr };
	QStateMachine* stateMachine{ nullptr };
	QState* uncheckedState{ nullptr };
	QState* checkedState{ nullptr };
	QState* disabledUncheckedState{ nullptr };
	QState* disabledCheckedState{ nullptr };
	QSignalTransition* uncheckedTransition{ nullptr };
	QSignalTransition* checkedTransition{ nullptr };
	CUVMaterialCheckable::LabelPosition labelPosition{};
	QColor checkedColor{};
	QColor uncheckedColor{};
	QColor textColor{};
	QColor disabledColor{};
	bool useThemeColors{};
};
