#pragma once

#include <QFrame>
#include <QHBoxLayout>
#include <QMap>
#include <QString>

#include "common/animation.h"
#include "components/navigation/navigation_widget.h"

class QScrollArea;

namespace qfw {

class ScrollArea;
class NavigationToolButton;
class RoundMenu;
class Router;

enum class TopNavigationDisplayMode {
    Compact = 0,  // only icons
    Expand = 1,   // icons + text
    Menu = 2      // menu mode
};

enum class TopNavigationItemPosition { Left = 0, Center = 1, Right = 2 };

class TopNavigationPushButton : public NavigationPushButton {
    Q_OBJECT

public:
    explicit TopNavigationPushButton(const QVariant& icon, const QString& text, bool isSelectable,
                                     QWidget* parent = nullptr);

    bool isExpanded() const { return isExpanded_; }
    void setExpanded(bool expanded);

    void setCompacted(bool compacted) override;

protected:
    bool canDrawIndicator() const override { return false; }

private:
    void updateSize();

private:
    bool isExpanded_ = false;
};

class TopNavigationPanel : public QFrame {
    Q_OBJECT

public:
    explicit TopNavigationPanel(QWidget* parent = nullptr);

    TopNavigationDisplayMode displayMode() const { return displayMode_; }
    void setDisplayMode(TopNavigationDisplayMode mode);

    bool isCompact() const { return displayMode_ == TopNavigationDisplayMode::Compact; }
    void expand() { setDisplayMode(TopNavigationDisplayMode::Expand); }
    void collapse() { setDisplayMode(TopNavigationDisplayMode::Compact); }
    void toggle();

    NavigationWidget* widget(const QString& routeKey) const;
    NavigationWidget* currentItem() const;

    void setCurrentItem(const QString& routeKey);
    void removeWidget(const QString& routeKey);

    TopNavigationPushButton* addItem(
        const QString& routeKey, const QVariant& icon, const QString& text,
        std::function<void()> onClick = nullptr, bool selectable = true,
        TopNavigationItemPosition position = TopNavigationItemPosition::Left,
        const QString& tooltip = QString(), bool expanded = false);

    TopNavigationPushButton* insertItem(
        int index, const QString& routeKey, const QVariant& icon, const QString& text,
        std::function<void()> onClick = nullptr, bool selectable = true,
        TopNavigationItemPosition position = TopNavigationItemPosition::Left,
        const QString& tooltip = QString(), bool expanded = false);

    void addWidget(const QString& routeKey, NavigationWidget* widget,
                   std::function<void()> onClick = nullptr,
                   TopNavigationItemPosition position = TopNavigationItemPosition::Left,
                   const QString& tooltip = QString());

    void insertWidget(int index, const QString& routeKey, NavigationWidget* widget,
                      std::function<void()> onClick = nullptr,
                      TopNavigationItemPosition position = TopNavigationItemPosition::Left,
                      const QString& tooltip = QString());

    void setItemExpanded(const QString& routeKey, bool expanded);

    void setReturnButtonVisible(bool visible);

    Router* history() const { return history_; }

    int layoutMinWidth() const;

    void setIndicatorColor(const QColor& light, const QColor& dark);

    bool isIndicatorAnimationEnabled() const { return isIndicatorAnimationEnabled_; }
    void setIndicatorAnimationEnabled(bool enabled);

    QRectF currentIndicatorGeometry() const;

signals:
    void displayModeChanged(TopNavigationDisplayMode mode);

protected:
    void showEvent(QShowEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

private slots:
    void onWidgetClicked(bool triggeredByUser);
    void onIndicatorAniFinished();
    void showOverflowMenu();

private:
    void initWidget();
    void initLayout();

    void registerWidget(const QString& routeKey, NavigationWidget* widget,
                        std::function<void()> onClick, const QString& tooltip);
    void insertWidgetToLayout(int index, NavigationWidget* widget,
                              TopNavigationItemPosition position);

    void updateOverflow();
    void adjustIndicatorPos();

private:
    TopNavigationDisplayMode displayMode_ = TopNavigationDisplayMode::Compact;
    bool isReturnButtonVisible_ = false;
    bool isIndicatorAnimationEnabled_ = true;

    QString currentRouteKey_;

    QMap<QString, NavigationWidget*> items_;
    QList<NavigationWidget*> overflowWidgets_;

    ScrollArea* scrollArea_ = nullptr;
    QWidget* scrollWidget_ = nullptr;

    NavigationToolButton* returnButton_ = nullptr;
    NavigationToolButton* moreButton_ = nullptr;

    QHBoxLayout* hBoxLayout_ = nullptr;
    QHBoxLayout* leftLayout_ = nullptr;
    QHBoxLayout* centerLayout_ = nullptr;
    QHBoxLayout* rightLayout_ = nullptr;
    QHBoxLayout* scrollLayout_ = nullptr;

    ScaleSlideAnimation* slideAni_ = nullptr;

    QColor lightIndicatorColor_;
    QColor darkIndicatorColor_;

    Router* history_ = nullptr;
};

// Alias for compatibility
using TopNavigationBar = TopNavigationPanel;

}  // namespace qfw
