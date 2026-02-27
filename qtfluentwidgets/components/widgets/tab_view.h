#pragma once

#include <QApplication>
#include <QBrush>
#include <QFontMetrics>
#include <QGraphicsDropShadowEffect>
#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPropertyAnimation>
#include <QStackedWidget>
#include <QUuid>
#include <QVBoxLayout>
#include <QVariant>

#include "button.h"
#include "scroll_area.h"

namespace qfw {

/**
 * @brief Tab close button display mode
 */
enum class TabCloseButtonDisplayMode { Always = 0, OnHover = 1, Never = 2 };

/**
 * @brief Check index decorator helper
 *
 * Returns true if index is valid (0 <= index < size), false otherwise.
 */
inline bool isIndexValid(int index, int size) { return 0 <= index && index < size; }

class TabBar;

/**
 * @brief Tab tool button
 */
class TabToolButton : public TransparentToolButton {
    Q_OBJECT

public:
    explicit TabToolButton(QWidget* parent = nullptr);
    explicit TabToolButton(FluentIconEnum icon, QWidget* parent = nullptr);

protected:
    void init();
    void paintEvent(QPaintEvent* event) override;
    void drawIcon(QPainter* painter, const QRectF& rect, QIcon::State state = QIcon::Off) override;

private:
    FluentIconEnum iconEnum_ = FluentIconEnum::Up;
};

/**
 * @brief Tab item
 */
class TabItem : public FluentPushButton {
    Q_OBJECT

public:
    explicit TabItem(QWidget* parent = nullptr);
    explicit TabItem(const QString& text, QWidget* parent = nullptr);
    explicit TabItem(FluentIconEnum icon, const QString& text, QWidget* parent = nullptr);

    void setBorderRadius(int radius);
    int borderRadius() const { return borderRadius_; }

    void setSelected(bool isSelected);
    bool isSelected() const { return isSelected_; }

    void setShadowEnabled(bool isEnabled);
    bool isShadowEnabled() const { return isShadowEnabled_; }

    void setCloseButtonDisplayMode(TabCloseButtonDisplayMode mode);
    TabCloseButtonDisplayMode closeButtonDisplayMode() const { return closeButtonDisplayMode_; }

    void setRouteKey(const QString& key);
    QString routeKey() const { return routeKey_; }

    void setTextColor(const QColor& color);
    QColor textColor() const { return textColor_; }

    void setSelectedBackgroundColor(const QColor& light, const QColor& dark);

    void setIconEnum(FluentIconEnum icon);
    FluentIconEnum iconEnum() const { return iconEnum_; }

    void slideTo(int x, int duration = 250);

    QSize sizeHint() const override;

    QPropertyAnimation* slideAni() const { return slideAni_; }
    bool isHovered() const { return isHover; }

signals:
    void closed();
    void doubleClicked();

protected:
    void init();
    void resizeEvent(QResizeEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

    void drawSelectedBackground(QPainter* painter);
    void drawNotSelectedBackground(QPainter* painter);
    void drawText(QPainter* painter);
    void forwardMouseEvent(QMouseEvent* event);
    bool canShowShadow() const;

private:
    int borderRadius_ = 5;
    bool isSelected_ = false;
    bool isShadowEnabled_ = true;
    TabCloseButtonDisplayMode closeButtonDisplayMode_ = TabCloseButtonDisplayMode::Always;

    QString routeKey_;
    QColor textColor_;
    QColor lightSelectedBackgroundColor_ = QColor(249, 249, 249);
    QColor darkSelectedBackgroundColor_ = QColor(40, 40, 40);

    FluentIconEnum iconEnum_ = FluentIconEnum::Up;
    bool hasIcon_ = false;

    TabToolButton* closeButton_ = nullptr;
    QGraphicsDropShadowEffect* shadowEffect_ = nullptr;
    QPropertyAnimation* slideAni_ = nullptr;
};

/**
 * @brief Tab bar
 */
class TabBar : public SingleDirectionScrollArea {
    Q_OBJECT
    Q_PROPERTY(bool movable READ isMovable WRITE setMovable)
    Q_PROPERTY(bool scrollable READ isScrollable WRITE setScrollable)
    Q_PROPERTY(int tabMaxWidth READ tabMaximumWidth WRITE setTabMaximumWidth)
    Q_PROPERTY(int tabMinWidth READ tabMinimumWidth WRITE setTabMinimumWidth)
    Q_PROPERTY(bool tabShadowEnabled READ isTabShadowEnabled WRITE setTabShadowEnabled)

public:
    explicit TabBar(QWidget* parent = nullptr);

    void setAddButtonVisible(bool visible);

    TabItem* addTab(const QString& routeKey, const QString& text,
                    FluentIconEnum icon = FluentIconEnum::Up);
    TabItem* insertTab(int index, const QString& routeKey, const QString& text,
                       FluentIconEnum icon = FluentIconEnum::Up);

    void removeTab(int index);
    void removeTabByKey(const QString& routeKey);

    void setCurrentIndex(int index);
    void setCurrentTab(const QString& routeKey);

    int currentIndex() const { return currentIndex_; }
    TabItem* currentTab() const;

    void setCloseButtonDisplayMode(TabCloseButtonDisplayMode mode);
    TabCloseButtonDisplayMode closeButtonDisplayMode() const { return closeButtonDisplayMode_; }

    TabItem* tabItem(int index) const;
    TabItem* tab(const QString& routeKey) const;

    QRect tabRegion() const;
    QRect tabRect(int index) const;

    QVariant tabData(int index) const;
    void setTabData(int index, const QVariant& data);

    QString tabText(int index) const;
    QIcon tabIcon(int index) const;

    bool isTabEnabled(int index) const;
    void setTabEnabled(int index, bool enabled);

    void setTabsClosable(bool closable);
    bool tabsClosable() const;

    void setTabIcon(int index, FluentIconEnum icon);
    void setTabText(int index, const QString& text);

    bool isTabVisible(int index) const;
    void setTabVisible(int index, bool visible);

    void setTabTextColor(int index, const QColor& color);
    void setTabToolTip(int index, const QString& toolTip);
    QString tabToolTip(int index) const;

    void setTabSelectedBackgroundColor(const QColor& light, const QColor& dark);

    void setTabShadowEnabled(bool enabled);
    bool isTabShadowEnabled() const { return isTabShadowEnabled_; }

    void setMovable(bool movable);
    bool isMovable() const { return isMovable_; }

    void setScrollable(bool scrollable);
    bool isScrollable() const { return isScrollable_; }

    void setTabMaximumWidth(int width);
    int tabMaximumWidth() const { return tabMaxWidth_; }

    void setTabMinimumWidth(int width);
    int tabMinimumWidth() const { return tabMinWidth_; }

    int count() const { return items_.size(); }
    void clear();

    QList<TabItem*> items() const { return items_; }

signals:
    void currentChanged(int index);
    void tabBarClicked(int index);
    void tabBarDoubleClicked(int index);
    void tabCloseRequested(int index);
    void tabAddRequested();
    void tabMoved(int from, int to);

protected:
    void init();
    void initLayout();
    void onItemPressed();
    void adjustLayout();

    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

    void swapItem(int index);

private:
    QList<TabItem*> items_;
    QMap<QString, TabItem*> itemMap_;

    int currentIndex_ = -1;
    bool isMovable_ = false;
    bool isScrollable_ = false;
    bool isTabShadowEnabled_ = true;
    bool isDraging_ = false;

    int tabMaxWidth_ = 240;
    int tabMinWidth_ = 64;

    QPoint dragPos_;

    QColor lightSelectedBackgroundColor_ = QColor(249, 249, 249);
    QColor darkSelectedBackgroundColor_ = QColor(40, 40, 40);
    TabCloseButtonDisplayMode closeButtonDisplayMode_ = TabCloseButtonDisplayMode::Always;

    QWidget* view_ = nullptr;
    QHBoxLayout* hBoxLayout_ = nullptr;
    QHBoxLayout* itemLayout_ = nullptr;
    QHBoxLayout* widgetLayout_ = nullptr;
    TabToolButton* addButton_ = nullptr;
};

/**
 * @brief Tab widget
 */
class TabWidget : public QWidget {
    Q_OBJECT

public:
    explicit TabWidget(QWidget* parent = nullptr);

    int addTab(QWidget* widget, const QString& label, FluentIconEnum icon = FluentIconEnum::Up,
               const QString& routeKey = QString());
    int insertTab(int index, QWidget* widget, const QString& label,
                  FluentIconEnum icon = FluentIconEnum::Up, const QString& routeKey = QString());

    void removeTab(int index);
    void clear();

    QWidget* widget(int index) const;
    QWidget* currentWidget() const;
    int currentIndex() const;

    void setTabBar(TabBar* tabBar);
    TabBar* tabBar() const { return tabBar_; }

    bool isMovable() const;
    void setMovable(bool movable);

    bool isTabEnabled(int index) const;
    void setTabEnabled(int index, bool enabled);

    bool isTabVisible(int index) const;
    void setTabVisible(int index, bool visible);

    QString tabText(int index) const;
    QIcon tabIcon(int index) const;
    QString tabToolTip(int index) const;

    void setTabsClosable(bool closable);
    bool tabsClosable() const;

    void setTabIcon(int index, FluentIconEnum icon);
    void setTabText(int index, const QString& text);
    void setTabToolTip(int index, const QString& tip);

    void setTabTextColor(int index, const QColor& color);
    void setTabSelectedBackgroundColor(const QColor& light, const QColor& dark);
    void setTabShadowEnabled(bool enabled);
    void setScrollable(bool scrollable);
    bool isScrollable() const;

    void setTabMaximumWidth(int width);
    void setTabMinimumWidth(int width);
    int tabMaximumWidth() const;
    int tabMinimumWidth() const;

    QVariant tabData(int index) const;
    void setTabData(int index, const QVariant& data);

    int count() const;

    void setCurrentIndex(int index);
    void setCurrentWidget(QWidget* widget);

    void setCloseButtonDisplayMode(TabCloseButtonDisplayMode mode);

signals:
    void currentChanged(int index);
    void tabBarClicked(int index);
    void tabCloseRequested(int index);
    void tabAddRequested();
    void tabBarDoubleClicked(int index);

private:
    void init();
    void connectTabBarSignalToSlot();
    void onCurrentTabChanged(int index);
    void onTabMoved(int fromIndex, int toIndex);

    TabBar* tabBar_ = nullptr;
    QStackedWidget* stackedWidget_ = nullptr;
    QVBoxLayout* vBoxLayout_ = nullptr;
};

}  // namespace qfw
