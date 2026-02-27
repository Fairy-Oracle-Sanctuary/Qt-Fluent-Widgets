#pragma once

#include <QColor>
#include <QFrame>
#include <QMap>
#include <QObject>
#include <QPointer>
#include <QPropertyAnimation>
#include <QRect>
#include <QString>
#include <QVBoxLayout>
#include <QVariant>
#include <functional>

#include "common/router.h"
#include "components/navigation/navigation_widget.h"
#include "components/widgets/acrylic_label.h"
#include "components/widgets/scroll_area.h"

namespace qfw {

class NavigationWidget;
class NavigationTreeWidget;
class NavigationTreeWidgetBase;
class NavigationSeparator;
class NavigationItemHeader;
class NavigationFlyoutMenu;
class NavigationIndicator;

enum class NavigationDisplayMode {
    Minimal = 0,
    Compact = 1,
    Expand = 2,
    Menu = 3,
};

enum class NavigationItemPosition {
    Top = 0,
    Scroll = 1,
    Bottom = 2,
};

class NavigationItemLayout : public QVBoxLayout {
public:
    using QVBoxLayout::QVBoxLayout;

    void setGeometry(const QRect& rect) override;
};

struct NavigationItem {
    QString routeKey;
    QString parentRouteKey;
    QPointer<NavigationWidget> widget;
};

class NavigationPanel : public QFrame {
    Q_OBJECT

public:
    explicit NavigationPanel(QWidget* parent = nullptr, bool isMinimalEnabled = false);

    NavigationDisplayMode displayMode() const { return displayMode_; }

    bool isMinimalEnabled() const { return isMinimalEnabled_; }

    bool isIndicatorAnimationEnabled() const { return isIndicatorAnimationEnabled_; }
    void setIndicatorAnimationEnabled(bool enabled);

    bool isUpdateIndicatorPosOnCollapseFinished() const {
        return isUpdateIndicatorPosOnCollapseFinished_;
    }
    void setUpdateIndicatorPosOnCollapseFinished(bool update);

    void setMenuButtonVisible(bool visible);
    void setReturnButtonVisible(bool visible);

    void setCollapsible(bool on);

    void setExpandWidth(int width);
    void setMinimumExpandWidth(int width);

    bool isMenuModeEnabledWhenNarrow() const { return isMenuModeEnabledWhenNarrow_; }
    void setMenuModeEnabledWhenNarrow(bool enabled) { isMenuModeEnabledWhenNarrow_ = enabled; }

    void setAcrylicEnabled(bool enabled);
    bool isAcrylicEnabled() const { return isAcrylicEnabled_; }

    NavigationWidget* widget(const QString& routeKey) const;

    Router* history() const { return history_; }

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
    void insertSeparator(int index, NavigationItemPosition position = NavigationItemPosition::Top);

    NavigationItemHeader* addItemHeader(
        const QString& text, NavigationItemPosition position = NavigationItemPosition::Top);
    NavigationItemHeader* insertItemHeader(
        int index, const QString& text,
        NavigationItemPosition position = NavigationItemPosition::Top);

    void removeWidget(const QString& routeKey);

    void expand(bool useAni = true);
    void collapse();
    void toggle();

    bool isCollapsed() const { return displayMode_ == NavigationDisplayMode::Compact; }

    void setCurrentItem(const QString& routeKey);
    NavigationWidget* currentItem() const;

    int layoutMinHeight() const;

    NavigationItemLayout* topLayout() const { return topLayout_; }
    NavigationToolButton* menuButton() const { return menuButton_; }

signals:
    void displayModeChanged(qfw::NavigationDisplayMode mode);

protected:
    bool eventFilter(QObject* obj, QEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

private:
    void initWidget();
    void initLayout();

    void updateAcrylicColor();
    bool canDrawAcrylic() const;

    void registerWidget(const QString& routeKey, const QString& parentRouteKey,
                        NavigationWidget* widget, const std::function<void()>& onClick,
                        const QString& tooltip);

    void insertWidgetToLayout(int index, NavigationWidget* widget, NavigationItemPosition position);

    NavigationWidget* findIndicatorItem(NavigationWidget* item) const;
    QRectF getIndicatorRect(NavigationWidget* item) const;

    void stopIndicatorAnimation();
    void restoreTreeExpandState(bool useAni = true);
    void setWidgetCompacted(bool compacted);

    void showFlyoutNavigationMenu(NavigationTreeWidget* widget);

private slots:
    void onWidgetClicked(bool triggeredByUser);
    void onExpandAniFinished();
    void onIndicatorAniFinished();

private:
    QWidget* parentWidget_ = nullptr;

    bool isMenuButtonVisible_ = true;
    bool isReturnButtonVisible_ = false;
    bool isCollapsible_ = true;
    bool isAcrylicEnabled_ = false;

    bool isIndicatorAnimationEnabled_ = true;
    bool isUpdateIndicatorPosOnCollapseFinished_ = false;

    QPointer<NavigationIndicator> indicator_;

    AcrylicBrush acrylicBrush_;

    QPointer<ScrollArea> scrollArea_;
    QPointer<QWidget> scrollWidget_;

    QPointer<NavigationToolButton> menuButton_;
    QPointer<NavigationToolButton> returnButton_;

    QPointer<NavigationItemLayout> vBoxLayout_;
    QPointer<NavigationItemLayout> topLayout_;
    QPointer<NavigationItemLayout> bottomLayout_;
    QPointer<NavigationItemLayout> scrollLayout_;

    QMap<QString, NavigationItem> items_;
    QPointer<Router> history_;
    QString currentRouteKey_;

    QPropertyAnimation* expandAni_ = nullptr;
    int expandWidth_ = 322;
    int minimumExpandWidth_ = 1008;

    bool isMenuModeEnabledWhenNarrow_ = true;

    bool isMinimalEnabled_ = false;
    NavigationDisplayMode displayMode_ = NavigationDisplayMode::Compact;
};

}  // namespace qfw
