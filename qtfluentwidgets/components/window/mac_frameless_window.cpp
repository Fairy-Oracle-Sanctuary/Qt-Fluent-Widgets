/**
 * macOS Frameless Window - Stub implementation for non-macOS platforms
 * This file provides empty implementations to satisfy linker requirements
 * when building on Windows or Linux.
 */

#include "components/window/mac_frameless_window.h"

#ifndef Q_OS_MAC

#include <QPaintEvent>

namespace qfw {

// MacFramelessWindowBase stub implementations
MacFramelessWindowBase::MacFramelessWindowBase() = default;
MacFramelessWindowBase::~MacFramelessWindowBase() = default;

void MacFramelessWindowBase::setResizeEnabled(bool enabled) { resizeEnabled_ = enabled; }

bool MacFramelessWindowBase::isResizeEnabled() const { return resizeEnabled_; }

void MacFramelessWindowBase::setSystemTitleBarButtonVisible(bool visible) {
    systemButtonVisible_ = visible;
}

bool MacFramelessWindowBase::isSystemButtonVisible() const { return systemButtonVisible_; }

QRect MacFramelessWindowBase::systemTitleBarRect(const QSize& size) const {
    return QRect(0, 0, 75, size.height());
}

void MacFramelessWindowBase::initFrameless(QWidget* window) { Q_UNUSED(window) }

void MacFramelessWindowBase::applyCocoaWindowStyle(QWidget* window) { Q_UNUSED(window) }

void MacFramelessWindowBase::updateSystemButtonRect() {
    // No-op on non-macOS
}

void MacFramelessWindowBase::hideSystemTitleBar() {
    // No-op on non-macOS
}

void MacFramelessWindowBase::extendTitleBarToClientArea() {
    // No-op on non-macOS
}

void MacFramelessWindowBase::updateSystemTitleBar() {
    // No-op on non-macOS
}

// MacFramelessWindow stub implementations
MacFramelessWindow::MacFramelessWindow(QWidget* parent)
    : QWidget(parent), MacFramelessWindowBase() {}

void MacFramelessWindow::showEvent(QShowEvent* e) { QWidget::showEvent(e); }

void MacFramelessWindow::paintEvent(QPaintEvent* e) { QWidget::paintEvent(e); }

// MacFramelessMainWindow stub implementations
MacFramelessMainWindow::MacFramelessMainWindow(QWidget* parent)
    : QMainWindow(parent), MacFramelessWindowBase() {}

void MacFramelessMainWindow::showEvent(QShowEvent* e) { QMainWindow::showEvent(e); }

bool MacFramelessMainWindow::event(QEvent* e) { return QMainWindow::event(e); }

void MacFramelessMainWindow::paintEvent(QPaintEvent* e) { QMainWindow::paintEvent(e); }

// MacFramelessDialog stub implementations
MacFramelessDialog::MacFramelessDialog(QWidget* parent)
    : QDialog(parent), MacFramelessWindowBase() {}

void MacFramelessDialog::showEvent(QShowEvent* e) { QDialog::showEvent(e); }

void MacFramelessDialog::paintEvent(QPaintEvent* e) { QDialog::paintEvent(e); }

}  // namespace qfw

#endif  // !Q_OS_MAC
