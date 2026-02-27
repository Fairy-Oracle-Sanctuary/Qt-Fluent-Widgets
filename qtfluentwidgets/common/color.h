#pragma once

#include <QColor>
#include <QString>

#include "common/config.h"

namespace qfw {

enum class FluentThemeColor {
    // Yellow/Gold colors
    YellowGold = 0,
    Gold,
    OrangeBright,
    OrangeDark,
    Rust,
    PaleRust,

    // Red colors
    BrickRed,
    ModRed,
    PaleRed,
    Red,
    RoseBright,
    Rose,
    PlumLight,
    Plum,
    OrchidLight,
    Orchid,

    // Blue/Purple colors
    DefaultBlue,
    NavyBlue,
    PurpleShadow,
    PurpleShadowDark,
    IrisPastel,
    IrisSpring,
    VioletRedLight,
    VioletRed,

    // Cool colors
    CoolBlueBright,
    CoolBlur,
    Seafoam,
    SeafoamTeal,
    MintLight,
    MintDark,
    TurfGreen,
    SportGreen,

    // Gray/Brown colors
    Gray,
    GrayBrown,
    StealBlue,
    MetalBlue,
    PaleMoss,
    Moss,
    MeadowGreen,
    Green,

    // Dark colors
    Overcast,
    Storm,
    BlueGray,
    GrayDark,
    LiddyGreen,
    Sage,
    CamouflageDesert,
    Camouflage
};

enum class FluentSystemColor {
    SuccessForeground,
    CautionForeground,
    CriticalForeground,
    SuccessBackground,
    CautionBackground,
    CriticalBackground
};

// Color utility functions
QColor getFluentThemeColor(FluentThemeColor color);
QColor getFluentSystemColor(FluentSystemColor color, Theme theme = Theme::Auto);

QColor themedColor(const QColor& base, bool darkTheme, const QString& token);
QColor validColor(const QColor& color, const QColor& defaultColor);
QColor fallbackThemeColor(const QColor& color);
QColor autoFallbackThemeColor(const QColor& light, const QColor& dark);

}  // namespace qfw
