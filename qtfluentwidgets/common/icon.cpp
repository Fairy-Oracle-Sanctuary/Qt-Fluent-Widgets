#include "common/icon.h"

#include <qlogging.h>
#include <qpainterpath.h>

#include <QAction>
#include <QColor>
#include <QFile>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QIcon>
#include <QMenu>
#include <QRegularExpression>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QtSvg/QSvgRenderer>
#include <cmath>
namespace qfw {

// ============================================================================
// FluentIconEngine
// ============================================================================

FluentIconEngine::FluentIconEngine(const FluentIconBase* icon, bool reverse)
    : icon_(icon ? icon->clone() : nullptr), isThemeReversed_(reverse), ownsIcon_(true) {}

FluentIconEngine::FluentIconEngine(const FluentIconBase& icon, bool reverse)
    : icon_(icon.clone()), isThemeReversed_(reverse), ownsIcon_(true) {}

FluentIconEngine::~FluentIconEngine() {
    if (ownsIcon_ && icon_) {
        delete icon_;
        icon_ = nullptr;
    }
}

void FluentIconEngine::paint(QPainter* painter, const QRect& rect, QIcon::Mode mode,
                             QIcon::State state) {
    painter->save();

    // Handle disabled/selected modes with opacity
    if (mode == QIcon::Disabled) {
        painter->setOpacity(0.5);
    } else if (mode == QIcon::Selected) {
        painter->setOpacity(0.7);
    }

    // Determine theme
    Theme theme = Theme::Auto;

    // Adjust rect (matching Python's rect adjustment)
    QRect adjustedRect = rect;
    if (rect.x() == 19) {
        adjustedRect = rect.adjusted(-1, 0, 0, 0);
    }

    // Render the icon
    if (icon_) {
        QVariantMap attributes;
        if (isThemeReversed_) {
            attributes.insert(QStringLiteral("reverse"), true);
        }
        icon_->render(painter, adjustedRect, theme, attributes);
    }

    painter->restore();
}

QIconEngine* FluentIconEngine::clone() const {
    // Clone with ownership of the icon
    return new FluentIconEngine(icon_, isThemeReversed_);
}

QPixmap FluentIconEngine::pixmap(const QSize& size, QIcon::Mode mode, QIcon::State state) {
    QImage image(size, QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPixmap pixmap = QPixmap::fromImage(image, Qt::NoFormatConversion);

    QPainter painter(&pixmap);
    QRect rect(0, 0, size.width(), size.height());
    paint(&painter, rect, mode, state);
    painter.end();

    return pixmap;
}

// ============================================================================
// SvgIconEngine
// ============================================================================

SvgIconEngine::SvgIconEngine(const QString& svg) : svgData_(svg.toUtf8()) {}

SvgIconEngine::SvgIconEngine(const QByteArray& svgData) : svgData_(svgData) {}

void SvgIconEngine::paint(QPainter* painter, const QRect& rect, QIcon::Mode mode,
                          QIcon::State state) {
    drawSvgIcon(svgData_, painter, rect);
}

QIconEngine* SvgIconEngine::clone() const { return new SvgIconEngine(svgData_); }

QPixmap SvgIconEngine::pixmap(const QSize& size, QIcon::Mode mode, QIcon::State state) {
    QImage image(size, QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPixmap pixmap = QPixmap::fromImage(image, Qt::NoFormatConversion);

    QPainter painter(&pixmap);
    QRect rect(0, 0, size.width(), size.height());
    paint(&painter, rect, mode, state);
    painter.end();

    return pixmap;
}

// ============================================================================
// FontIconEngine
// ============================================================================

FontIconEngine::FontIconEngine(const QString& fontFamily, const QString& character,
                               const QColor& color, bool bold)
    : fontFamily_(fontFamily), character_(character), color_(color), isBold_(bold) {}

void FontIconEngine::paint(QPainter* painter, const QRect& rect, QIcon::Mode mode,
                           QIcon::State state) {
    QFont font(fontFamily_);
    font.setBold(isBold_);
    font.setPixelSize(static_cast<int>(std::round(rect.height())));

    painter->setFont(font);
    painter->setPen(Qt::NoPen);
    painter->setBrush(color_);
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    QPainterPath path;
    path.addText(rect.x(), rect.y() + rect.height(), font, character_);
    painter->drawPath(path);
}

QIconEngine* FontIconEngine::clone() const {
    return new FontIconEngine(fontFamily_, character_, color_, isBold_);
}

QPixmap FontIconEngine::pixmap(const QSize& size, QIcon::Mode mode, QIcon::State state) {
    QImage image(size, QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPixmap pixmap = QPixmap::fromImage(image, Qt::NoFormatConversion);

    QPainter painter(&pixmap);
    QRect rect(0, 0, size.width(), size.height());
    paint(&painter, rect, mode, state);
    painter.end();

    return pixmap;
}

// ============================================================================
// Helper Functions
// ============================================================================

void drawSvgIcon(const QString& iconPath, QPainter* painter, const QRect& rect) {
    QSvgRenderer renderer(iconPath);
    renderer.render(painter, QRectF(rect));
}

void drawSvgIcon(const QByteArray& iconData, QPainter* painter, const QRect& rect) {
    QSvgRenderer renderer(iconData);
    renderer.render(painter, QRectF(rect));
}

QString getIconColor(Theme theme, bool reverse) {
    QString lightColor = reverse ? QStringLiteral("white") : QStringLiteral("black");
    QString darkColor = reverse ? QStringLiteral("black") : QStringLiteral("white");

    if (theme == Theme::Auto) {
        return isDarkTheme() ? darkColor : lightColor;
    } else {
        return (theme == Theme::Dark) ? darkColor : lightColor;
    }
}

QString writeSvg(const QString& iconPath, const QList<int>& indexes,
                 const QVariantMap& attributes) {
    if (!iconPath.toLower().endsWith(QStringLiteral(".svg"))) {
        return QString();
    }

    QFile file(iconPath);
    if (!file.open(QFile::ReadOnly)) {
        return QString();
    }

    QString svgContent = QString::fromUtf8(file.readAll());
    file.close();

    if (attributes.isEmpty()) {
        return svgContent;
    }

    // Apply attributes to <path> elements.
    // If indexes is empty -> apply to all paths (legacy behavior)
    // If indexes not empty -> apply only to selected paths by 0-based order
    QString modifiedSvg = svgContent;
    QSet<int> indexSet;
    if (!indexes.isEmpty()) {
        for (int idx : indexes) {
            indexSet.insert(idx);
        }
    }

    QRegularExpression pathStartRe(QStringLiteral("<path"));
    int offset = 0;
    int pathIndex = 0;
    QRegularExpressionMatch match;
    while ((match = pathStartRe.match(modifiedSvg, offset)).hasMatch()) {
        const int tagStart = match.capturedStart();
        int tagEnd = modifiedSvg.indexOf(QStringLiteral(">"), tagStart);
        if (tagEnd == -1) {
            break;
        }

        const bool shouldApply = indexSet.isEmpty() || indexSet.contains(pathIndex);
        QString tag = modifiedSvg.mid(tagStart, tagEnd - tagStart + 1);

        if (shouldApply) {
            // Remove and re-insert provided attributes
            for (auto it = attributes.begin(); it != attributes.end(); ++it) {
                const QString attrName = it.key();
                const QString attrValue = it.value().toString();

                QRegularExpression existingAttrRe(
                    QStringLiteral("\\s%1\\s*=\\s*\"[^\"]*\"").arg(attrName));
                tag.remove(existingAttrRe);

                const QString attrPair = QStringLiteral(" %1=\"%2\"").arg(attrName, attrValue);
                int insertPos = tag.lastIndexOf(QStringLiteral(">"));
                if (insertPos > 0 && tag.at(insertPos - 1) == QLatin1Char('/')) {
                    insertPos--;  // Move before the '/' in '/>'
                }
                tag.insert(insertPos, attrPair);
            }
        }

        modifiedSvg.replace(tagStart, tagEnd - tagStart + 1, tag);
        offset = tagStart + tag.length();
        ++pathIndex;
    }

    return modifiedSvg;
}

QIcon toQIcon(const FluentIconBase& icon) { return icon.icon(); }

QIcon toQIcon(const QString& iconPath) { return QIcon(iconPath); }

// ============================================================================
// FluentIconBase
// ============================================================================

QIcon FluentIconBase::icon(Theme theme, const QColor& color) const {
    QString iconPath = path(theme);

    // If it's an SVG with a custom color, use SvgIconEngine
    if (iconPath.endsWith(QStringLiteral(".svg")) && color.isValid()) {
        QVariantMap attributes;
        attributes.insert(QStringLiteral("fill"), color.name());
        attributes.insert(QStringLiteral("stroke"), color.name());
        const QString svg = writeSvg(iconPath, QList<int>(), attributes);
        if (!svg.isEmpty()) {
            return QIcon(new SvgIconEngine(svg.toUtf8()));
        }

        return QIcon(new SvgIconEngine(iconPath));
    }

    return QIcon(iconPath);
}

QIcon FluentIconBase::qicon(bool reverse) const {
    return QIcon(new FluentIconEngine(this, reverse));
}

void FluentIconBase::render(QPainter* painter, const QRect& rect, Theme theme,
                            const QVariantMap& attributes) const {
    QString iconPath = path(theme);

    if (iconPath.endsWith(QStringLiteral(".svg"))) {
        // Apply attributes (like fill color) to SVG before drawing
        QString svg = writeSvg(iconPath, QList<int>(), attributes);
        if (!svg.isEmpty()) {
            drawSvgIcon(svg.toUtf8(), painter, rect);
        } else {
            drawSvgIcon(iconPath, painter, rect);
        }
    } else {
        // For non-SVG icons, use QIcon
        QIcon icon(iconPath);
        painter->drawPixmap(rect, icon.pixmap(rect.size()));
    }
}

ColoredFluentIcon* FluentIconBase::colored(const QColor& lightColor,
                                           const QColor& darkColor) const {
    return new ColoredFluentIcon(this, lightColor, darkColor);
}

// ============================================================================
// ColoredFluentIcon
// ============================================================================

ColoredFluentIcon::ColoredFluentIcon(const FluentIconBase& icon, const QColor& lightColor,
                                     const QColor& darkColor)
    : fluentIcon_(&icon), lightColor_(lightColor), darkColor_(darkColor), ownsIcon_(false) {}

ColoredFluentIcon::ColoredFluentIcon(const FluentIconBase* icon, const QColor& lightColor,
                                     const QColor& darkColor)
    : fluentIcon_(icon), lightColor_(lightColor), darkColor_(darkColor), ownsIcon_(false) {}

QString ColoredFluentIcon::path(Theme theme) const {
    return fluentIcon_ ? fluentIcon_->path(theme) : QString();
}

QIcon ColoredFluentIcon::icon(Theme theme, const QColor& color) const {
    if (!fluentIcon_) {
        return QIcon();
    }

    QString iconPath = path(theme);
    if (!iconPath.endsWith(QStringLiteral(".svg"))) {
        return QIcon(iconPath);
    }

    // For SVG icons, modify the fill color
    QColor fillColor = color.isValid() ? color
                                       : (theme == Theme::Auto
                                              ? (isDarkTheme() ? darkColor_ : lightColor_)
                                              : (theme == Theme::Dark ? darkColor_ : lightColor_));

    QVariantMap attributes;
    attributes[QStringLiteral("fill")] = fillColor.name();
    QString modifiedSvg = writeSvg(iconPath, QList<int>(), attributes);

    if (!modifiedSvg.isEmpty()) {
        return QIcon(new SvgIconEngine(modifiedSvg.toUtf8()));
    }

    return QIcon(iconPath);
}

void ColoredFluentIcon::render(QPainter* painter, const QRect& rect, Theme theme,
                               const QVariantMap& attributes) const {
    if (!fluentIcon_) {
        return;
    }

    QString iconPath = path(theme);
    if (!iconPath.endsWith(QStringLiteral(".svg"))) {
        // For non-SVG icons, just render normally
        QIcon icon(iconPath);
        painter->drawPixmap(rect, icon.pixmap(rect.size()));
        return;
    }

    // Determine color based on theme
    QColor fillColor;
    QVariantMap finalAttributes = attributes;

    if (attributes.contains(QStringLiteral("fill"))) {
        fillColor = QColor(attributes.value(QStringLiteral("fill")).toString());
    } else {
        fillColor = (theme == Theme::Auto) ? (isDarkTheme() ? darkColor_ : lightColor_)
                                           : (theme == Theme::Dark ? darkColor_ : lightColor_);
        finalAttributes[QStringLiteral("fill")] = fillColor.name();
    }

    // Modify SVG with color
    QString modifiedSvg = writeSvg(iconPath, QList<int>(), finalAttributes);
    if (!modifiedSvg.isEmpty()) {
        drawSvgIcon(modifiedSvg.toUtf8(), painter, rect);
    } else {
        // Fallback to original
        drawSvgIcon(iconPath, painter, rect);
    }
}

FluentIconBase* ColoredFluentIcon::clone() const {
    // Clone with ownership of the underlying icon if we own it
    if (ownsIcon_ && fluentIcon_) {
        return new ColoredFluentIcon(fluentIcon_->clone(), lightColor_, darkColor_);
    }
    return new ColoredFluentIcon(fluentIcon_, lightColor_, darkColor_);
}

// ============================================================================
// Icon
// ============================================================================

Icon::Icon(const FluentIconBase& fluentIcon) : QIcon(fluentIcon.path()), fluentIcon_(&fluentIcon) {}

Icon::Icon(const FluentIconBase* fluentIcon)
    : QIcon(fluentIcon ? fluentIcon->path() : QString()), fluentIcon_(fluentIcon) {}

// ============================================================================
// Action Class
// ============================================================================

Action::Action(QObject* parent) : QAction(parent) {}

Action::Action(const QString& text, QObject* parent) : QAction(text, parent) {}

Action::Action(const QIcon& icon, const QString& text, QObject* parent)
    : QAction(icon, text, parent) {}

Action::Action(const FluentIconBase& icon, const QString& text, QObject* parent)
    : QAction(text, parent) {
    fluentIcon_ = icon.clone();
    ownsIcon_ = true;
    QAction::setIcon(fluentIcon_->icon());
}

Action::~Action() {
    if (ownsIcon_ && fluentIcon_) {
        delete fluentIcon_;
        fluentIcon_ = nullptr;
    }
}

QIcon Action::icon() const {
    if (fluentIcon_) {
        return fluentIcon_->icon();
    }
    return QAction::icon();
}

void Action::setIcon(const QIcon& icon) {
    QAction::setIcon(icon);
    fluentIcon_ = nullptr;
    ownsIcon_ = false;
}

void Action::setIcon(const FluentIconBase& icon) {
    if (ownsIcon_ && fluentIcon_) {
        delete fluentIcon_;
    }
    fluentIcon_ = icon.clone();
    ownsIcon_ = true;
    QAction::setIcon(fluentIcon_->icon());
}

// ============================================================================
// FluentIcon
// ============================================================================

FluentIcon::FluentIcon(FluentIconEnum icon) : icon_(icon) {}

QString FluentIcon::path(Theme theme) const {
    QString color = getIconColor(theme, false);
    QString iconName = enumToString(icon_);
    return QStringLiteral(":/qfluentwidgets/images/icons/%1_%2.svg").arg(iconName, color);
}

QIcon FluentIcon::icon(Theme theme, const QColor& color) const {
    const QString iconPath = path(theme);
    if (iconPath.endsWith(QStringLiteral(".svg")) && color.isValid()) {
        QVariantMap attributes;
        attributes.insert(QStringLiteral("fill"), color.name());
        attributes.insert(QStringLiteral("stroke"), color.name());
        const QString svg = writeSvg(iconPath, QList<int>(), attributes);
        if (!svg.isEmpty()) {
            return QIcon(new SvgIconEngine(svg.toUtf8()));
        }

        return QIcon(new SvgIconEngine(iconPath));
    }

    return QIcon(iconPath);
}

void FluentIcon::render(QPainter* painter, const QRect& rect, Theme theme,
                        const QVariantMap& attributes) const {
    const bool reverse = attributes.value(QStringLiteral("reverse")).toBool();
    const QString color = getIconColor(theme, reverse);
    const QString iconName = enumToString(icon_);
    QString iconPath =
        QStringLiteral(":/qfluentwidgets/images/icons/%1_%2.svg").arg(iconName, color);

    if (!QFile::exists(iconPath)) {
        iconPath = QStringLiteral(":/qfluentwidgets/images/icons/Info_%1.svg").arg(color);
    }

    if (attributes.contains(QStringLiteral("fill")) ||
        attributes.contains(QStringLiteral("stroke"))) {
        // Apply attributes (like fill color) to SVG before drawing
        QString svg = writeSvg(iconPath, QList<int>(), attributes);
        if (!svg.isEmpty()) {
            drawSvgIcon(svg.toUtf8(), painter, rect);
            return;
        }
    }

    drawSvgIcon(iconPath, painter, rect);
}

FluentIconBase* FluentIcon::clone() const { return new FluentIcon(*this); }

QString FluentIcon::enumToString(FluentIconEnum icon) {
    switch (icon) {
        // Navigation
        case FluentIconEnum::Up:
            return QStringLiteral("Up");
        case FluentIconEnum::Down:
            return QStringLiteral("Down");
        case FluentIconEnum::ChevronDown:
            return QStringLiteral("ChevronDown");
        case FluentIconEnum::ChevronRight:
            return QStringLiteral("ChevronRight");
        case FluentIconEnum::ChevronDownMed:
            return QStringLiteral("ChevronDownMed");
        case FluentIconEnum::ChevronRightMed:
            return QStringLiteral("ChevronRightMed");
        case FluentIconEnum::CareUpSolid:
            return QStringLiteral("CareUpSolid");
        case FluentIconEnum::CareDownSolid:
            return QStringLiteral("CareDownSolid");
        case FluentIconEnum::CareLeftSolid:
            return QStringLiteral("CareLeftSolid");
        case FluentIconEnum::CareRightSolid:
            return QStringLiteral("CareRightSolid");
        case FluentIconEnum::LeftArrow:
            return QStringLiteral("LeftArrow");
        case FluentIconEnum::RightArrow:
            return QStringLiteral("RightArrow");
        case FluentIconEnum::ArrowDown:
            return QStringLiteral("ChevronDown");

        // Actions
        case FluentIconEnum::Add:
            return QStringLiteral("Add");
        case FluentIconEnum::Remove:
            return QStringLiteral("Remove");
        case FluentIconEnum::Delete:
            return QStringLiteral("Delete");
        case FluentIconEnum::Edit:
            return QStringLiteral("Edit");
        case FluentIconEnum::Save:
            return QStringLiteral("Save");
        case FluentIconEnum::Cancel:
            return QStringLiteral("Cancel");
        case FluentIconEnum::Accept:
            return QStringLiteral("Accept");
        case FluentIconEnum::AcceptMedium:
            return QStringLiteral("AcceptMedium");
        case FluentIconEnum::CancelMedium:
            return QStringLiteral("CancelMedium");
        case FluentIconEnum::Copy:
            return QStringLiteral("Copy");
        case FluentIconEnum::Paste:
            return QStringLiteral("Paste");
        case FluentIconEnum::Cut:
            return QStringLiteral("Cut");
        case FluentIconEnum::Search:
            return QStringLiteral("Search");
        case FluentIconEnum::Sync:
            return QStringLiteral("Sync");
        case FluentIconEnum::Settings:
            return QStringLiteral("Setting");
        case FluentIconEnum::Update:
            return QStringLiteral("Update");
        case FluentIconEnum::Return:
            return QStringLiteral("Return");
        case FluentIconEnum::Connect:
            return QStringLiteral("Connect");
        case FluentIconEnum::History:
            return QStringLiteral("History");
        case FluentIconEnum::Filter:
            return QStringLiteral("Filter");
        case FluentIconEnum::Scroll:
            return QStringLiteral("Scroll");

        // Media
        case FluentIconEnum::Play:
            return QStringLiteral("Play");
        case FluentIconEnum::Pause:
            return QStringLiteral("Pause");
        case FluentIconEnum::Volume:
            return QStringLiteral("Volume");
        case FluentIconEnum::Mute:
            return QStringLiteral("Mute");
        case FluentIconEnum::SkipBack:
            return QStringLiteral("SkipBack");
        case FluentIconEnum::SkipForward:
            return QStringLiteral("SkipForward");
        case FluentIconEnum::PlaySolid:
            return QStringLiteral("PlaySolid");
        case FluentIconEnum::SpeedOff:
            return QStringLiteral("SpeedOff");
        case FluentIconEnum::SpeedHigh:
            return QStringLiteral("SpeedHigh");
        case FluentIconEnum::SpeedMedium:
            return QStringLiteral("SpeedMedium");

        // UI
        case FluentIconEnum::Menu:
            return QStringLiteral("Menu");
        case FluentIconEnum::More:
            return QStringLiteral("More");
        case FluentIconEnum::Close:
            return QStringLiteral("Close");
        case FluentIconEnum::BackToWindow:
            return QStringLiteral("BackToWindow");
        case FluentIconEnum::FullScreen:
            return QStringLiteral("FullScreen");
        case FluentIconEnum::Minimize:
            return QStringLiteral("Minimize");
        case FluentIconEnum::Home:
            return QStringLiteral("Home");
        case FluentIconEnum::Info:
            return QStringLiteral("Info");
        case FluentIconEnum::Layout:
            return QStringLiteral("Layout");
        case FluentIconEnum::Alignment:
            return QStringLiteral("Alignment");
        case FluentIconEnum::PageLeft:
            return QStringLiteral("PageLeft");
        case FluentIconEnum::PageRight:
            return QStringLiteral("PageRight");

        // Common
        case FluentIconEnum::Folder:
            return QStringLiteral("Folder");
        case FluentIconEnum::File:
            return QStringLiteral("Document");
        case FluentIconEnum::Download:
            return QStringLiteral("Download");
        case FluentIconEnum::Link:
            return QStringLiteral("Link");
        case FluentIconEnum::Share:
            return QStringLiteral("Share");
        case FluentIconEnum::Print:
            return QStringLiteral("Print");
        case FluentIconEnum::Mail:
            return QStringLiteral("Mail");
        case FluentIconEnum::Message:
            return QStringLiteral("Message");
        case FluentIconEnum::Calendar:
            return QStringLiteral("Calendar");
        case FluentIconEnum::Clock:
            return QStringLiteral("DateTime");
        case FluentIconEnum::DateTime:
            return QStringLiteral("DateTime");
        case FluentIconEnum::Document:
            return QStringLiteral("Document");
        case FluentIconEnum::Language:
            return QStringLiteral("Language");
        case FluentIconEnum::Question:
            return QStringLiteral("Question");
        case FluentIconEnum::Speakers:
            return QStringLiteral("Speakers");
        case FluentIconEnum::FontSize:
            return QStringLiteral("FontSize");

        // Misc
        case FluentIconEnum::Setting:
            return QStringLiteral("Setting");
        case FluentIconEnum::Help:
            return QStringLiteral("Help");
        case FluentIconEnum::Github:
            return QStringLiteral("GitHub");
        case FluentIconEnum::QrCode:
            return QStringLiteral("QRCode");
        case FluentIconEnum::Wifi:
            return QStringLiteral("Wifi");
        case FluentIconEnum::Bluetooth:
            return QStringLiteral("Bluetooth");
        case FluentIconEnum::Airplane:
            return QStringLiteral("Airplane");
        case FluentIconEnum::Train:
            return QStringLiteral("Train");
        case FluentIconEnum::Car:
            return QStringLiteral("Car");
        case FluentIconEnum::Bus:
            return QStringLiteral("Bus");
        case FluentIconEnum::Cafe:
            return QStringLiteral("Cafe");
        case FluentIconEnum::Chat:
            return QStringLiteral("Chat");
        case FluentIconEnum::Code:
            return QStringLiteral("Code");
        case FluentIconEnum::Game:
            return QStringLiteral("Game");
        case FluentIconEnum::Flag:
            return QStringLiteral("Flag");
        case FluentIconEnum::Font:
            return QStringLiteral("Font");
        case FluentIconEnum::Leaf:
            return QStringLiteral("Leaf");
        case FluentIconEnum::Movie:
            return QStringLiteral("Movie");
        case FluentIconEnum::Music:
            return QStringLiteral("Music");
        case FluentIconEnum::Robot:
            return QStringLiteral("Robot");
        case FluentIconEnum::Photo:
            return QStringLiteral("Photo");
        case FluentIconEnum::Phone:
            return QStringLiteral("Phone");
        case FluentIconEnum::Ringer:
            return QStringLiteral("Ringer");
        case FluentIconEnum::Rotate:
            return QStringLiteral("Rotate");
        case FluentIconEnum::Zoom:
            return QStringLiteral("Zoom");
        case FluentIconEnum::ZoomIn:
            return QStringLiteral("ZoomIn");
        case FluentIconEnum::ZoomOut:
            return QStringLiteral("ZoomOut");
        case FluentIconEnum::Album:
            return QStringLiteral("Album");
        case FluentIconEnum::Brush:
            return QStringLiteral("Brush");
        case FluentIconEnum::Broom:
            return QStringLiteral("Broom");
        case FluentIconEnum::Cloud:
            return QStringLiteral("Cloud");
        case FluentIconEnum::Embed:
            return QStringLiteral("Embed");
        case FluentIconEnum::Globe:
            return QStringLiteral("Globe");
        case FluentIconEnum::Heart:
            return QStringLiteral("Heart");
        case FluentIconEnum::Label:
            return QStringLiteral("Label");
        case FluentIconEnum::Media:
            return QStringLiteral("Media");
        case FluentIconEnum::PauseBold:
            return QStringLiteral("PauseBold");
        case FluentIconEnum::PencilInk:
            return QStringLiteral("PencilInk");
        case FluentIconEnum::Tiles:
            return QStringLiteral("Tiles");
        case FluentIconEnum::Unpin:
            return QStringLiteral("Unpin");
        case FluentIconEnum::Video:
            return QStringLiteral("Video");
        case FluentIconEnum::AddTo:
            return QStringLiteral("AddTo");
        case FluentIconEnum::Camera:
            return QStringLiteral("Camera");
        case FluentIconEnum::Market:
            return QStringLiteral("Market");
        case FluentIconEnum::People:
            return QStringLiteral("People");
        case FluentIconEnum::Frigid:
            return QStringLiteral("Frigid");
        case FluentIconEnum::SaveAs:
            return QStringLiteral("SaveAs");
        case FluentIconEnum::FitPage:
            return QStringLiteral("FitPage");
        case FluentIconEnum::BasketBall:
            return QStringLiteral("Basketball");
        case FluentIconEnum::Brightness:
            return QStringLiteral("Brightness");
        case FluentIconEnum::Dictionary:
            return QStringLiteral("Dictionary");
        case FluentIconEnum::Microphone:
            return QStringLiteral("Microphone");
        case FluentIconEnum::Headphone:
            return QStringLiteral("Headphone");
        case FluentIconEnum::Megaphone:
            return QStringLiteral("Megaphone");
        case FluentIconEnum::Projector:
            return QStringLiteral("Projector");
        case FluentIconEnum::Education:
            return QStringLiteral("Education");
        case FluentIconEnum::EraseTool:
            return QStringLiteral("EraseTool");
        case FluentIconEnum::BookShelf:
            return QStringLiteral("BookShelf");
        case FluentIconEnum::Highlight:
            return QStringLiteral("Highlight");
        case FluentIconEnum::PieSingle:
            return QStringLiteral("PieSingle");
        case FluentIconEnum::QuickNote:
            return QStringLiteral("QuickNote");
        case FluentIconEnum::StopWatch:
            return QStringLiteral("StopWatch");
        case FluentIconEnum::ZipFolder:
            return QStringLiteral("ZipFolder");
        case FluentIconEnum::Application:
            return QStringLiteral("Application");
        case FluentIconEnum::Certificate:
            return QStringLiteral("Certificate");
        case FluentIconEnum::ImageExport:
            return QStringLiteral("ImageExport");
        case FluentIconEnum::LibraryFill:
            return QStringLiteral("LibraryFill");
        case FluentIconEnum::MusicFolder:
            return QStringLiteral("MusicFolder");
        case FluentIconEnum::PowerButton:
            return QStringLiteral("PowerButton");
        case FluentIconEnum::EmojiTabSymbols:
            return QStringLiteral("EmojiTabSymbols");
        case FluentIconEnum::ExpressiveInputEntry:
            return QStringLiteral("ExpressiveInputEntry");
        case FluentIconEnum::ClippingTool:
            return QStringLiteral("ClippingTool");
        case FluentIconEnum::ClearSelection:
            return QStringLiteral("ClearSelection");
        case FluentIconEnum::DeveloperTools:
            return QStringLiteral("DeveloperTools");
        case FluentIconEnum::BackgroundColor:
            return QStringLiteral("BackgroundColor");
        case FluentIconEnum::MixVolumes:
            return QStringLiteral("MixVolumes");
        case FluentIconEnum::RemoveFrom:
            return QStringLiteral("RemoveFrom");
        case FluentIconEnum::QuietHours:
            return QStringLiteral("QuietHours");
        case FluentIconEnum::Fingerprint:
            return QStringLiteral("Fingerprint");
        case FluentIconEnum::CheckBox:
            return QStringLiteral("CheckBox");
        case FluentIconEnum::HomeFill:
            return QStringLiteral("HomeFill");
        case FluentIconEnum::SearchMirror:
            return QStringLiteral("SearchMirror");
        case FluentIconEnum::ShoppingCart:
            return QStringLiteral("ShoppingCart");
        case FluentIconEnum::FontIncrease:
            return QStringLiteral("FontIncrease");
        case FluentIconEnum::CommandPrompt:
            return QStringLiteral("CommandPrompt");
        case FluentIconEnum::CloudDownload:
            return QStringLiteral("CloudDownload");
        case FluentIconEnum::DictionaryAdd:
            return QStringLiteral("DictionaryAdd");
        case FluentIconEnum::SaveCopy:
            return QStringLiteral("SaveCopy");
        case FluentIconEnum::SendFill:
            return QStringLiteral("SendFill");
        case FluentIconEnum::Transparent:
            return QStringLiteral("Transparent");
        case FluentIconEnum::Palette:
            return QStringLiteral("Palette");
        case FluentIconEnum::Feedback:
            return QStringLiteral("Feedback");
        case FluentIconEnum::FolderAdd:
            return QStringLiteral("FolderAdd");
        case FluentIconEnum::Asterisk:
            return QStringLiteral("Asterisk");
        case FluentIconEnum::Calories:
            return QStringLiteral("Calories");
        case FluentIconEnum::Constract:
            return QStringLiteral("Constract");
        case FluentIconEnum::IOT:
            return QStringLiteral("IOT");
        case FluentIconEnum::Move:
            return QStringLiteral("Move");
        case FluentIconEnum::Pin:
            return QStringLiteral("Pin");
        case FluentIconEnum::Tag:
            return QStringLiteral("Tag");
        case FluentIconEnum::VPN:
            return QStringLiteral("VPN");
        case FluentIconEnum::View:
            return QStringLiteral("View");

        default:
            return QStringLiteral("Unknown");
    }
}

}  // namespace qfw
