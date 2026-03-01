#pragma once

/**
 * macOS Frameless Window
 * Ported from qframelesswindow/mac/__init__.py
 */

#include <QDialog>
#include <QMainWindow>
#include <QWidget>

namespace qfw {

class MacWindowEffect;
class TitleBar;

class MacFramelessWindowBase {
public:
    MacFramelessWindowBase();
    virtual ~MacFramelessWindowBase();

    void setResizeEnabled(bool enabled);
    bool isResizeEnabled() const;

    void setSystemTitleBarButtonVisible(bool visible);
    bool isSystemButtonVisible() const;

    void setStayOnTop(bool isTop);
    void toggleStayOnTop();

    void setTitleBar(TitleBar* titleBar);
    TitleBar* titleBar() const;
    void clearTitleBar();  // Remove and delete the title bar

protected:
    void initFrameless(QWidget* window);
    void updateFrameless();
    void hideSystemTitleBar();
    void extendTitleBarToClientArea();
    void updateSystemButtonRect();
    void updateSystemTitleBar();

    MacWindowEffect* windowEffect_ = nullptr;
    TitleBar* titleBar_ = nullptr;
    QWidget* window_ = nullptr;
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

class MacAcrylicWindow : public MacFramelessWindow {
    Q_OBJECT

public:
    explicit MacAcrylicWindow(QWidget* parent = nullptr);
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
