#include "components/widgets/stacked_widget.h"

#include <QGraphicsOpacityEffect>
#include <QPauseAnimation>
#include <QPixmap>

#include "common/animation.h"

namespace qfw {

// ============================================================================
// OpacityAniStackedWidget
// ============================================================================

OpacityAniStackedWidget::OpacityAniStackedWidget(QWidget* parent) : QStackedWidget(parent) {}

int OpacityAniStackedWidget::addWidget(QWidget* w) {
    const int idx = QStackedWidget::addWidget(w);

    auto* effect = new QGraphicsOpacityEffect(this);
    effect->setOpacity(1.0);

    auto* ani = new QPropertyAnimation(effect, QByteArrayLiteral("opacity"), this);
    ani->setDuration(220);
    connect(ani, &QPropertyAnimation::finished, this, &OpacityAniStackedWidget::onAniFinished);

    effects_.append(effect);
    anis_.append(ani);

    if (w) {
        w->setGraphicsEffect(effect);
    }

    return idx;
}

void OpacityAniStackedWidget::setCurrentIndex(int index) {
    const int index0 = currentIndex();
    if (index == index0) {
        return;
    }

    if (index < 0 || index >= count()) {
        return;
    }

    QPropertyAnimation* ani = nullptr;

    if (index > index0) {
        ani = anis_.value(index);
        if (ani) {
            ani->setStartValue(0.0);
            ani->setEndValue(1.0);
        }
        QStackedWidget::setCurrentIndex(index);
    } else {
        ani = anis_.value(index0);
        if (ani) {
            ani->setStartValue(1.0);
            ani->setEndValue(0.0);
        }
    }

    QWidget* w0 = widget(index0);
    if (w0) {
        w0->show();
    }

    nextIndex_ = index;
    if (ani) {
        ani->start();
    } else {
        onAniFinished();
    }
}

void OpacityAniStackedWidget::setCurrentWidget(QWidget* w) {
    setCurrentIndex(indexOf(w));
}

void OpacityAniStackedWidget::onAniFinished() {
    QStackedWidget::setCurrentIndex(nextIndex_);
}

// ============================================================================
// PopUpAniStackedWidget
// ============================================================================

PopUpAniStackedWidget::PopUpAniStackedWidget(QWidget* parent) : QStackedWidget(parent) {}

void PopUpAniStackedWidget::addWidget(QWidget* widget, int deltaX, int deltaY) {
    QStackedWidget::addWidget(widget);

    PopUpAniInfo info;
    info.widget = widget;
    info.deltaX = deltaX;
    info.deltaY = deltaY;
    info.ani = new QPropertyAnimation(widget, QByteArrayLiteral("pos"), this);
    aniInfos_.append(info);
}

void PopUpAniStackedWidget::removeWidget(QWidget* widget) {
    const int index = indexOf(widget);
    if (index < 0) {
        return;
    }

    aniInfos_.removeAt(index);
    QStackedWidget::removeWidget(widget);
}

void PopUpAniStackedWidget::setAnimationEnabled(bool enabled) {
    isAnimationEnabled_ = enabled;
}

void PopUpAniStackedWidget::setCurrentIndex(int index, bool needPopOut, bool showNextWidgetDirectly,
                                            int duration, const QEasingCurve& easingCurve) {
    if (index < 0 || index >= count()) {
        return;
    }

    if (index == currentIndex()) {
        return;
    }

    if (!isAnimationEnabled_) {
        QStackedWidget::setCurrentIndex(index);
        return;
    }

    if (ani_ && ani_->state() == QAbstractAnimation::Running) {
        ani_->stop();
        onAniFinished();
    }

    nextIndex_ = index;

    PopUpAniInfo nextAniInfo = aniInfos_.value(index);
    PopUpAniInfo currentAniInfo = aniInfos_.value(currentIndex());

    QWidget* currentW = currentWidget();
    QWidget* nextW = nextAniInfo.widget;

    QPropertyAnimation* ani = (needPopOut ? currentAniInfo.ani.data() : nextAniInfo.ani.data());
    ani_ = ani;

    if (!ani || !currentW || !nextW) {
        QStackedWidget::setCurrentIndex(index);
        return;
    }

    // keep only one finished connection
    if (finishedConn_) {
        QObject::disconnect(finishedConn_);
    }

    if (needPopOut) {
        const QPoint pos = currentW->pos() + QPoint(currentAniInfo.deltaX, currentAniInfo.deltaY);
        setAnimation(ani, currentW->pos(), pos, duration, easingCurve);
        nextW->setVisible(showNextWidgetDirectly);
    } else {
        const QPoint pos = nextW->pos() + QPoint(nextAniInfo.deltaX, nextAniInfo.deltaY);
        setAnimation(ani, pos, QPoint(nextW->x(), 0), duration, easingCurve);
        QStackedWidget::setCurrentIndex(index);
    }

    finishedConn_ = connect(ani, &QPropertyAnimation::finished, this, &PopUpAniStackedWidget::onAniFinished);
    ani->start();
    emit aniStart();
}

void PopUpAniStackedWidget::setCurrentWidget(QWidget* widget, bool needPopOut,
                                             bool showNextWidgetDirectly, int duration,
                                             const QEasingCurve& easingCurve) {
    setCurrentIndex(indexOf(widget), needPopOut, showNextWidgetDirectly, duration, easingCurve);
}

void PopUpAniStackedWidget::setAnimation(QPropertyAnimation* ani, const QVariant& startValue,
                                         const QVariant& endValue, int duration,
                                         const QEasingCurve& easingCurve) {
    if (!ani) {
        return;
    }

    ani->setEasingCurve(easingCurve);
    ani->setStartValue(startValue);
    ani->setEndValue(endValue);
    ani->setDuration(duration);
}

void PopUpAniStackedWidget::onAniFinished() {
    if (finishedConn_) {
        QObject::disconnect(finishedConn_);
        finishedConn_ = {};
    }

    QStackedWidget::setCurrentIndex(nextIndex_);
    emit aniFinished();
}

// ============================================================================
// TransitionStackedWidget
// ============================================================================

TransitionStackedWidget::TransitionStackedWidget(QWidget* parent)
    : QStackedWidget(parent), aniGroup_(new QParallelAnimationGroup(this)) {
    currentSnapshot_ = createSnapshotLabel();
    nextSnapshot_ = createSnapshotLabel();

    connect(aniGroup_, &QParallelAnimationGroup::finished, this,
            &TransitionStackedWidget::onAniGroupFinished);
}

void TransitionStackedWidget::setAnimationEnabled(bool enabled) {
    isAnimationEnabled_ = enabled;
}

int TransitionStackedWidget::addWidget(QWidget* w) {
    if (w) {
        w->setAttribute(Qt::WA_TranslucentBackground);
    }
    return QStackedWidget::addWidget(w);
}

int TransitionStackedWidget::insertWidget(int index, QWidget* w) {
    if (w) {
        w->setAttribute(Qt::WA_TranslucentBackground);
    }
    return QStackedWidget::insertWidget(index, w);
}

void TransitionStackedWidget::setCurrentWidget(QWidget* widget, int duration, bool isBack) {
    setCurrentIndex(indexOf(widget), duration, isBack);
}

void TransitionStackedWidget::setCurrentIndex(int index, int duration, bool isBack) {
    if (index < 0 || index >= count()) {
        return;
    }

    if (index == currentIndex()) {
        return;
    }

    if (!isAnimationEnabled_) {
        QStackedWidget::setCurrentIndex(index);
        return;
    }

    stopAnimation();

    nextIndex_ = index;

    setUpTransitionAnimation(index, duration, isBack);

    aniGroup_->start();
    emit aniStart();
}

void TransitionStackedWidget::stopAnimation() {
    if (!aniGroup_ || aniGroup_->state() != QAbstractAnimation::Running) {
        return;
    }

    aniGroup_->stop();
    onAniGroupFinished();
}

void TransitionStackedWidget::hideSnapshots() {
    if (currentSnapshot_) {
        currentSnapshot_->hide();
    }
    if (nextSnapshot_) {
        nextSnapshot_->hide();
    }
}

void TransitionStackedWidget::onAniGroupFinished() {
    hideSnapshots();
    QStackedWidget::setCurrentIndex(nextIndex_);
    emit aniFinished();
}

QLabel* TransitionStackedWidget::createSnapshotLabel() {
    auto* label = new QLabel(this);
    label->setAttribute(Qt::WA_TranslucentBackground);

    auto* effect = new QGraphicsOpacityEffect(label);
    label->setGraphicsEffect(effect);
    label->hide();

    return label;
}

void TransitionStackedWidget::renderSnapshot(QWidget* widget, QLabel* label) {
    if (!widget || !label) {
        return;
    }

    widget->resize(size());

    QPixmap pixmap = widget->grab();

    if (pixmap.isNull() || pixmap.size().isEmpty()) {
        pixmap = QPixmap(widget->size());
        pixmap.fill(Qt::transparent);
        widget->render(&pixmap);
    }

    label->setPixmap(pixmap);
    label->setGeometry(rect());
    label->show();
    label->raise();
}

// ============================================================================
// EntranceTransitionStackedWidget
// ============================================================================

EntranceTransitionStackedWidget::EntranceTransitionStackedWidget(QWidget* parent)
    : TransitionStackedWidget(parent) {
    currentFadeOutAni_ = new QPropertyAnimation(currentSnapshot()->graphicsEffect(),
                                                QByteArrayLiteral("opacity"), this);
    currentSlideOutAni_ = new QPropertyAnimation(currentSnapshot(), QByteArrayLiteral("pos"), this);
    nextSlideInAni_ = new QPropertyAnimation(this);

    nextWidgetAniGroup_ = new QSequentialAnimationGroup(this);
    aniGroup_->addAnimation(nextWidgetAniGroup_);
}

void EntranceTransitionStackedWidget::setUpTransitionAnimation(int nextIndex, int duration,
                                                              bool isBack) {
    const int inDuration = (duration >= 0) ? duration : 300;
    const QEasingCurve inCurve = FluentAnimation::createBezierCurve(0.1f, 0.9f, 0.2f, 1.0f);
    const QEasingCurve outCurve = FluentAnimation::createBezierCurve(0.7f, 0.0f, 1.0f, 0.5f);

    QWidget* currentW = currentWidget();
    QWidget* nextW = widget(nextIndex);

    // clear aniGroup animations except nextWidgetAniGroup_
    for (int i = aniGroup_->animationCount() - 1; i >= 0; --i) {
        QAbstractAnimation* a = aniGroup_->animationAt(i);
        if (a && a != nextWidgetAniGroup_) {
            aniGroup_->removeAnimation(a);
        }
    }

    // reset sequential group
    while (nextWidgetAniGroup_->animationCount() > 0) {
        QAbstractAnimation* a = nextWidgetAniGroup_->takeAnimation(0);
        if (a) {
            a->deleteLater();
        }
    }

    if (currentW) {
        renderSnapshot(currentW, currentSnapshot());
        currentW->hide();

        currentFadeOutAni_->setDuration(outDuration_);
        currentFadeOutAni_->setStartValue(1.0);
        currentFadeOutAni_->setEndValue(0.0);
        currentFadeOutAni_->setEasingCurve(outCurve);
        aniGroup_->addAnimation(currentFadeOutAni_);

        if (isBack) {
            currentSlideOutAni_->setDuration(outDuration_);
            currentSlideOutAni_->setStartValue(QPoint(0, 0));
            currentSlideOutAni_->setEndValue(QPoint(0, offset_));
            currentSlideOutAni_->setEasingCurve(outCurve);
            aniGroup_->addAnimation(currentSlideOutAni_);
        }
    }

    if (!nextW) {
        return;
    }

    nextW->hide();

    pauseAni_ = new QPauseAnimation(outDuration_, nextWidgetAniGroup_);
    connect(pauseAni_, &QPauseAnimation::finished, nextWidgetAniGroup_, [nextW]() { nextW->show(); });
    nextWidgetAniGroup_->addAnimation(pauseAni_);

    if (!isBack) {
        nextSlideInAni_->setTargetObject(nextW);
        nextW->setGeometry(0, offset_, width(), height());
        nextSlideInAni_->setPropertyName(QByteArrayLiteral("pos"));
        nextSlideInAni_->setDuration(inDuration);
        nextSlideInAni_->setStartValue(QPoint(0, offset_));
        nextSlideInAni_->setEndValue(QPoint(0, 0));
        nextSlideInAni_->setEasingCurve(inCurve);
        nextWidgetAniGroup_->addAnimation(nextSlideInAni_);
    } else {
        nextW->setGeometry(rect());
    }
}

// ============================================================================
// DrillInTransitionStackedWidget
// ============================================================================

DrillInTransitionStackedWidget::DrillInTransitionStackedWidget(QWidget* parent)
    : TransitionStackedWidget(parent) {
    currentScaleOutAni_ = new QPropertyAnimation(currentSnapshot(), QByteArrayLiteral("geometry"), this);
    currentFadeOutAni_ = new QPropertyAnimation(currentSnapshot()->graphicsEffect(),
                                                QByteArrayLiteral("opacity"), this);
    nextScaleInAni_ = new QPropertyAnimation(nextSnapshot(), QByteArrayLiteral("geometry"), this);
    nextFadeInAni_ = new QPropertyAnimation(nextSnapshot()->graphicsEffect(), QByteArrayLiteral("opacity"), this);
}

void DrillInTransitionStackedWidget::setUpTransitionAnimation(int nextIndex, int duration,
                                                             bool isBack) {
    const QEasingCurve scaleCurve = FluentAnimation::createBezierCurve(0.1f, 0.9f, 0.2f, 1.0f);
    const QEasingCurve opacityCurve = FluentAnimation::createBezierCurve(0.17f, 0.17f, 0.0f, 1.0f);
    const QEasingCurve backScaleCurve = FluentAnimation::createBezierCurve(0.12f, 0.0f, 0.0f, 1.0f);

    const qreal inScale = isBack ? 1.06 : 0.94;
    const qreal outScale = isBack ? 0.96 : 1.04;

    const int inDuration = (duration >= 0) ? duration : 333;
    const int outDuration = 100;

    const QEasingCurve inScaleCurve = isBack ? backScaleCurve : scaleCurve;

    QWidget* currentW = currentWidget();
    QWidget* nextW = widget(nextIndex);
    const QRect r = rect();

    // clear and re-add animations
    while (aniGroup_->animationCount() > 0) {
        aniGroup_->removeAnimation(aniGroup_->animationAt(0));
    }

    if (currentW) {
        renderSnapshot(currentW, currentSnapshot());
        currentSnapshot()->setScaledContents(true);
        currentW->hide();

        const int outW = int(r.width() * outScale);
        const int outH = int(r.height() * outScale);
        const int outX = (r.width() - outW) / 2;
        const int outY = (r.height() - outH) / 2;
        const QRect outRect(outX, outY, outW, outH);

        currentScaleOutAni_->setDuration(outDuration);
        currentScaleOutAni_->setStartValue(r);
        currentScaleOutAni_->setEndValue(outRect);
        currentScaleOutAni_->setEasingCurve(scaleCurve);
        aniGroup_->addAnimation(currentScaleOutAni_);

        currentFadeOutAni_->setDuration(outDuration);
        currentFadeOutAni_->setStartValue(1.0);
        currentFadeOutAni_->setEndValue(0.0);
        currentFadeOutAni_->setEasingCurve(opacityCurve);
        aniGroup_->addAnimation(currentFadeOutAni_);
    }

    if (!nextW) {
        return;
    }

    renderSnapshot(nextW, nextSnapshot());
    nextSnapshot()->setScaledContents(true);
    nextW->hide();

    const int inW = int(r.width() * inScale);
    const int inH = int(r.height() * inScale);
    const int inX = (r.width() - inW) / 2;
    const int inY = (r.height() - inH) / 2;
    const QRect inRect(inX, inY, inW, inH);

    nextSnapshot()->setGeometry(inRect);

    nextScaleInAni_->setDuration(inDuration);
    nextScaleInAni_->setStartValue(inRect);
    nextScaleInAni_->setEndValue(r);
    nextScaleInAni_->setEasingCurve(inScaleCurve);
    aniGroup_->addAnimation(nextScaleInAni_);

    nextFadeInAni_->setDuration(inDuration);
    nextFadeInAni_->setStartValue(0.0);
    nextFadeInAni_->setEndValue(1.0);
    nextFadeInAni_->setEasingCurve(opacityCurve);
    aniGroup_->addAnimation(nextFadeInAni_);
}

}  // namespace qfw
