#include "scroll_bar.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QHBoxLayout>
#include <QListView>
#include <QMouseEvent>
#include <QPainter>
#include <QVBoxLayout>
#include <QWheelEvent>

#include "../../common/style_sheet.h"

namespace qfw {

// ============================================================================
// ArrowButton
// ============================================================================
ArrowButton::ArrowButton(FluentIconEnum icon, QWidget* parent)
    : QToolButton(parent),
      icon_(icon),
      lightColor_(0, 0, 0, 114),
      darkColor_(255, 255, 255, 139),
      opacity_(1.0) {
    setFixedSize(10, 10);
}

void ArrowButton::setOpacity(qreal opacity) {
    opacity_ = opacity;
    update();
}

void ArrowButton::setLightColor(const QColor& color) {
    lightColor_ = color;
    update();
}

void ArrowButton::setDarkColor(const QColor& color) {
    darkColor_ = color;
    update();
}

void ArrowButton::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    QColor color = isDarkTheme() ? darkColor_ : lightColor_;
    painter.setOpacity(opacity_ * color.alpha() / 255.0);

    int s = isDown() ? 7 : 8;
    qreal x = (width() - s) / 2.0;

    FluentIcon icon(icon_);
    icon.render(&painter, QRectF(x, x, s, s).toRect(), Theme::Auto,
                QVariantMap{{"fill", color.name()}});
}

// ============================================================================
// ScrollBarHandle
// ============================================================================
ScrollBarHandle::ScrollBarHandle(Qt::Orientation orient, QWidget* parent)
    : QWidget(parent),
      orient_(orient),
      lightColor_(0, 0, 0, 114),
      darkColor_(255, 255, 255, 139),
      opacity_(1.0) {
    if (orient == Qt::Vertical) {
        setFixedWidth(3);
    } else {
        setFixedHeight(3);
    }

    opacityAni_ = new QPropertyAnimation(this, "opacity", this);
}

void ScrollBarHandle::setLightColor(const QColor& color) {
    lightColor_ = color;
    update();
}

void ScrollBarHandle::setDarkColor(const QColor& color) {
    darkColor_ = color;
    update();
}

void ScrollBarHandle::setOpacity(qreal opacity) {
    opacity_ = opacity;
    update();
}

void ScrollBarHandle::fadeIn() {
    opacityAni_->stop();
    opacityAni_->setStartValue(opacity_);
    opacityAni_->setEndValue(1.0);
    opacityAni_->setDuration(150);
    opacityAni_->start();
}

void ScrollBarHandle::fadeOut() {
    opacityAni_->stop();
    opacityAni_->setStartValue(opacity_);
    opacityAni_->setEndValue(0.0);
    opacityAni_->setDuration(150);
    opacityAni_->start();
}

void ScrollBarHandle::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    qreal r = (orient_ == Qt::Vertical) ? width() / 2.0 : height() / 2.0;
    painter.setOpacity(opacity_);
    painter.setBrush(isDarkTheme() ? darkColor_ : lightColor_);
    painter.drawRoundedRect(rect(), r, r);
}

// ============================================================================
// ScrollBarGroove
// ============================================================================
ScrollBarGroove::ScrollBarGroove(Qt::Orientation orient, QWidget* parent)
    : QWidget(parent),
      orient_(orient),
      lightBackgroundColor_(252, 252, 252, 217),
      darkBackgroundColor_(44, 44, 44, 245),
      opacity_(0.0) {
    if (orient == Qt::Vertical) {
        setFixedWidth(12);
        upButton_ = new ArrowButton(FluentIconEnum::CareUpSolid, this);
        downButton_ = new ArrowButton(FluentIconEnum::CareDownSolid, this);
        auto* layout = new QVBoxLayout(this);
        layout->addWidget(upButton_, 0, Qt::AlignHCenter);
        layout->addStretch(1);
        layout->addWidget(downButton_, 0, Qt::AlignHCenter);
        layout->setContentsMargins(0, 3, 0, 3);
    } else {
        setFixedHeight(12);
        upButton_ = new ArrowButton(FluentIconEnum::CareLeftSolid, this);
        downButton_ = new ArrowButton(FluentIconEnum::CareRightSolid, this);
        auto* layout = new QHBoxLayout(this);
        layout->addWidget(upButton_, 0, Qt::AlignVCenter);
        layout->addStretch(1);
        layout->addWidget(downButton_, 0, Qt::AlignVCenter);
        layout->setContentsMargins(3, 0, 3, 0);
    }

    opacityAni_ = new QPropertyAnimation(this, "opacity", this);
    setOpacity(0);
}

void ScrollBarGroove::setLightBackgroundColor(const QColor& color) {
    lightBackgroundColor_ = color;
    update();
}

void ScrollBarGroove::setDarkBackgroundColor(const QColor& color) {
    darkBackgroundColor_ = color;
    update();
}

void ScrollBarGroove::setOpacity(qreal opacity) {
    opacity_ = opacity;
    upButton_->setOpacity(opacity);
    downButton_->setOpacity(opacity);
    update();
}

void ScrollBarGroove::fadeIn() {
    opacityAni_->stop();
    opacityAni_->setStartValue(opacity_);
    opacityAni_->setEndValue(1.0);
    opacityAni_->setDuration(150);
    opacityAni_->start();
}

void ScrollBarGroove::fadeOut() {
    opacityAni_->stop();
    opacityAni_->setStartValue(opacity_);
    opacityAni_->setEndValue(0.0);
    opacityAni_->setDuration(150);
    opacityAni_->start();
}

void ScrollBarGroove::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    painter.setOpacity(opacity_);
    painter.setBrush(isDarkTheme() ? darkBackgroundColor_ : lightBackgroundColor_);
    painter.drawRoundedRect(rect(), 6, 6);
}

// ============================================================================
// ScrollBar
// ============================================================================
ScrollBar::ScrollBar(Qt::Orientation orient, QAbstractScrollArea* parent)
    : QWidget(parent),
      orientation_(orient),
      singleStep_(1),
      pageStep_(50),
      padding_(14),
      minimum_(0),
      maximum_(0),
      value_(0),
      isPressed_(false),
      isEnter_(false),
      isExpanded_(false),
      isForceHidden_(false),
      handleDisplayMode_(ScrollBarHandleDisplayMode::ALWAYS),
      groove_(new ScrollBarGroove(orient, this)),
      handle_(new ScrollBarHandle(orient, this)),
      partnerBar_(nullptr) {
    if (orient == Qt::Vertical) {
        partnerBar_ = parent->verticalScrollBar();
        parent->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    } else {
        partnerBar_ = parent->horizontalScrollBar();
        parent->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

    initWidget(parent);
}

void ScrollBar::initWidget(QAbstractScrollArea* parent) {
    connect(groove_->upButton(), &QToolButton::clicked, this, &ScrollBar::onPageUp);
    connect(groove_->downButton(), &QToolButton::clicked, this, &ScrollBar::onPageDown);
    connect(groove_->opacityAni(), &QPropertyAnimation::valueChanged, this,
            &ScrollBar::onOpacityAniValueChanged);

    if (partnerBar_) {
        connect(partnerBar_, &QScrollBar::rangeChanged, this,
                [this](int min, int max) { setRange(min, max); });
        connect(partnerBar_, &QScrollBar::valueChanged, this, &ScrollBar::onValueChanged);
        connect(this, &ScrollBar::valueChanged, partnerBar_, &QScrollBar::setValue);

        setRange(partnerBar_->minimum(), partnerBar_->maximum());
    }

    parent->installEventFilter(this);

    setVisible(maximum() > 0 && !isForceHidden_);
    adjustPos(parent->size());
}

void ScrollBar::onPageUp() { setValue(value() - pageStep()); }

void ScrollBar::onPageDown() { setValue(value() + pageStep()); }

void ScrollBar::onValueChanged(int value) { setValue(value); }

void ScrollBar::setValue(int value) {
    if (value == value_) {
        return;
    }

    value = qMax(minimum_, qMin(value, maximum_));
    value_ = value;
    emit valueChanged(value);

    adjustHandlePos();
}

void ScrollBar::setMinimum(int min) {
    if (min == minimum_) {
        return;
    }

    minimum_ = min;
    emit rangeChanged({min, maximum()});
}

void ScrollBar::setMaximum(int max) {
    if (max == maximum_) {
        return;
    }

    maximum_ = max;
    emit rangeChanged({minimum(), max});
}

void ScrollBar::setRange(int min, int max) {
    if (min > max || (min == minimum_ && max == maximum_)) {
        return;
    }

    setMinimum(min);
    setMaximum(max);

    adjustHandleSize();
    adjustHandlePos();
    setVisible(max > 0 && !isForceHidden_);

    emit rangeChanged({min, max});
}

void ScrollBar::setPageStep(int step) {
    if (step >= 1) {
        pageStep_ = step;
    }
}

void ScrollBar::setSingleStep(int step) {
    if (step >= 1) {
        singleStep_ = step;
    }
}

void ScrollBar::setSliderDown(bool isDown) {
    isPressed_ = isDown;
    if (isDown) {
        emit sliderPressed();
    } else {
        emit sliderReleased();
    }
}

void ScrollBar::setHandleColor(const QColor& light, const QColor& dark) {
    handle_->setLightColor(light);
    handle_->setDarkColor(dark);
}

void ScrollBar::setArrowColor(const QColor& light, const QColor& dark) {
    groove_->upButton()->setLightColor(light);
    groove_->upButton()->setDarkColor(dark);
    groove_->downButton()->setLightColor(light);
    groove_->downButton()->setDarkColor(dark);
}

void ScrollBar::setGrooveColor(const QColor& light, const QColor& dark) {
    groove_->setLightBackgroundColor(light);
    groove_->setDarkBackgroundColor(dark);
}

void ScrollBar::setHandleDisplayMode(ScrollBarHandleDisplayMode mode) {
    if (mode == handleDisplayMode_) {
        return;
    }

    handleDisplayMode_ = mode;
    if (mode == ScrollBarHandleDisplayMode::ON_HOVER && !isEnter_) {
        handle_->fadeOut();
    } else if (mode == ScrollBarHandleDisplayMode::ALWAYS) {
        handle_->fadeIn();
    }
}

void ScrollBar::setForceHidden(bool isHidden) {
    isForceHidden_ = isHidden;
    setVisible(maximum() > 0 && !isHidden);
}

void ScrollBar::expand() {
    if (isExpanded_ || !isEnter_) {
        return;
    }

    isExpanded_ = true;
    groove_->fadeIn();
    handle_->fadeIn();
}

void ScrollBar::collapse() {
    if (!isExpanded_ || isEnter_) {
        return;
    }

    isExpanded_ = false;
    groove_->fadeOut();

    if (handleDisplayMode_ == ScrollBarHandleDisplayMode::ON_HOVER) {
        handle_->fadeOut();
    }
}

void ScrollBar::enterEvent(QEnterEvent* event) {
    isEnter_ = true;
    QTimer::singleShot(200, this, &ScrollBar::expand);
}

void ScrollBar::leaveEvent(QEvent* event) {
    isEnter_ = false;
    QTimer::singleShot(200, this, &ScrollBar::collapse);
}

void ScrollBar::resizeEvent(QResizeEvent* event) { groove_->resize(size()); }

bool ScrollBar::eventFilter(QObject* obj, QEvent* event) {
    if (obj != parent()) {
        return QWidget::eventFilter(obj, event);
    }

    if (event->type() == QEvent::Resize) {
        auto* resizeEvent = static_cast<QResizeEvent*>(event);
        adjustPos(resizeEvent->size());
    }

    return QWidget::eventFilter(obj, event);
}

void ScrollBar::mousePressEvent(QMouseEvent* event) {
    QWidget::mousePressEvent(event);
    isPressed_ = true;
    pressedPos_ = event->pos();

    if (childAt(event->pos()) == handle_ || !isSlideRegion(event->pos())) {
        return;
    }

    int value;
    if (orientation() == Qt::Vertical) {
        if (event->pos().y() > handle_->geometry().bottom()) {
            value = event->pos().y() - handle_->height() - padding_;
        } else {
            value = event->pos().y() - padding_;
        }
    } else {
        if (event->pos().x() > handle_->geometry().right()) {
            value = event->pos().x() - handle_->width() - padding_;
        } else {
            value = event->pos().x() - padding_;
        }
    }

    setValue(static_cast<int>(value / qMax(slideLength(), 1) * maximum()));
    emit sliderPressed();
}

void ScrollBar::mouseReleaseEvent(QMouseEvent* event) {
    QWidget::mouseReleaseEvent(event);
    isPressed_ = false;
    emit sliderReleased();
}

void ScrollBar::mouseMoveEvent(QMouseEvent* event) {
    if (!isPressed_) {
        return;
    }

    int dv;
    if (orientation() == Qt::Vertical) {
        dv = event->pos().y() - pressedPos_.y();
    } else {
        dv = event->pos().x() - pressedPos_.x();
    }

    // don't use `self.setValue()`, because it could be reimplemented
    int total = maximum() - minimum();
    int deltaValue = static_cast<int>(static_cast<double>(dv) / qMax(slideLength(), 1) * total);

    // Correctly call the base class setValue to avoid animation loops in SmoothScrollBar
    ScrollBar::setValue(value() + deltaValue);

    pressedPos_ = event->pos();
    emit sliderMoved();
}

void ScrollBar::wheelEvent(QWheelEvent* event) {
    QApplication::sendEvent(static_cast<QAbstractScrollArea*>(parent())->viewport(), event);
}

void ScrollBar::adjustPos(const QSize& size) {
    if (orientation() == Qt::Vertical) {
        resize(12, size.height() - 2);
        move(size.width() - 13, 1);
    } else {
        resize(size.width() - 2, 12);
        move(1, size.height() - 13);
    }
}

void ScrollBar::adjustHandleSize() {
    QWidget* p = static_cast<QWidget*>(parent());
    if (orientation() == Qt::Vertical) {
        int total = maximum() - minimum() + p->height();
        int s = static_cast<int>(grooveLength() * p->height() / qMax(total, 1));
        handle_->setFixedHeight(qMax(30, s));
    } else {
        int total = maximum() - minimum() + p->width();
        int s = static_cast<int>(grooveLength() * p->width() / qMax(total, 1));
        handle_->setFixedWidth(qMax(30, s));
    }
}

void ScrollBar::adjustHandlePos() {
    int total = qMax(maximum() - minimum(), 1);
    int delta = static_cast<int>(static_cast<qreal>(value()) / total * slideLength());

    if (orientation() == Qt::Vertical) {
        int x = width() - handle_->width() - 3;
        handle_->move(x, padding_ + delta);
    } else {
        int y = height() - handle_->height() - 3;
        handle_->move(padding_ + delta, y);
    }
}

int ScrollBar::grooveLength() const {
    if (orientation() == Qt::Vertical) {
        return height() - 2 * padding_;
    }
    return width() - 2 * padding_;
}

int ScrollBar::slideLength() const {
    if (orientation() == Qt::Vertical) {
        return grooveLength() - handle_->height();
    }
    return grooveLength() - handle_->width();
}

bool ScrollBar::isSlideRegion(const QPoint& pos) const {
    if (orientation() == Qt::Vertical) {
        return padding_ <= pos.y() && pos.y() <= height() - padding_;
    }
    return padding_ <= pos.x() && pos.x() <= width() - padding_;
}

void ScrollBar::onOpacityAniValueChanged() {
    qreal opacity = groove_->opacity();
    if (orientation() == Qt::Vertical) {
        handle_->setFixedWidth(static_cast<int>(3 + opacity * 3));
    } else {
        handle_->setFixedHeight(static_cast<int>(3 + opacity * 3));
    }
    adjustHandlePos();
}

QPropertyAnimation* ScrollBarGroove::opacityAni() const { return opacityAni_; }

// ============================================================================
// SmoothScrollBar
// ============================================================================
SmoothScrollBar::SmoothScrollBar(Qt::Orientation orient, QAbstractScrollArea* parent)
    : ScrollBar(orient, parent), duration_(500), cachedValue_(0) {
    ani_ = new QPropertyAnimation(this, "val", this);
    ani_->setEasingCurve(QEasingCurve::OutCubic);
    ani_->setDuration(duration_);

    cachedValue_ = value();
}

void SmoothScrollBar::setValue(int value, bool useAni) {
    if (value == this->value()) {
        return;
    }

    ani_->stop();

    if (!useAni) {
        ScrollBar::setValue(value);
        return;
    }

    int dv = qAbs(value - this->value());
    if (dv < 50) {
        ani_->setDuration(static_cast<int>(duration_ * dv / 70));
    } else {
        ani_->setDuration(duration_);
    }

    ani_->setStartValue(this->value());
    ani_->setEndValue(value);
    ani_->start();
}

void SmoothScrollBar::scrollValue(int value, bool useAni) {
    cachedValue_ += value;
    cachedValue_ = qMax(minimum(), cachedValue_);
    cachedValue_ = qMin(maximum(), cachedValue_);
    setValue(cachedValue_, useAni);
}

void SmoothScrollBar::scrollTo(int value, bool useAni) {
    cachedValue_ = value;
    cachedValue_ = qMax(minimum(), cachedValue_);
    cachedValue_ = qMin(maximum(), cachedValue_);
    setValue(cachedValue_, useAni);
}

void SmoothScrollBar::resetValue(int value) { cachedValue_ = value; }

void SmoothScrollBar::setScrollAnimation(int duration, QEasingCurve::Type easing) {
    duration_ = duration;
    ani_->setDuration(duration);
    ani_->setEasingCurve(QEasingCurve(easing));
}

void SmoothScrollBar::mousePressEvent(QMouseEvent* event) {
    ani_->stop();
    ScrollBar::mousePressEvent(event);
    cachedValue_ = value();
}

void SmoothScrollBar::mouseMoveEvent(QMouseEvent* event) {
    ani_->stop();
    ScrollBar::mouseMoveEvent(event);
    cachedValue_ = value();
}

void SmoothScrollBar::setVal(int value) { ScrollBar::setValue(value); }

// ============================================================================
// SmoothScrollDelegate
// ============================================================================
SmoothScrollDelegate::SmoothScrollDelegate(QAbstractScrollArea* parent, bool useAni)
    : QObject(parent), useAni_(useAni) {
    vScrollBar_ = new SmoothScrollBar(Qt::Vertical, parent);
    hScrollBar_ = new SmoothScrollBar(Qt::Horizontal, parent);

    // Create SmoothScroll for all scroll areas (matches Python implementation)
    verticalSmoothScroll_ = new SmoothScroll(parent, Qt::Vertical, this);
    horizonSmoothScroll_ = new SmoothScroll(parent, Qt::Horizontal, this);

    if (auto* itemView = qobject_cast<QAbstractItemView*>(parent)) {
        itemView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        itemView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    }
    if (auto* listView = qobject_cast<QListView*>(parent)) {
        listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        listView->horizontalScrollBar()->setStyleSheet("QScrollBar:horizontal{height: 0px}");
    }

    parent->viewport()->installEventFilter(this);
}

bool SmoothScrollDelegate::eventFilter(QObject* obj, QEvent* event) {
    if (event->type() != QEvent::Wheel) {
        return QObject::eventFilter(obj, event);
    }

    auto* wheelEvent = static_cast<QWheelEvent*>(event);
    
    // On macOS trackpad, angleDelta may be (0,0) while pixelDelta has values
    // Use pixelDelta as fallback when angleDelta is zero
    QPoint angleDelta = wheelEvent->angleDelta();
    QPoint pixelDelta = wheelEvent->pixelDelta();
    
    // If angleDelta is zero but pixelDelta has values, use pixelDelta
    if (angleDelta.isNull() && !pixelDelta.isNull()) {
        angleDelta = pixelDelta;
    }

    bool verticalAtEnd =
        (angleDelta.y() < 0 && vScrollBar_->value() == vScrollBar_->maximum()) ||
        (angleDelta.y() > 0 && vScrollBar_->value() == vScrollBar_->minimum());

    bool horizontalAtEnd =
        (angleDelta.x() < 0 && hScrollBar_->value() == hScrollBar_->maximum()) ||
        (angleDelta.x() > 0 && hScrollBar_->value() == hScrollBar_->minimum());

    if (verticalAtEnd && horizontalAtEnd) {
        return false;
    }

    if (angleDelta.y() != 0 && !verticalAtEnd) {
        if (useAni_) {
            vScrollBar_->scrollValue(-angleDelta.y());
        } else if (verticalSmoothScroll_) {
            // Create a new wheel event with the corrected delta
            QWheelEvent adjustedEvent(wheelEvent->position(), wheelEvent->globalPosition(),
                                      wheelEvent->pixelDelta(), angleDelta,
                                      wheelEvent->buttons(), wheelEvent->modifiers(),
                                      wheelEvent->phase(), wheelEvent->inverted());
            verticalSmoothScroll_->wheelEvent(&adjustedEvent);
        }
    }

    if (angleDelta.x() != 0 && !horizontalAtEnd) {
        if (useAni_) {
            hScrollBar_->scrollValue(-angleDelta.x());
        } else if (horizonSmoothScroll_) {
            QWheelEvent adjustedEvent(wheelEvent->position(), wheelEvent->globalPosition(),
                                      wheelEvent->pixelDelta(), angleDelta,
                                      wheelEvent->buttons(), wheelEvent->modifiers(),
                                      wheelEvent->phase(), wheelEvent->inverted());
            horizonSmoothScroll_->wheelEvent(&adjustedEvent);
        }
    }

    wheelEvent->setAccepted(true);
    return true;
}

void SmoothScrollDelegate::setVerticalScrollBarPolicy(Qt::ScrollBarPolicy policy) {
    auto* p = static_cast<QAbstractScrollArea*>(parent());
    p->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    vScrollBar_->setForceHidden(policy == Qt::ScrollBarAlwaysOff);
}

void SmoothScrollDelegate::setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy policy) {
    auto* p = static_cast<QAbstractScrollArea*>(parent());
    p->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    hScrollBar_->setForceHidden(policy == Qt::ScrollBarAlwaysOff);
}

void SmoothScrollDelegate::setSmoothMode(SmoothMode mode, Qt::Orientation orientation) {
    if (orientation == Qt::Vertical) {
        if (verticalSmoothScroll_) {
            verticalSmoothScroll_->setSmoothMode(mode);
        }
        return;
    }

    if (horizonSmoothScroll_) {
        horizonSmoothScroll_->setSmoothMode(mode);
    }
}

void SmoothScrollDelegate::setScrollAnimation(Qt::Orientation orientation, int duration,
                                              QEasingCurve::Type easing) {
    if (orientation == Qt::Horizontal) {
        if (hScrollBar_) {
            hScrollBar_->setScrollAnimation(duration, easing);
        }
        return;
    }

    if (vScrollBar_) {
        vScrollBar_->setScrollAnimation(duration, easing);
    }
}

}  // namespace qfw
