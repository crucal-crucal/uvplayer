#pragma once

#include <QAbstractTransition>

#include "uvmaterialstatetransitionevent.hpp"

#ifdef CUVMATERIALSHARED_LIB
#define CUVMATERIALSHARED_EXPORT Q_DECL_EXPORT
#else
#define CUVMATERIALSHARED_EXPORT Q_DECL_IMPORT
#endif

/*
 * @brief 状态机状态转换类
 */
class CUVMATERIALSHARED_EXPORT CUVMaterialStateTransition final : public QAbstractTransition {
	Q_OBJECT

public:
	explicit CUVMaterialStateTransition(ECUVMaterialStateTransitionType type);

protected:
	bool eventTest(QEvent* event) override;
	void onTransition(QEvent* event) override;

private:
	ECUVMaterialStateTransitionType m_type{};
};
