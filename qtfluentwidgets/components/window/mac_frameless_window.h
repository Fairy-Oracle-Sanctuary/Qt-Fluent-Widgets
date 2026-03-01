#pragma once

/**
 * macOS Frameless Window
 * Ported from qframelesswindow/mac/__init__.py
 */

#ifdef Q_OS_MAC

#include <QDialog>
#include <QMainWindow>
#include <QWidget>

namespace qfw {

class MacWindowEffect;

class MacFramelessWindowBase {
public:
    MacFramelessWindowBase();
    virtual ~MacFramelessWindowBase();

    void setResizeEnabled(bool enabled);
    bool isResizeEnabled() const;

    void setSystemTitleBarButtonVisible(bool visible);
    bool isSystemButtonVisible() const;

protected:
    void initFrameless(QWidget* window);
    void updateFrameless();
    void hideSystemTitleBar();
    void extendTitleBarToClientArea();
    void updateSystemButtonRect();
    void updateSystemTitleBar();

    MacWindowEffect* windowEffect_ = nullptr;
    bool resizeEnabled_ = true;
    bool isSystemButtonVisible_ = false;
    void* nsWindow_ = nullptr;  // NSWindow* stored as void* to avoid Obj-C in header
};

class MacFramelessWindow : public QWidget, public MacFramelessWindowBase {
    Q_OBJECT

public:
    explicit MacFramelessWindow(QWidget* parent = nullptr);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void changeEvent(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
};

class MacFramelessMainWindow : public QMainWindow, public MacFramelessWindowBase {
    Q_OBJECT

public:
    explicit MacFramelessMainWindow(QWidget* parent = nullptr);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void changeEvent(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
};

class MacFramelessDialog : public QDialog, public MacFramelessWindowBase {
    Q_OBJECT

public:
    explicit MacFramelessDialog(QWidget* parent = nullptr);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void changeEvent(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
};

}  // namespace qfw

#endif  // Q_OS_MAC
