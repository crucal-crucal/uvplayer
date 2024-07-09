#pragma once

#include <QApplication>
#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <utility>

#ifdef Q_OS_WIN
#include <Windows.h>
#endif

/*
 * @brief 无边框带阴影Dialog
 */

constexpr int TITLE_HEIGHT = 30;
constexpr int BORDER_SHADOW_WIDTH = 6;

#ifdef FRAMELESSBASEDIALOG_LIB
#define FRAMELESSBASEDIALOG_EXPORT Q_DECL_EXPORT
#else
#define FRAMELESSBASEDIALOG_EXPORT Q_DECL_IMPORT
#endif

class CUVBaseDialogPrivate;

class FRAMELESSBASEDIALOG_EXPORT CUVBaseDialog : public QDialog {
	Q_OBJECT
	Q_DISABLE_COPY(CUVBaseDialog)
	Q_DECLARE_PRIVATE(CUVBaseDialog)

public:
	enum TitleButtonRole {
		NoButtonRole                 = 0x00000000,
		CloseRole                    = 0x000001,
		MaxRole                      = 0x000002,
		MinRole                      = 0x000004,
		SettingRole [[maybe_unused]] = 0x000008,
		HelpRole                     = 0x000010,
	};
	Q_DECLARE_FLAGS(TitleButtonRoles, TitleButtonRole)
	Q_FLAG(TitleButtonRoles)

public:
	explicit CUVBaseDialog(QWidget* parent = nullptr);
	~CUVBaseDialog() override;

public:
	void setTitle(QString strTitle);
	[[nodiscard]] QRect getTilteRect() const;
	[[nodiscard]] QList<QAbstractButton*> getTitleButton() const;
	void setIcon(const QString& strPath, bool bScale, QSize scaleSize = QSize());
	void setResizeable(bool bResizeable);
	void setEscEnable(bool bEnable);
	void setMoveEnable(bool bEnable);
	void setTitleVisible(bool bVisible);
	void setEnterEnable(bool bEnable);
	void setShadowVisible(bool bVisible);

	void setTitleBtnRole(TitleButtonRoles emTitleButtonRoles);
	void setDialogBtnRole(
		QDialogButtonBox::StandardButtons emBtns = (QDialogButtonBox::Ok | QDialogButtonBox::Cancel));
	void setContent(QWidget* pContentWidget) const;
	void setContent(QLayout* pLayout) const;
	[[nodiscard]] QPushButton* button(QDialogButtonBox::StandardButton emBtn) const;

protected:
	void paintEvent(QPaintEvent* event) override;
	bool nativeEvent(const QByteArray& eventType, void* message, long* result) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;

protected slots:
	virtual void apply();
	virtual void closeDialog();

protected:
	void addTitleButton(QAbstractButton* pButton, TitleButtonRole emButtonRole);
	QPushButton* addTitleButton(TitleButtonRole emButtonRole);
	QPushButton* insertTitleButton(int nIndex, TitleButtonRole emButtonRole);
	void insertTitleButton(QAbstractButton* pButton, int nIndex, TitleButtonRole emButtonRole);

	const QScopedPointer<CUVBaseDialogPrivate> d_ptr{ nullptr };
};
