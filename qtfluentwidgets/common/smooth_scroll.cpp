#include "common/smooth_scroll.h"

#include <QAbstractScrollArea>
#include <QScrollBar>
#include <QWheelEvent>
#include <QDebug>
#include <cmath>

namespace qfw {

SmoothScroll::SmoothScroll(QAbstractScrollArea* widget, Qt::Orientation orient, QObject* parent)
    : QObject(parent), widget_(widget), orient_(orient) {
    // Setup timer - no event filter, will be called directly by SmoothScrollDelegate
    smoothMoveTimer_.setSingleShot(false);
    connect(&smoothMoveTimer_, &QTimer::timeout, this, &SmoothScroll::smoothMove);
}

void SmoothScroll::setSmoothMode(SmoothMode mode) { smoothMode_ = mode; }

void SmoothScroll::wheelEvent(QWheelEvent* e) {
    int delta = e->angleDelta().y() != 0 ? e->angleDelta().y() : e->angleDelta().x();
    
    // Also check pixelDelta for macOS trackpad support
    if (delta == 0) {
        delta = e->pixelDelta().y() != 0 ? e->pixelDelta().y() : e->pixelDelta().x();
    }

    // For non-smooth mode, forward to the scroll area
    if (smoothMode_ == SmoothMode::NoSmooth) {
        QApplication::sendEvent(widget_, e);
        return;
    }

    // Handle all wheel events with smooth scrolling (including trackpad)
    handleWheelEvent(e);
}

void SmoothScroll::handleWheelEvent(QWheelEvent* e) {
    int delta = e->angleDelta().y() != 0 ? e->angleDelta().y() : e->angleDelta().x();

    // Push current time to queue
    qint64 now = QDateTime::currentDateTime().toMSecsSinceEpoch();
    scrollStamps_.append(now);

    // Remove timestamps older than 500ms
    while (!scrollStamps_.isEmpty() && now - scrollStamps_.first() > 500) {
        scrollStamps_.removeFirst();
    }

    // Adjust the acceleration ratio based on unprocessed events
    double accelerationRatio = std::min(scrollStamps_.size() / 15.0, 1.0);
    lastWheelPos_ = e->position().toPoint();
    lastWheelGlobalPos_ = e->globalPosition().toPoint();

    // Get the number of steps
    stepsTotal_ = fps_ * duration_ / 1000;

    // Get the moving distance corresponding to each event
    double adjustedDelta = delta * stepRatio_;
    if (acceleration_ > 0) {
        adjustedDelta += adjustedDelta * acceleration_ * accelerationRatio;
    }

    // Form a list of moving distances and steps, and insert it into the queue for processing
    stepsLeftQueue_.append(qMakePair(adjustedDelta, stepsTotal_));

    // Start timer: 1000ms/frames
    smoothMoveTimer_.start(1000 / fps_);
}

void SmoothScroll::smoothMove() {
    double totalDelta = 0;

    // Calculate the scrolling distance of all unprocessed events
    for (auto& item : stepsLeftQueue_) {
        totalDelta += subDelta(item.first, item.second, stepsTotal_);
        item.second--;
    }

    // If the event has been processed, move it out of the queue
    while (!stepsLeftQueue_.isEmpty() && stepsLeftQueue_.first().second == 0) {
        stepsLeftQueue_.removeFirst();
    }

    // Construct wheel event
    QPoint pixelDelta;
    QScrollBar* bar;

    if (orient_ == Qt::Vertical) {
        pixelDelta = QPoint(0, std::round(totalDelta));
        bar = widget_->verticalScrollBar();
    } else {
        pixelDelta = QPoint(std::round(totalDelta), 0);
        bar = widget_->horizontalScrollBar();
    }

    QWheelEvent wheelEvent(lastWheelPos_, lastWheelGlobalPos_, pixelDelta,
                           QPoint(std::round(totalDelta), 0), Qt::LeftButton, Qt::NoModifier,
                           Qt::ScrollBegin, false);

    // Send wheel event to app
    QApplication::sendEvent(bar, &wheelEvent);

    // Stop scrolling if the queue is empty
    if (stepsLeftQueue_.isEmpty()) {
        smoothMoveTimer_.stop();
    }
}

double SmoothScroll::subDelta(double delta, int stepsLeft, int stepsTotal) const {
    int m = stepsTotal / 2;
    int x = std::abs(stepsTotal - stepsLeft - m);

    double result = 0;
    switch (smoothMode_) {
        case SmoothMode::NoSmooth:
            result = 0;
            break;
        case SmoothMode::Constant:
            result = delta / stepsTotal;
            break;
        case SmoothMode::Linear:
            result = 2 * delta / stepsTotal * (m - x) / m;
            break;
        case SmoothMode::Quadratic:
            result = 3.0 / 4.0 / m * (1 - x * x / (double)m / m) * delta;
            break;
        case SmoothMode::Cosine:
            result = (std::cos(x * M_PI / m) + 1) / (2 * m) * delta;
            break;
    }

    return result;
}

}  // namespace qfw
