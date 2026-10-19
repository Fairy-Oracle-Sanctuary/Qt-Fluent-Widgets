#pragma once

#include <QHash>
#include <QPointer>
#include <QSet>
#include <QWidget>

#include "components/widgets/menu.h"

class QAbstractButton;
class QHBoxLayout;
class QKeyEvent;

namespace qfw {
class MenuBarButton;

// A compact, non-blocking menu bar. Removing menus does not delete them.
class MenuBar : public QWidget {
    Q_OBJECT
public:
    explicit MenuBar(QWidget* parent = nullptr);
    ~MenuBar() override;
    RoundMenu* addMenu(const QString& title);
    RoundMenu* addMenu(RoundMenu* menu);
    RoundMenu* insertMenu(QAction* before, const QString& title);
    RoundMenu* insertMenu(QAction* before, RoundMenu* menu);
    QList<RoundMenu*> menus() const;
    void removeMenu(RoundMenu* menu);
    void clear();
    RoundMenu* activeMenu() const { return activeMenu_; }
    void setActiveMenu(RoundMenu* menu);

signals:
    void triggered(QAction* action);
    void activeMenuChanged(qfw::RoundMenu* menu);

protected:
    void actionEvent(QActionEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    bool eventFilter(QObject* object, QEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    friend class MenuBarButton;
    void actionChanged(QAction* action);
    void syncShortcuts();
    void activateAction(QAction* action, bool keyboard = false);
    void openMenu(RoundMenu* menu, bool keyboard);
    void closeActiveMenu(bool restoreFocus = true);
    static void closeTree(RoundMenu* menu);
    void focusBar();
    QList<QAction*> enabledActions() const;
    void moveHeader(int step, bool open);
    static void stepRow(RoundMenu* menu, int step);
    bool handleKey(QKeyEvent* event, RoundMenu* menu = nullptr);
    RoundMenu* currentPopup(QWidget* widget) const;
    bool openSubMenu(RoundMenu* menu, QListWidgetItem* item);
    void updateButtons();

    QHBoxLayout* row_ = nullptr;
    QWidget* shortcutHost_ = nullptr;
    QHash<QAction*, QAbstractButton*> buttons_;
    QPointer<RoundMenu> activeMenu_;
    QPointer<QAction> keyboardAction_;
    QPointer<QWidget> previousFocus_;
    QSet<RoundMenu*> watchedMenus_;
    QList<QMetaObject::Connection> connections_;
    bool filterInstalled_ = false;
    bool syncing_ = false;
    bool destroying_ = false;
};
}  // namespace qfw
