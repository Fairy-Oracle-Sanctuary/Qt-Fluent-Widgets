#pragma once

#include <QtGlobal>

#ifdef Q_OS_WIN
// Disable Windows min/max macros to avoid conflicts with C++ standard library
#define NOMINMAX
#include <windows.h>
#else
using HWND = void*;
#endif

#include <QColor>
#include <QString>

namespace qfw {

class WindowsWindowEffect {
public:
    WindowsWindowEffect() = default;

#ifdef Q_OS_WIN
    void addWindowAnimation(HWND hWnd);
    void addShadowEffect(HWND hWnd);

    void enableBlurBehindWindow(HWND hWnd);

    void setAcrylicEffect(HWND hWnd, const QString& gradientColor = QStringLiteral("F2F2F299"),
                          bool enableShadow = true, int animationId = 0);

    void setMicaEffect(HWND hWnd, bool isDarkMode = false, bool isAlt = false);

    void setBorderAccentColor(HWND hWnd, const QColor& color);
    void removeBorderAccentColor(HWND hWnd);

    void disableMaximizeButton(HWND hWnd);
#else
    void addWindowAnimation(HWND) {}
    void addShadowEffect(HWND) {}

    void enableBlurBehindWindow(HWND) {}

    void setAcrylicEffect(HWND, const QString& = QStringLiteral("F2F2F299"), bool = true, int = 0) {
    }

    void setMicaEffect(HWND, bool = false, bool = false) {}

    void setBorderAccentColor(HWND, const QColor&) {}
    void removeBorderAccentColor(HWND) {}

    void disableMaximizeButton(HWND) {}
#endif
};

}  // namespace qfw
