#include "components/window/mac_frameless_window.h"

#ifdef Q_OS_MAC

#include <QDebug>
#include <QLayout>
#include <QTimer>
#include <QWheelEvent>
#include <QApplication>
#include <QPaintEvent>

#import <Cocoa/Cocoa.h>

namespace qfw {

// ============================================================================
// MacFramelessWindowBase
// ============================================================================

MacFramelessWindowBase::MacFramelessWindowBase() = default;
MacFramelessWindowBase::~MacFramelessWindowBase() = default;

void MacFramelessWindowBase::setResizeEnabled(bool enabled) { resizeEnabled_ = enabled; }

bool MacFramelessWindowBase::isResizeEnabled() const { return resizeEnabled_; }

void MacFramelessWindowBase::setSystemTitleBarButtonVisible(bool visible) {
    systemButtonVisible_ = visible;
    
    NSWindow* nsWindow = (__bridge NSWindow*)nsWindow_;
    if (!nsWindow) {
        return;
    }
    
    [nsWindow setShowsToolbarButton:visible];
    
    BOOL isHidden = !visible;
    [nsWindow standardWindowButton:NSWindowCloseButton].hidden = isHidden;
    [nsWindow standardWindowButton:NSWindowZoomButton].hidden = isHidden;
    [nsWindow standardWindowButton:NSWindowMiniaturizeButton].hidden = isHidden;
    
    if (visible) {
        updateSystemButtonRect();
    }
}

bool MacFramelessWindowBase::isSystemButtonVisible() const {
    return systemButtonVisible_;
}

QRect MacFramelessWindowBase::systemTitleBarRect(const QSize& size) const {
    // Default: left side. Override to move to right side.
    // For right side: return QRect(size.width() - 75, 0, 75, size.height());
    return QRect(0, 0, 75, size.height());
}

void MacFramelessWindowBase::initFrameless(QWidget* window) {
    if (!window) {
        return;
    }
    
    window_ = window;

    // Ensure this widget is treated as a top-level window.
    window->setWindowFlag(Qt::Window, true);

    // Use Qt frameless hint.
    const Qt::WindowFlags stayOnTop = (window->windowFlags() & Qt::WindowStaysOnTopHint)
                                          ? Qt::WindowStaysOnTopHint
                                          : Qt::WindowFlags{};

    window->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | window->windowFlags() | stayOnTop);

    // This window style is typically required for translucent titlebar/vibrancy.
    window->setAttribute(Qt::WA_TranslucentBackground, true);
    window->setAutoFillBackground(false);

    // Fix trackpad scrolling not working on macOS with frameless windows.
    window->setAttribute(Qt::WA_OpaquePaintEvent, false);
    window->setAttribute(Qt::WA_NoSystemBackground, false);
}

static NSWindow* resolveNSWindow(QWidget* widget) {
    if (!widget) {
        return nil;
    }

    // QWidget::winId() is an NSView* on macOS (via Cocoa platform plugin).
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
    
    nsWindow_ = (__bridge void*)nsWindow;

    // Use Cocoa API: configure titlebar to be transparent and content sized.
    nsWindow.titleVisibility = NSWindowTitleHidden;
    nsWindow.titlebarAppearsTransparent = YES;

    // Let content view extend into titlebar.
    nsWindow.styleMask |= NSWindowStyleMaskFullSizeContentView;

    // Ensure the window background is clear so Qt can draw its own background.
    nsWindow.opaque = NO;
    nsWindow.backgroundColor = [NSColor clearColor];

    // Add rounded corners for macOS (10px radius like Windows)
    nsWindow.contentView.wantsLayer = YES;
    nsWindow.contentView.layer.masksToBounds = YES;
    nsWindow.contentView.layer.cornerRadius = 8.0;

    if (!resizeEnabled_) {
        nsWindow.styleMask &= ~NSWindowStyleMaskResizable;
    } else {
        nsWindow.styleMask |= NSWindowStyleMaskResizable;
    }

    // Hide system title bar initially
    hideSystemTitleBar();

    if (nsWindow.contentView) {
        [nsWindow.contentView setNeedsLayout:YES];
        [nsWindow.contentView layoutSubtreeIfNeeded];
    }

    if (window->layout()) {
        window->layout()->activate();
    }

    // Use a slightly delayed timer to ensure Cocoa style has fully propagated
    QTimer::singleShot(10, window, [this, window]() {
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
        
        // Update system button position if visible
        if (systemButtonVisible_) {
            updateSystemButtonRect();
        }
    });
}

void MacFramelessWindowBase::hideSystemTitleBar() {
    extendTitleBarToClientArea();
    
    NSWindow* nsWindow = (__bridge NSWindow*)nsWindow_;
    if (!nsWindow) {
        return;
    }

    // Disable the moving behavior of system
    [nsWindow setMovableByWindowBackground:NO];
    [nsWindow setMovable:NO];

    // Hide title bar buttons and title
    [nsWindow setTitleVisibility:NSWindowTitleHidden];
    setSystemTitleBarButtonVisible(false);
}

void MacFramelessWindowBase::extendTitleBarToClientArea() {
    NSWindow* nsWindow = (__bridge NSWindow*)nsWindow_;
    if (!nsWindow) {
        return;
    }
    
    [nsWindow setStyleMask:[nsWindow styleMask] | NSWindowStyleMaskFullSizeContentView];
    [nsWindow setTitlebarAppearsTransparent:YES];
}

void MacFramelessWindowBase::updateSystemTitleBar() {
    extendTitleBarToClientArea();
    setSystemTitleBarButtonVisible(systemButtonVisible_);
}

void MacFramelessWindowBase::updateSystemButtonRect() {
    if (!systemButtonVisible_ || !window_) {
        return;
    }
    
    NSWindow* nsWindow = (__bridge NSWindow*)nsWindow_;
    if (!nsWindow) {
        return;
    }

    // Get system title bar buttons
    NSButton* leftButton = [nsWindow standardWindowButton:NSWindowCloseButton];
    NSButton* midButton = [nsWindow standardWindowButton:NSWindowMiniaturizeButton];
    NSButton* rightButton = [nsWindow standardWindowButton:NSWindowZoomButton];
    
    if (!leftButton || !midButton || !rightButton) {
        return;
    }

    // Get system title bar
    NSView* titlebar = [rightButton superview];
    CGFloat titlebarHeight = [titlebar frame].size.height;

    CGFloat spacing = [midButton frame].origin.x - [leftButton frame].origin.x;
    CGFloat width = [midButton frame].size.width;
    CGFloat height = [midButton frame].size.height;

    NSSize viewSize;
    if ([nsWindow contentView]) {
        viewSize = [[nsWindow contentView] frame].size;
    } else {
        viewSize = [nsWindow frame].size;
    }

    // Get the target rect from virtual method
    QSize qtSize(static_cast<int>(viewSize.width), static_cast<int>(titlebarHeight));
    QRect rect = systemTitleBarRect(qtSize);
    QPoint center = rect.center();

    // The origin of the NSWindow coordinate system is in the lower left corner,
    // we do the necessary transformations
    CGFloat centerY = titlebarHeight - center.y();

    // Adjust the position of minimize button
    NSPoint centerOrigin = NSMakePoint(center.x() - width / 2, centerY - height / 2);
    [midButton setFrameOrigin:centerOrigin];

    // Adjust the position of close button
    NSPoint leftOrigin = NSMakePoint(centerOrigin.x - spacing, centerOrigin.y);
    [leftButton setFrameOrigin:leftOrigin];

    // Adjust the position of zoom button
    NSPoint rightOrigin = NSMakePoint(centerOrigin.x + spacing, centerOrigin.y);
    [rightButton setFrameOrigin:rightOrigin];
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

void MacFramelessWindow::paintEvent(QPaintEvent* e) {
    QWidget::paintEvent(e);
    updateSystemTitleBar();
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
        
        // Find child widget under cursor and forward event
        QWidget* child = childAt(wheel->position().toPoint());
        
        if (child && child != this) {
            // Map position to child's coordinate system
            QPoint childPos = child->mapFromGlobal(mapToGlobal(wheel->position().toPoint()));
            QWheelEvent childEvent(childPos, wheel->globalPosition(), wheel->pixelDelta(),
                                   wheel->angleDelta(), wheel->buttons(), wheel->modifiers(),
                                   wheel->phase(), wheel->inverted());
            QApplication::sendEvent(child, &childEvent);
            return true;
        }
    }
    return QMainWindow::event(e);
}

void MacFramelessMainWindow::paintEvent(QPaintEvent* e) {
    QMainWindow::paintEvent(e);
    updateSystemTitleBar();
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

void MacFramelessDialog::paintEvent(QPaintEvent* e) {
    QDialog::paintEvent(e);
    updateSystemTitleBar();
}

}  // namespace qfw

#endif  // Q_OS_MAC
