#pragma once

/**
 * Linux Frameless Window - Placeholder
 * Full implementation needed for Linux support
 */

#include <QDialog>
#include <QMainWindow>
#include <QWidget>

namespace qfw {

class LinuxFramelessWindow : public QWidget {
    Q_OBJECT
public:
    explicit LinuxFramelessWindow(QWidget* parent = nullptr) : QWidget(parent) {}
};

class LinuxFramelessMainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit LinuxFramelessMainWindow(QWidget* parent = nullptr) : QMainWindow(parent) {}
};

class LinuxFramelessDialog : public QDialog {
    Q_OBJECT
public:
    explicit LinuxFramelessDialog(QWidget* parent = nullptr) : QDialog(parent) {}
};

class LinuxWindowEffect {
public:
    static void setAcrylicEffect(QWidget* widget) { Q_UNUSED(widget) }
    static void setMicaEffect(QWidget* widget) { Q_UNUSED(widget) }
};

} // namespace qfw
