#include "components/window/windows_frameless_window.h"

// Disable Windows min/max macros before including Windows headers
#ifdef Q_OS_WIN
#define NOMINMAX
#endif

#include <dwmapi.h>
#include <windows.h>
#include <windowsx.h>

#include <QDebug>
#include <QPainter>
#include <QScreen>
#include <QShowEvent>
#include <QWindow>
#include <QtGlobal>

#include "components/window/windows_window_effect.h"

#pragma comment(lib, "dwmapi.lib")

#ifndef DWMWA_SYSTEMBACKDROP_TYPE
// Win11 22H2+ (SDK 22621+) defines this as DWMWINDOWATTRIBUTE value 38.
#define DWMWA_SYSTEMBACKDROP_TYPE static_cast<DWMWINDOWATTRIBUTE>(38)
#endif

namespace qfw {

static bool isMaximized(HWND hWnd) { return hWnd && ::IsZoomed(hWnd); }

static bool isFullScreen(HWND hWnd) {
    if (!hWnd) {
        return false;
    }

    RECT winRect{};
    if (!::GetWindowRect(hWnd, &winRect)) {
        return false;
    }

    HMONITOR monitor = ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi{};
    mi.cbSize = sizeof(mi);
    if (!::GetMonitorInfoW(monitor, &mi)) {
        return false;
    }

    const RECT m = mi.rcMonitor;
    return winRect.left == m.left && winRect.top == m.top && winRect.right == m.right &&
           winRect.bottom == m.bottom;
}

static int resizeBorderThickness(bool horizontal) {
    const int frame = ::GetSystemMetrics(horizontal ? SM_CXSIZEFRAME : SM_CYSIZEFRAME);
    const int padded = ::GetSystemMetrics(SM_CXPADDEDBORDER);
    return frame + padded;
}

static bool hasSystemBackdrop(HWND hWnd) {
    if (!hWnd) {
        return false;
    }

    int backdropType = 0;
    const HRESULT hr = ::DwmGetWindowAttribute(hWnd, DWMWA_SYSTEMBACKDROP_TYPE, &backdropType,
                                               sizeof(backdropType));
    if (FAILED(hr)) {
        return false;
    }

    return backdropType == 2 || backdropType == 4;
}

static void ensureResizableStyle(HWND hWnd) {
    if (!hWnd) {
        return;
    }

    LONG_PTR style = ::GetWindowLongPtrW(hWnd, GWL_STYLE);
    if (style) {
        style |= WS_BORDER | WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SIZEBOX | WS_SYSMENU;
        ::SetWindowLongPtrW(hWnd, GWL_STYLE, style);
    }

    LONG_PTR exStyle = ::GetWindowLongPtrW(hWnd, GWL_EXSTYLE);
    if (exStyle & WS_EX_TRANSPARENT) {
        exStyle &= ~WS_EX_TRANSPARENT;
        ::SetWindowLongPtrW(hWnd, GWL_EXSTYLE, exStyle);
    }

    ::SetWindowPos(hWnd, nullptr, 0, 0, 0, 0,
                   SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
}

WindowsFramelessWindowBase::WindowsFramelessWindowBase() = default;
WindowsFramelessWindowBase::~WindowsFramelessWindowBase() = default;

void WindowsFramelessWindowBase::setResizeEnabled(bool enabled) { resizeEnabled_ = enabled; }

bool WindowsFramelessWindowBase::isResizeEnabled() const { return resizeEnabled_; }

void WindowsFramelessWindowBase::initFrameless(QWidget* window) {
    if (!window) {
        return;
    }

    window->setWindowFlag(Qt::Window, true);

    if (!windowEffect_) {
        windowEffect_ = new WindowsWindowEffect();
    }

    const Qt::WindowFlags stayOnTop = (window->windowFlags() & Qt::WindowStaysOnTopHint)
                                          ? Qt::WindowStaysOnTopHint
                                          : Qt::WindowFlags{};

    window->setWindowFlags(window->windowFlags() | Qt::FramelessWindowHint | stayOnTop);

#ifdef Q_OS_WIN
    const HWND hWnd = reinterpret_cast<HWND>(window->winId());
    if (hWnd) {
        MARGINS margins = {1, 1, 0, 1};
        DwmExtendFrameIntoClientArea(hWnd, &margins);

        LONG_PTR style = ::GetWindowLongPtrW(hWnd, GWL_STYLE);
        if (style) {
            style |=
                WS_BORDER | WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SIZEBOX | WS_SYSMENU;
            ::SetWindowLongPtrW(hWnd, GWL_STYLE, style);
        }
    }
#endif
}

bool WindowsFramelessWindowBase::handleNativeEvent(QWidget* window, void* message,
                                                   nativeEvent_qintptr* result) {
    if (!window || !message) {
        return false;
    }

    MSG* msg = reinterpret_cast<MSG*>(message);
    if (!msg->hwnd) {
        return false;
    }

    LONG_PTR exStyle = ::GetWindowLongPtrW(msg->hwnd, GWL_EXSTYLE);
    if (exStyle & WS_EX_TRANSPARENT) {
        exStyle &= ~WS_EX_TRANSPARENT;
        ::SetWindowLongPtrW(msg->hwnd, GWL_EXSTYLE, exStyle);
    }

    switch (msg->message) {
        case WM_MOUSEACTIVATE:
            *result = MA_ACTIVATE;
            return true;
        case WM_NCACTIVATE:
        case WM_NCPAINT:
            return false;
        case WM_NCHITTEST: {
            if (!resizeEnabled_ || isMaximized(msg->hwnd) || isFullScreen(msg->hwnd)) {
                *result = ::DefWindowProcW(msg->hwnd, msg->message, msg->wParam, msg->lParam);
                return true;
            }

            const HWND hWnd = msg->hwnd;

            RECT windowRect{};
            ::GetWindowRect(hWnd, &windowRect);

            POINT pt{GET_X_LPARAM(msg->lParam), GET_Y_LPARAM(msg->lParam)};

            const int xBorder = resizeBorderThickness(true);
            const int yBorder = resizeBorderThickness(false);

            // Right edge
            if (pt.x <= windowRect.right && pt.x >= windowRect.right - xBorder) {
                if (pt.y <= windowRect.bottom && pt.y >= windowRect.bottom - yBorder) {
                    *result = HTBOTTOMRIGHT;
                } else if (pt.y >= windowRect.top && pt.y <= windowRect.top + yBorder) {
                    *result = HTTOPRIGHT;
                } else {
                    *result = HTRIGHT;
                }
                return true;
            }
            // Bottom edge
            if (pt.y <= windowRect.bottom && pt.y >= windowRect.bottom - yBorder) {
                if (pt.x >= windowRect.left && pt.x <= windowRect.left + xBorder) {
                    *result = HTBOTTOMLEFT;
                } else {
                    *result = HTBOTTOM;
                }
                return true;
            }
            // Top edge
            if (pt.y >= windowRect.top && pt.y <= windowRect.top + yBorder) {
                if (pt.x >= windowRect.left && pt.x <= windowRect.left + xBorder) {
                    *result = HTTOPLEFT;
                } else {
                    *result = HTTOP;
                }
                return true;
            }
            // Left edge
            if (pt.x >= windowRect.left && pt.x <= windowRect.left + xBorder) {
                *result = HTLEFT;
                return true;
            }

            *result = ::DefWindowProcW(msg->hwnd, msg->message, msg->wParam, msg->lParam);
            return true;
        }
        case WM_NCCALCSIZE:
            *result = 0;
            return true;
        default:
            break;
    }

    Q_UNUSED(result);
    return false;
}

// ============================================================================
// WindowsFramelessWindow
// ============================================================================

WindowsFramelessWindow::WindowsFramelessWindow(QWidget* parent) : QWidget(parent) {
    initFrameless(this);
}

bool WindowsFramelessWindow::nativeEvent(const QByteArray& eventType, void* message,
                                         nativeEvent_qintptr* result) {
    Q_UNUSED(eventType);
    if (handleNativeEvent(this, message, result)) {
        return true;
    }
    return QWidget::nativeEvent(eventType, message, result);
}

void WindowsFramelessWindow::showEvent(QShowEvent* e) {
    QWidget::showEvent(e);

    if (effectsApplied_) {
        return;
    }

    effectsApplied_ = true;

    // Styles are already set in initFrameless (match FancyUI)
    const HWND hWnd = reinterpret_cast<HWND>(winId());
    if (windowEffect_) {
        windowEffect_->addWindowAnimation(hWnd);
        windowEffect_->addShadowEffect(hWnd);
        windowEffect_->setWindowCornerPreference(hWnd, 2);  // Round corners
    }
}

void WindowsFramelessWindow::paintEvent(QPaintEvent* e) {
#ifdef Q_OS_WIN
    // Fix click-through on Qt 5.15.2 with Mica/Acrylic:
    const HWND hWnd = reinterpret_cast<HWND>(winId());
    if (hWnd) {
        LONG_PTR exStyle = ::GetWindowLongPtrW(hWnd, GWL_EXSTYLE);
        if (exStyle & WS_EX_TRANSPARENT) {
            exStyle &= ~WS_EX_TRANSPARENT;
            ::SetWindowLongPtrW(hWnd, GWL_EXSTYLE, exStyle);
        }
    }
#endif
    QWidget::paintEvent(e);
}

// ============================================================================
// WindowsFramelessMainWindow
// ============================================================================

WindowsFramelessMainWindow::WindowsFramelessMainWindow(QWidget* parent) : QMainWindow(parent) {
    initFrameless(this);
}

bool WindowsFramelessMainWindow::nativeEvent(const QByteArray& eventType, void* message,
                                             nativeEvent_qintptr* result) {
    Q_UNUSED(eventType);
    if (handleNativeEvent(this, message, result)) {
        return true;
    }
    return QMainWindow::nativeEvent(eventType, message, result);
}

void WindowsFramelessMainWindow::showEvent(QShowEvent* e) {
    QMainWindow::showEvent(e);

    // Fix click-through on Qt 5.15.2 with Mica/Acrylic:
    const HWND hWnd = reinterpret_cast<HWND>(winId());
    if (hWnd) {
        LONG_PTR exStyle = ::GetWindowLongPtrW(hWnd, GWL_EXSTYLE);
        if (exStyle & WS_EX_TRANSPARENT) {
            exStyle &= ~WS_EX_TRANSPARENT;
            ::SetWindowLongPtrW(hWnd, GWL_EXSTYLE, exStyle);
        }
    }

    if (effectsApplied_) {
        return;
    }

    effectsApplied_ = true;

    // Styles are already set in initFrameless (match FancyUI)
    if (windowEffect_) {
        windowEffect_->addWindowAnimation(hWnd);
        windowEffect_->addShadowEffect(hWnd);
        windowEffect_->setWindowCornerPreference(hWnd, 2);  // Round corners
    }
}

// ============================================================================
// WindowsFramelessDialog
// ============================================================================

WindowsFramelessDialog::WindowsFramelessDialog(QWidget* parent) : QDialog(parent) {
    initFrameless(this);
}

bool WindowsFramelessDialog::nativeEvent(const QByteArray& eventType, void* message,
                                         nativeEvent_qintptr* result) {
    Q_UNUSED(eventType);
    if (handleNativeEvent(this, message, result)) {
        return true;
    }
    return QDialog::nativeEvent(eventType, message, result);
}

void WindowsFramelessDialog::showEvent(QShowEvent* e) {
    QDialog::showEvent(e);

    // Fix click-through on Qt 5.15.2 with Mica/Acrylic:
    const HWND hWnd = reinterpret_cast<HWND>(winId());
    if (hWnd) {
        LONG_PTR exStyle = ::GetWindowLongPtrW(hWnd, GWL_EXSTYLE);
        if (exStyle & WS_EX_TRANSPARENT) {
            exStyle &= ~WS_EX_TRANSPARENT;
            ::SetWindowLongPtrW(hWnd, GWL_EXSTYLE, exStyle);
        }
    }

    if (effectsApplied_) {
        return;
    }

    effectsApplied_ = true;

    // Styles are already set in initFrameless (match FancyUI)
    if (windowEffect_) {
        windowEffect_->addWindowAnimation(hWnd);
        windowEffect_->addShadowEffect(hWnd);
        windowEffect_->disableMaximizeButton(hWnd);
        windowEffect_->setWindowCornerPreference(hWnd, 2);  // Round corners
    }
}

}  // namespace qfw
