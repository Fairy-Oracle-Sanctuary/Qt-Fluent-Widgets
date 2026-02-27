#pragma once

#include <QColor>
#include <QEasingCurve>
#include <QEnterEvent>
#include <QEvent>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QObject>
#include <QParallelAnimationGroup>
#include <QPointF>
#include <QPointer>
#include <QPropertyAnimation>
#include <QRectF>
#include <QSequentialAnimationGroup>
#include <QVariant>
#include <QWidget>

namespace qfw {

/**
 * 动画基础类：负责事件拦截和分发
 */
class AnimationBase : public QObject {
    Q_OBJECT
public:
    explicit AnimationBase(QWidget* parent);

protected:
    bool eventFilter(QObject* obj, QEvent* e) override;
    virtual void _onHover(QEnterEvent* e) {}
    virtual void _onLeave(QEvent* e) {}
    virtual void _onPress(QMouseEvent* e) {}
    virtual void _onRelease(QMouseEvent* e) {}
};

/**
 * Y轴位移动画
 */
class TranslateYAnimation : public AnimationBase {
    Q_OBJECT
    Q_PROPERTY(float y READ getY WRITE setY NOTIFY valueChanged)

public:
    explicit TranslateYAnimation(QWidget* parent, float offset = 2.0f);

    float getY() const { return _y; }
    void setY(float y);

signals:
    void valueChanged(float y);

protected:
    void _onPress(QMouseEvent* e) override;
    void _onRelease(QMouseEvent* e) override;

private:
    float _y = 0;
    float maxOffset;
    QPropertyAnimation* ani;
};

/**
 * 阴影动画
 */
class DropShadowAnimation : public AnimationBase {
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor)
    Q_PROPERTY(qreal blurRadius READ blurRadius WRITE setBlurRadius)

public:
    explicit DropShadowAnimation(QWidget* parent, const QColor& color, qreal blurRadius);

    QColor color() const;
    void setColor(const QColor& c);
    qreal blurRadius() const;
    void setBlurRadius(qreal r);

protected:
    void _onHover(QEnterEvent* e) override;
    void _onLeave(QEvent* e) override;

private:
    QPointer<QGraphicsDropShadowEffect> shadow;
    QPropertyAnimation* colorAni;
    QPropertyAnimation* blurAni;
    QColor normalColor;
    qreal normalBlurRadius;
};

/**
 * Fluent 动画枚举
 */
enum class FluentAnimationSpeed { FAST, MEDIUM, SLOW };
enum class FluentAnimationType {
    FAST_INVOKE,
    STRONG_INVOKE,
    FAST_DISMISS,
    SOFT_DISMISS,
    POINT_TO_POINT,
    FADE_IN_OUT
};
enum class FluentAnimationProperty { POSITION, SCALE, ANGLE, OPACITY };

/**
 * 属性包装对象：用于给没有特定 Q_PROPERTY 的 Widget 扩展动画属性
 */
class FluentAnimationPropertyObject : public QObject {
    Q_OBJECT
public:
    explicit FluentAnimationPropertyObject(QWidget* parent) : QObject(parent) {}
};

class PositionObject : public FluentAnimationPropertyObject {
    Q_OBJECT
    Q_PROPERTY(QPoint position READ position WRITE setPosition)
public:
    using FluentAnimationPropertyObject::FluentAnimationPropertyObject;
    QPoint position() const { return static_cast<QWidget*>(parent())->pos(); }
    void setPosition(const QPoint& p) { static_cast<QWidget*>(parent())->move(p); }
};

/**
 * Fluent 动画核心类
 */
class FluentAnimation : public QPropertyAnimation {
    Q_OBJECT
public:
    explicit FluentAnimation(QObject* parent = nullptr);

    static QEasingCurve createBezierCurve(float x1, float y1, float x2, float y2);
    void setSpeed(FluentAnimationSpeed speed);
    virtual int speedToDuration(FluentAnimationSpeed speed);

    void startAnimation(const QVariant& endValue, const QVariant& startValue = QVariant());

    static FluentAnimation* create(FluentAnimationType aniType, FluentAnimationProperty propType,
                                   FluentAnimationSpeed speed = FluentAnimationSpeed::FAST,
                                   QWidget* parent = nullptr);

    virtual QEasingCurve getCurve() const;
};

/**
 * 背景色动画基类 (代替 Python 的 Mixin)
 */
class BackgroundAnimationWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor)

public:
    explicit BackgroundAnimationWidget(QWidget* parent = nullptr);

    QColor backgroundColor() const { return _curBgColor; }
    void setBackgroundColor(const QColor& c) {
        _curBgColor = c;
        update();
    }

protected:
    void enterEvent(QEnterEvent* e) override;
    void leaveEvent(QEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

    virtual QColor _normalBackgroundColor() { return QColor(0, 0, 0, 0); }
    virtual QColor _hoverBackgroundColor() { return _normalBackgroundColor(); }
    virtual QColor _pressedBackgroundColor() { return _hoverBackgroundColor(); }

private:
    void _updateBackgroundColor(const QColor& targetColor);
    bool isPressed = false;
    QColor _curBgColor;
    QPropertyAnimation* bgAni;
};

/**
 * 缩放滑动动画：用于导航指示器
 */
class ScaleSlideAnimation : public QObject {
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ getPos WRITE setPos)
    Q_PROPERTY(float length READ getLength WRITE setLength)
    Q_PROPERTY(QRectF geometry READ getGeometry WRITE setGeometry)

public:
    explicit ScaleSlideAnimation(QObject* parent = nullptr,
                                 Qt::Orientation orient = Qt::Horizontal);

    void startAnimation(const QRectF& endRect, bool useCrossFade = false);
    void stopAnimation();

    bool isHorizontal() const { return orient_ == Qt::Horizontal; }

    QPointF getPos() const { return geometry_.topLeft(); }
    void setPos(const QPointF& pos);

    float getLength() const;
    void setLength(float length);

    QRectF getGeometry() const { return geometry_; }
    void setGeometry(const QRectF& rect);

signals:
    void valueChanged(const QRectF& rect);
    void finished();

private:
    void startSlideAnimation(const QRectF& startRect, const QRectF& endRect, qreal from, qreal to,
                             qreal dimension);
    void startCrossFadeAnimation(const QRectF& startRect, const QRectF& endRect);

    Qt::Orientation orient_;
    QRectF geometry_;

    QParallelAnimationGroup* slideAniGroup_ = nullptr;
    QParallelAnimationGroup* crossAniGroup_ = nullptr;
    QObject* currentAni_ = nullptr;

    QPropertyAnimation* slidePosAni1_ = nullptr;
    QPropertyAnimation* slidePosAni2_ = nullptr;
    QPropertyAnimation* slideLengthAni1_ = nullptr;
    QPropertyAnimation* slideLengthAni2_ = nullptr;
    QSequentialAnimationGroup* seqSlidePosAniGroup_ = nullptr;
    QSequentialAnimationGroup* seqLengthAniGroup_ = nullptr;

    QPropertyAnimation* crossLenAni_ = nullptr;
    QPropertyAnimation* crossPosAni_ = nullptr;
};

}  // namespace qfw