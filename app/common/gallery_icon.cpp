#include "gallery_icon.h"

#include "common/config.h"

namespace qfw {

QString galleryIconEnumToString(GalleryIconEnum icon) {
    switch (icon) {
        case GalleryIconEnum::Grid:
            return QStringLiteral("Grid");
        case GalleryIconEnum::Menu:
            return QStringLiteral("Menu");
        case GalleryIconEnum::Text:
            return QStringLiteral("Text");
        case GalleryIconEnum::Price:
            return QStringLiteral("Price");
        case GalleryIconEnum::EmojiTabSymbols:
            return QStringLiteral("EmojiTabSymbols");
        default:
            return QStringLiteral("Grid");
    }
}

GalleryIcon::GalleryIcon(GalleryIconEnum icon) : icon_(icon) {}

QString GalleryIcon::path(Theme theme) const {
    QString color = getIconColor(theme, false);
    QString iconName = galleryIconEnumToString(icon_);
    return QStringLiteral(":/gallery/images/icons/%1_%2.svg").arg(iconName, color);
}

QIcon GalleryIcon::icon(Theme theme, const QColor& color) const {
    Q_UNUSED(color)
    return QIcon(path(theme));
}

void GalleryIcon::render(QPainter* painter, const QRect& rect, Theme theme,
                         const QVariantMap& attributes) const {
    Q_UNUSED(attributes)
    drawSvgIcon(path(theme), painter, rect);
}

FluentIconBase* GalleryIcon::clone() const {
    return new GalleryIcon(icon_);
}

}  // namespace qfw
