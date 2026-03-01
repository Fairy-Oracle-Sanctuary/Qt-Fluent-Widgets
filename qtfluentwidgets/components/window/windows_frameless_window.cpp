#include "components/window/windows_frameless_window.h"

#ifdef Q_OS_WIN

#include <dwmapi.h>
#include <windows.h>
#include <windowsx.h>

#include <QDebug>
#include <QPainter>
#include <QScreen>
#include <QShowEvent>
#include <QWindow>

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

    // 0 = Auto, 1 = None, 2 = Mica, 3 = Acrylic, 4 = MicaAlt
    // Only Mica/MicaAlt are known to degrade when we intercept WM_NCCALCSIZE.
    return backdropType == 2 || backdropType == 4;
}

static void ensureResizableStyle(HWND hWnd) {
    if (!hWnd) {
        return;
    }

    LONG_PTR style = ::GetWindowLongPtrW(hWnd, GWL_STYLE);
    if (!style) {
        return;
    }

    // Keep only WS_THICKFRAME for resizing via WM_NCHITTEST.
    // Remove WS_SYSMENU/WS_MINIMIZEBOX/WS_MAXIMIZEBOX to prevent native titlebar buttons.
    // Remove WS_CAPTION to ensure no native title bar is rendered.
    style &= ~(WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
    style |= WS_THICKFRAME;
    ::SetWindowLongPtrW(hWnd, GWL_STYLE, style);

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

    // Ensure this widget is treated as a top-level window.
    // Some composition effects (acrylic/mica) won't apply reliably to non-window widgets.
    window->setWindowFlag(Qt::Window, true);

    if (!windowEffect_) {
        windowEffect_ = new WindowsWindowEffect();
    }

    const Qt::WindowFlags stayOnTop = (window->windowFlags() & Qt::WindowStaysOnTopHint)
                                          ? Qt::WindowStaysOnTopHint
                                          : Qt::WindowFlags{};

    // Do NOT force Qt::FramelessWindowHint.
    // We implement frameless behavior via nativeEvent (WM_NCCALCSIZE/WM_NCHITTEST), which is
    // more compatible with Win11 system backdrop (Mica).
    window->setWindowFlags(Qt::Window | window->windowFlags() | stayOnTop);
}

bool WindowsFramelessWindowBase::handleNativeEvent(QWidget* window, void* message,
                                                   qintptr* result) {
    if (!window || !message) {
        return false;
    }

    MSG* msg = reinterpret_cast<MSG*>(message);
    if (!msg->hwnd) {
        return false;
    }

    switch (msg->message) {
        case WM_NCACTIVATE:
        case WM_NCPAINT:
            // Do not suppress default non-client processing.
            // Suppressing these messages may break maximize/restore animations.
            return false;
        case WM_NCHITTEST: {
            if (!resizeEnabled_) {
                return false;
            }

            const HWND hWnd = msg->hwnd;

            POINT globalPt{GET_X_LPARAM(msg->lParam), GET_Y_LPARAM(msg->lParam)};
            POINT clientPt = globalPt;
            ::ScreenToClient(hWnd, &clientPt);

            RECT clientRect{};
            ::GetClientRect(hWnd, &clientRect);

            const int w = clientRect.right - clientRect.left;
            const int h = clientRect.bottom - clientRect.top;

            const int bw = (isMaximized(hWnd) || isFullScreen(hWnd)) ? 0 : BORDER_WIDTH;

            const bool lx = clientPt.x < bw;
            const bool rx = clientPt.x > w - bw;
            const bool ty = clientPt.y < bw;
            const bool by = clientPt.y > h - bw;

            if (lx && ty) {
                *result = HTTOPLEFT;
                return true;
            }
            if (rx && by) {
                *result = HTBOTTOMRIGHT;
                return true;
            }
            if (rx && ty) {
                *result = HTTOPRIGHT;
                return true;
            }
            if (lx && by) {
                *result = HTBOTTOMLEFT;
                return true;
            }
            if (ty) {
                *result = HTTOP;
                return true;
            }
            if (by) {
                *result = HTBOTTOM;
                return true;
            }
            if (lx) {
                *result = HTLEFT;
                return true;
            }
            if (rx) {
                *result = HTRIGHT;
                return true;
            }

            return false;
        }
        case WM_NCCALCSIZE: {
            // If mica/acrylic has been enabled for this HWND, handle it specially.
            // The window property is set by WindowsWindowEffect::setMicaEffect/setAcrylicEffect.
            if (::GetPropW(msg->hwnd, L"qfw_mica_enabled") ||
                ::GetPropW(msg->hwnd, L"qfw_acrylic_enabled")) {
                // Match FancyUI: return 0 to remove the native title bar & borders.
                // But when maximized, still adjust the client rect to avoid overflow.
                if (msg->wParam) {
                    NCCALCSIZE_PARAMS* params = reinterpret_cast<NCCALCSIZE_PARAMS*>(msg->lParam);
                    if (params) {
                        const HWND hWnd = msg->hwnd;
                        const bool max = isMaximized(hWnd);
                        const bool full = isFullScreen(hWnd);
                        if (max && !full) {
                            const int ty = resizeBorderThickness(false);
                            const int tx = resizeBorderThickness(true);
                            params->rgrc[0].top += ty;
                            params->rgrc[0].bottom -= ty;
                            params->rgrc[0].left += tx;
                            params->rgrc[0].right -= tx;
                        }
                    }
                }
                *result = 0;
                return true;
            }

            // If Mica/system backdrop is enabled, do not override default non-client
            // calculations. Intercepting WM_NCCALCSIZE can cause the backdrop to degrade
            // into a solid color on some Qt frameless windows.
            if (hasSystemBackdrop(msg->hwnd)) {
                return false;
            }

            if (msg->wParam) {
                // Remove the default frame.
                // When maximized, shrink client rect to avoid covering the monitor edges.
                NCCALCSIZE_PARAMS* params = reinterpret_cast<NCCALCSIZE_PARAMS*>(msg->lParam);
                if (!params) {
                    return false;
                }

                const HWND hWnd = msg->hwnd;
                const bool max = isMaximized(hWnd);
                const bool full = isFullScreen(hWnd);

                if (max && !full) {
                    const int ty = resizeBorderThickness(false);
                    const int tx = resizeBorderThickness(true);
                    params->rgrc[0].top += ty;
                    params->rgrc[0].bottom -= ty;
                    params->rgrc[0].left += tx;
                    params->rgrc[0].right -= tx;
                }

                // Match qframelesswindow (python): return WVR_REDRAW when wParam is set.
                // This helps Windows re-run non-client calculations and keeps animations working.
                *result = WVR_REDRAW;
                return true;
            }
            return false;
        }
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
                                         qintptr* result) {
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

    const HWND hWnd = reinterpret_cast<HWND>(winId());
    ensureResizableStyle(hWnd);
    if (windowEffect_) {
        windowEffect_->addWindowAnimation(hWnd);
        windowEffect_->addShadowEffect(hWnd);
    }
}

void WindowsFramelessWindow::paintEvent(QPaintEvent* e) {
    const HWND hWnd = reinterpret_cast<HWND>(winId());
    if (hWnd &&
        (::GetPropW(hWnd, L"qfw_mica_enabled") || ::GetPropW(hWnd, L"qfw_acrylic_enabled"))) {
        QPainter painter(this);
        painter.setCompositionMode(QPainter::CompositionMode_Clear);
        painter.eraseRect(rect());
        return QWidget::paintEvent(e);
    }

    QWidget::paintEvent(e);
}

// ============================================================================
// WindowsFramelessMainWindow
// ============================================================================

WindowsFramelessMainWindow::WindowsFramelessMainWindow(QWidget* parent) : QMainWindow(parent) {
    initFrameless(this);
}

bool WindowsFramelessMainWindow::nativeEvent(const QByteArray& eventType, void* message,
                                             qintptr* result) {
    Q_UNUSED(eventType);
    if (handleNativeEvent(this, message, result)) {
        return true;
    }
    return QMainWindow::nativeEvent(eventType, message, result);
}

void WindowsFramelessMainWindow::showEvent(QShowEvent* e) {
    QMainWindow::showEvent(e);

    if (effectsApplied_) {
        return;
    }

    effectsApplied_ = true;

    const HWND hWnd = reinterpret_cast<HWND>(winId());
    ensureResizableStyle(hWnd);
    if (windowEffect_) {
        windowEffect_->addWindowAnimation(hWnd);
        windowEffect_->addShadowEffect(hWnd);
    }
}

// ============================================================================
// WindowsFramelessDialog
// ============================================================================

WindowsFramelessDialog::WindowsFramelessDialog(QWidget* parent) : QDialog(parent) {
    initFrameless(this);
}

bool WindowsFramelessDialog::nativeEvent(const QByteArray& eventType, void* message,
                                         qintptr* result) {
    Q_UNUSED(eventType);
    if (handleNativeEvent(this, message, result)) {
        return true;
    }
    return QDialog::nativeEvent(eventType, message, result);
}

void WindowsFramelessDialog::showEvent(QShowEvent* e) {
    QDialog::showEvent(e);

    if (effectsApplied_) {
        return;
    }

    effectsApplied_ = true;

    const HWND hWnd = reinterpret_cast<HWND>(winId());
    ensureResizableStyle(hWnd);
    if (windowEffect_) {
        windowEffect_->addWindowAnimation(hWnd);
        windowEffect_->addShadowEffect(hWnd);
        windowEffect_->disableMaximizeButton(hWnd);
    }
}

}  // namespace qfw

#endif  // Q_OS_WIN
