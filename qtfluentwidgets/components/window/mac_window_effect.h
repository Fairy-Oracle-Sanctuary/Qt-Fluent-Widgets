#pragma once

/**
 * macOS Window Effect
 * Ported from qframelesswindow/mac/window_effect.py
 */

#include <QColor>
#include <QString>

class QWidget;

namespace qfw {

class MacWindowEffect {
public:
    MacWindowEffect() = default;
    
    void setAcrylicEffect(QWidget* widget, const QString& gradientColor = QStringLiteral("F2F2F230"),
                         bool enableShadow = true, int animationId = 0);
    
    void setMicaEffect(QWidget* widget, bool isDarkMode = false, bool isAlt = false);
    
    void setBorderAccentColor(QWidget* widget, const QColor& color);
    void removeBorderAccentColor(QWidget* widget);
    
    void addShadowEffect(QWidget* widget);
    void removeShadowEffect(QWidget* widget);
    
    void addWindowAnimation(QWidget* widget);
    void disableMaximizeButton(QWidget* widget);
    
    void enableBlurBehindWindow(QWidget* widget);
};

}  // namespace qfw
