#pragma once

#include <QWidget>

#include "common/config.h"
#include "common/style_sheet.h"

namespace qfw {

// Gallery style sheet enumeration
enum class GalleryStyleSheet {
    LinkCard,
    SampleCard,
    HomeInterface,
    IconInterface,
    ViewInterface,
    SettingInterface,
    GalleryInterface,
    NavigationViewInterface
};

// Get style sheet path for the given style and theme
QString galleryStyleSheetPath(GalleryStyleSheet style, Theme theme = Theme::Auto);

// Custom StyleSheetBase that dynamically resolves theme for gallery stylesheets
class GalleryStyleSheetSource : public StyleSheetBase {
public:
    explicit GalleryStyleSheetSource(GalleryStyleSheet style) : style_(style) {}

    QString content(Theme theme = Theme::Auto) const override {
        QString path = galleryStyleSheetPath(style_, theme);
        return getStyleSheet(path, theme);
    }

    GalleryStyleSheet style() const { return style_; }

private:
    GalleryStyleSheet style_;
};

// Apply style sheet to widget with automatic theme update support
void applyGalleryStyleSheet(QWidget* widget, GalleryStyleSheet style);

}  // namespace qfw
