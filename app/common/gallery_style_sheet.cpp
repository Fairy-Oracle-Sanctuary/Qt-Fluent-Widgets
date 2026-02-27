#include "gallery_style_sheet.h"

#include <QWidget>

#include "common/config.h"

namespace qfw {

QString galleryStyleSheetPath(GalleryStyleSheet style, Theme theme) {
    // Resolve Theme::Auto to actual Light/Dark based on system or config
    QString themeStr = isDarkThemeMode(theme) ? QStringLiteral("dark") : QStringLiteral("light");

    QString styleName;
    switch (style) {
        case GalleryStyleSheet::LinkCard:
            styleName = QStringLiteral("link_card");
            break;
        case GalleryStyleSheet::SampleCard:
            styleName = QStringLiteral("sample_card");
            break;
        case GalleryStyleSheet::HomeInterface:
            styleName = QStringLiteral("home_interface");
            break;
        case GalleryStyleSheet::IconInterface:
            styleName = QStringLiteral("icon_interface");
            break;
        case GalleryStyleSheet::ViewInterface:
            styleName = QStringLiteral("view_interface");
            break;
        case GalleryStyleSheet::SettingInterface:
            styleName = QStringLiteral("setting_interface");
            break;
        case GalleryStyleSheet::GalleryInterface:
            styleName = QStringLiteral("gallery_interface");
            break;
        case GalleryStyleSheet::NavigationViewInterface:
            styleName = QStringLiteral("navigation_view_interface");
            break;
        default:
            styleName = QStringLiteral("link_card");
            break;
    }

    return QStringLiteral(":/gallery/qss/%1/%2.qss").arg(themeStr, styleName);
}

void applyGalleryStyleSheet(QWidget* widget, GalleryStyleSheet style) {
    GalleryStyleSheetSource source(style);
    // Directly register with StyleSheetManager for proper theme update support
    StyleSheetManager::instance().registerWidget(
        widget, QSharedPointer<StyleSheetBase>(new GalleryStyleSheetSource(style)), true);
    widget->setStyleSheet(source.content(Theme::Auto));
}

}  // namespace qfw
