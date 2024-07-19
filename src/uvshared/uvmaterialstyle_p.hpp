#pragma once

#include <QtGlobal>

class CUVMaterialStyle;
class CUVMaterialTheme;

class Q_DECL_EXPORT CUVMaterialStylePrivate {
	Q_DISABLE_COPY(CUVMaterialStylePrivate)
	Q_DECLARE_PUBLIC(CUVMaterialStyle)

public:
	explicit CUVMaterialStylePrivate(CUVMaterialStyle* q);
	~CUVMaterialStylePrivate();

	void init();

	CUVMaterialStyle* const q_ptr{ nullptr };
	CUVMaterialTheme* theme{ nullptr };
};
