/**
 * Linux Frameless Window Implementation
 * Based on PyQt-Frameless-Window Linux implementation
 */

#include "components/window/linux_frameless_window.h"

#include <QApplication>
#include <QDebug>
#include <QMouseEvent>
#include <QScreen>
#include <QWindow>

namespace qfw {

// ============================================================================
// LinuxFramelessWindowBase
// ============================================================================

LinuxFramelessWindowBase::LinuxFramelessWindowBase() = default;

LinuxFramelessWindowBase::~LinuxFramelessWindowBase() = default;

void LinuxFramelessWindowBase::setResizeEnabled(bool enabled) { resizeEnabled_ = enabled; }

bool LinuxFramelessWindowBase::isResizeEnabled() const { return resizeEnabled_; }

void LinuxFramelessWindowBase::setStayOnTop(bool isTop) {
    if (!window_) {
        return;
    }

    Qt::WindowFlags flags = window_->windowFlags();
    if (isTop) {
        flags |= Qt::WindowStaysOnTopHint;
    } else {
        flags &= ~Qt::WindowStaysOnTopHint;
    }

    window_->setWindowFlags(flags);
    window_->show();
}

void LinuxFramelessWindowBase::toggleStayOnTop() {
    if (!window_) {
        return;
    }

    bool isTop = window_->windowFlags() & Qt::WindowStaysOnTopHint;
    setStayOnTop(!isTop);
}

bool LinuxFramelessWindowBase::isSystemButtonVisible() const { return isSystemButtonVisible_; }

void LinuxFramelessWindowBase::setSystemTitleBarButtonVisible(bool visible) {
    isSystemButtonVisible_ = visible;
}

QRect LinuxFramelessWindowBase::systemTitleBarRect(const QSize& size) const {
    // Only meaningful on macOS, return full size on Linux
    return QRect(0, 0, size.width(), size.height());
}

void LinuxFramelessWindowBase::initFrameless(QWidget* window) {
    window_ = window;

    // Set frameless window hint
    Qt::WindowFlags flags = window_->windowFlags();
    flags |= Qt::FramelessWindowHint;
    window_->setWindowFlags(flags);

    // Create window effect (stub on Linux)
    windowEffect_ = new LinuxWindowEffect(window_);
}

bool LinuxFramelessWindowBase::handleEventFilter(QObject* obj, QEvent* event) {
    if (!resizeEnabled_ || !window_) {
        return false;
    }

    QEvent::Type et = event->type();

    // Only handle mouse move and button press
    if (et != QEvent::MouseMove && et != QEvent::MouseButtonPress) {
        return false;
    }

    // Calculate edges for resize
    Qt::Edges edges;

    QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
    QPoint pos = mouseEvent->globalPos() - window_->pos();

    if (pos.x() < BORDER_WIDTH) {
        edges |= Qt::LeftEdge;
    }
    if (pos.x() >= window_->width() - BORDER_WIDTH) {
        edges |= Qt::RightEdge;
    }
    if (pos.y() < BORDER_WIDTH) {
        edges |= Qt::TopEdge;
    }
    if (pos.y() >= window_->height() - BORDER_WIDTH) {
        edges |= Qt::BottomEdge;
    }

    // Change cursor on mouse move
    if (et == QEvent::MouseMove && window_->windowState() == Qt::WindowNoState) {
        if (edges == (Qt::LeftEdge | Qt::TopEdge) || edges == (Qt::RightEdge | Qt::BottomEdge)) {
            window_->setCursor(Qt::SizeFDiagCursor);
        } else if (edges == (Qt::RightEdge | Qt::TopEdge) ||
                   edges == (Qt::LeftEdge | Qt::BottomEdge)) {
            window_->setCursor(Qt::SizeBDiagCursor);
        } else if (edges == Qt::TopEdge || edges == Qt::BottomEdge) {
            window_->setCursor(Qt::SizeVerCursor);
        } else if (edges == Qt::LeftEdge || edges == Qt::RightEdge) {
            window_->setCursor(Qt::SizeHorCursor);
        } else {
            window_->setCursor(Qt::ArrowCursor);
        }
    }

    // Start system resize on button press
    if (et == QEvent::MouseButtonPress && edges) {
        // Use Qt6 public API for system resize (works on Linux/X11 and Wayland)
        QWindow* win = window_->windowHandle();
        if (win) {
            win->startSystemResize(edges);
            return true;
        }
    }

    return false;
}

// ============================================================================
// LinuxFramelessWindow
// ============================================================================

LinuxFramelessWindow::LinuxFramelessWindow(QWidget* parent)
    : QWidget(parent), LinuxFramelessWindowBase() {
    initFrameless(this);
    framelessInitialized_ = true;
}

bool LinuxFramelessWindow::event(QEvent* e) {
    if (e->type() == QEvent::MouseMove || e->type() == QEvent::MouseButtonPress) {
        if (handleEventFilter(this, e)) {
            return true;
        }
    }
    return QWidget::event(e);
}

void LinuxFramelessWindow::showEvent(QShowEvent* e) {
    QWidget::showEvent(e);
}

// ============================================================================
// LinuxFramelessMainWindow
// ============================================================================

LinuxFramelessMainWindow::LinuxFramelessMainWindow(QWidget* parent)
    : QMainWindow(parent), LinuxFramelessWindowBase() {
    initFrameless(this);
    framelessInitialized_ = true;
}

bool LinuxFramelessMainWindow::event(QEvent* e) {
    if (e->type() == QEvent::MouseMove || e->type() == QEvent::MouseButtonPress) {
        if (handleEventFilter(this, e)) {
            return true;
        }
    }
    return QMainWindow::event(e);
}

void LinuxFramelessMainWindow::showEvent(QShowEvent* e) {
    QMainWindow::showEvent(e);
}

// ============================================================================
// LinuxFramelessDialog
// ============================================================================

LinuxFramelessDialog::LinuxFramelessDialog(QWidget* parent)
    : QDialog(parent), LinuxFramelessWindowBase() {
    initFrameless(this);
    framelessInitialized_ = true;
}

bool LinuxFramelessDialog::event(QEvent* e) {
    if (e->type() == QEvent::MouseMove || e->type() == QEvent::MouseButtonPress) {
        if (handleEventFilter(this, e)) {
            return true;
        }
    }
    return QDialog::event(e);
}

void LinuxFramelessDialog::showEvent(QShowEvent* e) {
    QDialog::showEvent(e);
}

}  // namespace qfw
