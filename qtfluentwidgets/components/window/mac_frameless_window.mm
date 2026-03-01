#include "components/window/mac_frameless_window.h"

#include <QDebug>
#include <QPaintEvent>
#include <QRect>
#include <QResizeEvent>
#include <QWidget>
#include <QtGlobal>

#include "components/window/mac_window_effect.h"
#include "components/window/title_bar.h"

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

bool MacFramelessWindowBase::isResizeEnabled() const { return resizeEnabled_; }

void MacFramelessWindowBase::setSystemTitleBarButtonVisible(bool visible) {
    isSystemButtonVisible_ = visible;

    if (!nsWindow_) {
        return;
    }

    NSWindow* window = static_cast<NSWindow*>(nsWindow_);

    [window setShowsToolbarButton:visible];

    const BOOL hidden = visible ? NO : YES;
    [[window standardWindowButton:NSWindowCloseButton] setHidden:hidden];
    [[window standardWindowButton:NSWindowMiniaturizeButton] setHidden:hidden];
    [[window standardWindowButton:NSWindowZoomButton] setHidden:hidden];

    if (visible) {
        updateSystemButtonRect();
    }
}

bool MacFramelessWindowBase::isSystemButtonVisible() const {
    return isSystemButtonVisible_;
}

void MacFramelessWindowBase::setStayOnTop(bool isTop) {
    if (!window_) {
        return;
    }

    const QRect oldGeometry = window_->geometry();

    if (isTop) {
        window_->setWindowFlags(window_->windowFlags() | Qt::WindowStaysOnTopHint);
    } else {
        window_->setWindowFlags(window_->windowFlags() & ~Qt::WindowStaysOnTopHint);
    }

    updateFrameless();
    window_->show();

    window_->setGeometry(oldGeometry);
}

void MacFramelessWindowBase::toggleStayOnTop() {
    if (!window_) {
        return;
    }

    const bool isTop = (window_->windowFlags() & Qt::WindowStaysOnTopHint);
    setStayOnTop(!isTop);
}

void MacFramelessWindowBase::setTitleBar(TitleBar* titleBar) {
    if (!window_ || !titleBar) {
        return;
    }

    if (titleBar_) {
        titleBar_->hide();
        titleBar_->deleteLater();
    }

    titleBar_ = titleBar;
    titleBar_->setParent(window_);
    titleBar_->raise();

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    titleBar_->setAttribute(Qt::WA_LayoutOnEntireRect, true);
#endif
}

TitleBar* MacFramelessWindowBase::titleBar() const { return titleBar_; }

void MacFramelessWindowBase::clearTitleBar() {
    if (titleBar_) {
        titleBar_->hide();
        titleBar_->setParent(nullptr);
        titleBar_->deleteLater();
        titleBar_ = nullptr;
    }
}

void MacFramelessWindowBase::initFrameless(QWidget* window) {
    if (!window) return;

    window_ = window;

    // Window effect
    if (!windowEffect_) {
        windowEffect_ = new MacWindowEffect();
    }

    // must enable acrylic effect before creating title bar
    if (qobject_cast<MacAcrylicWindow*>(window_)) {
        window_->setAttribute(Qt::WA_TranslucentBackground);
        window_->setStyleSheet(QStringLiteral("background: transparent"));
        windowEffect_->setAcrylicEffect(window_);
    }

    // Create default title bar
    if (!titleBar_) {
        titleBar_ = new TitleBar(window_);
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    window_->setAttribute(Qt::WA_ContentsMarginsRespectsSafeArea, false);
    titleBar_->setAttribute(Qt::WA_LayoutOnEntireRect, true);
#endif

    // Get NSWindow from QWidget
    NSWindow* nsWindow = getNSWindow(window->winId());
    if (!nsWindow) {
        qWarning() << "Failed to get NSWindow from QWidget";
        return;
    }
    nsWindow_ = nsWindow;

    // Hide system title bar
    hideSystemTitleBar();

    // Keep python-like behavior
    updateFrameless();
    window_->resize(500, 500);
    titleBar_->raise();
}

void MacFramelessWindowBase::updateFrameless() {
    if (!nsWindow_) return;

    NSWindow* window = static_cast<NSWindow*>(nsWindow_);

    // Re-extend title bar to client area
    extendTitleBarToClientArea();

    // Restore button visibility
    setSystemTitleBarButtonVisible(isSystemButtonVisible_);
}

void MacFramelessWindowBase::updateSystemTitleBar() {
    extendTitleBarToClientArea();
    setSystemTitleBarButtonVisible(isSystemButtonVisible_);
}

void MacFramelessWindowBase::hideSystemTitleBar() {
    if (!nsWindow_) {
        return;
    }

    NSWindow* window = static_cast<NSWindow*>(nsWindow_);

    extendTitleBarToClientArea();

    [window setMovableByWindowBackground:NO];
    [window setMovable:NO];

    [window setTitleVisibility:NSWindowTitleHidden];
    setSystemTitleBarButtonVisible(false);
}

void MacFramelessWindowBase::extendTitleBarToClientArea() {
    if (!nsWindow_) {
        return;
    }

    NSWindow* window = static_cast<NSWindow*>(nsWindow_);

    NSUInteger styleMask = [window styleMask];
    styleMask |= NSWindowStyleMaskFullSizeContentView;
    [window setStyleMask:styleMask];

    [window setTitlebarAppearsTransparent:YES];
}

void MacFramelessWindowBase::updateSystemButtonRect() {
    if (!nsWindow_ || !isSystemButtonVisible_) {
        return;
    }

    NSWindow* window = static_cast<NSWindow*>(nsWindow_);

    NSButton* closeButton = [window standardWindowButton:NSWindowCloseButton];
    NSButton* miniaturizeButton = [window standardWindowButton:NSWindowMiniaturizeButton];
    NSButton* zoomButton = [window standardWindowButton:NSWindowZoomButton];

    if (!closeButton || !miniaturizeButton || !zoomButton) {
        return;
    }

    NSView* titleBar = [zoomButton superview];
    if (!titleBar) {
        return;
    }

    const CGFloat titleBarHeight = [titleBar frame].size.height;

    const CGFloat spacing = [miniaturizeButton frame].origin.x - [closeButton frame].origin.x;
    const CGFloat width = [miniaturizeButton frame].size.width;
    const CGFloat height = [miniaturizeButton frame].size.height;

    // Match python's default systemTitleBarRect(): QRect(0, 0, 75, h)
    const CGFloat centerX = 37.5;
    CGFloat centerY = titleBarHeight / 2.0;
    centerY = titleBarHeight - centerY;

    const NSPoint midOrigin = NSMakePoint(centerX - width / 2.0, centerY - height / 2.0);
    [miniaturizeButton setFrameOrigin:midOrigin];

    const NSPoint leftOrigin = NSMakePoint(midOrigin.x - spacing, midOrigin.y);
    [closeButton setFrameOrigin:leftOrigin];

    const NSPoint rightOrigin = NSMakePoint(midOrigin.x + spacing, midOrigin.y);
    [zoomButton setFrameOrigin:rightOrigin];
}

// ============================================================================
// MacFramelessWindow
// ============================================================================

MacFramelessWindow::MacFramelessWindow(QWidget* parent) : QWidget(parent) {
    initFrameless(this);
}

MacAcrylicWindow::MacAcrylicWindow(QWidget* parent) : MacFramelessWindow(parent) {}

void MacFramelessWindow::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    if (titleBar()) {
        titleBar()->resize(width(), titleBar()->height());
    }
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
    if (titleBar()) {
        titleBar()->resize(width(), titleBar()->height());
    }
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
    if (auto* tb = titleBar()) {
        if (tb->minimizeButton()) {
            tb->minimizeButton()->hide();
        }
        if (tb->maximizeButton()) {
            tb->maximizeButton()->hide();
        }
        tb->setDoubleClickEnabled(false);
    }
}

void MacFramelessDialog::resizeEvent(QResizeEvent* event) {
    QDialog::resizeEvent(event);
    if (titleBar()) {
        titleBar()->resize(width(), titleBar()->height());
    }
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
