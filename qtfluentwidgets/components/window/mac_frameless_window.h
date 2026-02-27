#pragma once

/**
 * macOS Frameless Window - Placeholder
 * Full implementation needed for macOS support
 */

#include <QDialog>
#include <QMainWindow>
#include <QWidget>

namespace qfw {

class MacFramelessWindow : public QWidget {
    Q_OBJECT
public:
    explicit MacFramelessWindow(QWidget* parent = nullptr) : QWidget(parent) {}
};

class MacFramelessMainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MacFramelessMainWindow(QWidget* parent = nullptr) : QMainWindow(parent) {}
};

class MacFramelessDialog : public QDialog {
    Q_OBJECT
public:
    explicit MacFramelessDialog(QWidget* parent = nullptr) : QDialog(parent) {}
};

class MacWindowEffect {
public:
    static void setAcrylicEffect(QWidget* widget) { Q_UNUSED(widget) }
    static void setMicaEffect(QWidget* widget) { Q_UNUSED(widget) }
};

} // namespace qfw
