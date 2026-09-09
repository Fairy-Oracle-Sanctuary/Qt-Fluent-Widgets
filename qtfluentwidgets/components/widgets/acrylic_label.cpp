#include "components/widgets/acrylic_label.h"

#include <QApplication>
#include <QGraphicsBlurEffect>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QPainter>
#include <QScreen>

#include "common/screen.h"

namespace qfw {

namespace {

QPixmap blurPixmap(const QPixmap& src, int radius, const QSize& maxSize = QSize()) {
    if (src.isNull() || radius <= 0) {
        return src;
    }

    QPixmap input = src;
    if (maxSize.isValid() &&
        (input.width() > maxSize.width() || input.height() > maxSize.height())) {
        input = input.scaled(maxSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    auto* effect = new QGraphicsBlurEffect;
    effect->setBlurRadius(radius);

    QGraphicsScene scene;
    QGraphicsPixmapItem item;
    item.setPixmap(input);
    item.setGraphicsEffect(effect);
    scene.addItem(&item);

    const QRectF bounds = item.boundingRect();
    QImage result(bounds.size().toSize(), QImage::Format_ARGB32_Premultiplied);
    result.fill(Qt::transparent);

    QPainter painter(&result);
    scene.render(&painter, QRectF(), bounds);

    delete effect;
    return QPixmap::fromImage(result);
}

}  // namespace

AcrylicTextureLabel::AcrylicTextureLabel(const QColor& tintColor, const QColor& luminosityColor,
                                         qreal noiseOpacity, QWidget* parent)
    : QLabel(parent),
      tintColor_(tintColor),
      luminosityColor_(luminosityColor),
      noiseOpacity_(noiseOpacity),
      noiseImage_(QStringLiteral(":/qfluentwidgets/images/acrylic/noise.png")) {
    setAttribute(Qt::WA_TranslucentBackground);
    rebuildTexture();
}

void AcrylicTextureLabel::setTintColor(const QColor& color) {
    if (tintColor_ == color) {
        return;
    }

    tintColor_ = color;
    rebuildTexture();
    update();
}

void AcrylicTextureLabel::rebuildTexture() {
    textureImage_ = QImage(64, 64, QImage::Format_ARGB32_Premultiplied);
    textureImage_.fill(luminosityColor_);

    QPainter painter(&textureImage_);
    painter.fillRect(textureImage_.rect(), tintColor_);
    painter.setOpacity(noiseOpacity_);
    painter.drawImage(textureImage_.rect(), noiseImage_);
}

void AcrylicTextureLabel::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);

    QPainter painter(this);
    painter.fillRect(rect(), QBrush(textureImage_));
}

AcrylicLabel::AcrylicLabel(int blurRadius, const QColor& tintColor, const QColor& luminosityColor,
                           const QSize& maxBlurSize, QWidget* parent)
    : QLabel(parent), blurRadius_(blurRadius), maxBlurSize_(maxBlurSize) {
    acrylicTextureLabel_ = new AcrylicTextureLabel(tintColor, luminosityColor, 0.03, this);
}

void AcrylicLabel::setImage(const QString& imagePath) {
    imagePath_ = imagePath;
    updateBlur();
}

void AcrylicLabel::setTintColor(const QColor& color) {
    if (acrylicTextureLabel_) {
        acrylicTextureLabel_->setTintColor(color);
    }
}

void AcrylicLabel::setBlurRadius(int radius) {
    if (radius == blurRadius_) {
        return;
    }
    blurRadius_ = radius;
    updateBlur();
}

void AcrylicLabel::resizeEvent(QResizeEvent* e) {
    QLabel::resizeEvent(e);

    if (acrylicTextureLabel_) {
        acrylicTextureLabel_->resize(size());
    }

    if (!blurPixmap_.isNull() && blurPixmap_.size() != size()) {
        setPixmap(
            blurPixmap_.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    }
}

void AcrylicLabel::updateBlur() {
    if (imagePath_.isEmpty()) {
        return;
    }

    QPixmap src(imagePath_);
    if (src.isNull()) {
        return;
    }

    blurPixmap_ = blurPixmap(src, blurRadius_, maxBlurSize_);
    setPixmap(blurPixmap_);
    adjustSize();
}

AcrylicBrush::AcrylicBrush(QWidget* device, int blurRadius, const QColor& tintColor,
                           const QColor& luminosityColor, qreal noiseOpacity)
    : device_(device),
      blurRadius_(blurRadius),
      tintColor_(tintColor),
      luminosityColor_(luminosityColor),
      noiseOpacity_(noiseOpacity),
      noiseImage_(QStringLiteral(":/qfluentwidgets/images/acrylic/noise.png")) {
    rebuildTexture();
}

void AcrylicBrush::setBlurRadius(int radius) {
    if (radius == blurRadius_) {
        return;
    }

    blurRadius_ = radius;
    setImage(originalImage_);
}

void AcrylicBrush::setTintColor(const QColor& color) {
    if (tintColor_ == color) {
        return;
    }

    tintColor_ = color;
    rebuildTexture();
    if (device_) {
        device_->update();
    }
}

void AcrylicBrush::setLuminosityColor(const QColor& color) {
    if (luminosityColor_ == color) {
        return;
    }

    luminosityColor_ = color;
    rebuildTexture();
    if (device_) {
        device_->update();
    }
}

void AcrylicBrush::grabImage(const QRect& rect) {
    QScreen* screen = getCurrentScreen();
    if (!screen) {
        const auto screens = QApplication::screens();
        if (!screens.isEmpty()) {
            screen = screens.first();
        }
    }

    if (!screen) {
        return;
    }

    int x = rect.x();
    int y = rect.y();
    int w = rect.width();
    int h = rect.height();

    x -= screen->geometry().x();
    y -= screen->geometry().y();

    setImage(screen->grabWindow(0, x, y, w, h));
}

void AcrylicBrush::setImage(const QPixmap& image) {
    originalImage_ = image;
    image_ = image.isNull() ? QPixmap() : blurPixmap(image, blurRadius_);
    scaledImage_ = QPixmap();
    scaledImageSize_ = QSize();

    if (device_) {
        device_->update();
    }
}

void AcrylicBrush::setClipPath(const QPainterPath& path) {
    if (clipPath_ == path) {
        return;
    }

    clipPath_ = path;
    if (device_) {
        device_->update();
    }
}

void AcrylicBrush::rebuildTexture() {
    textureImageCache_ = QImage(64, 64, QImage::Format_ARGB32_Premultiplied);
    textureImageCache_.fill(luminosityColor_);

    QPainter painter(&textureImageCache_);
    painter.fillRect(textureImageCache_.rect(), tintColor_);

    painter.setOpacity(noiseOpacity_);
    painter.drawImage(textureImageCache_.rect(), noiseImage_);
}

void AcrylicBrush::updateScaledImage() {
    const QSize targetSize = device_ ? device_->size() : QSize();
    if (targetSize == scaledImageSize_) {
        return;
    }

    scaledImageSize_ = targetSize;
    if (image_.isNull() || targetSize.isEmpty()) {
        scaledImage_ = QPixmap();
        return;
    }

    scaledImage_ =
        image_.scaled(targetSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
}

QImage AcrylicBrush::textureImage() const { return textureImageCache_; }

void AcrylicBrush::paint(QPainter* painter) {
    if (!device_ || !painter) {
        return;
    }

    painter->setRenderHints(QPainter::Antialiasing);

    if (!clipPath_.isEmpty()) {
        painter->save();
        painter->setClipPath(clipPath_);
    }

    updateScaledImage();
    if (!scaledImage_.isNull()) {
        painter->drawPixmap(0, 0, scaledImage_);
    }

    painter->fillRect(device_->rect(), QBrush(textureImageCache_));

    if (!clipPath_.isEmpty()) {
        painter->restore();
    }
}

}  // namespace qfw
