#include "common/color.h"

#include <algorithm>

#include "common/config.h"
#include "common/style_sheet.h"

namespace qfw {

QColor getFluentThemeColor(FluentThemeColor color) {
    switch (color) {
        // Yellow/Gold colors
        case FluentThemeColor::YellowGold:
            return QColor("#FFB900");
        case FluentThemeColor::Gold:
            return QColor("#FF8C00");
        case FluentThemeColor::OrangeBright:
            return QColor("#F7630C");
        case FluentThemeColor::OrangeDark:
            return QColor("#CA5010");
        case FluentThemeColor::Rust:
            return QColor("#DA3B01");
        case FluentThemeColor::PaleRust:
            return QColor("#EF6950");

        // Red colors
        case FluentThemeColor::BrickRed:
            return QColor("#D13438");
        case FluentThemeColor::ModRed:
            return QColor("#FF4343");
        case FluentThemeColor::PaleRed:
            return QColor("#E74856");
        case FluentThemeColor::Red:
            return QColor("#E81123");
        case FluentThemeColor::RoseBright:
            return QColor("#EA005E");
        case FluentThemeColor::Rose:
            return QColor("#C30052");
        case FluentThemeColor::PlumLight:
            return QColor("#E3008C");
        case FluentThemeColor::Plum:
            return QColor("#BF0077");
        case FluentThemeColor::OrchidLight:
            return QColor("#BF0077");
        case FluentThemeColor::Orchid:
            return QColor("#9A0089");

        // Blue/Purple colors
        case FluentThemeColor::DefaultBlue:
            return QColor("#0078D7");
        case FluentThemeColor::NavyBlue:
            return QColor("#0063B1");
        case FluentThemeColor::PurpleShadow:
            return QColor("#8E8CD8");
        case FluentThemeColor::PurpleShadowDark:
            return QColor("#6B69D6");
        case FluentThemeColor::IrisPastel:
            return QColor("#8764B8");
        case FluentThemeColor::IrisSpring:
            return QColor("#744DA9");
        case FluentThemeColor::VioletRedLight:
            return QColor("#B146C2");
        case FluentThemeColor::VioletRed:
            return QColor("#881798");

        // Cool colors
        case FluentThemeColor::CoolBlueBright:
            return QColor("#0099BC");
        case FluentThemeColor::CoolBlur:
            return QColor("#2D7D9A");
        case FluentThemeColor::Seafoam:
            return QColor("#00B7C3");
        case FluentThemeColor::SeafoamTeal:
            return QColor("#038387");
        case FluentThemeColor::MintLight:
            return QColor("#00B294");
        case FluentThemeColor::MintDark:
            return QColor("#018574");
        case FluentThemeColor::TurfGreen:
            return QColor("#00CC6A");
        case FluentThemeColor::SportGreen:
            return QColor("#10893E");

        // Gray/Brown colors
        case FluentThemeColor::Gray:
            return QColor("#7A7574");
        case FluentThemeColor::GrayBrown:
            return QColor("#5D5A58");
        case FluentThemeColor::StealBlue:
            return QColor("#68768A");
        case FluentThemeColor::MetalBlue:
            return QColor("#515C6B");
        case FluentThemeColor::PaleMoss:
            return QColor("#567C73");
        case FluentThemeColor::Moss:
            return QColor("#486860");
        case FluentThemeColor::MeadowGreen:
            return QColor("#498205");
        case FluentThemeColor::Green:
            return QColor("#107C10");

        // Dark colors
        case FluentThemeColor::Overcast:
            return QColor("#767676");
        case FluentThemeColor::Storm:
            return QColor("#4C4A48");
        case FluentThemeColor::BlueGray:
            return QColor("#69797E");
        case FluentThemeColor::GrayDark:
            return QColor("#4A5459");
        case FluentThemeColor::LiddyGreen:
            return QColor("#647C64");
        case FluentThemeColor::Sage:
            return QColor("#525E54");
        case FluentThemeColor::CamouflageDesert:
            return QColor("#847545");
        case FluentThemeColor::Camouflage:
            return QColor("#7E735F");

        default:
            return QColor();
    }
}

QColor getFluentSystemColor(FluentSystemColor color, Theme theme) {
    bool isDark = QConfig::instance().theme() == Theme::Dark;

    switch (color) {
        case FluentSystemColor::SuccessForeground:
            return QColor(isDark ? "#6ccb5f" : "#0f7b0f");
        case FluentSystemColor::CautionForeground:
            return QColor(isDark ? "#fce100" : "#9d5d00");
        case FluentSystemColor::CriticalForeground:
            return QColor(isDark ? "#ff99a4" : "#c42b1c");

        case FluentSystemColor::SuccessBackground:
            return QColor(isDark ? "#393d1b" : "#dff6dd");
        case FluentSystemColor::CautionBackground:
            return QColor(isDark ? "#433519" : "#fff4ce");
        case FluentSystemColor::CriticalBackground:
            return QColor(isDark ? "#442726" : "#fde7e9");

        default:
            return QColor();
    }
}

QColor themedColor(const QColor& base, bool darkTheme, const QString& token) {
    float h = 0, s = 0, v = 0, a = 0;
    base.getHsvF(&h, &s, &v, &a);

    if (darkTheme) {
        s *= 0.84f;
        v = 1.0f;

        if (token == QStringLiteral("ThemeColorDark1")) {
            v *= 0.9f;
        } else if (token == QStringLiteral("ThemeColorDark2")) {
            s *= 0.977f;
            v *= 0.82f;
        } else if (token == QStringLiteral("ThemeColorDark3")) {
            s *= 0.95f;
            v *= 0.7f;
        } else if (token == QStringLiteral("ThemeColorLight1")) {
            s *= 0.92f;
        } else if (token == QStringLiteral("ThemeColorLight2")) {
            s *= 0.78f;
        } else if (token == QStringLiteral("ThemeColorLight3")) {
            s *= 0.65f;
        }
    } else {
        if (token == QStringLiteral("ThemeColorDark1")) {
            v *= 0.75f;
        } else if (token == QStringLiteral("ThemeColorDark2")) {
            s *= 1.05f;
            v *= 0.5f;
        } else if (token == QStringLiteral("ThemeColorDark3")) {
            s *= 1.1f;
            v *= 0.4f;
        } else if (token == QStringLiteral("ThemeColorLight1")) {
            v *= 1.05f;
        } else if (token == QStringLiteral("ThemeColorLight2")) {
            s *= 0.75f;
            v *= 1.05f;
        } else if (token == QStringLiteral("ThemeColorLight3")) {
            s *= 0.65f;
            v *= 1.05f;
        }
    }

    return QColor::fromHsvF(static_cast<qreal>(h), static_cast<qreal>(std::min(s, 1.0f)),
                            static_cast<qreal>(std::min(v, 1.0f)), static_cast<qreal>(a));
}

QColor validColor(const QColor& color, const QColor& defaultColor) {
    return color.isValid() ? color : defaultColor;
}

QColor fallbackThemeColor(const QColor& color) { return color.isValid() ? color : themeColor(); }

QColor autoFallbackThemeColor(const QColor& light, const QColor& dark) {
    QColor color = QConfig::instance().theme() == Theme::Dark ? dark : light;
    return fallbackThemeColor(color);
}

}  // namespace qfw
