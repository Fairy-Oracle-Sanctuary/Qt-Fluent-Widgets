#include "components/window/mac_frameless_window.h"

#ifndef Q_OS_MAC

// Empty implementations for non-macOS platforms
// This ensures MOC-generated code links correctly

#include <QResizeEvent>

namespace qfw {

MacFramelessWindowBase::MacFramelessWindowBase() {}
MacFramelessWindowBase::~MacFramelessWindowBase() {}
void MacFramelessWindowBase::setResizeEnabled(bool) {}
bool MacFramelessWindowBase::isResizeEnabled() const { return false; }
void MacFramelessWindowBase::setSystemTitleBarButtonVisible(bool) {}
bool MacFramelessWindowBase::isSystemButtonVisible() const { return false; }
void MacFramelessWindowBase::initFrameless(QWidget*) {}
void MacFramelessWindowBase::updateFrameless() {}
void MacFramelessWindowBase::hideSystemTitleBar() {}
void MacFramelessWindowBase::extendTitleBarToClientArea() {}
void MacFramelessWindowBase::updateSystemButtonRect() {}
void MacFramelessWindowBase::updateSystemTitleBar() {}

MacFramelessWindow::MacFramelessWindow(QWidget* parent) : QWidget(parent) {}
void MacFramelessWindow::resizeEvent(QResizeEvent* event) { QWidget::resizeEvent(event); }
void MacFramelessWindow::changeEvent(QEvent* event) { QWidget::changeEvent(event); }
void MacFramelessWindow::paintEvent(QPaintEvent* event) { QWidget::paintEvent(event); }

MacFramelessMainWindow::MacFramelessMainWindow(QWidget* parent) : QMainWindow(parent) {}
void MacFramelessMainWindow::resizeEvent(QResizeEvent* event) { QMainWindow::resizeEvent(event); }
void MacFramelessMainWindow::changeEvent(QEvent* event) { QMainWindow::changeEvent(event); }
void MacFramelessMainWindow::paintEvent(QPaintEvent* event) { QMainWindow::paintEvent(event); }

MacFramelessDialog::MacFramelessDialog(QWidget* parent) : QDialog(parent) {}
void MacFramelessDialog::resizeEvent(QResizeEvent* event) { QDialog::resizeEvent(event); }
void MacFramelessDialog::changeEvent(QEvent* event) { QDialog::changeEvent(event); }
void MacFramelessDialog::paintEvent(QPaintEvent* event) { QDialog::paintEvent(event); }

}  // namespace qfw

#endif  // !Q_OS_MAC
