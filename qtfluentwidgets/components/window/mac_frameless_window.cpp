/**
 * macOS Frameless Window - Stub implementation for non-macOS platforms
 * This file provides empty implementations to satisfy linker requirements
 * when building on Windows or Linux.
 */

#include "components/window/mac_frameless_window.h"

#ifndef Q_OS_MAC

namespace qfw {

// MacFramelessWindowBase stub implementations
MacFramelessWindowBase::MacFramelessWindowBase() = default;
MacFramelessWindowBase::~MacFramelessWindowBase() = default;

void MacFramelessWindowBase::setResizeEnabled(bool enabled) {
    resizeEnabled_ = enabled;
}

bool MacFramelessWindowBase::isResizeEnabled() const {
    return resizeEnabled_;
}

void MacFramelessWindowBase::initFrameless(QWidget* window) {
    Q_UNUSED(window)
}

void MacFramelessWindowBase::applyCocoaWindowStyle(QWidget* window) {
    Q_UNUSED(window)
}

// MacFramelessWindow stub implementations
MacFramelessWindow::MacFramelessWindow(QWidget* parent)
    : QWidget(parent), MacFramelessWindowBase() {
}

void MacFramelessWindow::showEvent(QShowEvent* e) {
    QWidget::showEvent(e);
}

// MacFramelessMainWindow stub implementations
MacFramelessMainWindow::MacFramelessMainWindow(QWidget* parent)
    : QMainWindow(parent), MacFramelessWindowBase() {
}

void MacFramelessMainWindow::showEvent(QShowEvent* e) {
    QMainWindow::showEvent(e);
}

bool MacFramelessMainWindow::event(QEvent* e) {
    return QMainWindow::event(e);
}

// MacFramelessDialog stub implementations
MacFramelessDialog::MacFramelessDialog(QWidget* parent)
    : QDialog(parent), MacFramelessWindowBase() {
}

void MacFramelessDialog::showEvent(QShowEvent* e) {
    QDialog::showEvent(e);
}

} // namespace qfw

#endif // !Q_OS_MAC
