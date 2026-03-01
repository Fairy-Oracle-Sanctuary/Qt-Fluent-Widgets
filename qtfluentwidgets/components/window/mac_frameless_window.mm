#include "components/window/mac_frameless_window.h"

#ifdef Q_OS_MAC

#include <QDebug>
#include <QLayout>
#include <QTimer>
#include <QWheelEvent>
#include <QApplication>

#import <Cocoa/Cocoa.h>

namespace qfw {

MacFramelessWindowBase::MacFramelessWindowBase() = default;
MacFramelessWindowBase::~MacFramelessWindowBase() = default;

void MacFramelessWindowBase::setResizeEnabled(bool enabled) { resizeEnabled_ = enabled; }

bool MacFramelessWindowBase::isResizeEnabled() const { return resizeEnabled_; }

void MacFramelessWindowBase::initFrameless(QWidget* window) {
    if (!window) {
        return;
    }

    // Ensure this widget is treated as a top-level window.
    window->setWindowFlag(Qt::Window, true);

    // Scheme 1 baseline: use Qt frameless hint.
    const Qt::WindowFlags stayOnTop = (window->windowFlags() & Qt::WindowStaysOnTopHint)
                                          ? Qt::WindowStaysOnTopHint
                                          : Qt::WindowFlags{};

    window->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | window->windowFlags() | stayOnTop);

    // This window style is typically required for translucent titlebar/vibrancy.
    window->setAttribute(Qt::WA_TranslucentBackground, true);
    window->setAutoFillBackground(false);

    // Fix trackpad scrolling not working on macOS with frameless windows.
    // Without WA_OpaquePaintEvent, Qt's scroll events from trackpad are not delivered.
    window->setAttribute(Qt::WA_OpaquePaintEvent, false);
    window->setAttribute(Qt::WA_NoSystemBackground, false);
}

static NSWindow* resolveNSWindow(QWidget* widget) {
    if (!widget) {
        return nil;
    }

    // QWidget::winId() is an NSView* on macOS (via Cocoa platform plugin).
    // Using it avoids relying on Qt private QPA headers.
    NSView* view = (__bridge NSView*)reinterpret_cast<void*>(widget->winId());
    if (!view) {
        return nil;
    }
    return view.window;
}

void MacFramelessWindowBase::applyCocoaWindowStyle(QWidget* window) {
    NSWindow* nsWindow = resolveNSWindow(window);
    if (!nsWindow) {
        return;
    }

    // Use Cocoa API even for scheme 1: configure titlebar to be transparent and content sized.
    nsWindow.titleVisibility = NSWindowTitleHidden;
    nsWindow.titlebarAppearsTransparent = YES;

    // Let content view extend into titlebar.
    nsWindow.styleMask |= NSWindowStyleMaskFullSizeContentView;

    // Keep standard buttons (traffic lights) as Cocoa-managed; the window is still frameless on Qt side.
    // Do NOT hide them here; you may later reposition them in a more advanced scheme.

    // Ensure the window background is clear so Qt can draw its own background.
    nsWindow.opaque = NO;
    nsWindow.backgroundColor = [NSColor clearColor];

    if (!resizeEnabled_) {
        nsWindow.styleMask &= ~NSWindowStyleMaskResizable;
    } else {
        nsWindow.styleMask |= NSWindowStyleMaskResizable;
    }

    if (nsWindow.contentView) {
        [nsWindow.contentView setNeedsLayout:YES];
        [nsWindow.contentView layoutSubtreeIfNeeded];
    }

    if (window->layout()) {
        window->layout()->activate();
    }

    // Use a slightly delayed timer to ensure Cocoa style has fully propagated
    // before forcing layout recompute. This fixes titlebar child widget
    // vertical misalignment on initial show.
    QTimer::singleShot(10, window, [window]() {
        if (!window) {
            return;
        }
        // Force geometry update
        const QSize s = window->size();
        window->resize(s);
        window->updateGeometry();
        window->update();

        // Force all child layouts to recompute, especially titlebar
        foreach (QObject* child, window->children()) {
            if (auto* w = qobject_cast<QWidget*>(child)) {
                if (w->layout()) {
                    w->layout()->invalidate();
                    w->layout()->activate();
                }
                w->updateGeometry();
            }
        }
    });
}

// ============================================================================
// MacFramelessWindow
// ============================================================================

MacFramelessWindow::MacFramelessWindow(QWidget* parent) : QWidget(parent) { initFrameless(this); }

void MacFramelessWindow::showEvent(QShowEvent* e) {
    QWidget::showEvent(e);

    if (cocoaApplied_) {
        return;
    }

    cocoaApplied_ = true;
    applyCocoaWindowStyle(this);
}

// ============================================================================
// MacFramelessMainWindow
// ============================================================================

MacFramelessMainWindow::MacFramelessMainWindow(QWidget* parent) : QMainWindow(parent) {
    initFrameless(this);
}

void MacFramelessMainWindow::showEvent(QShowEvent* e) {
    QMainWindow::showEvent(e);

    if (cocoaApplied_) {
        return;
    }

    cocoaApplied_ = true;
    applyCocoaWindowStyle(this);
}

bool MacFramelessMainWindow::event(QEvent* e) {
    // Forward wheel events to child widgets under cursor
    if (e->type() == QEvent::Wheel) {
        auto* wheel = static_cast<QWheelEvent*>(e);
        qInfo().noquote() << "[qfw][scroll] event() wheel delta" << wheel->angleDelta() << "pos" << wheel->position();
        
        // Find child widget under cursor and forward event
        QWidget* child = childAt(wheel->position().toPoint());
        qInfo().noquote() << "[qfw][scroll] child" << child << (child ? child->metaObject()->className() : "null");
        
        if (child && child != this) {
            // Map position to child's coordinate system
            QPoint childPos = child->mapFromGlobal(mapToGlobal(wheel->position().toPoint()));
            QWheelEvent childEvent(childPos, wheel->globalPosition(), wheel->pixelDelta(),
                                   wheel->angleDelta(), wheel->buttons(), wheel->modifiers(),
                                   wheel->phase(), wheel->inverted());
            qInfo().noquote() << "[qfw][scroll] forwarding to child" << child << "childPos" << childPos;
            QApplication::sendEvent(child, &childEvent);
            return true;
        }
    }
    return QMainWindow::event(e);
}

// ============================================================================
// MacFramelessDialog
// ============================================================================

MacFramelessDialog::MacFramelessDialog(QWidget* parent) : QDialog(parent) { initFrameless(this); }

void MacFramelessDialog::showEvent(QShowEvent* e) {
    QDialog::showEvent(e);

    if (cocoaApplied_) {
        return;
    }

    cocoaApplied_ = true;
    applyCocoaWindowStyle(this);
}

}  // namespace qfw

#endif  // Q_OS_MAC
