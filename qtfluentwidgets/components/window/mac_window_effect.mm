#include "components/window/mac_window_effect.h"

#include <QDebug>
#include <QWidget>

#import <Cocoa/Cocoa.h>

namespace qfw {

// Helper to convert QWidget winId to NSWindow
static NSWindow* getNSWindow(WId winId) {
    NSView* view = reinterpret_cast<NSView*>(winId);
    return view.window;
}

void MacWindowEffect::setAcrylicEffect(QWidget* widget, const QString& gradientColor,
                                       bool enableShadow, int animationId) {
    if (!widget) return;
    
    Q_UNUSED(gradientColor);
    Q_UNUSED(enableShadow);
    Q_UNUSED(animationId);
    
    NSWindow* nsWindow = getNSWindow(widget->winId());
    if (!nsWindow) {
        qWarning() << "Failed to get NSWindow for acrylic effect";
        return;
    }
    
    // Create NSVisualEffectView for blur effect
    NSRect frame = NSMakeRect(0, 0, widget->width(), widget->height());
    NSVisualEffectView* visualEffectView = [[NSVisualEffectView alloc] initWithFrame:frame];
    
    // Configure visual effect view
    [visualEffectView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [visualEffectView setState:NSVisualEffectStateActive];
    [visualEffectView setMaterial:NSVisualEffectMaterialPopover];
    [visualEffectView setBlendingMode:NSVisualEffectBlendingModeBehindWindow];
    
    // Add visual effect view to content view
    NSView* contentView = [nsWindow contentView];
    [contentView addSubview:visualEffectView positioned:NSWindowBelow relativeTo:nil];
}

void MacWindowEffect::setMicaEffect(QWidget* widget, bool isDarkMode, bool isAlt) {
    // On macOS, mica effect is simulated with acrylic
    Q_UNUSED(isDarkMode);
    Q_UNUSED(isAlt);
    
    setAcrylicEffect(widget);
}

void MacWindowEffect::setBorderAccentColor(QWidget* widget, const QColor& color) {
    // macOS doesn't support border accent color like Windows
    Q_UNUSED(widget);
    Q_UNUSED(color);
}

void MacWindowEffect::removeBorderAccentColor(QWidget* widget) {
    // macOS doesn't support border accent color like Windows
    Q_UNUSED(widget);
}

void MacWindowEffect::addShadowEffect(QWidget* widget) {
    if (!widget) return;
    
    NSWindow* nsWindow = getNSWindow(widget->winId());
    if (!nsWindow) {
        qWarning() << "Failed to get NSWindow for shadow effect";
        return;
    }
    
    [nsWindow setHasShadow:YES];
}

void MacWindowEffect::removeShadowEffect(QWidget* widget) {
    if (!widget) return;
    
    NSWindow* nsWindow = getNSWindow(widget->winId());
    if (!nsWindow) {
        qWarning() << "Failed to get NSWindow for removing shadow";
        return;
    }
    
    [nsWindow setHasShadow:NO];
}

void MacWindowEffect::addWindowAnimation(QWidget* widget) {
    // macOS has built-in window animation, no additional setup needed
    Q_UNUSED(widget);
}

void MacWindowEffect::disableMaximizeButton(QWidget* widget) {
    if (!widget) return;
    
    NSWindow* nsWindow = getNSWindow(widget->winId());
    if (!nsWindow) {
        qWarning() << "Failed to get NSWindow for disabling maximize button";
        return;
    }
    
    // Hide zoom button (maximize button on macOS)
    [[nsWindow standardWindowButton:NSWindowZoomButton] setHidden:YES];
}

void MacWindowEffect::enableBlurBehindWindow(QWidget* widget) {
    // Similar to acrylic effect
    setAcrylicEffect(widget);
}

}  // namespace qfw
