#include "components/window/mac_window_effect.h"
 
#include <QDebug>
#include <QtGlobal>
#include <QWidget>

#import <Cocoa/Cocoa.h>

namespace qfw {

// Helper to convert QWidget winId to NSWindow
static NSWindow* getNSWindow(WId winId) {
    NSView* view = reinterpret_cast<NSView*>(winId);
    return view.window;
}

static NSView* findContentSibling(NSView* contentView, NSView* qtView) {
    if (!contentView || !qtView) {
        return nullptr;
    }

    NSView* v = qtView;
    NSView* last = qtView;
    while (v && v != contentView) {
        last = v;
        v = [v superview];
    }

    if (v == contentView) {
        return last;
    }

    return nullptr;
}

void MacWindowEffect::setAcrylicEffect(QWidget* widget, const QString& gradientColor,
                                       bool enableShadow, int animationId) {
    if (!widget) return;

    if (qEnvironmentVariableIsSet("QFW_DISABLE_MAC_EFFECT")) {
        return;
    }
    
    Q_UNUSED(gradientColor);
    Q_UNUSED(enableShadow);
    Q_UNUSED(animationId);
    
    NSWindow* nsWindow = getNSWindow(widget->winId());
    if (!nsWindow) {
        qWarning() << "Failed to get NSWindow for acrylic effect";
        return;
    }

    NSView* qtView = reinterpret_cast<NSView*>(widget->winId());
    if (!qtView) {
        qWarning() << "Failed to get NSView for acrylic effect";
        return;
    }
     
    // Create NSVisualEffectView for blur effect
    NSView* contentView = [nsWindow contentView];
    if (!contentView) {
        qWarning() << "Failed to get contentView for acrylic effect";
        return;
    }

    // Avoid stacking multiple effect views: remove the existing one
    static NSString* const kQfwAcrylicTag = @"qfw_acrylic_effect_view";
    for (NSView* sub in [contentView subviews]) {
        if ([sub isKindOfClass:[NSVisualEffectView class]] &&
            [[sub identifier] isEqualToString:kQfwAcrylicTag]) {
            [sub removeFromSuperview];
            break;
        }
    }

    NSRect frame = [contentView bounds];
    NSVisualEffectView* visualEffectView =
        [[NSVisualEffectView alloc] initWithFrame:frame];
    [visualEffectView setIdentifier:kQfwAcrylicTag];
     
    // Configure visual effect view
    [visualEffectView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [visualEffectView setState:NSVisualEffectStateActive];
    [visualEffectView setMaterial:NSVisualEffectMaterialPopover];
    [visualEffectView setBlendingMode:NSVisualEffectBlendingModeBehindWindow];

    // Qt 6.9 on macOS: top-level widget's winId() returns the NSWindow's contentView itself (QNSView).
    // Qt draws directly on this QNSView without using subviews.
    // 
    // Strategy: Make NSVisualEffectView the new contentView, and put QNSView on top of it.
    // This ensures the blur effect is behind Qt's content.
    
    if (qtView == contentView) {
        // qtView IS contentView - Qt 6.9 behavior
        // Swap contentView with NSVisualEffectView, put QNSView on top
        [nsWindow setContentView:visualEffectView];
        [visualEffectView addSubview:qtView];
        
        // Ensure QNSView fills the visual effect view
        [qtView setFrame:[visualEffectView bounds]];
        [qtView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    } else {
        // Older Qt behavior: qtView is a subview or nested view under contentView
        NSView* relative = findContentSibling(contentView, qtView);
        if (!relative) {
            NSArray<NSView*>* subs = [contentView subviews];
            if ([subs count] > 0) {
                relative = subs[0];
            }
        }

        if (relative && relative != contentView) {
            [contentView addSubview:visualEffectView positioned:NSWindowBelow relativeTo:relative];
        } else {
            [contentView addSubview:visualEffectView];
        }
    }
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
