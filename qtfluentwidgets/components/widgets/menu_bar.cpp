#include "components/widgets/menu_bar.h"

#include <QAbstractButton>
#include <QActionEvent>
#include <QApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QPainter>
#include <QScreen>
#include <QShortcut>
#include <QShortcutEvent>
#include <stdexcept>

#include "common/config.h"
#include "common/font.h"

namespace qfw {
namespace {
QChar mnemonic(const QString& text) {
    for (int i = 0; i + 1 < text.size(); ++i) {
        if (text[i] != '&') continue;
        if (text[i + 1] != '&') return text[i + 1].toCaseFolded();
        ++i;
    }
    return {};
}
}  // namespace

class MenuBarButton : public QAbstractButton {
public:
    MenuBarButton(QAction* action, MenuBar* bar)
        : QAbstractButton(bar), action_(action), bar_(bar) {
        setFont(getFont(14));
        setFocusPolicy(Qt::NoFocus);
        setCursor(Qt::PointingHandCursor);
        connect(this, &QAbstractButton::clicked, bar,
                [bar, action] { bar->activateAction(action); });
        connect(action, &QAction::changed, this, [this] { sync(); });
        connect(&QConfig::instance(), &QConfig::themeChangedFinished, this, [this] { update(); });
        sync();
    }
    QSize sizeHint() const override {
        return QSize(action_ && action_->isSeparator()
                         ? 9
                         : fontMetrics().size(Qt::TextShowMnemonic, text()).width() + 16,
                     32);
    }
    void sync() {
        if (!action_) return;
        setText(action_->text());
        setEnabled(action_->isEnabled() && !action_->isSeparator());
        setVisible(action_->isVisible());
        setFixedSize(sizeHint());
        setAccessibleName(QString(text()).remove('&'));
        bar_->actionChanged(action_);
        update();
    }

protected:
    bool event(QEvent* event) override {
        if (event->type() == QEvent::Shortcut &&
            !static_cast<QShortcutEvent*>(event)->isAmbiguous()) {
            bar_->activateAction(action_, true);
            return true;
        }
        if (event->type() == QEvent::Enter || event->type() == QEvent::Leave) update();
        return QAbstractButton::event(event);
    }
    void paintEvent(QPaintEvent*) override {
        if (!action_) return;
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        const int shade = isDarkTheme() ? 255 : 0;
        if (action_->isSeparator()) {
            painter.setPen(QColor(shade, shade, shade, 30));
            painter.drawLine(4, 8, 4, 24);
            return;
        }
        const bool selected = (bar_->activeMenu() && bar_->activeMenu() == action_->menu()) ||
                              (bar_->hasFocus() && bar_->keyboardAction_ == action_);
        if (isEnabled() && (underMouse() || isDown() || selected)) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(shade, shade, shade, isDown() ? 18 : 9));
            painter.drawRoundedRect(QRectF(rect()), 5, 5);
        }
        painter.setFont(font());
        painter.setPen(isDarkTheme() ? Qt::white : Qt::black);
        if (!isEnabled()) painter.setOpacity(.4);
        painter.drawText(
            rect(),
            Qt::AlignCenter | (bar_->hasFocus() ? Qt::TextShowMnemonic : Qt::TextHideMnemonic),
            text());
    }

private:
    QPointer<QAction> action_;
    MenuBar* bar_;
};

MenuBar::MenuBar(QWidget* parent) : QWidget(parent) {
    setFixedHeight(32);
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    row_ = new QHBoxLayout(this);
    row_->setContentsMargins(0, 0, 0, 0);
    row_->setSpacing(0);
    row_->addStretch();
    shortcutHost_ = new QWidget(this);
    shortcutHost_->setFixedSize(0, 0);
    shortcutHost_->setFocusPolicy(Qt::NoFocus);
    shortcutHost_->show();
    auto* shortcut = new QShortcut(QKeySequence(Qt::Key_F10), this);
    connect(shortcut, &QShortcut::activated, this, &MenuBar::focusBar);
}
MenuBar::~MenuBar() {
    destroying_ = true;
    for (auto connection : connections_) disconnect(connection);
    for (auto* menu : watchedMenus_) menu->removeEventFilter(this);
    closeActiveMenu(false);
}
RoundMenu* MenuBar::addMenu(const QString& title) { return insertMenu(nullptr, title); }
RoundMenu* MenuBar::addMenu(RoundMenu* menu) { return insertMenu(nullptr, menu); }
RoundMenu* MenuBar::insertMenu(QAction* before, const QString& title) {
    if (before && !actions().contains(before)) throw std::invalid_argument("Unknown menu anchor");
    return insertMenu(before, new RoundMenu(title, this));
}
RoundMenu* MenuBar::insertMenu(QAction* before, RoundMenu* menu) {
    if (!menu || menu->isSubMenu_) throw std::invalid_argument("Expected a top-level RoundMenu");
    if (before && !actions().contains(before)) throw std::invalid_argument("Unknown menu anchor");
    insertAction(before, menu->menuAction());
    syncShortcuts();
    return menu;
}
QList<RoundMenu*> MenuBar::menus() const {
    QList<RoundMenu*> result;
    for (auto* action : actions())
        if (auto* menu = qobject_cast<RoundMenu*>(action->menu())) result.append(menu);
    return result;
}
void MenuBar::removeMenu(RoundMenu* menu) {
    if (menu) removeAction(menu->menuAction());
}
void MenuBar::clear() {
    closeActiveMenu();
    for (auto* action : actions()) removeAction(action);
}
void MenuBar::setActiveMenu(RoundMenu* menu) {
    if (!menu) {
        closeActiveMenu();
        return;
    }
    if (!menus().contains(menu)) throw std::invalid_argument("Menu does not belong to this bar");
    openMenu(menu, true);
}
void MenuBar::actionEvent(QActionEvent* event) {
    QWidget::actionEvent(event);
    auto* action = event->action();
    if (event->type() == QEvent::ActionAdded) {
        auto* button = new MenuBarButton(action, this);
        buttons_.insert(action, button);
        row_->insertWidget(actions().indexOf(action), button);
    } else if (event->type() == QEvent::ActionRemoved) {
        if (activeMenu_ && activeMenu_->menuAction() == action) closeActiveMenu();
        if (keyboardAction_ == action) keyboardAction_.clear();
        if (auto* button = buttons_.take(action)) {
            row_->removeWidget(button);
            button->hide();
            button->deleteLater();
        }
    }
    syncShortcuts();
    updateGeometry();
}
void MenuBar::actionChanged(QAction* action) {
    QPointer<MenuBar> guard(this);
    if (activeMenu_ && activeMenu_->menuAction() == action &&
        (!action->isEnabled() || !action->isVisible()))
        closeActiveMenu();
    if (guard) syncShortcuts();
}
void MenuBar::syncShortcuts() {
    if (syncing_) return;
    syncing_ = true;
    for (auto connection : connections_) disconnect(connection);
    connections_.clear();
    for (auto* menu : watchedMenus_) menu->removeEventFilter(this);
    watchedMenus_.clear();
    for (auto* action : shortcutHost_->actions()) shortcutHost_->removeAction(action);
    QSet<QAction*> leaves;
    std::function<void(RoundMenu*, bool)> visit = [&](RoundMenu* menu, bool enabled) {
        if (!menu || watchedMenus_.contains(menu)) return;
        watchedMenus_.insert(menu);
        menu->installEventFilter(this);
        enabled = enabled && menu->menuAction()->isEnabled() && menu->menuAction()->isVisible();
        connections_.append(
            connect(menu->menuAction(), &QAction::changed, this, [this] { syncShortcuts(); }));
        connections_.append(connect(menu, &RoundMenu::aboutToBeDestroyed, this, [this, menu] {
            if (actions().contains(menu->menuAction())) removeMenu(menu);
            else if (menu->parentMenu_) menu->parentMenu_->removeMenu(menu);
        }));
        connections_.append(connect(menu, &QObject::destroyed, this, [this, menu] {
            watchedMenus_.remove(menu);
            QTimer::singleShot(0, this, [this] { syncShortcuts(); });
        }));
        if (menu->view()) {
            auto* model = menu->view()->model();
            connections_.append(connect(model, &QAbstractItemModel::rowsInserted, this,
                                        [this] { syncShortcuts(); }));
            connections_.append(connect(model, &QAbstractItemModel::rowsRemoved, this,
                                        [this] { syncShortcuts(); }));
            connections_.append(
                connect(model, &QAbstractItemModel::modelReset, this, [this] { syncShortcuts(); }));
        }
        for (auto* action : menu->menuActions()) {
            leaves.insert(action);
            if (enabled) shortcutHost_->addAction(action);
        }
        for (auto child : menu->subMenus_) visit(child, enabled);
    };
    for (auto* action : actions()) {
        if (auto* menu = qobject_cast<RoundMenu*>(action->menu()))
            visit(menu, true);
        else if (!action->isSeparator())
            leaves.insert(action);
    }
    for (auto* action : leaves) {
        connections_.append(connect(action, &QAction::triggered, this, [this, action] {
            QPointer<MenuBar> guard(this);
            QPointer<QAction> command(action);
            closeActiveMenu();
            if (guard && command) emit triggered(command);
        }));
    }
    syncing_ = false;
}
void MenuBar::activateAction(QAction* action, bool keyboard) {
    if (!action || !action->isEnabled() || !action->isVisible() || action->isSeparator()) return;
    if (auto* menu = qobject_cast<RoundMenu*>(action->menu())) {
        if (menu == activeMenu_)
            closeActiveMenu();
        else
            openMenu(menu, keyboard);
    } else {
        QPointer<MenuBar> guard(this);
        QPointer<QAction> command(action);
        closeActiveMenu();
        if (guard && command) command->trigger();
    }
}
void MenuBar::openMenu(RoundMenu* menu, bool keyboard) {
    auto* button = buttons_.value(menu->menuAction());
    if (!isEnabled() || !isVisible() || !button || !button->isEnabled() || button->isHidden() ||
        menu == activeMenu_)
        return;
    QPointer<MenuBar> guard(this);
    QPointer<RoundMenu> popup(menu);
    QPointer<QWidget> previous =
        previousFocus_ ? previousFocus_.data() : QApplication::focusWidget();
    closeActiveMenu(false);
    if (!guard || !popup || activeMenu_) return;
    previousFocus_ = previous;
    activeMenu_ = menu;
    keyboardAction_ = menu->menuAction();
    qApp->installEventFilter(this);
    filterInstalled_ = true;
    syncShortcuts();
    const QPoint anchor = button->mapToGlobal(QPoint(12, button->height() + 3));
    menu->clearMask();
    menu->execAt(anchor, false, MenuAnimationType::None);
    QScreen* screen = button->screen();
    const QRect available = screen ? screen->availableGeometry() : QRect(anchor, menu->size());
    const auto margin = menu->layout()->contentsMargins();
    int x = qBound(available.left(), anchor.x() - margin.left(),
                   qMax(available.left(), available.right() - menu->width() + 1));
    int y = anchor.y() - 4;
    if (y + menu->height() > available.bottom() + 1)
        y = button->mapToGlobal(QPoint()).y() - menu->height() + margin.bottom() - 7;
    y = qBound(available.top(), y, qMax(available.top(), available.bottom() - menu->height() + 1));
    menu->move(x, y);
    menu->view()->setFocus(Qt::PopupFocusReason);
    menu->view()->setCurrentRow(-1);
    if (keyboard) stepRow(menu, 1);
    updateButtons();
    emit activeMenuChanged(menu);
}
void MenuBar::closeTree(RoundMenu* menu) {
    if (!menu) return;
    QPointer<RoundMenu> guard(menu);
    const auto children = menu->subMenus_;
    for (auto child : children) {
        closeTree(child);
        if (!guard) return;
    }
    if (menu->timer_) menu->timer_->stop();
    // Stop pending popup animations, including opacity groups and position animations.
    for (auto* animation : menu->findChildren<QAbstractAnimation*>()) animation->stop();
    menu->isHideBySystem_ = false;
    menu->close();
}
void MenuBar::closeActiveMenu(bool restoreFocus) {
    QPointer<MenuBar> guard(this);
    QPointer<RoundMenu> menu = activeMenu_;
    activeMenu_.clear();
    if (filterInstalled_) {
        qApp->removeEventFilter(this);
        filterInstalled_ = false;
    }
    if (menu) {
        closeTree(menu);
        if (!guard) return;
        keyboardAction_.clear();
    }
    updateButtons();
    QPointer<QWidget> previous = previousFocus_;
    previousFocus_.clear();
    if (restoreFocus && previous && previous->isVisible() && previous->isEnabled())
        previous->setFocus(Qt::PopupFocusReason);
    if (guard && menu && !destroying_) emit activeMenuChanged(nullptr);
}
void MenuBar::updateButtons() {
    for (auto* button : buttons_) button->update();
}
QList<QAction*> MenuBar::enabledActions() const {
    QList<QAction*> result;
    for (auto* action : actions())
        if (action->isEnabled() && action->isVisible() && !action->isSeparator())
            result.append(action);
    return result;
}
void MenuBar::focusBar() {
    const auto list = enabledActions();
    if (!isEnabled() || !isVisible() || list.isEmpty()) return;
    if (hasFocus()) {
        closeActiveMenu();
        clearFocus();
    } else {
        previousFocus_ = QApplication::focusWidget();
        keyboardAction_ = list.first();
        setFocus(Qt::ShortcutFocusReason);
    }
    updateButtons();
}
void MenuBar::moveHeader(int step, bool open) {
    const auto list = enabledActions();
    if (list.isEmpty()) return;
    int index = (list.indexOf(keyboardAction_) + step + list.size()) % list.size();
    QPointer<QAction> action = list[index];
    keyboardAction_ = action;
    if (open) {
        if (auto* menu = qobject_cast<RoundMenu*>(action->menu()))
            openMenu(menu, true);
        else {
            closeActiveMenu(false);
            keyboardAction_ = action;
            setFocus(Qt::PopupFocusReason);
        }
    }
    updateButtons();
}
void MenuBar::stepRow(RoundMenu* menu, int step) {
    auto* view = menu->view().data();
    if (!view || !view->count()) return;
    int row = view->currentRow();
    if (row < 0 && step < 0) row = view->count();
    for (int i = 0; i < view->count(); ++i) {
        row = (row + step + view->count()) % view->count();
        auto* item = view->item(row);
        if (!item->isHidden() && item->flags().testFlag(Qt::ItemIsEnabled) &&
            item->flags().testFlag(Qt::ItemIsSelectable)) {
            view->setCurrentRow(row);
            return;
        }
    }
}
RoundMenu* MenuBar::currentPopup(QWidget* widget) const {
    for (auto* menu : watchedMenus_)
        if (menu->isVisible() && (widget == menu || menu->isAncestorOf(widget))) return menu;
    auto* popup = qobject_cast<RoundMenu*>(QApplication::activePopupWidget());
    return watchedMenus_.contains(popup) ? popup : activeMenu_.data();
}
bool MenuBar::openSubMenu(RoundMenu* menu, QListWidgetItem* item) {
    if (!item || !item->flags().testFlag(Qt::ItemIsEnabled)) return false;
    auto* widget = qobject_cast<SubMenuItemWidget*>(menu->view()->itemWidget(item));
    if (!widget || !widget->menu()) return false;
    menu->showSubMenu(item);
    menu->timer_->stop();
    menu->onShowMenuTimeout();
    auto* child = widget->menu();
    child->view()->setFocus(Qt::PopupFocusReason);
    child->view()->setCurrentRow(-1);
    stepRow(child, 1);
    return true;
}
bool MenuBar::handleKey(QKeyEvent* event, RoundMenu* menu) {
    const int key = event->key();
    if (menu) {
        if (key == Qt::Key_F10) {
            closeActiveMenu();
            return true;
        }
        if (key == Qt::Key_Escape || (key == Qt::Key_Left && menu->parentMenu_)) {
            if (menu->parentMenu_) {
                auto parent = menu->parentMenu_;
                closeTree(menu);
                if (parent && parent->view()) parent->view()->setFocus(Qt::PopupFocusReason);
            } else
                closeActiveMenu();
            return true;
        }
        if (key == Qt::Key_Return || key == Qt::Key_Enter || key == Qt::Key_Space ||
            key == Qt::Key_Right) {
            auto* item = menu->view()->currentItem();
            if (openSubMenu(menu, item)) return true;
            if (key != Qt::Key_Right) {
                if (item) menu->onItemClicked(item);
                return true;
            }
        }
        if (key == Qt::Key_Down || key == Qt::Key_Up) {
            stepRow(menu, key == Qt::Key_Down ? 1 : -1);
            return true;
        }
        if (key == Qt::Key_Home || key == Qt::Key_End) {
            menu->view()->setCurrentRow(-1);
            stepRow(menu, key == Qt::Key_Home ? 1 : -1);
            return true;
        }
        if (event->modifiers() & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier)) {
            const QKeySequence sequence(key | int(event->modifiers()));
            for (auto* action : shortcutHost_->actions()) {
                if (action->isEnabled() && action->isVisible() &&
                    action->shortcuts().contains(sequence)) {
                    closeActiveMenu();
                    action->trigger();
                    return true;
                }
            }
        }
    }
    if (key == Qt::Key_Left || key == Qt::Key_Right) {
        moveHeader(key == Qt::Key_Left ? -1 : 1, menu);
        return true;
    }
    if (event->modifiers().testFlag(Qt::AltModifier) && !event->text().isEmpty()) {
        for (auto* action : enabledActions())
            if (mnemonic(action->text()) == event->text().at(0).toCaseFolded()) {
                activateAction(action, true);
                return true;
            }
    }
    if (!menu && (key == Qt::Key_Down || key == Qt::Key_Return || key == Qt::Key_Enter ||
                  key == Qt::Key_Space)) {
        activateAction(keyboardAction_, true);
        return true;
    }
    if (!menu && key == Qt::Key_Escape) {
        keyboardAction_.clear();
        clearFocus();
        auto previous = previousFocus_;
        previousFocus_.clear();
        if (previous && previous->isVisible() && previous->isEnabled()) previous->setFocus();
        updateButtons();
        return true;
    }
    return false;
}
void MenuBar::keyPressEvent(QKeyEvent* event) {
    if (handleKey(event))
        event->accept();
    else
        QWidget::keyPressEvent(event);
}
bool MenuBar::eventFilter(QObject* object, QEvent* event) {
    if (auto* menu = qobject_cast<RoundMenu*>(object)) {
        if (watchedMenus_.contains(menu) && event->type() == QEvent::Hide && menu == activeMenu_)
            closeActiveMenu();
    }
    if (filterInstalled_) {
        const auto type = event->type();
        if (object == window() && (type == QEvent::Move || type == QEvent::Resize ||
                                   type == QEvent::Hide || type == QEvent::Close))
            closeActiveMenu();
        if (type == QEvent::MouseMove || type == QEvent::MouseButtonPress ||
            type == QEvent::MouseButtonRelease) {
            auto* mouse = static_cast<QMouseEvent*>(event);
            const QPoint point = mapFromGlobal(mouse->QMouseEvent_globalPosition_toPoint());
            for (auto* action : actions()) {
                auto* button = buttons_.value(action);
                if (!button || button->isHidden() || !button->geometry().contains(point)) continue;
                if (type == QEvent::MouseMove && action->isEnabled()) {
                    auto* menu = qobject_cast<RoundMenu*>(action->menu());
                    if (menu && menu != activeMenu_) openMenu(menu, false);
                } else if (type == QEvent::MouseButtonPress && mouse->button() == Qt::LeftButton)
                    activateAction(action);
                if (type != QEvent::MouseMove || action->menu() == activeMenu_) {
                    event->accept();
                    return true;
                }
            }
        }
        if (type == QEvent::KeyPress || type == QEvent::ShortcutOverride) {
            if (auto* widget = qobject_cast<QWidget*>(object)) {
                if (auto* menu = currentPopup(widget)) {
                    if (type == QEvent::ShortcutOverride ||
                        handleKey(static_cast<QKeyEvent*>(event), menu)) {
                        event->accept();
                        return true;
                    }
                }
            }
        }
    }
    return QWidget::eventFilter(object, event);
}
void MenuBar::hideEvent(QHideEvent* event) {
    closeActiveMenu();
    QWidget::hideEvent(event);
}
void MenuBar::changeEvent(QEvent* event) {
    if (!isEnabled()) closeActiveMenu();
    QWidget::changeEvent(event);
}
}  // namespace qfw
