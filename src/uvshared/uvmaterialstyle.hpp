#pragma once

#include <QtWidgets/QCommonStyle>

#include "uvmaterialstyle_p.hpp"

#define MATERIAL_DISABLE_THEME_COLORS \
    if (d->useThemeColors == true) { d->useThemeColors = false; }

class CUVMaterialTheme;

#ifdef CUVMATERIALSHARED_LIB
#define CUVMATERIALSHARED_EXPORT Q_DECL_EXPORT
#else
#define CUVMATERIALSHARED_EXPORT Q_DECL_IMPORT
#endif

class CUVMATERIALSHARED_EXPORT CUVMaterialStyle final : public QCommonStyle {
	Q_OBJECT
	Q_DECLARE_PRIVATE(CUVMaterialStyle)

public:
	inline static CUVMaterialStyle& instance();

	void setTheme(CUVMaterialTheme* theme);
	[[nodiscard]] QColor themeColor(const QString& key) const;

protected:
	const QScopedPointer<CUVMaterialStylePrivate> d_ptr{ nullptr };

private:
	CUVMaterialStyle();
};

inline CUVMaterialStyle& CUVMaterialStyle::instance() {
	static CUVMaterialStyle instance;
	return instance;
}
