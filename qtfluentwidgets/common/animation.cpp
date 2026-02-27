#include "common/animation.h"

#include <QApplication>
#include <QPainter>

namespace qfw {

// --- AnimationBase ---
AnimationBase::AnimationBase(QWidget* parent) : QObject(parent) {
    if (parent) parent->installEventFilter(this);
}

bool AnimationBase::eventFilter(QObject* obj, QEvent* e) {
    if (obj == parent()) {
        switch (e->type()) {
            case QEvent::MouseButtonPress:
                _onPress(static_cast<QMouseEvent*>(e));
                break;
            case QEvent::MouseButtonRelease:
                _onRelease(static_cast<QMouseEvent*>(e));
                break;
            case QEvent::Enter:
                _onHover(static_cast<QEnterEvent*>(e));
                break;
            case QEvent::Leave:
                _onLeave(e);
                break;
            default:
                break;
        }
    }
    return QObject::eventFilter(obj, e);
}

// --- TranslateYAnimation ---
TranslateYAnimation::TranslateYAnimation(QWidget* parent, float offset)
    : AnimationBase(parent), maxOffset(offset) {
    ani = new QPropertyAnimation(this, "y", this);
}

void TranslateYAnimation::setY(float y) {
    _y = y;
    if (auto w = qobject_cast<QWidget*>(parent())) {
        // 通过 Margins 模拟 Python 中的平移效果
        w->setContentsMargins(0, static_cast<int>(_y), 0, -static_cast<int>(_y));
        w->update();
    }
    emit valueChanged(y);
}

void TranslateYAnimation::_onPress(QMouseEvent* e) {
    ani->stop();
    ani->setEndValue(maxOffset);
    ani->setDuration(150);
    ani->setEasingCurve(QEasingCurve::OutQuad);
    ani->start();
}

void TranslateYAnimation::_onRelease(QMouseEvent* e) {
    ani->stop();
    ani->setEndValue(0.0f);
    ani->setDuration(500);
    ani->setEasingCurve(QEasingCurve::OutElastic);
    ani->start();
}

// --- DropShadowAnimation ---
DropShadowAnimation::DropShadowAnimation(QWidget* parent, const QColor& color, qreal blurRadius)
    : AnimationBase(parent), normalColor(color), normalBlurRadius(blurRadius) {
    shadow = new QGraphicsDropShadowEffect(parent);
    shadow->setColor(Qt::transparent);
    shadow->setBlurRadius(0);
    shadow->setOffset(0, 0);
    parent->setGraphicsEffect(shadow);

    colorAni = new QPropertyAnimation(this, "color", this);
    blurAni = new QPropertyAnimation(this, "blurRadius", this);
}

QColor DropShadowAnimation::color() const {
    return shadow ? shadow->color() : QColor(Qt::transparent);
}
void DropShadowAnimation::setColor(const QColor& c) {
    if (shadow) shadow->setColor(c);
}
qreal DropShadowAnimation::blurRadius() const { return shadow ? shadow->blurRadius() : 0.0; }
void DropShadowAnimation::setBlurRadius(qreal r) {
    if (shadow) shadow->setBlurRadius(r);
}

void DropShadowAnimation::_onHover(QEnterEvent* e) {
    colorAni->stop();
    blurAni->stop();
    colorAni->setEndValue(normalColor);
    blurAni->setEndValue(normalBlurRadius);
    colorAni->setDuration(150);
    blurAni->setDuration(150);
    colorAni->start();
    blurAni->start();
}

// 修复报错的关键位置
void DropShadowAnimation::_onLeave(QEvent* e) {
    colorAni->stop();
    blurAni->stop();

    // 修复：显式构造 QColor 和 qreal 避免 QVariant 判定为指针
    colorAni->setEndValue(QColor(Qt::transparent));
    blurAni->setEndValue(0.0);

    colorAni->setDuration(150);
    blurAni->setDuration(150);
    colorAni->start();
    blurAni->start();
}

// --- FluentAnimation ---
FluentAnimation::FluentAnimation(QObject* parent) : QPropertyAnimation(parent) {}

QEasingCurve FluentAnimation::createBezierCurve(float x1, float y1, float x2, float y2) {
    QEasingCurve curve(QEasingCurve::BezierSpline);
    curve.addCubicBezierSegment(QPointF(x1, y1), QPointF(x2, y2), QPointF(1, 1));
    return curve;
}

void FluentAnimation::setSpeed(FluentAnimationSpeed speed) { setDuration(speedToDuration(speed)); }

int FluentAnimation::speedToDuration(FluentAnimationSpeed speed) { return 167; }
QEasingCurve FluentAnimation::getCurve() const { return QEasingCurve::Linear; }

void FluentAnimation::startAnimation(const QVariant& endValue, const QVariant& startValue) {
    stop();
    if (startValue.isValid()) setStartValue(startValue);
    setEndValue(endValue);
    setEasingCurve(getCurve());
    start();
}

// 特化类实现
class FastInvokeAnimation : public FluentAnimation {
public:
    using FluentAnimation::FluentAnimation;
    QEasingCurve getCurve() const override { return createBezierCurve(0, 0, 0, 1); }
    int speedToDuration(FluentAnimationSpeed speed) override {
        if (speed == FluentAnimationSpeed::FAST) return 187;
        if (speed == FluentAnimationSpeed::MEDIUM) return 333;
        return 500;
    }
};

FluentAnimation* FluentAnimation::create(FluentAnimationType aniType,
                                         FluentAnimationProperty propType,
                                         FluentAnimationSpeed speed, QWidget* parent) {
    FluentAnimation* ani = nullptr;
    if (aniType == FluentAnimationType::FAST_INVOKE)
        ani = new FastInvokeAnimation(parent);
    else
        ani = new FluentAnimation(parent);

    if (propType == FluentAnimationProperty::POSITION) {
        ani->setTargetObject(new PositionObject(parent));
        ani->setPropertyName("position");
    }
    ani->setSpeed(speed);
    return ani;
}

// --- BackgroundAnimationWidget ---
BackgroundAnimationWidget::BackgroundAnimationWidget(QWidget* parent) : QWidget(parent) {
    _curBgColor = _normalBackgroundColor();
    bgAni = new QPropertyAnimation(this, "backgroundColor", this);
}

void BackgroundAnimationWidget::_updateBackgroundColor(const QColor& targetColor) {
    bgAni->stop();
    bgAni->setEndValue(targetColor);
    bgAni->setDuration(150);
    bgAni->start();
}

void BackgroundAnimationWidget::enterEvent(QEnterEvent* e) {
    _updateBackgroundColor(_hoverBackgroundColor());
}
void BackgroundAnimationWidget::leaveEvent(QEvent* e) {
    _updateBackgroundColor(_normalBackgroundColor());
}
void BackgroundAnimationWidget::mousePressEvent(QMouseEvent* e) {
    _updateBackgroundColor(_pressedBackgroundColor());
    QWidget::mousePressEvent(e);
}
void BackgroundAnimationWidget::mouseReleaseEvent(QMouseEvent* e) {
    _updateBackgroundColor(_hoverBackgroundColor());
    QWidget::mouseReleaseEvent(e);
}

void BackgroundAnimationWidget::paintEvent(QPaintEvent* e) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(_curBgColor);
    painter.drawRoundedRect(rect(), 4, 4);
}

// --- ScaleSlideAnimation ---
ScaleSlideAnimation::ScaleSlideAnimation(QObject* parent, Qt::Orientation orient)
    : QObject(parent), orient_(orient) {
    if (isHorizontal()) {
        geometry_ = QRectF(0, 0, 16, 3);
    } else {
        geometry_ = QRectF(0, 0, 3, 16);
    }

    slideAniGroup_ = new QParallelAnimationGroup(this);
    crossAniGroup_ = new QParallelAnimationGroup(this);
    currentAni_ = slideAniGroup_;

    slidePosAni1_ = new QPropertyAnimation(this, "pos", this);
    slidePosAni2_ = new QPropertyAnimation(this, "pos", this);
    slideLengthAni1_ = new QPropertyAnimation(this, "length", this);
    slideLengthAni2_ = new QPropertyAnimation(this, "length", this);
    seqSlidePosAniGroup_ = new QSequentialAnimationGroup(this);
    seqLengthAniGroup_ = new QSequentialAnimationGroup(this);

    crossLenAni_ = new QPropertyAnimation(this, "length", this);
    crossPosAni_ = new QPropertyAnimation(this, "pos", this);

    seqSlidePosAniGroup_->addAnimation(slidePosAni1_);
    seqSlidePosAniGroup_->addAnimation(slidePosAni2_);
    seqLengthAniGroup_->addAnimation(slideLengthAni1_);
    seqLengthAniGroup_->addAnimation(slideLengthAni2_);

    slideAniGroup_->addAnimation(seqSlidePosAniGroup_);
    slideAniGroup_->addAnimation(seqLengthAniGroup_);

    crossAniGroup_->addAnimation(crossLenAni_);
    crossAniGroup_->addAnimation(crossPosAni_);

    connect(slideAniGroup_, &QParallelAnimationGroup::finished, this,
            &ScaleSlideAnimation::finished);
    connect(crossAniGroup_, &QParallelAnimationGroup::finished, this,
            &ScaleSlideAnimation::finished);
}

void ScaleSlideAnimation::setPos(const QPointF& pos) {
    geometry_.moveTopLeft(pos);
    emit valueChanged(geometry_);
}

float ScaleSlideAnimation::getLength() const {
    return isHorizontal() ? geometry_.width() : geometry_.height();
}

void ScaleSlideAnimation::setLength(float length) {
    if (isHorizontal()) {
        geometry_.setWidth(length);
    } else {
        geometry_.setHeight(length);
    }
    emit valueChanged(geometry_);
}

void ScaleSlideAnimation::setGeometry(const QRectF& rect) {
    geometry_ = rect;
    emit valueChanged(geometry_);
}

void ScaleSlideAnimation::startAnimation(const QRectF& endRect, bool useCrossFade) {
    stopAnimation();

    QRectF startRect = geometry_;

    bool sameLevel;
    qreal dim, from, to;

    if (isHorizontal()) {
        sameLevel = qAbs(startRect.y() - endRect.y()) < 1;
        dim = startRect.width();
        from = startRect.x();
        to = endRect.x();
    } else {
        sameLevel = qAbs(startRect.x() - endRect.x()) < 1;
        dim = startRect.height();
        from = startRect.y();
        to = endRect.y();
    }

    if (sameLevel && !useCrossFade) {
        startSlideAnimation(startRect, endRect, from, to, dim);
    } else {
        startCrossFadeAnimation(startRect, endRect);
    }
}

void ScaleSlideAnimation::stopAnimation() {
    slideAniGroup_->stop();
    crossAniGroup_->stop();
}

void ScaleSlideAnimation::startSlideAnimation(const QRectF& startRect, const QRectF& endRect,
                                              qreal from, qreal to, qreal dimension) {
    currentAni_ = slideAniGroup_;

    slidePosAni1_->setDuration(200);
    slidePosAni2_->setDuration(400);
    slidePosAni1_->setEasingCurve(FluentAnimation::createBezierCurve(0.9f, 0.1f, 1.0f, 0.2f));
    slidePosAni2_->setEasingCurve(FluentAnimation::createBezierCurve(0.1f, 0.9f, 0.2f, 1.0f));

    slideLengthAni1_->setDuration(200);
    slideLengthAni2_->setDuration(400);
    slideLengthAni1_->setEasingCurve(FluentAnimation::createBezierCurve(0.9f, 0.1f, 1.0f, 0.2f));
    slideLengthAni2_->setEasingCurve(FluentAnimation::createBezierCurve(0.1f, 0.9f, 0.2f, 1.0f));

    qreal dist = qAbs(to - from);
    qreal midLength = dist + dimension;
    bool isForward = to > from;

    QPointF startPos = startRect.topLeft();
    QPointF endPos = endRect.topLeft();

    if (isForward) {
        slidePosAni1_->setStartValue(startPos);
        slidePosAni1_->setEndValue(startPos);
        slideLengthAni1_->setStartValue(dimension);
        slideLengthAni1_->setEndValue(midLength);

        slidePosAni2_->setStartValue(startPos);
        slidePosAni2_->setEndValue(endPos);
        slideLengthAni2_->setStartValue(midLength);
        slideLengthAni2_->setEndValue(dimension);
    } else {
        slidePosAni1_->setStartValue(startPos);
        slidePosAni1_->setEndValue(endPos);
        slideLengthAni1_->setStartValue(dimension);
        slideLengthAni1_->setEndValue(midLength);

        slidePosAni2_->setStartValue(endPos);
        slidePosAni2_->setEndValue(endPos);
        slideLengthAni2_->setStartValue(midLength);
        slideLengthAni2_->setEndValue(dimension);
    }

    slideAniGroup_->start();
}

void ScaleSlideAnimation::startCrossFadeAnimation(const QRectF& startRect, const QRectF& endRect) {
    currentAni_ = crossAniGroup_;
    setGeometry(endRect);

    bool isNextBelow =
        isHorizontal() ? (endRect.x() > startRect.x()) : (endRect.y() > startRect.y());

    QRectF startGeo;
    qreal dim;

    if (isHorizontal()) {
        dim = endRect.width();
        startGeo = QRectF(endRect.x() + (isNextBelow ? 0 : dim), endRect.y(), 0, endRect.height());
    } else {
        dim = endRect.height();
        startGeo = QRectF(endRect.x(), endRect.y() + (isNextBelow ? 0 : dim), endRect.width(), 0);
    }

    setGeometry(startGeo);

    crossLenAni_->setDuration(600);
    crossLenAni_->setStartValue(0.0);
    crossLenAni_->setEndValue(dim);
    crossLenAni_->setEasingCurve(QEasingCurve::OutQuint);

    crossPosAni_->setDuration(600);
    crossPosAni_->setStartValue(startGeo.topLeft());
    crossPosAni_->setEndValue(endRect.topLeft());
    crossPosAni_->setEasingCurve(QEasingCurve::OutQuint);

    crossAniGroup_->start();
}

}  // namespace qfw