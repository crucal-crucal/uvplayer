#pragma once

#include <QColor>
#include <QHash>

class CUVMaterialTheme;

class Q_DECL_EXPORT CUVMaterialThemePrivate final {
	Q_DISABLE_COPY(CUVMaterialThemePrivate)
	Q_DECLARE_PUBLIC(CUVMaterialTheme)

public:
	explicit CUVMaterialThemePrivate(CUVMaterialTheme* q);
	~CUVMaterialThemePrivate();

	QColor rgba(int r, int g, int b, qreal a);

	CUVMaterialTheme* const q_ptr{ nullptr };
	QHash<QString, QColor> colors{};
};
