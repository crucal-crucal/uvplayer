#include "uvmaterialstyle.hpp"

#include <QFontDatabase>

#include "uvmaterialtheme.hpp"

/*!
 *  \class CUVMaterialStylePrivate
 */
CUVMaterialStylePrivate::CUVMaterialStylePrivate(CUVMaterialStyle* q): q_ptr(q) {
}

CUVMaterialStylePrivate::~CUVMaterialStylePrivate() = default;

void CUVMaterialStylePrivate::init() {
	Q_Q(CUVMaterialStyle);

	QFontDatabase::addApplicationFont(":/fonts/roboto_regular");
	QFontDatabase::addApplicationFont(":/fonts/roboto_medium");
	QFontDatabase::addApplicationFont(":/fonts/roboto_bold");

	q->setTheme(new CUVMaterialTheme);
}

/*!
 *  \class CUVMaterialStyle
 */
void CUVMaterialStyle::setTheme(CUVMaterialTheme* theme) {
	Q_D(CUVMaterialStyle);

	d->theme = theme;
	theme->setParent(this);
}

QColor CUVMaterialStyle::themeColor(const QString& key) const {
	Q_D(const CUVMaterialStyle);

	Q_ASSERT(d->theme);

	return d->theme->getColor(key);
}

CUVMaterialStyle::CUVMaterialStyle(): QCommonStyle(), d_ptr(new CUVMaterialStylePrivate(this)) {
	d_func()->init();
}
