#pragma once

/**
 * macOS Frameless Window with traffic light button support
 * Based on Python implementation from PyQt-Fluent-Widgets
 */

#include <QDialog>
#include <QMainWindow>
#include <QRect>
#include <QShowEvent>
#include <QSize>
#include <QWidget>

namespace qfw {

class MacFramelessWindowBase {
public:
    MacFramelessWindowBase();
    virtual ~MacFramelessWindowBase();

    void setResizeEnabled(bool enabled);
    bool isResizeEnabled() const;

    // System traffic light button visibility
    void setSystemTitleBarButtonVisible(bool visible);
    bool isSystemButtonVisible() const;

    // Override to customize traffic light button position
    // Returns the rect where traffic lights should be positioned
    // Default: left side (0, 0, 75, height)
    // Override to move to right side: (width-75, 0, 75, height)
    virtual QRect systemTitleBarRect(const QSize& size) const;

protected:
    void initFrameless(QWidget* window);
    void applyCocoaWindowStyle(QWidget* window);
    void updateSystemButtonRect();
    void updateSystemTitleBar();
    void hideSystemTitleBar();
    void extendTitleBarToClientArea();

    bool resizeEnabled_ = true;
    bool systemButtonVisible_ = false;

private:

    QWidget* window_ = nullptr;
    void* nsWindow_ = nullptr;  // NSWindow* stored as void* to avoid Objective-C in header
};

class MacFramelessWindow : public QWidget, public MacFramelessWindowBase {
    Q_OBJECT
public:
    explicit MacFramelessWindow(QWidget* parent = nullptr);

protected:
    void showEvent(QShowEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

private:
    bool cocoaApplied_ = false;
};

class MacFramelessMainWindow : public QMainWindow, public MacFramelessWindowBase {
    Q_OBJECT
public:
    explicit MacFramelessMainWindow(QWidget* parent = nullptr);

protected:
    void showEvent(QShowEvent* e) override;
    bool event(QEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

private:
    bool cocoaApplied_ = false;
};

class MacFramelessDialog : public QDialog, public MacFramelessWindowBase {
    Q_OBJECT
public:
    explicit MacFramelessDialog(QWidget* parent = nullptr);

protected:
    void showEvent(QShowEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

private:
    bool cocoaApplied_ = false;
};

class MacWindowEffect {
public:
    static void setAcrylicEffect(QWidget* widget) { Q_UNUSED(widget) }
    static void setMicaEffect(QWidget* widget) { Q_UNUSED(widget) }
};

}  // namespace qfw
