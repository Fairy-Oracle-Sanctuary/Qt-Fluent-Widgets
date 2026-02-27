#pragma once

#include <QColor>
#include <QPropertyAnimation>
#include <QString>
#include <QVariant>

#include "components/navigation/pivot.h"

namespace qfw {

class SegmentedItem : public PivotItem {
    Q_OBJECT

public:
    explicit SegmentedItem(QWidget* parent = nullptr);
    explicit SegmentedItem(const QString& text, QWidget* parent = nullptr);
};

class SegmentedToolItem : public PivotItem {
    Q_OBJECT

public:
    explicit SegmentedToolItem(QWidget* parent = nullptr);
    explicit SegmentedToolItem(const QVariant& icon, QWidget* parent = nullptr);
};

class SegmentedToggleToolItem : public PivotItem {
    Q_OBJECT

public:
    explicit SegmentedToggleToolItem(QWidget* parent = nullptr);
    explicit SegmentedToggleToolItem(const QVariant& icon, QWidget* parent = nullptr);

    void setSelected(bool isSelected);
    bool isSelected() const { return isSelected_; }

protected:
    void drawIcon(QPainter* painter, const QRectF& rect, QIcon::State state = QIcon::Off) override;

private:
    bool isSelected_ = false;
};

class SegmentedWidget : public Pivot {
    Q_OBJECT
    Q_PROPERTY(qreal slideValue READ slideValue WRITE setSlideValue)

public:
    explicit SegmentedWidget(QWidget* parent = nullptr);

    PivotItem* insertItem(int index, const QString& routeKey, const QString& text,
                          const std::function<void(bool)>& onClick = {},
                          const QVariant& icon = {}) override;

    qreal slideValue() const { return slideValue_; }
    void setSlideValue(qreal v);

    // Set horizontal padding for all items (in pixels)
    void setItemPadding(int padding);
    int itemPadding() const { return itemPadding_; }

protected:
    void adjustIndicatorPos() override;
    void startIndicatorAnimation() override;

    void paintEvent(QPaintEvent* e) override;

    QRectF currentIndicatorGeometry() const override;

protected:
    void startSlideAnimation(qreal endValue);

    qreal slideValue_ = 0;
    QPropertyAnimation* slideAni_ = nullptr;
    int itemPadding_ = 12;  // default horizontal padding in pixels
};

class SegmentedToolWidget : public SegmentedWidget {
    Q_OBJECT

public:
    explicit SegmentedToolWidget(QWidget* parent = nullptr);

    PivotItem* addItem(const QString& routeKey, const QVariant& icon,
                       const std::function<void(bool)>& onClick = {});

protected:
    PivotItem* insertItem(int index, const QString& routeKey, const QString& text,
                          const std::function<void(bool)>& onClick = {},
                          const QVariant& icon = {}) override;

    virtual PivotItem* createItem(const QVariant& icon);
};

class SegmentedToggleToolWidget : public SegmentedToolWidget {
    Q_OBJECT

public:
    explicit SegmentedToggleToolWidget(QWidget* parent = nullptr);

protected:
    PivotItem* createItem(const QVariant& icon) override;
    void paintEvent(QPaintEvent* e) override;
};

}  // namespace qfw
