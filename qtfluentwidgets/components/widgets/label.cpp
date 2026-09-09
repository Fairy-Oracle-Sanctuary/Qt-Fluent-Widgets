#include "components/widgets/label.h"

#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QImageReader>
#include <QKeySequence>
#include <QMouseEvent>
#include <QMovie>
#include <QPainter>
#include <QPainterPath>

#include "common/config.h"
#include "common/font.h"
#include "common/style_sheet.h"
#include "components/widgets/menu.h"

namespace qfw {

// ==========================================================================
// PixmapLabel
// ==========================================================================
PixmapLabel::PixmapLabel(QWidget* parent) : QLabel(parent) {}

void PixmapLabel::setPixmap(const QPixmap& pixmap) {
    pixmap_ = pixmap;
    setFixedSize(pixmap.size());
    update();
}

QPixmap PixmapLabel::pixmap() const { return pixmap_; }

void PixmapLabel::paintEvent(QPaintEvent* e) {
    if (pixmap_.isNull()) {
        QLabel::paintEvent(e);
        return;
    }

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter.setPen(Qt::NoPen);
    painter.drawPixmap(rect(), pixmap_);
}

// ==========================================================================
// FluentLabelBase
// ==========================================================================
FluentLabelBase::FluentLabelBase(QWidget* parent) : QLabel(parent) { init(); }

FluentLabelBase::FluentLabelBase(const QString& text, QWidget* parent) : QLabel(parent) {
    init();
    setText(text);
}

void FluentLabelBase::init() {
    setProperty("qssClass", "FluentLabelBase");
    qfw::setStyleSheet(this, qfw::FluentStyleSheet::Label);
    setTextColor();

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this,
            &FluentLabelBase::onContextMenuRequested);

    connect(&QConfig::instance(), &QConfig::themeChanged, this,
            [this](qfw::Theme) { setTextColor(lightColor_, darkColor_); });
}

void FluentLabelBase::setTextColor(const QColor& light, const QColor& dark) {
    lightColor_ = QColor(light);
    darkColor_ = QColor(dark);

    // This stylesheet belongs to the label itself, so use a stable Qt base-class
    // selector. Derived labels change qssClass after FluentLabelBase::init();
    // binding the color rule to that temporary value makes the initial rule stop
    // matching until the first theme change.
    const QString selector = QStringLiteral("QLabel");
    qfw::setCustomStyleSheet(
        this,
        QStringLiteral("%1{color:%2}").arg(selector, lightColor_.name(QColor::NameFormat::HexArgb)),
        QStringLiteral("%1{color:%2}").arg(selector, darkColor_.name(QColor::NameFormat::HexArgb)));
}

QColor FluentLabelBase::lightColor() const { return lightColor_; }
void FluentLabelBase::setLightColor(const QColor& c) { setTextColor(c, darkColor_); }

QColor FluentLabelBase::darkColor() const { return darkColor_; }
void FluentLabelBase::setDarkColor(const QColor& c) { setTextColor(lightColor_, c); }

int FluentLabelBase::pixelFontSize() const { return font().pixelSize(); }

void FluentLabelBase::setPixelFontSize(int size) {
    QFont f = font();
    f.setPixelSize(size);
    setFont(f);
}

bool FluentLabelBase::strikeOut() const { return font().strikeOut(); }

void FluentLabelBase::setStrikeOut(bool v) {
    QFont f = font();
    f.setStrikeOut(v);
    setFont(f);
}

bool FluentLabelBase::underline() const { return font().underline(); }

void FluentLabelBase::setUnderline(bool v) {
    QFont f = font();
    f.setUnderline(v);
    setFont(f);
}

void FluentLabelBase::onContextMenuRequested(const QPoint& pos) {
    auto* menu = new LabelContextMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->execAt(mapToGlobal(pos), true, MenuAnimationType::DropDown);
}

// ==========================================================================
// Text label variants
// ==========================================================================
CaptionLabel::CaptionLabel(QWidget* parent) : FluentLabelBase(parent) {
    setProperty("qssClass", "CaptionLabel");
    setFont(getFont());
}

CaptionLabel::CaptionLabel(const QString& text, QWidget* parent) : FluentLabelBase(text, parent) {
    setProperty("qssClass", "CaptionLabel");
    setFont(getFont());
}

QFont CaptionLabel::getFont() const { return qfw::getFont(12); }

BodyLabel::BodyLabel(QWidget* parent) : FluentLabelBase(parent) {
    setProperty("qssClass", "BodyLabel");
    setFont(getFont());
}

BodyLabel::BodyLabel(const QString& text, QWidget* parent) : FluentLabelBase(text, parent) {
    setProperty("qssClass", "BodyLabel");
    setFont(getFont());
}

QFont BodyLabel::getFont() const { return qfw::getFont(14); }

StrongBodyLabel::StrongBodyLabel(QWidget* parent) : FluentLabelBase(parent) {
    setProperty("qssClass", "StrongBodyLabel");
    setFont(getFont());
}

StrongBodyLabel::StrongBodyLabel(const QString& text, QWidget* parent)
    : FluentLabelBase(text, parent) {
    setProperty("qssClass", "StrongBodyLabel");
    setFont(getFont());
}

QFont StrongBodyLabel::getFont() const { return qfw::getFont(14, QFont::DemiBold); }

SubtitleLabel::SubtitleLabel(QWidget* parent) : FluentLabelBase(parent) {
    setProperty("qssClass", "SubtitleLabel");
    setFont(getFont());
}

SubtitleLabel::SubtitleLabel(const QString& text, QWidget* parent) : FluentLabelBase(text, parent) {
    setProperty("qssClass", "SubtitleLabel");
    setFont(getFont());
}

QFont SubtitleLabel::getFont() const { return qfw::getFont(20, QFont::DemiBold); }

TitleLabel::TitleLabel(QWidget* parent) : FluentLabelBase(parent) {
    setProperty("qssClass", "TitleLabel");
    setFont(getFont());
}

TitleLabel::TitleLabel(const QString& text, QWidget* parent) : FluentLabelBase(text, parent) {
    setProperty("qssClass", "TitleLabel");
    setFont(getFont());
}

QFont TitleLabel::getFont() const { return qfw::getFont(28, QFont::DemiBold); }

LargeTitleLabel::LargeTitleLabel(QWidget* parent) : FluentLabelBase(parent) {
    setProperty("qssClass", "LargeTitleLabel");
    setFont(getFont());
}

LargeTitleLabel::LargeTitleLabel(const QString& text, QWidget* parent)
    : FluentLabelBase(text, parent) {
    setProperty("qssClass", "LargeTitleLabel");
    setFont(getFont());
}

QFont LargeTitleLabel::getFont() const { return qfw::getFont(40, QFont::DemiBold); }

DisplayLabel::DisplayLabel(QWidget* parent) : FluentLabelBase(parent) {
    setProperty("qssClass", "DisplayLabel");
    setFont(getFont());
}

DisplayLabel::DisplayLabel(const QString& text, QWidget* parent) : FluentLabelBase(text, parent) {
    setProperty("qssClass", "DisplayLabel");
    setFont(getFont());
}

QFont DisplayLabel::getFont() const { return qfw::getFont(68, QFont::DemiBold); }

// ==========================================================================
// ImageLabel
// ==========================================================================
ImageLabel::ImageLabel(QWidget* parent) : QLabel(parent) {
    image_ = QImage();
    setBorderRadius(0, 0, 0, 0);
    postInit();
}

ImageLabel::ImageLabel(const QString& imagePath, QWidget* parent) : ImageLabel(parent) {
    setImage(imagePath);
}

ImageLabel::ImageLabel(const QImage& image, QWidget* parent) : ImageLabel(parent) {
    setImage(image);
}

ImageLabel::ImageLabel(const QPixmap& pixmap, QWidget* parent) : ImageLabel(parent) {
    setImage(pixmap);
}

void ImageLabel::postInit() {}

void ImageLabel::onFrameChanged(int index) {
    Q_UNUSED(index);
    if (!movie_) {
        return;
    }
    image_ = movie_->currentImage();
    invalidateScaledImage();
    update();
}

void ImageLabel::setBorderRadius(int topLeft, int topRight, int bottomLeft, int bottomRight) {
    topLeftRadius_ = topLeft;
    topRightRadius_ = topRight;
    bottomLeftRadius_ = bottomLeft;
    bottomRightRadius_ = bottomRight;
    clipPathSize_ = QSize();
    update();
}

void ImageLabel::invalidateScaledImage() {
    scaledImage_ = QImage();
    scaledImagePixelSize_ = QSize();
    scaledImageDevicePixelRatio_ = 0;
    scaledImageSourceKey_ = -1;
}

void ImageLabel::rebuildClipPath() {
    clipPath_ = QPainterPath();
    clipPathSize_ = size();

    const int w = width();
    const int h = height();

    clipPath_.moveTo(topLeftRadius_, 0);
    clipPath_.lineTo(w - topRightRadius_, 0);

    int d = topRightRadius_ * 2;
    clipPath_.arcTo(w - d, 0, d, d, 90, -90);
    clipPath_.lineTo(w, h - bottomRightRadius_);

    d = bottomRightRadius_ * 2;
    clipPath_.arcTo(w - d, h - d, d, d, 0, -90);
    clipPath_.lineTo(bottomLeftRadius_, h);

    d = bottomLeftRadius_ * 2;
    clipPath_.arcTo(0, h - d, d, d, -90, -90);
    clipPath_.lineTo(0, topLeftRadius_);

    d = topLeftRadius_ * 2;
    clipPath_.arcTo(0, 0, d, d, -180, -90);
}

void ImageLabel::updateScaledImage() {
    const qreal dpr = devicePixelRatioF();
    const QSize pixelSize = size() * dpr;
    const qint64 sourceKey = image_.cacheKey();

    if (scaledImageSourceKey_ == sourceKey && scaledImagePixelSize_ == pixelSize &&
        scaledImageDevicePixelRatio_ == dpr) {
        return;
    }

    scaledImageSourceKey_ = sourceKey;
    scaledImagePixelSize_ = pixelSize;
    scaledImageDevicePixelRatio_ = dpr;
    scaledImage_ = pixelSize.isEmpty()
                       ? QImage()
                       : image_.scaled(pixelSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

void ImageLabel::setImage(const QString& imagePath) {
    image_ = QImage();
    invalidateScaledImage();
    movie_.clear();

    QImageReader reader(imagePath);
    if (reader.supportsAnimation()) {
        setMovie(new QMovie(imagePath));
    } else {
        image_ = reader.read();
    }

    if (!image_.isNull()) {
        setFixedSize(image_.size());
    }
    update();
}

void ImageLabel::setImage(const QImage& image) {
    movie_.clear();
    image_ = image;
    invalidateScaledImage();
    if (!image_.isNull()) {
        setFixedSize(image_.size());
    }
    update();
}

void ImageLabel::setImage(const QPixmap& pixmap) { setImage(pixmap.toImage()); }

void ImageLabel::scaledToWidth(int width) {
    if (isNull()) {
        return;
    }

    const int h = static_cast<int>(static_cast<double>(width) / image_.width() * image_.height());
    setFixedSize(width, h);

    if (movie_) {
        movie_->setScaledSize(QSize(width, h));
    }
}

void ImageLabel::scaledToHeight(int height) {
    if (isNull()) {
        return;
    }

    const int w = static_cast<int>(static_cast<double>(height) / image_.height() * image_.width());
    setFixedSize(w, height);

    if (movie_) {
        movie_->setScaledSize(QSize(w, height));
    }
}

void ImageLabel::setScaledSize(const QSize& size) {
    if (isNull()) {
        return;
    }

    setFixedSize(size);

    if (movie_) {
        movie_->setScaledSize(size);
    }
}

bool ImageLabel::isNull() const { return image_.isNull(); }

QPixmap ImageLabel::pixmap() const { return QPixmap::fromImage(image_); }

int ImageLabel::topLeftRadius() const { return topLeftRadius_; }
void ImageLabel::setTopLeftRadius(int radius) {
    setBorderRadius(radius, topRightRadius_, bottomLeftRadius_, bottomRightRadius_);
}

int ImageLabel::topRightRadius() const { return topRightRadius_; }
void ImageLabel::setTopRightRadius(int radius) {
    setBorderRadius(topLeftRadius_, radius, bottomLeftRadius_, bottomRightRadius_);
}

int ImageLabel::bottomLeftRadius() const { return bottomLeftRadius_; }
void ImageLabel::setBottomLeftRadius(int radius) {
    setBorderRadius(topLeftRadius_, topRightRadius_, radius, bottomRightRadius_);
}

int ImageLabel::bottomRightRadius() const { return bottomRightRadius_; }
void ImageLabel::setBottomRightRadius(int radius) {
    setBorderRadius(topLeftRadius_, topRightRadius_, bottomLeftRadius_, radius);
}

void ImageLabel::mouseReleaseEvent(QMouseEvent* e) {
    QLabel::mouseReleaseEvent(e);
    emit clicked();
}

void ImageLabel::setMovie(QMovie* movie) {
    if (!movie) {
        movie_.clear();
        return;
    }

    movie_ = movie;
    movie_->setParent(this);
    movie_->start();
    image_ = movie_->currentImage();
    invalidateScaledImage();
    connect(movie_, &QMovie::frameChanged, this, &ImageLabel::onFrameChanged);
}

void ImageLabel::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);
    if (isNull()) {
        return;
    }

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    if (clipPathSize_ != size()) {
        rebuildClipPath();
    }
    updateScaledImage();

    painter.setPen(Qt::NoPen);
    painter.setClipPath(clipPath_);
    painter.drawImage(rect(), scaledImage_);
}

// ==========================================================================
// AvatarWidget
// ==========================================================================
AvatarWidget::AvatarWidget(QWidget* parent) : ImageLabel(parent) {}

AvatarWidget::AvatarWidget(const QString& imagePath, QWidget* parent) : AvatarWidget(parent) {
    setImage(imagePath);
}

AvatarWidget::AvatarWidget(const QImage& image, QWidget* parent) : AvatarWidget(parent) {
    setImage(image);
}

AvatarWidget::AvatarWidget(const QPixmap& pixmap, QWidget* parent) : AvatarWidget(parent) {
    setImage(pixmap);
}

void AvatarWidget::postInit() {
    setRadius(48);
    lightBackgroundColor_ = QColor(0, 0, 0, 50);
    darkBackgroundColor_ = QColor(255, 255, 255, 50);
}

int AvatarWidget::radius() const { return radius_; }

void AvatarWidget::setRadius(int radius) {
    radius_ = radius;
    qfw::setFont(this, radius);
    setFixedSize(2 * radius, 2 * radius);
    clipPathSize_ = QSize();
    invalidateImageCache();
    update();
}

void AvatarWidget::setBackgroundColor(const QColor& light, const QColor& dark) {
    lightBackgroundColor_ = QColor(light);
    darkBackgroundColor_ = QColor(dark);
    update();
}

void AvatarWidget::setImage(const QString& imagePath) {
    ImageLabel::setImage(imagePath);
    setRadius(radius_);
}

void AvatarWidget::setImage(const QImage& image) {
    ImageLabel::setImage(image);
    setRadius(radius_);
}

void AvatarWidget::setImage(const QPixmap& pixmap) {
    ImageLabel::setImage(pixmap);
    setRadius(radius_);
}

void AvatarWidget::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    if (!isNull()) {
        drawImageAvatar(painter);
    } else {
        drawTextAvatar(painter);
    }
}

void AvatarWidget::drawImageAvatar(QPainter& painter) {
    if (clipPathSize_ != size()) {
        rebuildClipPath();
    }
    updateImageCache();

    painter.setPen(Qt::NoPen);
    painter.setClipPath(clipPath_);
    painter.drawImage(rect(), imageCache_);
}

void AvatarWidget::invalidateImageCache() {
    imageCache_ = QImage();
    imageCachePixelSize_ = QSize();
    imageCacheDevicePixelRatio_ = 0;
    imageCacheSourceKey_ = -1;
}

void AvatarWidget::rebuildClipPath() {
    clipPath_ = QPainterPath();
    clipPath_.addEllipse(QRectF(rect()));
    clipPathSize_ = size();
}

void AvatarWidget::updateImageCache() {
    const QImage& source = sourceImage();
    const qreal dpr = devicePixelRatioF();
    const QSize pixelSize = size() * dpr;
    const qint64 sourceKey = source.cacheKey();

    if (imageCacheSourceKey_ == sourceKey && imageCachePixelSize_ == pixelSize &&
        imageCacheDevicePixelRatio_ == dpr) {
        return;
    }

    imageCacheSourceKey_ = sourceKey;
    imageCachePixelSize_ = pixelSize;
    imageCacheDevicePixelRatio_ = dpr;

    if (source.isNull() || pixelSize.isEmpty()) {
        imageCache_ = QImage();
        return;
    }

    const QImage scaled = source.scaled(pixelSize, Qt::KeepAspectRatioByExpanding,
                                        Qt::SmoothTransformation);
    const int d = static_cast<int>(radius_ * 2 * dpr);
    const int x = (scaled.width() - d) / 2;
    const int y = (scaled.height() - d) / 2;
    imageCache_ = scaled.copy(x, y, d, d);
}

void AvatarWidget::drawTextAvatar(QPainter& painter) {
    if (text().isEmpty()) {
        return;
    }

    painter.setBrush(qfw::isDarkTheme() ? darkBackgroundColor_ : lightBackgroundColor_);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(QRectF(rect()));

    painter.setFont(font());
    painter.setPen(qfw::isDarkTheme() ? Qt::white : Qt::black);
    painter.drawText(rect(), Qt::AlignCenter, text().left(1).toUpper());
}

// ==========================================================================
// HyperlinkLabel
// ==========================================================================
HyperlinkLabel::HyperlinkLabel(QWidget* parent) : QPushButton(parent) {
    setProperty("qssClass", "HyperlinkLabel");
    qfw::setFont(this, 14);
    setUnderlineVisible(false);

    qfw::setStyleSheet(this, qfw::FluentStyleSheet::Label);

    setFlat(true);
    setCheckable(false);
    setDefault(false);
    setAutoDefault(false);

    setCursor(Qt::PointingHandCursor);
    connect(this, &QPushButton::clicked, this, &HyperlinkLabel::onClicked);
}

HyperlinkLabel::HyperlinkLabel(const QString& text, QWidget* parent) : HyperlinkLabel(parent) {
    setText(text);
}

HyperlinkLabel::HyperlinkLabel(const QUrl& url, const QString& text, QWidget* parent)
    : HyperlinkLabel(parent) {
    setText(text);
    url_ = url;
}

QUrl HyperlinkLabel::url() const { return url_; }

void HyperlinkLabel::setUrl(const QUrl& url) { url_ = url; }

void HyperlinkLabel::setUrl(const QString& url) { url_ = QUrl(url); }

bool HyperlinkLabel::underlineVisible() const { return underlineVisible_; }

void HyperlinkLabel::setUnderlineVisible(bool visible) {
    underlineVisible_ = visible;
    setProperty("underline", visible);
    style()->unpolish(this);
    style()->polish(this);
    update();
}

void HyperlinkLabel::onClicked() {
    if (url_.isValid()) {
        QDesktopServices::openUrl(url_);
    }
}

}  // namespace qfw
