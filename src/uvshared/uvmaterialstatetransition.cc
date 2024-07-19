#include "uvmaterialstatetransition.hpp"

CUVMaterialStateTransition::CUVMaterialStateTransition(const ECUVMaterialStateTransitionType type): m_type(type) {
}

bool CUVMaterialStateTransition::eventTest(QEvent* event) {
	if (event->type() != static_cast<QEvent::Type>(QEvent::User + 1)) {
		return false;
	}
	const auto transition = dynamic_cast<CUVMaterialStateTransitionEvent*>(event);
	return (m_type == transition->type);
}

void CUVMaterialStateTransition::onTransition(QEvent* event) {
	Q_UNUSED(event)
}
