#include "components/navigation/navigation_interface.h"

#include <QEvent>
#include <QResizeEvent>

#include "common/icon.h"
#include "components/navigation/navigation_widget.h"

namespace qfw {

NavigationInterface::NavigationInterface(QWidget* parent, bool showMenuButton,
                                         bool showReturnButton, bool collapsible)
    : QWidget(parent), panel_(new NavigationPanel(this)) {
    panel_->setMenuButtonVisible(showMenuButton && collapsible);
    panel_->setReturnButtonVisible(showReturnButton);
    panel_->setCollapsible(collapsible);

    panel_->installEventFilter(this);
    connect(panel_, &NavigationPanel::displayModeChanged, this,
            &NavigationInterface::displayModeChanged);

    resize(48, height());
    setMinimumWidth(48);
    setAttribute(Qt::WA_TranslucentBackground);
}

NavigationTreeWidget* NavigationInterface::addItem(const QString& routeKey, const QVariant& icon,
                                                   const QString& text,
                                                   const std::function<void()>& onClick,
                                                   bool selectable, NavigationItemPosition position,
                                                   const QString& tooltip,
                                                   const QString& parentRouteKey) {
    return insertItem(-1, routeKey, icon, text, onClick, selectable, position, tooltip,
                      parentRouteKey);
}

void NavigationInterface::addWidget(const QString& routeKey, NavigationWidget* widget,
                                    const std::function<void()>& onClick,
                                    NavigationItemPosition position, const QString& tooltip,
                                    const QString& parentRouteKey) {
    insertWidget(-1, routeKey, widget, onClick, position, tooltip, parentRouteKey);
}

NavigationTreeWidget* NavigationInterface::insertItem(
    int index, const QString& routeKey, const QVariant& icon, const QString& text,
    const std::function<void()>& onClick, bool selectable, NavigationItemPosition position,
    const QString& tooltip, const QString& parentRouteKey) {
    if (!panel_) {
        return nullptr;
    }

    NavigationTreeWidget* w = panel_->insertItem(index, routeKey, icon, text, onClick, selectable,
                                                 position, tooltip, parentRouteKey);
    setMinimumHeight(panel_->layoutMinHeight());
    return w;
}

void NavigationInterface::insertWidget(int index, const QString& routeKey, NavigationWidget* widget,
                                       const std::function<void()>& onClick,
                                       NavigationItemPosition position, const QString& tooltip,
                                       const QString& parentRouteKey) {
    if (!panel_) {
        return;
    }

    panel_->insertWidget(index, routeKey, widget, onClick, position, tooltip, parentRouteKey);
    setMinimumHeight(panel_->layoutMinHeight());
}

void NavigationInterface::addSeparator(NavigationItemPosition position) {
    insertSeparator(-1, position);
}

NavigationItemHeader* NavigationInterface::addItemHeader(const QString& text,
                                                         NavigationItemPosition position) {
    return panel_ ? panel_->addItemHeader(text, position) : nullptr;
}

NavigationItemHeader* NavigationInterface::insertItemHeader(int index, const QString& text,
                                                            NavigationItemPosition position) {
    return panel_ ? panel_->insertItemHeader(index, text, position) : nullptr;
}

NavigationUserCard* NavigationInterface::addUserCard(
    const QString& routeKey, const QVariant& avatar, const QString& title, const QString& subtitle,
    const std::function<void()>& onClick, NavigationItemPosition position, bool aboveMenuButton) {
    if (!panel_) {
        return nullptr;
    }

    auto* card = new NavigationUserCard(this);

    if (avatar.isValid()) {
        if (avatar.canConvert<const FluentIconBase*>()) {
            card->setAvatarIcon(avatar);
        } else if (avatar.canConvert<QIcon>()) {
            card->setAvatar(avatar.value<QIcon>().pixmap(64, 64));
        } else if (avatar.canConvert<QString>()) {
            card->setAvatar(avatar.toString());
        }
    }

    card->setTitle(title);
    card->setSubtitle(subtitle);

    int insertIndex = -1;
    if (aboveMenuButton && position == NavigationItemPosition::Top) {
        if (auto* layout = panel_->topLayout()) {
            for (int i = 0; i < layout->count(); ++i) {
                auto* it = layout->itemAt(i);
                if (it && it->widget() == panel_->menuButton()) {
                    insertIndex = i;
                    break;
                }
            }
        }
    }

    if (insertIndex >= 0) {
        panel_->insertWidget(insertIndex, routeKey, card, onClick, position);
    } else {
        panel_->addWidget(routeKey, card, onClick, position);
    }

    setMinimumHeight(panel_->layoutMinHeight());
    return card;
}

void NavigationInterface::insertSeparator(int index, NavigationItemPosition position) {
    if (!panel_) {
        return;
    }

    panel_->insertSeparator(index, position);
    setMinimumHeight(panel_->layoutMinHeight());
}

void NavigationInterface::removeWidget(const QString& routeKey) {
    if (panel_) {
        panel_->removeWidget(routeKey);
    }
}

void NavigationInterface::setCurrentItem(const QString& routeKey) {
    if (panel_) {
        panel_->setCurrentItem(routeKey);
    }
}

void NavigationInterface::expand(bool useAni) {
    if (panel_) {
        panel_->expand(useAni);
    }
}

void NavigationInterface::toggle() {
    if (panel_) {
        panel_->toggle();
    }
}

void NavigationInterface::setExpandWidth(int width) {
    if (panel_) {
        panel_->setExpandWidth(width);
    }
}

void NavigationInterface::setMinimumExpandWidth(int width) {
    if (panel_) {
        panel_->setMinimumExpandWidth(width);
    }
}

void NavigationInterface::setMenuModeEnabledWhenNarrow(bool enabled) {
    if (panel_) {
        panel_->setMenuModeEnabledWhenNarrow(enabled);
    }
}

bool NavigationInterface::isMenuModeEnabledWhenNarrow() const {
    return panel_ ? panel_->isMenuModeEnabledWhenNarrow() : true;
}

void NavigationInterface::setMenuButtonVisible(bool visible) {
    if (panel_) {
        panel_->setMenuButtonVisible(visible);
    }
}

void NavigationInterface::setReturnButtonVisible(bool visible) {
    if (panel_) {
        panel_->setReturnButtonVisible(visible);
    }
}

void NavigationInterface::setCollapsible(bool collapsible) {
    if (panel_) {
        panel_->setCollapsible(collapsible);
    }
}

bool NavigationInterface::isAcrylicEnabled() const {
    return panel_ ? panel_->isAcrylicEnabled() : false;
}

void NavigationInterface::setAcrylicEnabled(bool enabled) {
    if (panel_) {
        panel_->setAcrylicEnabled(enabled);
    }
}

bool NavigationInterface::isIndicatorAnimationEnabled() const {
    return panel_ ? panel_->isIndicatorAnimationEnabled() : false;
}

void NavigationInterface::setIndicatorAnimationEnabled(bool enabled) {
    if (panel_) {
        panel_->setIndicatorAnimationEnabled(enabled);
    }
}

bool NavigationInterface::isUpdateIndicatorPosOnCollapseFinished() const {
    return panel_ ? panel_->isUpdateIndicatorPosOnCollapseFinished() : false;
}

void NavigationInterface::setUpdateIndicatorPosOnCollapseFinished(bool update) {
    if (panel_) {
        panel_->setUpdateIndicatorPosOnCollapseFinished(update);
    }
}

NavigationWidget* NavigationInterface::widget(const QString& routeKey) const {
    return panel_ ? panel_->widget(routeKey) : nullptr;
}

Router* NavigationInterface::history() const { return panel_ ? panel_->history() : nullptr; }

bool NavigationInterface::eventFilter(QObject* obj, QEvent* e) {
    if (obj != panel_ || !e || e->type() != QEvent::Resize) {
        return QWidget::eventFilter(obj, e);
    }

    if (panel_->displayMode() != NavigationDisplayMode::Menu) {
        auto* event = static_cast<QResizeEvent*>(e);
        if (event->oldSize().width() != event->size().width()) {
            setFixedWidth(event->size().width());
        }
    }

    return QWidget::eventFilter(obj, e);
}

void NavigationInterface::resizeEvent(QResizeEvent* e) {
    QWidget::resizeEvent(e);

    if (!panel_ || !e) {
        return;
    }

    if (e->oldSize().height() != height()) {
        panel_->setFixedHeight(height());
    }
}

}  // namespace qfw
