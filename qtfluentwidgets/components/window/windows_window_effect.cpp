#include "components/window/windows_window_effect.h"

#ifdef Q_OS_WIN

#include <dwmapi.h>
#include <windows.h>

#include <QDebug>

#pragma comment(lib, "dwmapi.lib")

#ifndef DWMWA_BORDER_COLOR
// Win11+ (SDK 22000+) defines this as DWMWINDOWATTRIBUTE value 34.
#define DWMWA_BORDER_COLOR static_cast<DWMWINDOWATTRIBUTE>(34)
#endif

#ifndef DWMWA_SYSTEMBACKDROP_TYPE
// Win11 22H2+ (SDK 22621+) defines this as DWMWINDOWATTRIBUTE value 38.
#define DWMWA_SYSTEMBACKDROP_TYPE static_cast<DWMWINDOWATTRIBUTE>(38)
#endif

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
// Win10 1903+ (SDK 18362+) defines this as DWMWINDOWATTRIBUTE value 20.
#define DWMWA_USE_IMMERSIVE_DARK_MODE static_cast<DWMWINDOWATTRIBUTE>(20)
#endif

#ifndef DWMWA_USE_HOSTBACKDROPBRUSH
// Win11 defines this as DWMWINDOWATTRIBUTE value 17.
#define DWMWA_USE_HOSTBACKDROPBRUSH static_cast<DWMWINDOWATTRIBUTE>(17)
#endif

namespace qfw {

// Helper to get Windows build number
static DWORD getWindowsBuildNumber() {
    // GetVersionExW can lie on Win10/Win11 depending on app manifest.
    // Prefer RtlGetVersion for an accurate build number.
    using RtlGetVersionPtr = LONG(WINAPI*)(PRTL_OSVERSIONINFOW);
    HMODULE ntdll = ::GetModuleHandleW(L"ntdll.dll");
    if (!ntdll) {
        ntdll = ::LoadLibraryW(L"ntdll.dll");
    }

    if (ntdll) {
        auto rtlGetVersion =
            reinterpret_cast<RtlGetVersionPtr>(::GetProcAddress(ntdll, "RtlGetVersion"));
        if (rtlGetVersion) {
            RTL_OSVERSIONINFOEXW osvi{};
            osvi.dwOSVersionInfoSize = sizeof(osvi);
            if (rtlGetVersion(reinterpret_cast<PRTL_OSVERSIONINFOW>(&osvi)) == 0) {
                return osvi.dwBuildNumber;
            }
        }
    }

    return 0;
}

// ============================================================================
// Win32 Acrylic helpers
// ============================================================================

// Structures copied from Windows undocumented API usage (same as many acrylic implementations)
enum ACCENT_STATE {
    ACCENT_DISABLED = 0,
    ACCENT_ENABLE_GRADIENT = 1,
    ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
    ACCENT_ENABLE_BLURBEHIND = 3,
    ACCENT_ENABLE_ACRYLICBLURBEHIND = 4,
    ACCENT_ENABLE_HOSTBACKDROP = 5
};

struct ACCENT_POLICY {
    int AccentState;
    int AccentFlags;
    DWORD GradientColor;
    int AnimationId;
};

enum WINDOWCOMPOSITIONATTRIB { WCA_ACCENT_POLICY = 19, WCA_USEDARKMODECOLORS = 26 };

struct WINDOWCOMPOSITIONATTRIBDATA {
    int Attribute;
    PVOID Data;
    SIZE_T SizeOfData;
};

using pfnSetWindowCompositionAttribute = BOOL(WINAPI*)(HWND, WINDOWCOMPOSITIONATTRIBDATA*);

static pfnSetWindowCompositionAttribute resolveSetWindowCompositionAttribute() {
    static pfnSetWindowCompositionAttribute fn = nullptr;
    static bool resolved = false;
    if (resolved) {
        return fn;
    }
    resolved = true;

    HMODULE user32 = ::GetModuleHandleW(L"user32.dll");
    if (!user32) {
        user32 = ::LoadLibraryW(L"user32.dll");
    }
    if (!user32) {
        return nullptr;
    }

    fn = reinterpret_cast<pfnSetWindowCompositionAttribute>(
        ::GetProcAddress(user32, "SetWindowCompositionAttribute"));
    return fn;
}

static DWORD parseGradientColor(const QString& argb) {
    // Match Python version: gradientColor is a hex string in RGBA order: "RRGGBBAA"
    // ACCENT_POLICY expects a DWORD in AABBGGRR order.
    QString s = argb;
    s = s.trimmed();
    if (s.startsWith(QStringLiteral("#"))) {
        s = s.mid(1);
    }
    if (s.size() != 8) {
        return 0;
    }

    bool ok = false;
    quint32 v = s.toUInt(&ok, 16);
    if (!ok) {
        return 0;
    }

    const quint32 rr = (v >> 24) & 0xFF;
    const quint32 gg = (v >> 16) & 0xFF;
    const quint32 bb = (v >> 8) & 0xFF;
    const quint32 aa = (v) & 0xFF;

    return static_cast<DWORD>((aa << 24) | (bb << 16) | (gg << 8) | rr);
}

void WindowsWindowEffect::addWindowAnimation(HWND hWnd) {
    if (!hWnd) {
        return;
    }

    // Match Python logic: ensure required window styles so minimize/maximize animations work.
    // Qt may strip these when using Qt::FramelessWindowHint.
    LONG_PTR style = ::GetWindowLongPtrW(hWnd, GWL_STYLE);
    if (style) {
        style |= (WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CAPTION | WS_THICKFRAME);
        ::SetWindowLongPtrW(hWnd, GWL_STYLE, style);
    }

    ULONG_PTR clsStyle = ::GetClassLongPtrW(hWnd, GCL_STYLE);
    clsStyle |= CS_DBLCLKS;
    ::SetClassLongPtrW(hWnd, GCL_STYLE, clsStyle);

    ::SetWindowPos(hWnd, nullptr, 0, 0, 0, 0,
                   SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

    const BOOL disabled = FALSE;
    HRESULT hr =
        DwmSetWindowAttribute(hWnd, DWMWA_TRANSITIONS_FORCEDISABLED, &disabled, sizeof(disabled));
    if (FAILED(hr)) {
        qWarning() << "DwmSetWindowAttribute(DWMWA_TRANSITIONS_FORCEDISABLED) failed:" << hr;
    }
}

void WindowsWindowEffect::addShadowEffect(HWND hWnd) {
    if (!hWnd) {
        return;
    }

    // Enable non-client rendering and extend a tiny frame into client area to get DWM shadow.
    const DWMNCRENDERINGPOLICY policy = DWMNCRP_ENABLED;
    HRESULT hr = DwmSetWindowAttribute(hWnd, DWMWA_NCRENDERING_POLICY, &policy, sizeof(policy));
    if (FAILED(hr)) {
        qWarning() << "DwmSetWindowAttribute(DWMWA_NCRENDERING_POLICY) failed:" << hr;
    }

    MARGINS m{1, 1, 1, 1};
    hr = DwmExtendFrameIntoClientArea(hWnd, &m);
    if (FAILED(hr)) {
        qWarning() << "DwmExtendFrameIntoClientArea failed:" << hr;
    }
}

void WindowsWindowEffect::enableBlurBehindWindow(HWND hWnd) {
    if (!hWnd) {
        return;
    }

    DWM_BLURBEHIND bb{};
    bb.dwFlags = DWM_BB_ENABLE;
    bb.fEnable = TRUE;
    bb.hRgnBlur = nullptr;

    const HRESULT hr = DwmEnableBlurBehindWindow(hWnd, &bb);
    if (FAILED(hr)) {
        qWarning() << "DwmEnableBlurBehindWindow failed:" << hr;
    }
}

void WindowsWindowEffect::setAcrylicEffect(HWND hWnd, const QString& gradientColor,
                                           bool enableShadow, int animationId) {
    if (!hWnd) {
        return;
    }

    // Mark this window so our nativeEvent handler can bypass WM_NCCALCSIZE logic
    // after acrylic is enabled (same as mica).
    ::SetPropW(hWnd, L"qfw_acrylic_enabled", reinterpret_cast<HANDLE>(1));

    // Prefer Win11 system backdrop acrylic (transient) when available.
    // This is more reliable than SetWindowCompositionAttribute on some Qt windows.
    {
        const bool disable = gradientColor.trimmed().isEmpty();
        const int backdropType = disable ? 1 : 3;  // 1=None, 3=TransientWindow (Acrylic-like)
        const HRESULT hr = DwmSetWindowAttribute(hWnd, DWMWA_SYSTEMBACKDROP_TYPE, &backdropType,
                                                 sizeof(backdropType));
        if (SUCCEEDED(hr)) {
            // Extend frame into the whole client area so backdrop is visible.
            MARGINS margins{-1, -1, -1, -1};
            (void)DwmExtendFrameIntoClientArea(hWnd, &margins);

            ::SetWindowPos(
                hWnd, nullptr, 0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
            qDebug() << "Acrylic (system backdrop) applied, disable=" << disable;
            return;
        }
    }

    BOOL compEnabled = FALSE;
    const HRESULT compHr = ::DwmIsCompositionEnabled(&compEnabled);
    if (FAILED(compHr) || !compEnabled) {
        qWarning() << "DWM composition is disabled, acrylic cannot be applied";
        return;
    }

    auto fn = resolveSetWindowCompositionAttribute();
    if (!fn) {
        qWarning() << "SetWindowCompositionAttribute not available";
        return;
    }

    ACCENT_POLICY policy{};

    const bool disable = gradientColor.trimmed().isEmpty();
    policy.AccentState = disable ? ACCENT_DISABLED : ACCENT_ENABLE_ACRYLICBLURBEHIND;
    policy.AnimationId = animationId;
    policy.GradientColor = disable ? 0 : parseGradientColor(gradientColor);

    // Flags match Python: 0x20 | 0x40 | 0x80 | 0x100 if enableShadow
    policy.AccentFlags = (enableShadow && !disable) ? (0x20 | 0x40 | 0x80 | 0x100) : 0;

    WINDOWCOMPOSITIONATTRIBDATA data{};
    data.Attribute = WCA_ACCENT_POLICY;
    data.Data = &policy;
    data.SizeOfData = sizeof(policy);

    // Extend frame into the whole client area. This helps acrylic/mica effects show up
    // reliably on Win10/Win11 (similar to the Python implementation for mica).
    MARGINS margins{-1, -1, -1, -1};
    (void)DwmExtendFrameIntoClientArea(hWnd, &margins);

    const BOOL ok = fn(hWnd, &data);
    if (!ok) {
        qWarning() << "SetWindowCompositionAttribute(WCA_ACCENT_POLICY) failed, gle="
                   << static_cast<qulonglong>(::GetLastError());
    } else {
        qDebug() << "Acrylic applied, disable=" << disable << ", color=" << gradientColor;
    }

    // Force DWM to refresh the window composition so acrylic shows immediately
    // without requiring a focus change or resize.
    ::SetWindowPos(hWnd, nullptr, 0, 0, 0, 0,
                   SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
}

void WindowsWindowEffect::setMicaEffect(HWND hWnd, bool isDarkMode, bool isAlt) {
    if (!hWnd) {
        return;
    }

    // Mica/system backdrop often won't render correctly on layered windows.
    // Qt may set WS_EX_LAYERED when using translucent background attributes.
    LONG_PTR exStyle = ::GetWindowLongPtrW(hWnd, GWL_EXSTYLE);
    if (exStyle & WS_EX_LAYERED) {
        exStyle &= ~WS_EX_LAYERED;
        ::SetWindowLongPtrW(hWnd, GWL_EXSTYLE, exStyle);
    }

    // Match Python implementation for frameless windows.
    // Using extremely large left/right margins helps DWM treat the whole client area
    // as extended frame on some frameless Qt windows.
    MARGINS margins{16777215, 16777215, 0, 0};
    const HRESULT extendHr = DwmExtendFrameIntoClientArea(hWnd, &margins);
    if (FAILED(extendHr)) {
        qWarning() << "DwmExtendFrameIntoClientArea failed:" << extendHr;
    }

    // Mark this window so our nativeEvent handler can bypass WM_NCCALCSIZE logic
    // after mica is enabled.
    ::SetPropW(hWnd, L"qfw_mica_enabled", reinterpret_cast<HANDLE>(1));

    const BOOL hostBackdropBrush = TRUE;
    const HRESULT brushHr = DwmSetWindowAttribute(hWnd, DWMWA_USE_HOSTBACKDROPBRUSH,
                                                  &hostBackdropBrush, sizeof(hostBackdropBrush));
    if (FAILED(brushHr)) {
        qWarning() << "DwmSetWindowAttribute(DWMWA_USE_HOSTBACKDROPBRUSH) failed:" << brushHr;
    }

    // Get Windows build number (best-effort, for diagnostics only).
    // We still *prefer* trying DWMWA_SYSTEMBACKDROP_TYPE first below.
    const DWORD buildNumber = getWindowsBuildNumber();

    // Prefer DWMWA_SYSTEMBACKDROP_TYPE (Win11 22H2+). This is the modern, documented path.
    // 2 = Mica, 4 = MicaAlt
    const int backdropType = isAlt ? 4 : 2;
    HRESULT backdropHr =
        DwmSetWindowAttribute(hWnd, DWMWA_SYSTEMBACKDROP_TYPE, &backdropType, sizeof(backdropType));

    if (FAILED(backdropHr)) {
        qWarning() << "DwmSetWindowAttribute(DWMWA_SYSTEMBACKDROP_TYPE) failed:" << backdropHr;

        // Fallback for early Win11 builds: undocumented attribute 1029.
        const int value = 1;
        const HRESULT hr = DwmSetWindowAttribute(hWnd, static_cast<DWMWINDOWATTRIBUTE>(1029),
                                                 &value, sizeof(value));
        if (FAILED(hr)) {
            qWarning() << "DwmSetWindowAttribute(1029) failed:" << hr;
        }
    }

    // Set immersive dark mode
    const BOOL darkModeValue = isDarkMode ? TRUE : FALSE;
    const HRESULT darkHr = DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE,
                                                 &darkModeValue, sizeof(darkModeValue));
    if (FAILED(darkHr)) {
        qWarning() << "DwmSetWindowAttribute(DWMWA_USE_IMMERSIVE_DARK_MODE) failed:" << darkHr;
    }

    // Force refresh
    ::SetWindowPos(hWnd, nullptr, 0, 0, 0, 0,
                   SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
}

void WindowsWindowEffect::setBorderAccentColor(HWND hWnd, const QColor& color) {
    if (!hWnd) {
        return;
    }

    const DWORD colorref =
        static_cast<DWORD>(color.red() | (color.green() << 8) | (color.blue() << 16));
    const HRESULT hr = DwmSetWindowAttribute(hWnd, DWMWA_BORDER_COLOR, &colorref, sizeof(colorref));
    if (FAILED(hr)) {
        qWarning() << "DwmSetWindowAttribute(DWMWA_BORDER_COLOR) failed:" << hr;
    }
}

void WindowsWindowEffect::removeBorderAccentColor(HWND hWnd) {
    if (!hWnd) {
        return;
    }

    const DWORD none = 0xFFFFFFFF;
    const HRESULT hr = DwmSetWindowAttribute(hWnd, DWMWA_BORDER_COLOR, &none, sizeof(none));
    if (FAILED(hr)) {
        qWarning() << "DwmSetWindowAttribute(DWMWA_BORDER_COLOR reset) failed:" << hr;
    }
}

void WindowsWindowEffect::disableMaximizeButton(HWND hWnd) {
    if (!hWnd) {
        return;
    }

    LONG_PTR style = ::GetWindowLongPtrW(hWnd, GWL_STYLE);
    if (!style) {
        return;
    }

    style &= ~WS_MAXIMIZEBOX;
    ::SetWindowLongPtrW(hWnd, GWL_STYLE, style);
    ::SetWindowPos(hWnd, nullptr, 0, 0, 0, 0,
                   SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
}

}  // namespace qfw

#endif  // Q_OS_WIN
