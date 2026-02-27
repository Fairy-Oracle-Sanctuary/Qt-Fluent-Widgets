#pragma once

#include <QColor>
#include <QMap>
#include <QObject>
#include <QPointer>
#include <QPropertyAnimation>
#include <QRectF>
#include <QString>
#include <QVBoxLayout>
#include <QVariant>
#include <QWidget>
#include <functional>

#include "common/router.h"
#include "components/navigation/navigation_panel.h"
#include "components/navigation/navigation_widget.h"
#include "components/widgets/scroll_area.h"

namespace qfw {

class NavigationIndicator;

class IconSlideAnimation : public QPropertyAnimation {
    Q_OBJECT
    Q_PROPERTY(qreal offset READ offset WRITE setOffset)

public:
    explicit IconSlideAnimation(QWidget* parent = nullptr);

    qreal offset() const { return offset_; }
    void setOffset(qreal value);

    void slideDown();
    void slideUp();

    qreal maxOffset() const { return maxOffset_; }
    void setMaxOffset(qreal v) { maxOffset_ = v; }

private:
    qreal offset_ = 0;
    qreal maxOffset_ = 6;
};

class NavigationBarPushButton : public NavigationPushButton {
    Q_OBJECT

public:
    NavigationBarPushButton(const QVariant& icon, const QString& text, bool isSelectable,
                            const QVariant& selectedIcon = QVariant(), QWidget* parent = nullptr);

    void setSelectedColor(const QColor& light, const QColor& dark);

    QVariant selectedIconVariant() const { return selectedIcon_; }
    void setSelectedIcon(const QVariant& icon);

    void setSelectedTextVisible(bool visible);
    bool isSelectedTextVisible() const { return isSelectedTextVisible_; }

    QRectF indicatorRect() const override;

    void setSelected(bool selected) override;

protected:
    void paintEvent(QPaintEvent* e) override;

private:
    void drawBackground(QPainter* painter);
    void drawIcon(QPainter* painter);
    void drawText(QPainter* painter);

    QPointer<IconSlideAnimation> iconAni_;
    QVariant selectedIcon_;

    bool isSelectedTextVisible_ = true;

    QColor lightSelectedColor_;
    QColor darkSelectedColor_;
};

class NavigationBar : public QWidget {
    Q_OBJECT

public:
    explicit NavigationBar(QWidget* parent = nullptr);

    NavigationWidget* widget(const QString& routeKey) const;

    NavigationBarPushButton* addItem(
        const QString& routeKey, const QVariant& icon, const QString& text,
        const std::function<void()>& onClick = {}, bool selectable = true,
        const QVariant& selectedIcon = QVariant(),
        qfw::NavigationItemPosition position = qfw::NavigationItemPosition::Top);

    void addWidget(const QString& routeKey, NavigationWidget* widget,
                   const std::function<void()>& onClick = {},
                   qfw::NavigationItemPosition position = qfw::NavigationItemPosition::Top);

    NavigationBarPushButton* insertItem(
        int index, const QString& routeKey, const QVariant& icon, const QString& text,
        const std::function<void()>& onClick = {}, bool selectable = true,
        const QVariant& selectedIcon = QVariant(),
        qfw::NavigationItemPosition position = qfw::NavigationItemPosition::Top);

    void insertWidget(int index, const QString& routeKey, NavigationWidget* widget,
                      const std::function<void()>& onClick = {},
                      qfw::NavigationItemPosition position = qfw::NavigationItemPosition::Top);

    void removeWidget(const QString& routeKey);

    void setCurrentItem(const QString& routeKey);
    NavigationWidget* currentItem() const;

    void setSelectedTextVisible(bool visible);
    bool isSelectedTextVisible() const { return isSelectedTextVisible_; }

    void setSelectedColor(const QColor& light, const QColor& dark);

    bool isIndicatorAnimationEnabled() const { return isIndicatorAnimationEnabled_; }
    void setIndicatorAnimationEnabled(bool enabled) { isIndicatorAnimationEnabled_ = enabled; }

    Router* history() const { return history_; }

private slots:
    void onWidgetClicked(bool triggeredByUser);
    void onIndicatorAniFinished();

private:
    void initWidget();
    void initLayout();

    QRectF getIndicatorRect(NavigationWidget* item) const;

    void stopIndicatorAnimation();

private:
    QPointer<NavigationIndicator> indicator_;
    bool isIndicatorAnimationEnabled_ = true;
    bool isSelectedTextVisible_ = true;

    QColor lightSelectedColor_;
    QColor darkSelectedColor_;

    QPointer<ScrollArea> scrollArea_;
    QPointer<QWidget> scrollWidget_;

    QPointer<QVBoxLayout> vBoxLayout_;
    QPointer<QVBoxLayout> topLayout_;
    QPointer<QVBoxLayout> bottomLayout_;
    QPointer<QVBoxLayout> scrollLayout_;

    QMap<QString, QPointer<NavigationWidget>> items_;
    QPointer<Router> history_;
    QString currentRouteKey_;
};

}  // namespace qfw
