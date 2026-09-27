#pragma once

#include <windows.h>

#include <QColor>
#include <QString>

namespace qfw {

class WindowsWindowEffect {
public:
    WindowsWindowEffect() = default;

    void addWindowAnimation(HWND hWnd);
    void addShadowEffect(HWND hWnd);

    void enableBlurBehindWindow(HWND hWnd);

    void setAcrylicEffect(HWND hWnd, const QString& gradientColor = QStringLiteral("F2F2F299"),
                          bool enableShadow = true, int animationId = 0);

    void setMicaEffect(HWND hWnd, bool isDarkMode = false, bool isAlt = false);
    void removeMicaEffect(HWND hWnd);
    // Lightweight refresh: only re-applies the DWM backdrop type attribute,
    // without touching window styles or frame. Use this to restore Mica after
    // hide/show without breaking native event/mouse tracking.
    void refreshMicaEffect(HWND hWnd, bool isDarkMode = false, bool isAlt = false);

    void setBorderAccentColor(HWND hWnd, const QColor& color);
    void removeBorderAccentColor(HWND hWnd);

    void disableMaximizeButton(HWND hWnd);

    void setWindowCornerPreference(HWND hWnd, int preference = 2);
};

}  // namespace qfw
