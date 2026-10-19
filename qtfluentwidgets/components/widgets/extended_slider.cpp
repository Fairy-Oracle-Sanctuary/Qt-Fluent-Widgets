#include "components/widgets/extended_slider.h"

#include <QHideEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>
#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "common/color.h"
#include "common/config.h"
#include "components/widgets/tool_tip.h"

namespace qfw {
ToolTipSlider::ToolTipSlider(QWidget* parent) : ToolTipSlider(Qt::Horizontal, parent) {}
ToolTipSlider::ToolTipSlider(Qt::Orientation orientation, QWidget* parent)
    : Slider(orientation, parent), hideTimer_(this) {
    handle_ = findChild<SliderHandle*>();
    if (handle_) {
        handle_->installEventFilter(this);
    }
    hideTimer_.setSingleShot(true);
    connect(&hideTimer_, &QTimer::timeout, this, &ToolTipSlider::hideToolTip);
    connect(this, &QSlider::valueChanged, this, [this] { showToolTip(); });
}
void ToolTipSlider::showToolTip() {
    if (!isVisible() || !isEnabled() || !handle_) {
        return;
    }
    if (!tip_) {
        tip_ = new ToolTip(QString(), this);
        tip_->setDuration(-1);
    }
    tip_->setText(QString::number(value()));
    tip_->adjustPos(
        handle_, orientation() == Qt::Horizontal ? ToolTipPosition::Top : ToolTipPosition::Right);
    tip_->show();
    hideTimer_.start(500);
}
void ToolTipSlider::hideToolTip() {
    hideTimer_.stop();
    if (tip_) {
        tip_->hide();
    }
}
void ToolTipSlider::enterEvent(enterEvent_QEnterEvent* event) {
    Slider::enterEvent(event);
    showToolTip();
}
void ToolTipSlider::leaveEvent(QEvent* event) {
    Slider::leaveEvent(event);
    if (!dragging_) {
        hideTimer_.start(200);
    }
}
void ToolTipSlider::mousePressEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton || !isEnabled()) {
        event->ignore();
        return;
    }
    dragging_ = true;
    setFocus(Qt::MouseFocusReason);
    setSliderDown(true);
    Slider::mousePressEvent(event);
    showToolTip();
    hideTimer_.stop();
}
void ToolTipSlider::mouseMoveEvent(QMouseEvent* event) {
    if (dragging_) {
        Slider::mouseMoveEvent(event);
        showToolTip();
        hideTimer_.stop();
    } else {
        event->ignore();
    }
}
void ToolTipSlider::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && dragging_) {
        finishDrag();
        hideTimer_.start(500);
        event->accept();
    } else {
        Slider::mouseReleaseEvent(event);
    }
}
void ToolTipSlider::moveFromHandle(QMouseEvent* event) {
    const QPoint point = handle_->mapTo(this, event->pos()) + pressOffset_;
    QMouseEvent moved(QEvent::MouseMove, QPointF(point), Qt::NoButton, event->buttons(),
                      event->modifiers());
    Slider::mouseMoveEvent(&moved);
    showToolTip();
    hideTimer_.stop();
}
bool ToolTipSlider::eventFilter(QObject* watched, QEvent* event) {
    if (watched == handle_) {
        if (event->type() == QEvent::Enter) {
            showToolTip();
        } else if (event->type() == QEvent::MouseButtonPress) {
            auto* mouse = static_cast<QMouseEvent*>(event);
            if (mouse->button() != Qt::LeftButton) {
                return true;
            }
            if (mouse->button() == Qt::LeftButton && isEnabled()) {
                dragging_ = true;
                setFocus(Qt::MouseFocusReason);
                pressOffset_ = QPoint(handle_->width() / 2, handle_->height() / 2) - mouse->pos();
                setSliderDown(true);
                if (auto* animation = handle_->findChild<QPropertyAnimation*>()) {
                    animation->stop();
                }
                handle_->setRadius(4);
                showToolTip();
                hideTimer_.stop();
                mouse->accept();
                return true;
            }
        } else if (event->type() == QEvent::MouseMove && dragging_) {
            moveFromHandle(static_cast<QMouseEvent*>(event));
            return true;
        } else if (event->type() == QEvent::MouseButtonRelease) {
            auto* mouse = static_cast<QMouseEvent*>(event);
            if (mouse->button() != Qt::LeftButton) {
                return true;
            }
            if (dragging_) {
                finishDrag();
                hideTimer_.start(500);
                mouse->accept();
                return true;
            }
        }
    }
    return Slider::eventFilter(watched, event);
}
void ToolTipSlider::hideEvent(QHideEvent* event) {
    finishDrag();
    hideToolTip();
    Slider::hideEvent(event);
}
void ToolTipSlider::changeEvent(QEvent* event) {
    Slider::changeEvent(event);
    if (event->type() == QEvent::EnabledChange && !isEnabled()) {
        finishDrag();
        hideToolTip();
    }
}

void ToolTipSlider::finishDrag() {
    dragging_ = false;
    setSliderDown(false);
    if (handle_) {
        if (auto* animation = handle_->findChild<QPropertyAnimation*>()) {
            animation->stop();
        }
        handle_->setRadius(5);
    }
}

RangeSlider::RangeSlider(QWidget* parent) : RangeSlider(Qt::Horizontal, parent) {}
RangeSlider::RangeSlider(Qt::Orientation orientation, QWidget* parent)
    : QWidget(parent), orientation_(orientation), hideTimer_(this) {
    setFocusPolicy(Qt::StrongFocus);
    setAccessibleName(tr("Range"));
    setOrientation(orientation);
    hideTimer_.setSingleShot(true);
    connect(&hideTimer_, &QTimer::timeout, this, &RangeSlider::hideToolTips);
    connect(&QConfig::instance(), &QConfig::themeChanged, this, [this] { update(); });
    connect(&QConfig::instance(), &QConfig::themeColorChanged, this, [this] { update(); });
    commitValues(minValue_, maxValue_);
}
Qt::Orientation RangeSlider::orientation() const { return orientation_; }
void RangeSlider::setOrientation(Qt::Orientation orientation) {
    if (orientation != Qt::Horizontal && orientation != Qt::Vertical) {
        throw std::invalid_argument("Invalid range slider orientation");
    }
    orientation_ = orientation;
    setMinimumSize(0, 0);
    setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    if (orientation == Qt::Horizontal) {
        setFixedHeight(22);
        setMinimumWidth(100);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    } else {
        setFixedWidth(22);
        setMinimumHeight(100);
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    }
    hideToolTips();
    updateGeometry();
    update();
}
int RangeSlider::minimum() const { return minimum_; }
int RangeSlider::maximum() const { return maximum_; }
void RangeSlider::setMinimum(int value) { setRange(value, std::max(value, maximum_)); }
void RangeSlider::setMaximum(int value) { setRange(std::min(value, minimum_), value); }
qint64 RangeSlider::effectiveGap() const {
    return std::min(qint64(gap_), qint64(maximum_) - minimum_);
}
void RangeSlider::setRange(int minimum, int maximum) {
    if (minimum > maximum) {
        throw std::invalid_argument("Range slider minimum exceeds maximum");
    }
    minimum_ = minimum;
    maximum_ = maximum;
    const int high = int(std::max(qint64(std::clamp(maxValue_, minimum_, maximum_)),
                                  qint64(minimum_) + effectiveGap()));
    const int low = int(
        std::min(qint64(std::clamp(minValue_, minimum_, maximum_)), qint64(high) - effectiveGap()));
    commitValues(low, high);
    update();
}
int RangeSlider::minValue() const { return minValue_; }
int RangeSlider::maxValue() const { return maxValue_; }
void RangeSlider::setMinValue(int value) {
    commitValues(
        int(std::clamp(qint64(value), qint64(minimum_), qint64(maxValue_) - effectiveGap())),
        maxValue_);
}
void RangeSlider::setMaxValue(int value) {
    commitValues(minValue_, int(std::clamp(qint64(value), qint64(minValue_) + effectiveGap(),
                                           qint64(maximum_))));
}
void RangeSlider::setValues(int low, int high) {
    low = std::clamp(low, minimum_, maximum_);
    high = std::clamp(high, minimum_, maximum_);
    if (qint64(high) - low < effectiveGap()) {
        return;
    }
    commitValues(low, high);
}
void RangeSlider::commitValues(int low, int high) {
    const bool lowChanged = low != minValue_, highChanged = high != maxValue_;
    minValue_ = low;
    maxValue_ = high;
    update();
    setAccessibleDescription(QStringLiteral("%1 – %2").arg(low).arg(high));
    if (lowChanged) {
        emit minValueChanged(low);
    }
    if (highChanged) {
        emit maxValueChanged(high);
    }
    if (lowChanged || highChanged) {
        emit rangeChanged(low, high);
        if (tips_[0] && tips_[0]->isVisible()) {
            showToolTips();
        }
    }
}
int RangeSlider::minimumGap() const { return gap_; }
void RangeSlider::setMinimumGap(int gap) {
    if (gap < 0) {
        throw std::invalid_argument("Range slider gap must be nonnegative");
    }
    gap_ = gap;
    const int high = int(std::min(qint64(maximum_),
                                  std::max(qint64(maxValue_), qint64(minValue_) + effectiveGap())));
    commitValues(int(std::min(qint64(minValue_), qint64(high) - effectiveGap())), high);
}
int RangeSlider::singleStep() const { return step_; }
void RangeSlider::setSingleStep(int step) {
    if (step <= 0) {
        throw std::invalid_argument("Range slider step must be positive");
    }
    step_ = step;
}
void RangeSlider::setThemeColor(const QColor& light, const QColor& dark) {
    light_ = light;
    dark_ = dark;
    update();
}
bool RangeSlider::isToolTipEnabled() const { return toolTipEnabled_; }
void RangeSlider::setToolTipEnabled(bool enabled) {
    toolTipEnabled_ = enabled;
    if (!enabled) {
        hideToolTips();
    }
}
QSize RangeSlider::sizeHint() const {
    return orientation_ == Qt::Horizontal ? QSize(200, 22) : QSize(22, 200);
}
QSize RangeSlider::minimumSizeHint() const {
    return orientation_ == Qt::Horizontal ? QSize(100, 22) : QSize(22, 100);
}
QPointF RangeSlider::handleCenter(int value) const {
    const qreal length = std::max(0, (orientation_ == Qt::Horizontal ? width() : height()) - 22);
    const qreal total = qreal(qint64(maximum_) - minimum_);
    qreal fraction = total ? (qreal(value) - minimum_) / total : 0;
    if (orientation_ == Qt::Horizontal && layoutDirection() == Qt::RightToLeft) {
        fraction = 1 - fraction;
    }
    const qreal axis = 11 + fraction * length;
    return orientation_ == Qt::Horizontal ? QPointF(axis, height() / 2.0)
                                          : QPointF(width() / 2.0, axis);
}
int RangeSlider::valueAt(const QPoint& pos) const {
    const qreal length = std::max(1, (orientation_ == Qt::Horizontal ? width() : height()) - 22);
    qreal fraction = std::clamp(
        ((orientation_ == Qt::Horizontal ? pos.x() : pos.y()) - 11.0) / length, 0.0, 1.0);
    if (orientation_ == Qt::Horizontal && layoutDirection() == Qt::RightToLeft) {
        fraction = 1 - fraction;
    }
    return int(std::clamp(qint64(std::llround(minimum_ + fraction * (qint64(maximum_) - minimum_))),
                          qint64(minimum_), qint64(maximum_)));
}
void RangeSlider::dragTo(const QPoint& pos) {
    if (pressed_ == 0) {
        setMinValue(valueAt(pos));
    } else if (pressed_ == 1) {
        setMaxValue(valueAt(pos));
    }
    showToolTips();
    hideTimer_.stop();
}
void RangeSlider::mousePressEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton || !isEnabled()) {
        event->ignore();
        return;
    }
    const QPointF low = handleCenter(minValue_), high = handleCenter(maxValue_);
    const qreal axis = orientation_ == Qt::Horizontal ? event->pos().x() : event->pos().y();
    const qreal ld = std::abs(axis - (orientation_ == Qt::Horizontal ? low.x() : low.y()));
    const qreal hd = std::abs(axis - (orientation_ == Qt::Horizontal ? high.x() : high.y()));
    pressed_ = ld < hd ? 0 : hd < ld ? 1 : active_;
    active_ = pressed_;
    setFocus(Qt::MouseFocusReason);
    emit sliderPressed(pressed_);
    dragTo(event->pos());
    event->accept();
}
void RangeSlider::mouseMoveEvent(QMouseEvent* event) {
    if (pressed_ >= 0 && isEnabled()) {
        dragTo(event->pos());
        event->accept();
    } else {
        QWidget::mouseMoveEvent(event);
    }
}
void RangeSlider::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && pressed_ >= 0) {
        const int handle = pressed_;
        pressed_ = -1;
        hideTimer_.start(300);
        update();
        emit sliderReleased(handle);
        event->accept();
    } else {
        QWidget::mouseReleaseEvent(event);
    }
}
void RangeSlider::stepActive(qint64 step) {
    const int value = active_ == 0 ? minValue_ : maxValue_;
    const int next = int(std::clamp(qint64(value) + step, qint64(minimum_), qint64(maximum_)));
    if (active_ == 0) {
        setMinValue(next);
    } else {
        setMaxValue(next);
    }
    showToolTips();
    hideTimer_.start(500);
}
void RangeSlider::keyPressEvent(QKeyEvent* event) {
    if (!isEnabled()) {
        QWidget::keyPressEvent(event);
        return;
    }
    if (event->modifiers() & Qt::ControlModifier &&
        (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right)) {
        active_ = event->key() == Qt::Key_Left ? 0 : 1;
        update();
        event->accept();
        return;
    }
    switch (event->key()) {
        case Qt::Key_Left:
            stepActive((layoutDirection() == Qt::RightToLeft ? 1LL : -1LL) * step_);
            break;
        case Qt::Key_Right:
            stepActive((layoutDirection() == Qt::RightToLeft ? -1LL : 1LL) * step_);
            break;
        case Qt::Key_Up:
            stepActive(-qint64(step_));
            break;
        case Qt::Key_Down:
            stepActive(step_);
            break;
        case Qt::Key_PageUp:
            stepActive(qint64(step_) * 10);
            break;
        case Qt::Key_PageDown:
            stepActive(-qint64(step_) * 10);
            break;
        case Qt::Key_Home:
            if (active_ == 0) {
                setMinValue(minimum_);
            } else {
                setMaxValue(minValue_);
            }
            break;
        case Qt::Key_End:
            if (active_ == 0) {
                setMinValue(maxValue_);
            } else {
                setMaxValue(maximum_);
            }
            break;
        default:
            QWidget::keyPressEvent(event);
            return;
    }
    event->accept();
}
void RangeSlider::wheelEvent(QWheelEvent* event) {
    if (!hasFocus() || !isEnabled()) {
        event->ignore();
        return;
    }
    if (event->angleDelta().y()) {
        stepActive(event->angleDelta().y() > 0 ? step_ : -qint64(step_));
        event->accept();
    } else {
        event->ignore();
    }
}
void RangeSlider::showToolTips() {
    if (!isVisible() || !isEnabled() || !toolTipEnabled_) {
        return;
    }
    const int values[] = {minValue_, maxValue_};
    for (int i = 0; i < 2; ++i) {
        if (!tips_[i]) {
            tips_[i] = new ToolTip(QString(), this);
            tips_[i]->setDuration(-1);
        }
        tips_[i]->setText(QString::number(values[i]));
        const QPoint center = mapToGlobal(handleCenter(values[i]).toPoint());
        if (orientation_ == Qt::Horizontal) {
            tips_[i]->move(center.x() - tips_[i]->width() / 2,
                           center.y() - 11 - tips_[i]->height());
        } else {
            tips_[i]->move(center.x() + 11, center.y() - tips_[i]->height() / 2);
        }
        tips_[i]->show();
    }
}
void RangeSlider::hideToolTips() {
    hideTimer_.stop();
    for (auto& tip : tips_) {
        if (tip) {
            tip->hide();
        }
    }
}
void RangeSlider::hideEvent(QHideEvent* event) {
    pressed_ = -1;
    hideToolTips();
    QWidget::hideEvent(event);
}
void RangeSlider::changeEvent(QEvent* event) {
    QWidget::changeEvent(event);
    if (event->type() == QEvent::EnabledChange && !isEnabled()) {
        pressed_ = -1;
        hideToolTips();
    }
    update();
}
void RangeSlider::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    if (tips_[0] && tips_[0]->isVisible()) {
        showToolTips();
    }
}
void RangeSlider::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    if (!isEnabled()) {
        painter.setOpacity(0.43);
    }
    painter.setBrush(isDarkTheme() ? QColor(255, 255, 255, 115) : QColor(0, 0, 0, 100));
    if (orientation_ == Qt::Horizontal) {
        painter.drawRoundedRect(QRectF(11, height() / 2.0 - 2, std::max(0, width() - 22), 4), 2, 2);
    } else {
        painter.drawRoundedRect(QRectF(width() / 2.0 - 2, 11, 4, std::max(0, height() - 22)), 2, 2);
    }
    const auto low = handleCenter(minValue_), high = handleCenter(maxValue_);
    const QColor accent = autoFallbackThemeColor(light_, dark_);
    painter.setBrush(accent);
    if (orientation_ == Qt::Horizontal) {
        painter.drawRoundedRect(QRectF(std::min(low.x(), high.x()), height() / 2.0 - 2,
                                       std::abs(high.x() - low.x()), 4),
                                2, 2);
    } else {
        painter.drawRoundedRect(QRectF(width() / 2.0 - 2, low.y(), 4, high.y() - low.y()), 2, 2);
    }
    const int first = active_ == 0 ? 1 : 0;
    const int order[] = {first, active_};
    for (int i : order) {
        const auto center = i == 0 ? low : high;
        painter.setPen(QColor(0, 0, 0, isDarkTheme() ? 90 : 25));
        painter.setBrush(isDarkTheme() ? QColor(69, 69, 69) : QColor(Qt::white));
        painter.drawEllipse(center, 10, 10);
        painter.setPen(Qt::NoPen);
        painter.setBrush(accent);
        painter.drawEllipse(center, pressed_ == i ? 4 : 5, pressed_ == i ? 4 : 5);
    }
}
}  // namespace qfw
