#pragma once
#include "components/widgets/flyout.h"
#include "components/widgets/acrylic_label.h"

namespace qfw {

class AcrylicFlyoutViewBase : public FlyoutViewBase {
    Q_OBJECT
public:
    explicit AcrylicFlyoutViewBase(QWidget* parent = nullptr);
    AcrylicBrush& acrylicBrush() { return brush_; }
    virtual QPainterPath acrylicClipPath() const;
protected:
    void paintEvent(QPaintEvent* event) override;
private:
    AcrylicBrush brush_;
};

class AcrylicFlyoutView : public FlyoutView {
    Q_OBJECT
public:
    explicit AcrylicFlyoutView(const QString& title, const QString& content, QWidget* parent = nullptr);
    AcrylicFlyoutView(const QString& title, const QString& content, const QIcon& icon, QWidget* parent = nullptr);
    AcrylicFlyoutView(const QString& title, const QString& content, const FluentIconBase& icon, QWidget* parent = nullptr);
    AcrylicFlyoutView(const QString& title, const QString& content, const QString& image, QWidget* parent = nullptr);
    AcrylicBrush& acrylicBrush() { return brush_; }
    virtual QPainterPath acrylicClipPath() const;
protected:
    void paintEvent(QPaintEvent* event) override;
    void adjustImage() override;
private:
    AcrylicBrush brush_;
};

class AcrylicFlyout : public Flyout {
    Q_OBJECT
public:
    explicit AcrylicFlyout(FlyoutViewBase* view, QWidget* parent = nullptr, bool deleteOnClose = true);
    void exec(const QPoint& pos, FlyoutAnimationType type = FlyoutAnimationType::PullUp) override;
    static AcrylicFlyout* make(FlyoutViewBase* view, const QVariant& target = {}, QWidget* parent = nullptr,
                              FlyoutAnimationType type = FlyoutAnimationType::PullUp, bool deleteOnClose = true);
    static AcrylicFlyout* create(const QString& title, const QString& content, const QVariant& icon = {},
                                const QVariant& image = {}, bool closable = false, const QVariant& target = {},
                                QWidget* parent = nullptr, FlyoutAnimationType type = FlyoutAnimationType::PullUp,
                                bool deleteOnClose = true);
protected:
    void prepareShowBackground(const QPoint& adjustedPosition) override;
    virtual void captureBackground(AcrylicBrush& brush, const QRect& globalRect);
private:
    QPointer<FlyoutViewBase> acrylicView_;
};
}
