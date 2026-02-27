#pragma once

#include <QHBoxLayout>
#include <QMap>
#include <QPointer>
#include <QString>
#include <QWidget>

#include "common/animation.h"
#include "components/widgets/button.h"

namespace qfw {

class PivotItem : public PushButton {
    Q_OBJECT

public:
    explicit PivotItem(QWidget* parent = nullptr);
    explicit PivotItem(const QString& text, QWidget* parent = nullptr);

    void setSelected(bool isSelected);
    bool isSelected() const { return isSelected_; }

signals:
    void itemClicked(bool checked);

private:
    bool isSelected_ = false;
};

class Pivot : public QWidget {
    Q_OBJECT

public:
    explicit Pivot(QWidget* parent = nullptr);

    PivotItem* addItem(const QString& routeKey, const QString& text,
                       const std::function<void(bool)>& onClick = {}, const QVariant& icon = {});
    void addWidget(const QString& routeKey, PivotItem* widget,
                   const std::function<void(bool)>& onClick = {});

    virtual PivotItem* insertItem(int index, const QString& routeKey, const QString& text,
                                  const std::function<void(bool)>& onClick = {},
                                  const QVariant& icon = {});
    void insertWidget(int index, const QString& routeKey, PivotItem* widget,
                      const std::function<void(bool)>& onClick = {});

    void removeWidget(const QString& routeKey);
    void clear();

    PivotItem* currentItem() const;
    QString currentRouteKey() const { return currentRouteKey_; }

    virtual void setCurrentItem(const QString& routeKey);

    void setIndicatorLength(int len);
    int indicatorLength() const { return indicatorLength_; }

    void setItemFontSize(int size);
    void setItemText(const QString& routeKey, const QString& text);

    void setIndicatorColor(const QColor& light, const QColor& dark);

    PivotItem* widget(const QString& routeKey) const;

signals:
    void currentItemChanged(const QString& routeKey);

protected:
    void showEvent(QShowEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

private:
    void onItemClicked(bool checked);

protected:
    virtual void adjustIndicatorPos();
    virtual void startIndicatorAnimation();
    virtual QRectF currentIndicatorGeometry() const;

    QMap<QString, QPointer<PivotItem>> items_;
    QString currentRouteKey_;
    int indicatorLength_ = 16;

    QColor lightIndicatorColor_;
    QColor darkIndicatorColor_;

    QHBoxLayout* hBoxLayout_ = nullptr;
    ScaleSlideAnimation* slideAni_ = nullptr;
};

}  // namespace qfw
