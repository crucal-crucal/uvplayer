#pragma once

#include <QEvent>

enum ECUVMaterialStateTransitionType {
	// Snackbar
	SnackbarShowTransition = 1,
	SnackbarHideTransition,
	SnackbarWaitTransition,
	SnackbarNextTransition,
	// FlatButton
	FlatButtonPressedTransition,
	FlatButtonCheckedTransition,
	FlatButtonUncheckedTransition,
	// CollapsibleMenu
	CollapsibleMenuExpand,
	CollapsibleMenuCollapse,
	// Slider
	SliderChangedToMinimum,
	SliderChangedFromMinimum,
	SliderNoFocusMouseEnter,
	SliderNoFocusMouseLeave,
	// Dialog
	DialogShowTransition,
	DialogHideTransition,
	//
	MaxTransitionType = 65535
};

struct CUVMaterialStateTransitionEvent final : public QEvent {
	explicit CUVMaterialStateTransitionEvent(const ECUVMaterialStateTransitionType type)
	: QEvent(static_cast<QEvent::Type>(QEvent::User + 1)), type(type) {
	}

	ECUVMaterialStateTransitionType type{};
};
