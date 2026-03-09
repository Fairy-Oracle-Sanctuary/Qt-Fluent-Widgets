#pragma once

/**
 * Linux Frameless Window
 * Cross-platform frameless window implementation for Linux
 */

#include <QDialog>
#include <QEvent>
#include <QMainWindow>
#include <QPoint>
#include <QRect>
#include <QSize>
#include <QWidget>

#include "common/qtcompat.h"

namespace qfw {

class LinuxWindowEffect;

class LinuxFramelessWindowBase {
public:
    static constexpr int BORDER_WIDTH = 5;

    LinuxFramelessWindowBase();
    virtual ~LinuxFramelessWindowBase();

    void setResizeEnabled(bool enabled);
    bool isResizeEnabled() const;

    void setStayOnTop(bool isTop);
    void toggleStayOnTop();

    bool isSystemButtonVisible() const;
    void setSystemTitleBarButtonVisible(bool visible);

    QRect systemTitleBarRect(const QSize& size) const;

protected:
    void initFrameless(QWidget* window);

    bool handleEventFilter(QObject* obj, QEvent* event);

    LinuxWindowEffect* windowEffect_ = nullptr;
    bool resizeEnabled_ = true;
    bool isSystemButtonVisible_ = false;

private:
    QWidget* window_ = nullptr;
};

class LinuxFramelessWindow : public QWidget, public LinuxFramelessWindowBase {
    Q_OBJECT

public:
    explicit LinuxFramelessWindow(QWidget* parent = nullptr);

protected:
    bool event(QEvent* e) override;
    void showEvent(QShowEvent* e) override;

private:
    bool framelessInitialized_ = false;
};

class LinuxFramelessMainWindow : public QMainWindow, public LinuxFramelessWindowBase {
    Q_OBJECT

public:
    explicit LinuxFramelessMainWindow(QWidget* parent = nullptr);

protected:
    bool event(QEvent* e) override;
    void showEvent(QShowEvent* e) override;

private:
    bool framelessInitialized_ = false;
};

class LinuxFramelessDialog : public QDialog, public LinuxFramelessWindowBase {
    Q_OBJECT

public:
    explicit LinuxFramelessDialog(QWidget* parent = nullptr);

protected:
    bool event(QEvent* e) override;
    void showEvent(QShowEvent* e) override;

private:
    bool framelessInitialized_ = false;
};

class LinuxWindowEffect {
public:
    explicit LinuxWindowEffect(QWidget* window) { Q_UNUSED(window) }

    void setAcrylicEffect(qintptr hWnd, const QString& gradientColor = QString(),
                          bool enableShadow = true, int animationId = 0) {
        Q_UNUSED(hWnd)
        Q_UNUSED(gradientColor)
        Q_UNUSED(enableShadow)
        Q_UNUSED(animationId)
    }

    void setBorderAccentColor(qintptr hWnd, const QColor& color) {
        Q_UNUSED(hWnd)
        Q_UNUSED(color)
    }

    void removeBorderAccentColor(qintptr hWnd) { Q_UNUSED(hWnd) }

    void setMicaEffect(qintptr hWnd, bool isDarkMode = false, bool isAlt = false) {
        Q_UNUSED(hWnd)
        Q_UNUSED(isDarkMode)
        Q_UNUSED(isAlt)
    }

    void setAeroEffect(qintptr hWnd) { Q_UNUSED(hWnd) }

    void setTransparentEffect(qintptr hWnd) { Q_UNUSED(hWnd) }

    void removeBackgroundEffect(qintptr hWnd) { Q_UNUSED(hWnd) }

    void addShadowEffect(qintptr hWnd) { Q_UNUSED(hWnd) }

    void addMenuShadowEffect(qintptr hWnd) { Q_UNUSED(hWnd) }

    static void removeMenuShadowEffect(qintptr hWnd) { Q_UNUSED(hWnd) }

    void removeShadowEffect(qintptr hWnd) { Q_UNUSED(hWnd) }

    static void addWindowAnimation(qintptr hWnd) { Q_UNUSED(hWnd) }

    static void disableMaximizeButton(qintptr hWnd) { Q_UNUSED(hWnd) }

    void enableBlurBehindWindow(qintptr hWnd) { Q_UNUSED(hWnd) }
};

}  // namespace qfw
