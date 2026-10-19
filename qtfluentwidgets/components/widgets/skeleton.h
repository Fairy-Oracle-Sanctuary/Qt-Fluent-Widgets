#pragma once

#include <QColor>
#include <QPainterPath>
#include <QVariantAnimation>
#include <QWidget>

namespace qfw {

class SkeletonWidget : public QWidget {
    Q_OBJECT
public:
    explicit SkeletonWidget(QWidget* parent = nullptr);
    void addRect(const QRectF& rect, qreal radius = 6, bool relative = false);
    void addEllipse(const QRectF& rect, bool relative = false);
    void clear();
    virtual QPainterPath skeletonPath() const;
    QColor baseColor() const;
    QColor highlightColor() const;
    void setColors(const QColor& baseLight, const QColor& baseDark,
                   const QColor& highlightLight = QColor(), const QColor& highlightDark = QColor());
    bool isAnimationEnabled() const;
    void setAnimationEnabled(bool enabled);
    int animationDuration() const;
    void setAnimationDuration(int duration);
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    QPainterPath personalInfoPath(bool circular) const;

private:
    struct Shape {
        QRectF rect;
        qreal radius;
        bool ellipse;
        bool relative;
    };
    QList<Shape> shapes_;
    qreal phase_ = 0;
    bool animationEnabled_ = true;
    QColor baseLight_{QStringLiteral("#e5e5e5")}, baseDark_{QStringLiteral("#3d3d3d")};
    QColor highlightLight_{255, 255, 255, 220}, highlightDark_{255, 255, 255, 80};
    QVariantAnimation animation_;
};

class CirclePersonalInfoSkeleton : public SkeletonWidget {
    Q_OBJECT
public:
    using SkeletonWidget::SkeletonWidget;
    QPainterPath skeletonPath() const override;
};
class RectanglePersonalInfoSkeleton : public SkeletonWidget {
    Q_OBJECT
public:
    using SkeletonWidget::SkeletonWidget;
    QPainterPath skeletonPath() const override;
};
class ArticleSkeleton : public SkeletonWidget {
    Q_OBJECT
public:
    using SkeletonWidget::SkeletonWidget;
    QSize sizeHint() const override;
    QPainterPath skeletonPath() const override;
};

}  // namespace qfw
