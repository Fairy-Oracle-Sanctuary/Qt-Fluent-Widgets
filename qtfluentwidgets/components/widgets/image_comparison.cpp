#include "components/widgets/image_comparison.h"

#include <QHideEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <cmath>
#include <stdexcept>

namespace qfw {
namespace {
void validateSize(const QSize& size) {
    if (size.width() <= 0 || size.height() <= 0 || size.width() > QWIDGETSIZE_MAX ||
        size.height() > QWIDGETSIZE_MAX)
        throw std::invalid_argument("Image comparison size must be positive and fit QWidget");
}
int scaledDimension(qreal value) {
    // Python round() uses ties-to-even; use the same rounding for size helpers.
    const qreal rounded = qMax(qreal(1), std::nearbyint(value));
    if (rounded > QWIDGETSIZE_MAX)
        throw std::invalid_argument("Scaled image comparison size exceeds QWidget limits");
    return static_cast<int>(rounded);
}
}  // namespace

ImageComparisonSlider::ImageComparisonSlider(QWidget* parent) : QWidget(parent) {
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::SplitHCursor);
    setMinimumSize(40, 40);
}
ImageComparisonSlider::ImageComparisonSlider(const QImage& before, const QImage& after,
                                             QWidget* parent)
    : ImageComparisonSlider(parent) {
    setImages(before, after);
}
ImageComparisonSlider::ImageComparisonSlider(const QPixmap& before, const QPixmap& after,
                                             QWidget* parent)
    : ImageComparisonSlider(before.toImage(), after.toImage(), parent) {}
ImageComparisonSlider::ImageComparisonSlider(const QString& before, const QString& after,
                                             QWidget* parent)
    : ImageComparisonSlider(QImage(before), QImage(after), parent) {}
void ImageComparisonSlider::setImages(const QImage& before, const QImage& after) {
    before_ = before;
    after_ = after;
    updateGeometry();
    update();
}
void ImageComparisonSlider::setImages(const QPixmap& before, const QPixmap& after) {
    setImages(before.toImage(), after.toImage());
}
void ImageComparisonSlider::setImages(const QString& before, const QString& after) {
    setImages(QImage(before), QImage(after));
}
void ImageComparisonSlider::setBeforeImage(const QImage& image) {
    before_ = image;
    updateGeometry();
    update();
}
void ImageComparisonSlider::setBeforeImage(const QPixmap& image) {
    setBeforeImage(image.toImage());
}
void ImageComparisonSlider::setBeforeImage(const QString& path) { setBeforeImage(QImage(path)); }
void ImageComparisonSlider::setAfterImage(const QImage& image) {
    after_ = image;
    updateGeometry();
    update();
}
void ImageComparisonSlider::setAfterImage(const QPixmap& image) { setAfterImage(image.toImage()); }
void ImageComparisonSlider::setAfterImage(const QString& path) { setAfterImage(QImage(path)); }
QImage ImageComparisonSlider::beforeImage() const { return before_; }
QImage ImageComparisonSlider::afterImage() const { return after_; }
qreal ImageComparisonSlider::value() const { return value_; }
void ImageComparisonSlider::setValue(qreal value) {
    if (!std::isfinite(value)) throw std::invalid_argument("Comparison value must be finite");
    value = qBound(qreal(0), value, qreal(100));
    if (value == value_) return;
    value_ = value;
    update();
    emit valueChanged(value);
}
QSize ImageComparisonSlider::sizeHint() const {
    const auto& image = before_.isNull() ? after_ : before_;
    return image.isNull() ? QSize(400, 300) : image.size();
}
void ImageComparisonSlider::scaledToWidth(int width) {
    if (width <= 0 || width > QWIDGETSIZE_MAX)
        throw std::invalid_argument("Comparison width must be positive and fit QWidget");
    const auto size = sizeHint();
    setScaledSize(QSize(width, scaledDimension(qreal(width) * size.height() / size.width())));
}
void ImageComparisonSlider::scaledToHeight(int height) {
    if (height <= 0 || height > QWIDGETSIZE_MAX)
        throw std::invalid_argument("Comparison height must be positive and fit QWidget");
    const auto size = sizeHint();
    setScaledSize(QSize(scaledDimension(qreal(height) * size.width() / size.height()), height));
}
void ImageComparisonSlider::setScaledSize(const QSize& size) {
    validateSize(size);
    setFixedSize(size);
}
void ImageComparisonSlider::moveDivider(const QPointF& position) {
    if (width()) setValue(position.x() / width() * 100);
}
void ImageComparisonSlider::mousePressEvent(QMouseEvent* event) {
    if (isEnabled() && event->button() == Qt::LeftButton) {
        dragging_ = true;
        setFocus(Qt::MouseFocusReason);
        event->accept();
        moveDivider(event->localPos());
    } else {
        QWidget::mousePressEvent(event);
    }
}
void ImageComparisonSlider::mouseMoveEvent(QMouseEvent* event) {
    if (dragging_) {
        event->accept();
        moveDivider(event->localPos());
    } else {
        QWidget::mouseMoveEvent(event);
    }
}
void ImageComparisonSlider::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && dragging_) {
        dragging_ = false;
        event->accept();
        moveDivider(event->localPos());
    } else {
        QWidget::mouseReleaseEvent(event);
    }
}
void ImageComparisonSlider::keyPressEvent(QKeyEvent* event) {
    if (!isEnabled()) return QWidget::keyPressEvent(event);
    qreal value = value_;
    switch (event->key()) {
        case Qt::Key_Left:
            value -= 1;
            break;
        case Qt::Key_Right:
            value += 1;
            break;
        case Qt::Key_Home:
            value = 0;
            break;
        case Qt::Key_End:
            value = 100;
            break;
        default:
            return QWidget::keyPressEvent(event);
    }
    event->accept();
    setValue(value);
}
void ImageComparisonSlider::hideEvent(QHideEvent* event) {
    dragging_ = false;
    QWidget::hideEvent(event);
}
void ImageComparisonSlider::changeEvent(QEvent* event) {
    if (event->type() == QEvent::EnabledChange && !isEnabled()) dragging_ = false;
    QWidget::changeEvent(event);
}
void ImageComparisonSlider::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    const QRectF target(rect());
    const qreal x = width() * value_ / 100;
    auto draw = [&](const QImage& image, const QRectF& clip) {
        if (image.isNull()) return;
        painter.save();
        painter.setClipRect(clip);
        painter.drawImage(target, image, QRectF(image.rect()));
        painter.restore();
    };
    draw(before_, QRectF(0, 0, x, height()));
    draw(after_, QRectF(x, 0, width() - x, height()));
    if (before_.isNull() && after_.isNull()) return;
    painter.setPen(QPen(QColor(255, 255, 255, 210), 2));
    painter.drawLine(QPointF(x, 0), QPointF(x, height()));
    const qreal center = height() / 2.0;
    painter.setPen(QPen(QColor(255, 255, 255, 150), 1));
    painter.setBrush(QColor(255, 255, 255, 185));
    painter.drawRoundedRect(QRectF(x - 12, center - 26, 24, 52), 4, 4);
    painter.setPen(QPen(QColor(0, 0, 0, 210), 1));
    for (int direction : {1, -1}) {
        const qreal y = center - 3 * direction;
        const qreal tip = x + 5 * direction;
        painter.drawLine(QPointF(x - 5, y), QPointF(x + 5, y));
        painter.drawLine(QPointF(tip - 3 * direction, y - 3), QPointF(tip, y));
        painter.drawLine(QPointF(tip - 3 * direction, y + 3), QPointF(tip, y));
    }
}

}  // namespace qfw
