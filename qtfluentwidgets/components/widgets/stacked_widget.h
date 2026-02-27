#pragma once

#include <QAbstractAnimation>
#include <QEasingCurve>
#include <QLabel>
#include <QObject>
#include <QParallelAnimationGroup>
#include <QPoint>
#include <QPointer>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QStackedWidget>
#include <QVector>

class QGraphicsOpacityEffect;
class QPauseAnimation;

namespace qfw {

class OpacityAniStackedWidget : public QStackedWidget {
    Q_OBJECT

public:
    explicit OpacityAniStackedWidget(QWidget* parent = nullptr);

    int addWidget(QWidget* w);

    void setCurrentIndex(int index);
    void setCurrentWidget(QWidget* w);

private slots:
    void onAniFinished();

private:
    int nextIndex_ = 0;
    QVector<QPointer<QGraphicsOpacityEffect>> effects_;
    QVector<QPointer<QPropertyAnimation>> anis_;
};

struct PopUpAniInfo {
    QPointer<QWidget> widget;
    int deltaX = 0;
    int deltaY = 76;
    QPointer<QPropertyAnimation> ani;
};

class PopUpAniStackedWidget : public QStackedWidget {
    Q_OBJECT

public:
    explicit PopUpAniStackedWidget(QWidget* parent = nullptr);

    void addWidget(QWidget* widget, int deltaX = 0, int deltaY = 76);
    void removeWidget(QWidget* widget);

    void setAnimationEnabled(bool enabled);
    bool isAnimationEnabled() const { return isAnimationEnabled_; }

    void setCurrentIndex(int index, bool needPopOut = false, bool showNextWidgetDirectly = true,
                         int duration = 250,
                         const QEasingCurve& easingCurve = QEasingCurve(QEasingCurve::OutQuad));

    void setCurrentWidget(QWidget* widget, bool needPopOut = false,
                          bool showNextWidgetDirectly = true, int duration = 250,
                          const QEasingCurve& easingCurve = QEasingCurve(QEasingCurve::OutQuad));

signals:
    void aniFinished();
    void aniStart();

private:
    void setAnimation(QPropertyAnimation* ani, const QVariant& startValue, const QVariant& endValue,
                      int duration, const QEasingCurve& easingCurve);
    void onAniFinished();

    QVector<PopUpAniInfo> aniInfos_;
    bool isAnimationEnabled_ = true;
    int nextIndex_ = -1;
    QPointer<QPropertyAnimation> ani_;
    QMetaObject::Connection finishedConn_;
};

class TransitionStackedWidget : public QStackedWidget {
    Q_OBJECT

public:
    explicit TransitionStackedWidget(QWidget* parent = nullptr);

    void setAnimationEnabled(bool enabled);
    bool isAnimationEnabled() const { return isAnimationEnabled_; }

    int addWidget(QWidget* w);
    int insertWidget(int index, QWidget* w);

    void setCurrentWidget(QWidget* widget, int duration = -1, bool isBack = false);
    void setCurrentIndex(int index, int duration = -1, bool isBack = false);

signals:
    void aniFinished();
    void aniStart();

protected:
    virtual void setUpTransitionAnimation(int nextIndex, int duration, bool isBack) = 0;

    void stopAnimation();

    QLabel* currentSnapshot() const { return currentSnapshot_; }
    QLabel* nextSnapshot() const { return nextSnapshot_; }

    void hideSnapshots();
    void renderSnapshot(QWidget* widget, QLabel* label);

protected slots:
    void onAniGroupFinished();

protected:
    QPointer<QParallelAnimationGroup> aniGroup_;
    QPointer<QLabel> currentSnapshot_;
    QPointer<QLabel> nextSnapshot_;
    int nextIndex_ = -1;

private:
    QLabel* createSnapshotLabel();

    bool isAnimationEnabled_ = true;
};

class EntranceTransitionStackedWidget : public TransitionStackedWidget {
    Q_OBJECT

public:
    explicit EntranceTransitionStackedWidget(QWidget* parent = nullptr);

    void setOutDuration(int ms) { outDuration_ = ms; }
    int outDuration() const { return outDuration_; }

    void setOffset(int offset) { offset_ = offset; }
    int offset() const { return offset_; }

protected:
    void setUpTransitionAnimation(int nextIndex, int duration, bool isBack) override;

private:
    int outDuration_ = 150;
    int offset_ = 140;

    QPointer<QPropertyAnimation> currentFadeOutAni_;
    QPointer<QPropertyAnimation> currentSlideOutAni_;
    QPointer<QPropertyAnimation> nextSlideInAni_;

    QPointer<QSequentialAnimationGroup> nextWidgetAniGroup_;
    QPointer<QPauseAnimation> pauseAni_;
};

class DrillInTransitionStackedWidget : public TransitionStackedWidget {
    Q_OBJECT

public:
    explicit DrillInTransitionStackedWidget(QWidget* parent = nullptr);

protected:
    void setUpTransitionAnimation(int nextIndex, int duration, bool isBack) override;

private:
    QPointer<QPropertyAnimation> currentScaleOutAni_;
    QPointer<QPropertyAnimation> currentFadeOutAni_;
    QPointer<QPropertyAnimation> nextScaleInAni_;
    QPointer<QPropertyAnimation> nextFadeInAni_;
};

}  // namespace qfw
