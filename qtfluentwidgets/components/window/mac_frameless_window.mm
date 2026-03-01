#include "components/window/mac_frameless_window.h"

#include <QDebug>
#include <QResizeEvent>

#import <Cocoa/Cocoa.h>

namespace qfw {

// Helper to convert QWidget winId to NSWindow
static NSWindow* getNSWindow(WId winId) {
    NSView* view = reinterpret_cast<NSView*>(winId);
    return view.window;
}

// ============================================================================
// MacFramelessWindowBase
// ============================================================================

MacFramelessWindowBase::MacFramelessWindowBase() = default;

MacFramelessWindowBase::~MacFramelessWindowBase() = default;

void MacFramelessWindowBase::setResizeEnabled(bool enabled) {
    resizeEnabled_ = enabled;
}

bool MacFramelessWindowBase::isResizeEnabled() const {
    return resizeEnabled_;
}

void MacFramelessWindowBase::setSystemTitleBarButtonVisible(bool visible) {
    isSystemButtonVisible_ = visible;
    
    if (!nsWindow_) return;
    
    NSWindow* window = static_cast<NSWindow*>(nsWindow_);
    
    // Show/hide toolbar button
    [window setShowsToolbarButton:visible];
    
    // Show/hide standard window buttons (close, minimize, zoom)
    [[window standardWindowButton:NSWindowCloseButton] setHidden:!visible];
    [[window standardWindowButton:NSWindowMiniaturizeButton] setHidden:!visible];
    [[window standardWindowButton:NSWindowZoomButton] setHidden:!visible];
    
    if (visible) {
        updateSystemButtonRect();
    }
}

bool MacFramelessWindowBase::isSystemButtonVisible() const {
    return isSystemButtonVisible_;
}

void MacFramelessWindowBase::initFrameless(QWidget* window) {
    if (!window) return;
    
    // Get NSWindow from QWidget
    NSWindow* nsWindow = getNSWindow(window->winId());
    if (!nsWindow) {
        qWarning() << "Failed to get NSWindow from QWidget";
        return;
    }
    
    nsWindow_ = nsWindow;
    
    // Hide system title bar
    hideSystemTitleBar();
}

void MacFramelessWindowBase::updateFrameless() {
    if (!nsWindow_) return;
    
    NSWindow* window = static_cast<NSWindow*>(nsWindow_);
    
    // Re-extend title bar to client area
    extendTitleBarToClientArea();
    
    // Restore button visibility
    setSystemTitleBarButtonVisible(isSystemButtonVisible_);
}

void MacFramelessWindowBase::hideSystemTitleBar() {
    if (!nsWindow_) return;
    
    NSWindow* window = static_cast<NSWindow*>(nsWindow_);
    
    // Extend view to title bar region
    extendTitleBarToClientArea();
    
    // Disable the moving behavior of system
    [window setMovableByWindowBackground:NO];
    [window setMovable:NO];
    
    // Hide title bar buttons and title
    [window setTitleVisibility:NSWindowTitleHidden];
    setSystemTitleBarButtonVisible(false);
}

void MacFramelessWindowBase::extendTitleBarToClientArea() {
    if (!nsWindow_) return;
    
    NSWindow* window = static_cast<NSWindow*>(nsWindow_);
    
    // Set style mask to include full size content view
    NSUInteger styleMask = [window styleMask];
    styleMask |= NSWindowStyleMaskFullSizeContentView;
    [window setStyleMask:styleMask];
    
    // Make title bar transparent
    [window setTitlebarAppearsTransparent:YES];
}

void MacFramelessWindowBase::updateSystemButtonRect() {
    if (!nsWindow_ || !isSystemButtonVisible_) return;
    
    NSWindow* window = static_cast<NSWindow*>(nsWindow_);
    
    // Get system title bar buttons
    NSButton* closeButton = [window standardWindowButton:NSWindowCloseButton];
    NSButton* miniaturizeButton = [window standardWindowButton:NSWindowMiniaturizeButton];
    NSButton* zoomButton = [window standardWindowButton:NSWindowZoomButton];
    
    if (!closeButton || !miniaturizeButton || !zoomButton) return;
    
    // Get title bar
    NSView* titleBar = [zoomButton superview];
    if (!titleBar) return;
    
    CGFloat titleBarHeight = [titleBar frame].size.height;
    
    // Calculate spacing between buttons
    CGFloat spacing = [miniaturizeButton frame].origin.x - [closeButton frame].origin.x;
    CGFloat width = [miniaturizeButton frame].size.width;
    CGFloat height = [miniaturizeButton frame].size.height;
    
    // Get window content view size
    NSView* contentView = [window contentView];
    NSSize viewSize = contentView ? [contentView frame].size : [window frame].size;
    
    // Calculate center position for buttons
    // Default: left side of title bar (similar to Python version)
    CGFloat centerX = 37.5;  // Half of default 75 width
    CGFloat centerY = titleBarHeight / 2.0;
    
    // NSWindow coordinate system origin is bottom-left, so we need to flip Y
    centerY = titleBarHeight - centerY;
    
    // Adjust button positions
    NSPoint midOrigin = NSMakePoint(centerX - width / 2.0, centerY - height / 2.0);
    [miniaturizeButton setFrameOrigin:midOrigin];
    
    NSPoint leftOrigin = NSMakePoint(midOrigin.x - spacing, midOrigin.y);
    [closeButton setFrameOrigin:leftOrigin];
    
    NSPoint rightOrigin = NSMakePoint(midOrigin.x + spacing, midOrigin.y);
    [zoomButton setFrameOrigin:rightOrigin];
}

void MacFramelessWindowBase::updateSystemTitleBar() {
    extendTitleBarToClientArea();
    setSystemTitleBarButtonVisible(isSystemButtonVisible_);
}

// ============================================================================
// MacFramelessWindow
// ============================================================================

MacFramelessWindow::MacFramelessWindow(QWidget* parent) : QWidget(parent) {
    initFrameless(this);
}

void MacFramelessWindow::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    updateSystemTitleBar();
}

void MacFramelessWindow::changeEvent(QEvent* event) {
    QWidget::changeEvent(event);
    
    if (event->type() == QEvent::WindowStateChange) {
        updateSystemTitleBar();
    } else if (event->type() == QEvent::Resize) {
        updateSystemButtonRect();
    }
}

void MacFramelessWindow::paintEvent(QPaintEvent* event) {
    QWidget::paintEvent(event);
    updateSystemTitleBar();
}

// ============================================================================
// MacFramelessMainWindow
// ============================================================================

MacFramelessMainWindow::MacFramelessMainWindow(QWidget* parent) : QMainWindow(parent) {
    initFrameless(this);
}

void MacFramelessMainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    updateSystemTitleBar();
}

void MacFramelessMainWindow::changeEvent(QEvent* event) {
    QMainWindow::changeEvent(event);
    
    if (event->type() == QEvent::WindowStateChange) {
        updateSystemTitleBar();
    } else if (event->type() == QEvent::Resize) {
        updateSystemButtonRect();
    }
}

void MacFramelessMainWindow::paintEvent(QPaintEvent* event) {
    QMainWindow::paintEvent(event);
    updateSystemTitleBar();
}

// ============================================================================
// MacFramelessDialog
// ============================================================================

MacFramelessDialog::MacFramelessDialog(QWidget* parent) : QDialog(parent) {
    initFrameless(this);
}

void MacFramelessDialog::resizeEvent(QResizeEvent* event) {
    QDialog::resizeEvent(event);
    updateSystemTitleBar();
}

void MacFramelessDialog::changeEvent(QEvent* event) {
    QDialog::changeEvent(event);
    
    if (event->type() == QEvent::WindowStateChange) {
        updateSystemTitleBar();
    } else if (event->type() == QEvent::Resize) {
        updateSystemButtonRect();
    }
}

void MacFramelessDialog::paintEvent(QPaintEvent* event) {
    QDialog::paintEvent(event);
    updateSystemTitleBar();
}

}  // namespace qfw
