#pragma once

#include <QAction>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QIcon>
#include <QLineEdit>
#include <QList>
#include <QListWidget>
#include <QMenu>
#include <QObject>
#include <QParallelAnimationGroup>
#include <QPlainTextEdit>
#include <QPointer>
#include <QPropertyAnimation>
#include <QProxyStyle>
#include <QStyledItemDelegate>
#include <QTextEdit>
#include <QTimer>
#include <QWidget>
#include <functional>

#include "components/widgets/scroll_bar.h"
#include "components/widgets/tool_tip.h"

namespace qfw {

class RoundMenu;
class MenuAnimationManager;

class CustomMenuStyle : public QProxyStyle {
    Q_OBJECT

public:
    explicit CustomMenuStyle(int iconSize = 14);

    int pixelMetric(PixelMetric metric, const QStyleOption* option,
                    const QWidget* widget) const override;

private:
    int iconSize_;
};

enum class MenuAnimationType {
    None = 0,
    DropDown = 1,
    PullUp = 2,
    FadeInDropDown = 3,
    FadeInPullUp = 4
};

class SubMenuItemWidget : public QWidget {
    Q_OBJECT

public:
    explicit SubMenuItemWidget(RoundMenu* menu, QListWidgetItem* item, QWidget* parent = nullptr);

    RoundMenu* menu() const { return menu_; }

signals:
    void showMenuSig(QListWidgetItem* item);

protected:
    void enterEvent(QEnterEvent* e) override;

private:
    QPointer<RoundMenu> menu_;
    QListWidgetItem* item_ = nullptr;
};

class MenuItemDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit MenuItemDelegate(QObject* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    bool helpEvent(QHelpEvent* event, QAbstractItemView* view, const QStyleOptionViewItem& option,
                   const QModelIndex& index) override;

protected:
    bool isSeparator(const QModelIndex& index) const;

private:
    mutable QPointer<ItemViewToolTipDelegate> tooltipDelegate_;
};

class ShortcutMenuItemDelegate : public MenuItemDelegate {
    Q_OBJECT

public:
    using MenuItemDelegate::MenuItemDelegate;

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
};

class IndicatorMenuItemDelegate : public MenuItemDelegate {
    Q_OBJECT

public:
    using MenuItemDelegate::MenuItemDelegate;

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
};

class CheckableMenuItemDelegate : public ShortcutMenuItemDelegate {
    Q_OBJECT

public:
    using ShortcutMenuItemDelegate::ShortcutMenuItemDelegate;

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

protected:
    virtual void drawIndicator(QPainter* painter, const QStyleOptionViewItem& option,
                               const QModelIndex& index) const = 0;
};

class RadioIndicatorMenuItemDelegate : public CheckableMenuItemDelegate {
    Q_OBJECT

public:
    using CheckableMenuItemDelegate::CheckableMenuItemDelegate;

protected:
    void drawIndicator(QPainter* painter, const QStyleOptionViewItem& option,
                       const QModelIndex& index) const override;
};

class CheckIndicatorMenuItemDelegate : public CheckableMenuItemDelegate {
    Q_OBJECT

public:
    using CheckableMenuItemDelegate::CheckableMenuItemDelegate;

protected:
    void drawIndicator(QPainter* painter, const QStyleOptionViewItem& option,
                       const QModelIndex& index) const override;
};

enum class MenuIndicatorType { Check = 0, Radio = 1 };

MenuItemDelegate* createCheckableMenuItemDelegate(MenuIndicatorType style,
                                                  QObject* parent = nullptr);

class MenuActionListWidget : public QListWidget {
    Q_OBJECT

public:
    explicit MenuActionListWidget(QWidget* parent = nullptr);

    void setViewportMargins(int left, int top, int right, int bottom);
    void setViewportMargins(const QMargins& margins);
    QMargins menuViewportMargins() const { return viewportMargins_; }

    void setItemHeight(int height);

    void setMaxVisibleItems(int num);
    int maxVisibleItems() const;

    int itemsHeight() const;
    int heightForAnimation(const QPoint& pos, MenuAnimationType aniType) const;

    void adjustSizeForMenu(const QPoint& pos = QPoint(),
                           MenuAnimationType aniType = MenuAnimationType::None);

    void insertItem(int row, QListWidgetItem* item);
    void addItem(QListWidgetItem* item);
    QListWidgetItem* takeItem(int row);

private:
    int itemHeight_ = 28;
    int maxVisibleItems_ = -1;
    QPointer<SmoothScrollDelegate> scrollDelegate_;
    QMargins viewportMargins_;
};

class RoundMenu : public QMenu {
    Q_OBJECT

public:
    explicit RoundMenu(const QString& title = QString(), QWidget* parent = nullptr);

    QPointer<MenuActionListWidget> view() const { return view_; }
    void adjustSize();

    QSize sizeHint() const override;
    void popup(const QPoint& pos, QAction* at = nullptr);

    void setMaxVisibleItems(int num);
    void setItemHeight(int height);

    void setShadowEffect(int blurRadius = 30, const QPoint& offset = QPoint(0, 8),
                         const QColor& color = QColor(0, 0, 0, 30));

    void clear();

    void setIcon(const QIcon& icon);
    QIcon icon() const;

    void setTitle(const QString& title);
    QString title() const;

    void addAction(QAction* action);
    void insertAction(QAction* before, QAction* action);
    void addActions(const QList<QAction*>& actions);
    void insertActions(QAction* before, const QList<QAction*>& actions);
    void removeAction(QAction* action);

    void addWidget(QWidget* widget, bool selectable = true,
                   const std::function<void()>& onClick = nullptr);

    void addMenu(RoundMenu* menu);
    void insertMenu(QAction* before, RoundMenu* menu);
    void removeMenu(RoundMenu* menu);

    void addSeparator();

    QList<QAction*> menuActions() const;

    virtual void execAt(const QPoint& pos, bool ani = true,
                        MenuAnimationType aniType = MenuAnimationType::DropDown);

signals:
    void closedSignal();

protected:
    void hideEvent(QHideEvent* e) override;
    void closeEvent(QCloseEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;

private slots:
    void onItemClicked(QListWidgetItem* item);
    void onItemEntered(QListWidgetItem* item);
    void onShowMenuTimeout();
    void onActionChanged();

private:
    friend class SubMenuItemWidget;
    friend class MenuAnimationManager;
    friend class CheckableMenu;
    friend class ComboBoxBase;
    friend class ComboBoxMenu;

    void initWidgets();

    void setParentMenu(RoundMenu* parent, QListWidgetItem* item);

    QListWidgetItem* createActionItem(QAction* action, QAction* before = nullptr);
    QPair<QListWidgetItem*, QWidget*> createSubMenuItem(RoundMenu* menu);

    QIcon createItemIcon(QObject* w) const;
    bool hasItemIcon() const;
    int adjustItemText(QListWidgetItem* item, QAction* action);
    int longestShortcutWidth() const;

    void removeItem(QListWidgetItem* item);

    void showSubMenu(QListWidgetItem* item);
    void hideMenu(bool isHideBySystem);
    void closeParentMenu();

    QIcon icon_;
    QString title_;

    QList<QPointer<QAction>> actions_;
    QList<QPointer<RoundMenu>> subMenus_;

    bool isSubMenu_ = false;
    QPointer<RoundMenu> parentMenu_;
    QListWidgetItem* menuItem_ = nullptr;
    QListWidgetItem* lastHoverItem_ = nullptr;
    QListWidgetItem* lastHoverSubMenuItem_ = nullptr;
    bool isHideBySystem_ = true;
    int itemHeight_ = 28;

    QPointer<QTimer> timer_;
    QPointer<QHBoxLayout> hBoxLayout_;
    QPointer<QWidget> panel_;
    QPointer<QHBoxLayout> panelLayout_;
    QPointer<MenuActionListWidget> view_;
    QPointer<QGraphicsDropShadowEffect> shadowEffect_;

    MenuAnimationManager* aniManager_ = nullptr;
};

class CheckableMenu : public RoundMenu {
    Q_OBJECT

public:
    explicit CheckableMenu(const QString& title = QString(), QWidget* parent = nullptr,
                           MenuIndicatorType indicatorType = MenuIndicatorType::Check);

    void addAction(QAction* action);
    void addActions(const QList<QAction*>& actions);
    int adjustItemText(QListWidgetItem* item, QAction* action);

    void execAt(const QPoint& pos, bool ani = true,
                MenuAnimationType aniType = MenuAnimationType::DropDown) override;
};

class SystemTrayMenu : public RoundMenu {
    Q_OBJECT

public:
    explicit SystemTrayMenu(const QString& title = QString(), QWidget* parent = nullptr);

    QSize sizeHint() const override;
};

class CheckableSystemTrayMenu : public CheckableMenu {
    Q_OBJECT

public:
    explicit CheckableSystemTrayMenu(const QString& title = QString(), QWidget* parent = nullptr,
                                     MenuIndicatorType indicatorType = MenuIndicatorType::Check);

    QSize sizeHint() const override;
};

class LabelContextMenu : public RoundMenu {
    Q_OBJECT

public:
    explicit LabelContextMenu(QLabel* parent);

    QLabel* label() const;

    void execAt(const QPoint& pos, bool ani = true,
                MenuAnimationType aniType = MenuAnimationType::DropDown) override;

private slots:
    void onCopy();
    void onSelectAll();

private:
    QString selectedText_;
    QAction* copyAct_ = nullptr;
    QAction* selectAllAct_ = nullptr;
};

class MenuAnimationManager : public QObject {
    Q_OBJECT

public:
    explicit MenuAnimationManager(RoundMenu* menu);

    virtual void exec(const QPoint& pos) = 0;
    virtual QSize availableViewSize(const QPoint& pos) const;

protected:
    QPoint endPosition(const QPoint& pos) const;
    QSize menuSize() const;
    void updateMenuViewport();

    QPointer<RoundMenu> menu_;
    QPropertyAnimation* posAni_ = nullptr;

protected slots:
    virtual void onValueChanged();
};

class EditMenu : public RoundMenu {
    Q_OBJECT

public:
    explicit EditMenu(QWidget* parent = nullptr);

protected:
    QAction* undoAct_ = nullptr;
    QAction* redoAct_ = nullptr;
    QAction* cutAct_ = nullptr;
    QAction* copyAct_ = nullptr;
    QAction* pasteAct_ = nullptr;
    QAction* deleteAct_ = nullptr;
    QAction* selectAllAct_ = nullptr;

    void createActions();
};

class LineEditMenu : public EditMenu {
    Q_OBJECT

public:
    explicit LineEditMenu(::QLineEdit* parent);

private slots:
    void updateActions();

private:
    QPointer<::QLineEdit> edit_;
    bool undoAvailable_ = false;
    bool redoAvailable_ = false;
};

class TextEditMenu : public EditMenu {
    Q_OBJECT

public:
    explicit TextEditMenu(::QTextEdit* parent);

private slots:
    void updateActions();

private:
    QPointer<::QTextEdit> edit_;
};

class PlainTextEditMenu : public EditMenu {
    Q_OBJECT

public:
    explicit PlainTextEditMenu(::QPlainTextEdit* parent);

private slots:
    void updateActions();

private:
    QPointer<::QPlainTextEdit> edit_;
};

class DummyMenuAnimationManager : public MenuAnimationManager {
    Q_OBJECT

public:
    using MenuAnimationManager::MenuAnimationManager;
    void exec(const QPoint& pos) override;
};

class DropDownMenuAnimationManager : public MenuAnimationManager {
    Q_OBJECT

public:
    using MenuAnimationManager::MenuAnimationManager;
    void exec(const QPoint& pos) override;
    QSize availableViewSize(const QPoint& pos) const override;

protected slots:
    void onValueChanged() override;
};

class PullUpMenuAnimationManager : public MenuAnimationManager {
    Q_OBJECT

public:
    using MenuAnimationManager::MenuAnimationManager;
    void exec(const QPoint& pos) override;
    QSize availableViewSize(const QPoint& pos) const override;

protected:
    QPoint endPositionPullUp(const QPoint& pos) const;

protected slots:
    void onValueChanged() override;
};

class FadeInDropDownMenuAnimationManager : public DropDownMenuAnimationManager {
    Q_OBJECT

public:
    explicit FadeInDropDownMenuAnimationManager(RoundMenu* menu);
    void exec(const QPoint& pos) override;

private:
    QPropertyAnimation* opacityAni_ = nullptr;
    QParallelAnimationGroup* group_ = nullptr;
};

class FadeInPullUpMenuAnimationManager : public PullUpMenuAnimationManager {
    Q_OBJECT

public:
    explicit FadeInPullUpMenuAnimationManager(RoundMenu* menu);
    void exec(const QPoint& pos) override;

private:
    QPropertyAnimation* opacityAni_ = nullptr;
    QParallelAnimationGroup* group_ = nullptr;
};

}  // namespace qfw
