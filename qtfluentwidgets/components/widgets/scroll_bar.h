#pragma once

#include <QAbstractScrollArea>
#include <QGraphicsOpacityEffect>
#include <QObject>
#include <QPropertyAnimation>
#include <QScrollBar>
#include <QTimer>
#include <QToolButton>
#include <QWidget>

#include "../../common/icon.h"
#include "../../common/smooth_scroll.h"

namespace qfw {

class ArrowButton : public QToolButton {
    Q_OBJECT

public:
    explicit ArrowButton(FluentIconEnum icon, QWidget* parent = nullptr);

    void setOpacity(qreal opacity);
    void setLightColor(const QColor& color);
    void setDarkColor(const QColor& color);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    FluentIconEnum icon_;
    QColor lightColor_;
    QColor darkColor_;
    qreal opacity_;
};

class ScrollBarHandle : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)

public:
    explicit ScrollBarHandle(Qt::Orientation orient, QWidget* parent = nullptr);

    void setLightColor(const QColor& color);
    void setDarkColor(const QColor& color);

    qreal opacity() const { return opacity_; }
    void setOpacity(qreal opacity);

    void fadeIn();
    void fadeOut();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    Qt::Orientation orient_;
    QColor lightColor_;
    QColor darkColor_;
    qreal opacity_;
    QPropertyAnimation* opacityAni_;
};

class ScrollBarGroove : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)

public:
    explicit ScrollBarGroove(Qt::Orientation orient, QWidget* parent = nullptr);

    void setLightBackgroundColor(const QColor& color);
    void setDarkBackgroundColor(const QColor& color);

    qreal opacity() const { return opacity_; }
    void setOpacity(qreal opacity);

    void fadeIn();
    void fadeOut();

    ArrowButton* upButton() const { return upButton_; }
    ArrowButton* downButton() const { return downButton_; }
    QPropertyAnimation* opacityAni() const;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    Qt::Orientation orient_;
    QColor lightBackgroundColor_;
    QColor darkBackgroundColor_;
    qreal opacity_;
    QPropertyAnimation* opacityAni_;
    ArrowButton* upButton_;
    ArrowButton* downButton_;
};

enum class ScrollBarHandleDisplayMode { ALWAYS = 0, ON_HOVER = 1 };

class ScrollBar : public QWidget {
    Q_OBJECT

public:
    explicit ScrollBar(Qt::Orientation orient, QAbstractScrollArea* parent);

    int value() const { return value_; }
    void setValue(int value);

    int minimum() const { return minimum_; }
    void setMinimum(int min);

    int maximum() const { return maximum_; }
    void setMaximum(int max);

    void setRange(int min, int max);

    Qt::Orientation orientation() const { return orientation_; }

    int pageStep() const { return pageStep_; }
    void setPageStep(int step);

    int singleStep() const { return singleStep_; }
    void setSingleStep(int step);

    bool isSliderDown() const { return isPressed_; }
    void setSliderDown(bool isDown);

    void setHandleColor(const QColor& light, const QColor& dark);
    void setArrowColor(const QColor& light, const QColor& dark);
    void setGrooveColor(const QColor& light, const QColor& dark);

    void setHandleDisplayMode(ScrollBarHandleDisplayMode mode);

    void setForceHidden(bool isHidden);

    void expand();
    void collapse();

    ScrollBarHandle* handle() const { return handle_; }
    ScrollBarGroove* groove() const { return groove_; }

signals:
    void rangeChanged(QPair<int, int> range);
    void valueChanged(int value);
    void sliderPressed();
    void sliderReleased();
    void sliderMoved();

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

    bool eventFilter(QObject* obj, QEvent* event) override;

    void initWidget(QAbstractScrollArea* parent);

    void onPageUp();
    void onPageDown();
    void onValueChanged(int value);

    void adjustPos(const QSize& size);
    void adjustHandleSize();
    void adjustHandlePos();

    int grooveLength() const;
    int slideLength() const;
    bool isSlideRegion(const QPoint& pos) const;

    void onOpacityAniValueChanged();

private:
    Qt::Orientation orientation_;
    int singleStep_;
    int pageStep_;
    int padding_;

    int minimum_;
    int maximum_;
    int value_;

    bool isPressed_;
    bool isEnter_;
    bool isExpanded_;
    QPoint pressedPos_;
    bool isForceHidden_;

    ScrollBarHandleDisplayMode handleDisplayMode_;

    ScrollBarGroove* groove_;
    ScrollBarHandle* handle_;
    QScrollBar* partnerBar_;
};

class SmoothScrollBar : public ScrollBar {
    Q_OBJECT
    Q_PROPERTY(int val READ value WRITE setVal)

public:
    explicit SmoothScrollBar(Qt::Orientation orient, QAbstractScrollArea* parent);

    void setValue(int value, bool useAni = true);

    void scrollValue(int value, bool useAni = true);
    void scrollTo(int value, bool useAni = true);
    void resetValue(int value);

    void setScrollAnimation(int duration, QEasingCurve::Type easing = QEasingCurve::OutCubic);

    int duration() const { return duration_; }
    void setDuration(int duration) { duration_ = duration; }

    bool isAnimationRunning() const { return ani_->state() == QPropertyAnimation::Running; }

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    void setVal(int value);

private:
    int duration_;
    QPropertyAnimation* ani_;
    int cachedValue_;
};

class SmoothScrollDelegate : public QObject {
    Q_OBJECT

public:
    explicit SmoothScrollDelegate(QAbstractScrollArea* parent, bool useAni = false);

    void setSmoothMode(SmoothMode mode, Qt::Orientation orientation);
    void setScrollAnimation(Qt::Orientation orientation, int duration,
                            QEasingCurve::Type easing = QEasingCurve::OutCubic);

    void setVerticalScrollBarPolicy(Qt::ScrollBarPolicy policy);
    void setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy policy);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    bool useAni_;
    SmoothScrollBar* vScrollBar_;
    SmoothScrollBar* hScrollBar_;
    SmoothScroll* verticalSmoothScroll_;
    SmoothScroll* horizonSmoothScroll_;
};

}  // namespace qfw
