#include "components/widgets/scroll_area.h"

#include <QKeyEvent>
#include <QScrollBar>
#include <QWheelEvent>

#include "components/widgets/scroll_bar.h"

namespace qfw {

ScrollArea::ScrollArea(QWidget* parent) : QScrollArea(parent) {
    scrollDelegate_ = new SmoothScrollDelegate(this);
}

void ScrollArea::setSmoothMode(SmoothMode mode, Qt::Orientation orientation) {
    if (scrollDelegate_) {
        scrollDelegate_->setSmoothMode(mode, orientation);
    }
}

void ScrollArea::enableTransparentBackground() {
    setStyleSheet(QStringLiteral("QScrollArea{border: none; background: transparent}"));
    if (widget()) {
        widget()->setStyleSheet(QStringLiteral("QWidget{background: transparent}"));
    }
}

void ScrollArea::setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy policy) {
    if (scrollDelegate_) {
        scrollDelegate_->setHorizontalScrollBarPolicy(policy);
    }
}

void ScrollArea::setVerticalScrollBarPolicy(Qt::ScrollBarPolicy policy) {
    if (scrollDelegate_) {
        scrollDelegate_->setVerticalScrollBarPolicy(policy);
    }
}

// ============================================================================
// SingleDirectionScrollArea
// ============================================================================
SingleDirectionScrollArea::SingleDirectionScrollArea(QWidget* parent, Qt::Orientation orient)
    : QScrollArea(parent), orient_(orient) {
    smoothScroll_ = new SmoothScroll(this, orient, this);
    vScrollBar_ = new SmoothScrollBar(Qt::Vertical, this);
    hScrollBar_ = new SmoothScrollBar(Qt::Horizontal, this);

    // Install event filter on viewport to intercept wheel events
    viewport()->installEventFilter(this);
}

void SingleDirectionScrollArea::setVerticalScrollBarPolicy(Qt::ScrollBarPolicy policy) {
    QScrollArea::setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    if (vScrollBar_) {
        vScrollBar_->setForceHidden(policy == Qt::ScrollBarAlwaysOff);
    }
}

void SingleDirectionScrollArea::setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy policy) {
    QScrollArea::setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    if (hScrollBar_) {
        hScrollBar_->setForceHidden(policy == Qt::ScrollBarAlwaysOff);
    }
}

void SingleDirectionScrollArea::setSmoothMode(SmoothMode mode) {
    if (smoothScroll_) {
        smoothScroll_->setSmoothMode(mode);
    }
}

void SingleDirectionScrollArea::keyPressEvent(QKeyEvent* e) {
    if (e && (e->key() == Qt::Key_Left || e->key() == Qt::Key_Right)) {
        return;
    }
    QScrollArea::keyPressEvent(e);
}

void SingleDirectionScrollArea::wheelEvent(QWheelEvent* e) {
    if (!e) {
        return;
    }

    // Use smooth scroll if available
    if (smoothScroll_) {
        smoothScroll_->wheelEvent(e);
        return;
    }

    QScrollArea::wheelEvent(e);
}

bool SingleDirectionScrollArea::eventFilter(QObject* watched, QEvent* event) {
    if (watched == viewport() && event->type() == QEvent::Wheel) {
        auto* wheelEvent = static_cast<QWheelEvent*>(event);
        if (smoothScroll_) {
            smoothScroll_->wheelEvent(wheelEvent);
            return true;
        }
    }
    return QScrollArea::eventFilter(watched, event);
}

void SingleDirectionScrollArea::enableTransparentBackground() {
    setStyleSheet(QStringLiteral("QScrollArea{border: none; background: transparent}"));
    if (widget()) {
        widget()->setStyleSheet(QStringLiteral("QWidget{background: transparent}"));
    }
}

// ============================================================================
// SmoothScrollArea
// ============================================================================
SmoothScrollArea::SmoothScrollArea(QWidget* parent) : QScrollArea(parent) {
    delegate_ = new SmoothScrollDelegate(this, true);
}

void SmoothScrollArea::setScrollAnimation(Qt::Orientation orient, int duration,
                                          QEasingCurve::Type easing) {
    if (delegate_) {
        delegate_->setScrollAnimation(orient, duration, easing);
    }
}

void SmoothScrollArea::enableTransparentBackground() {
    setStyleSheet(QStringLiteral("QScrollArea{border: none; background: transparent}"));
    if (widget()) {
        widget()->setStyleSheet(QStringLiteral("QWidget{background: transparent}"));
    }
}

}  // namespace qfw
