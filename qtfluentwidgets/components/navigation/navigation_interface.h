#pragma once

#include <QObject>
#include <QPointer>
#include <QString>
#include <QWidget>

#include "components/navigation/navigation_panel.h"

namespace qfw {

class NavigationTreeWidget;
class NavigationWidget;
class NavigationUserCard;
class FluentIconBase;

class NavigationInterface : public QWidget {
    Q_OBJECT

public:
    explicit NavigationInterface(QWidget* parent = nullptr, bool showMenuButton = true,
                                 bool showReturnButton = false, bool collapsible = true);

    NavigationTreeWidget* addItem(const QString& routeKey, const QVariant& icon,
                                  const QString& text, const std::function<void()>& onClick = {},
                                  bool selectable = true,
                                  NavigationItemPosition position = NavigationItemPosition::Top,
                                  const QString& tooltip = QString(),
                                  const QString& parentRouteKey = QString());

    void addWidget(const QString& routeKey, NavigationWidget* widget,
                   const std::function<void()>& onClick = {},
                   NavigationItemPosition position = NavigationItemPosition::Top,
                   const QString& tooltip = QString(), const QString& parentRouteKey = QString());

    NavigationTreeWidget* insertItem(int index, const QString& routeKey, const QVariant& icon,
                                     const QString& text, const std::function<void()>& onClick = {},
                                     bool selectable = true,
                                     NavigationItemPosition position = NavigationItemPosition::Top,
                                     const QString& tooltip = QString(),
                                     const QString& parentRouteKey = QString());

    void insertWidget(int index, const QString& routeKey, NavigationWidget* widget,
                      const std::function<void()>& onClick = {},
                      NavigationItemPosition position = NavigationItemPosition::Top,
                      const QString& tooltip = QString(),
                      const QString& parentRouteKey = QString());

    void addSeparator(NavigationItemPosition position = NavigationItemPosition::Top);

    NavigationItemHeader* addItemHeader(
        const QString& text, NavigationItemPosition position = NavigationItemPosition::Top);

    NavigationItemHeader* insertItemHeader(
        int index, const QString& text,
        NavigationItemPosition position = NavigationItemPosition::Top);

    NavigationUserCard* addUserCard(const QString& routeKey, const QVariant& avatar = QVariant(),
                                    const QString& title = QString(),
                                    const QString& subtitle = QString(),
                                    const std::function<void()>& onClick = {},
                                    NavigationItemPosition position = NavigationItemPosition::Top,
                                    bool aboveMenuButton = false);

    void insertSeparator(int index, NavigationItemPosition position = NavigationItemPosition::Top);

    void removeWidget(const QString& routeKey);

    void setCurrentItem(const QString& routeKey);

    void expand(bool useAni = true);
    void toggle();

    void setExpandWidth(int width);
    void setMinimumExpandWidth(int width);

    void setMenuModeEnabledWhenNarrow(bool enabled);
    bool isMenuModeEnabledWhenNarrow() const;

    void setMenuButtonVisible(bool visible);
    void setReturnButtonVisible(bool visible);
    void setCollapsible(bool collapsible);

    bool isAcrylicEnabled() const;
    void setAcrylicEnabled(bool enabled);

    bool isIndicatorAnimationEnabled() const;
    void setIndicatorAnimationEnabled(bool enabled);

    bool isUpdateIndicatorPosOnCollapseFinished() const;
    void setUpdateIndicatorPosOnCollapseFinished(bool update);

    NavigationWidget* widget(const QString& routeKey) const;

    Router* history() const;

signals:
    void displayModeChanged(qfw::NavigationDisplayMode mode);

protected:
    bool eventFilter(QObject* obj, QEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;

private:
    QPointer<NavigationPanel> panel_;
};

}  // namespace qfw
