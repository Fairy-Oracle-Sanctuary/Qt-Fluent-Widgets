/**
 * Linux Frameless Window - Stub implementation for non-Linux platforms
 * This file provides empty implementations to satisfy linker requirements
 * when building on Windows or macOS.
 */

#include "components/window/linux_frameless_window.h"

#ifndef Q_OS_LINUX

namespace qfw {

// LinuxFramelessWindowBase stub implementations
LinuxFramelessWindowBase::LinuxFramelessWindowBase() = default;
LinuxFramelessWindowBase::~LinuxFramelessWindowBase() = default;

void LinuxFramelessWindowBase::setResizeEnabled(bool enabled) {
    resizeEnabled_ = enabled;
}

bool LinuxFramelessWindowBase::isResizeEnabled() const {
    return resizeEnabled_;
}

void LinuxFramelessWindowBase::setStayOnTop(bool isTop) {
    Q_UNUSED(isTop)
}

void LinuxFramelessWindowBase::toggleStayOnTop() {}

bool LinuxFramelessWindowBase::isSystemButtonVisible() const {
    return isSystemButtonVisible_;
}

void LinuxFramelessWindowBase::setSystemTitleBarButtonVisible(bool visible) {
    isSystemButtonVisible_ = visible;
}

QRect LinuxFramelessWindowBase::systemTitleBarRect(const QSize& size) const {
    return QRect(0, 0, size.width(), size.height());
}

void LinuxFramelessWindowBase::initFrameless(QWidget* window) {
    Q_UNUSED(window)
}

bool LinuxFramelessWindowBase::handleEventFilter(QObject* obj, QEvent* event) {
    Q_UNUSED(obj)
    Q_UNUSED(event)
    return false;
}

// LinuxFramelessWindow stub implementations
LinuxFramelessWindow::LinuxFramelessWindow(QWidget* parent)
    : QWidget(parent), LinuxFramelessWindowBase() {
}

bool LinuxFramelessWindow::event(QEvent* e) {
    return QWidget::event(e);
}

void LinuxFramelessWindow::showEvent(QShowEvent* e) {
    QWidget::showEvent(e);
}

// LinuxFramelessMainWindow stub implementations
LinuxFramelessMainWindow::LinuxFramelessMainWindow(QWidget* parent)
    : QMainWindow(parent), LinuxFramelessWindowBase() {
}

bool LinuxFramelessMainWindow::event(QEvent* e) {
    return QMainWindow::event(e);
}

void LinuxFramelessMainWindow::showEvent(QShowEvent* e) {
    QMainWindow::showEvent(e);
}

// LinuxFramelessDialog stub implementations
LinuxFramelessDialog::LinuxFramelessDialog(QWidget* parent)
    : QDialog(parent), LinuxFramelessWindowBase() {
}

bool LinuxFramelessDialog::event(QEvent* e) {
    return QDialog::event(e);
}

void LinuxFramelessDialog::showEvent(QShowEvent* e) {
    QDialog::showEvent(e);
}

}  // namespace qfw

#endif  // !Q_OS_LINUX
