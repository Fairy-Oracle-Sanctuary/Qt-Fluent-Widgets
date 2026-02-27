#pragma once

#include <QString>

#include "common/icon.h"

namespace qfw {

// Gallery-specific icons that use :/gallery/images/icons/ prefix
enum class GalleryIconEnum {
    Grid,
    Menu,
    Text,
    Price,
    EmojiTabSymbols
};

// Gallery icon implementation - uses gallery resource prefix
class GalleryIcon : public FluentIconBase {
public:
    explicit GalleryIcon(GalleryIconEnum icon);

    QString path(Theme theme = Theme::Auto) const override;
    QIcon icon(Theme theme = Theme::Auto, const QColor& color = QColor()) const override;
    void render(QPainter* painter, const QRect& rect, Theme theme = Theme::Auto,
                const QVariantMap& attributes = {}) const override;
    FluentIconBase* clone() const override;

private:
    GalleryIconEnum icon_;
};

// Helper function to convert enum to string name
QString galleryIconEnumToString(GalleryIconEnum icon);

}  // namespace qfw
