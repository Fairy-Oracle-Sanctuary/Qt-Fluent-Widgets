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

#ifdef Q_OS_LINUX
#include <QX11Info>

#include <X11/Xlib.h>
#endif

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
#ifdef Q_OS_LINUX
        // Use X11 for window resizing on Linux
        QWindow* win = window_->windowHandle();
        if (win) {
            // Convert edges to X11 gravity
            uint gravity = 0;
            if (edges & Qt::LeftEdge) {
                gravity |= 1;  // West
            }
            if (edges & Qt::RightEdge) {
                gravity |= 2;  // East
            }
            if (edges & Qt::TopEdge) {
                gravity |= 4;  // North
            }
            if (edges & Qt::BottomEdge) {
                gravity |= 8;  // South
            }

            // Map Qt gravity to X11 gravity
            // X11: 1=SizeTopLeft, 2=SizeTop, 3=SizeTopRight, etc.
            static const uint x11Gravity[] = {0, 6, 10, 2, 12, 8, 4, 5, 3, 11, 7, 9, 1};

            Display* display = QX11Info::display();
            if (display) {
                WId wid = win->winId();
                XEvent xev;
                memset(&xev, 0, sizeof(xev));

                xev.xclient.type = ClientMessage;
                xev.xclient.window = wid;
                xev.xclient.message_type =
                    XInternAtom(display, "_NET_WM_MOVERESIZE", False);
                xev.xclient.format = 32;
                xev.xclient.data.l[0] = mouseEvent->globalPos().x();
                xev.xclient.data.l[1] = mouseEvent->globalPos().y();
                xev.xclient.data.l[2] = x11Gravity[gravity];
                xev.xclient.data.l[3] = 0;  // Button 1
                xev.xclient.data.l[4] = 0;

                XUngrabPointer(display, CurrentTime);
                XSendEvent(display, QX11Info::appRootWindow(QX11Info::appScreen()), False,
                           SubstructureRedirectMask | SubstructureNotifyMask, &xev);
                XFlush(display);

                return true;
            }
        }
#endif
    }

    return false;
}

// ============================================================================
// LinuxFramelessWindow
// ============================================================================

LinuxFramelessWindow::LinuxFramelessWindow(QWidget* parent)
    : QWidget(parent), LinuxFramelessWindowBase() {
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

    if (!framelessInitialized_) {
        framelessInitialized_ = true;
        initFrameless(this);
    }
}

// ============================================================================
// LinuxFramelessMainWindow
// ============================================================================

LinuxFramelessMainWindow::LinuxFramelessMainWindow(QWidget* parent)
    : QMainWindow(parent), LinuxFramelessWindowBase() {
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

    if (!framelessInitialized_) {
        framelessInitialized_ = true;
        initFrameless(this);
    }
}

// ============================================================================
// LinuxFramelessDialog
// ============================================================================

LinuxFramelessDialog::LinuxFramelessDialog(QWidget* parent)
    : QDialog(parent), LinuxFramelessWindowBase() {
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

    if (!framelessInitialized_) {
        framelessInitialized_ = true;
        initFrameless(this);
    }
}

}  // namespace qfw
