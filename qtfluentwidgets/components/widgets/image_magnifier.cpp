#include "components/widgets/image_magnifier.h"

#include <QCursor>
#include <QFocusEvent>
#include <QHideEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <cmath>
#include <stdexcept>

#include "common/config.h"
#include "common/style_sheet.h"

namespace qfw {

ImageMagnifierWidget::ImageMagnifierWidget(QWidget* parent) : ImageLabel(parent) {
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    connect(&QConfig::instance(), &QConfig::themeColorChanged, this, [this] { update(); });
    connect(&QConfig::instance(), &QConfig::themeChangedFinished, this, [this] { update(); });
}
ImageMagnifierWidget::ImageMagnifierWidget(const QImage& image, QWidget* parent)
    : ImageMagnifierWidget(parent) {
    setImage(image);
}
ImageMagnifierWidget::ImageMagnifierWidget(const QPixmap& image, QWidget* parent)
    : ImageMagnifierWidget(parent) {
    setImage(image);
}
ImageMagnifierWidget::ImageMagnifierWidget(const QString& path, QWidget* parent)
    : ImageMagnifierWidget(parent) {
    setImage(path);
}
qreal ImageMagnifierWidget::magnification() const { return magnification_; }
void ImageMagnifierWidget::setMagnification(qreal factor) {
    if (!std::isfinite(factor) || factor < 1)
        throw std::invalid_argument("Magnification must be finite and at least 1");
    magnification_ = factor;
    update();
}
int ImageMagnifierWidget::radius() const { return radius_; }
void ImageMagnifierWidget::setRadius(int radius) {
    radius_ = qMax(8, radius);
    update();
}
bool ImageMagnifierWidget::isMagnifierEnabled() const { return magnifierEnabled_; }
void ImageMagnifierWidget::setMagnifierEnabled(bool enabled) {
    magnifierEnabled_ = enabled;
    update();
}
QPointF ImageMagnifierWidget::lensPosition() const { return position_; }
void ImageMagnifierWidget::enterEvent(enterEvent_QEnterEvent* event) {
    hovering_ = true;
    // Qt 5's virtual signature is QEvent*, but normal enter events are QEnterEvent.
    const auto* enter = dynamic_cast<QEnterEvent*>(event);
    position_ = enter ? enter->localPos() : QPointF(mapFromGlobal(QCursor::pos()));
    update();
    ImageLabel::enterEvent(event);
}
void ImageMagnifierWidget::mouseMoveEvent(QMouseEvent* event) {
    position_ = event->localPos();
    update();
    ImageLabel::mouseMoveEvent(event);
}
void ImageMagnifierWidget::leaveEvent(QEvent* event) {
    hovering_ = false;
    update();
    ImageLabel::leaveEvent(event);
}
void ImageMagnifierWidget::focusInEvent(QFocusEvent* event) {
    if (!hovering_) position_ = QPointF(width() / 2.0, height() / 2.0);
    update();
    ImageLabel::focusInEvent(event);
}
void ImageMagnifierWidget::focusOutEvent(QFocusEvent* event) {
    update();
    ImageLabel::focusOutEvent(event);
}
void ImageMagnifierWidget::keyPressEvent(QKeyEvent* event) {
    if (!isEnabled() || !magnifierEnabled_) return ImageLabel::keyPressEvent(event);
    QPointF delta;
    switch (event->key()) {
        case Qt::Key_Left:
            delta = QPointF(-5, 0);
            break;
        case Qt::Key_Right:
            delta = QPointF(5, 0);
            break;
        case Qt::Key_Up:
            delta = QPointF(0, -5);
            break;
        case Qt::Key_Down:
            delta = QPointF(0, 5);
            break;
        default:
            return ImageLabel::keyPressEvent(event);
    }
    position_ = QPointF(qBound(qreal(0), position_.x() + delta.x(), qreal(width())),
                        qBound(qreal(0), position_.y() + delta.y(), qreal(height())));
    update();
    event->accept();
}
void ImageMagnifierWidget::hideEvent(QHideEvent* event) {
    hovering_ = false;
    ImageLabel::hideEvent(event);
}
void ImageMagnifierWidget::changeEvent(QEvent* event) {
    if (event->type() == QEvent::EnabledChange && !isEnabled()) hovering_ = false;
    ImageLabel::changeEvent(event);
    update();
}
void ImageMagnifierWidget::paintEvent(QPaintEvent* event) {
    ImageLabel::paintEvent(event);
    if (isNull() || !magnifierEnabled_ || !isEnabled() || !(hovering_ || hasFocus()) ||
        width() <= 0 || height() <= 0)
        return;
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    const qreal x = position_.x(), y = position_.y();
    const qreal diameter = qreal(radius_) * 2;
    const QRectF lens(x - radius_, y - radius_, diameter, diameter);
    QPainterPath path;
    path.addEllipse(lens);
    painter.save();
    painter.setClipPath(path);
    // Sample the original image, not ImageLabel's downscaled preview cache.
    const auto& image = sourceImage();
    painter.drawImage(QRectF(x * (1 - magnification_), y * (1 - magnification_),
                             width() * magnification_, height() * magnification_),
                      image, QRectF(image.rect()));
    painter.restore();
    const auto color = themeColor();
    painter.setPen(QPen(color, 3));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(lens.adjusted(1.5, 1.5, -1.5, -1.5));
    painter.setPen(QPen(color, 0.5));
    painter.drawLine(QPointF(x - 12, y), QPointF(x + 12, y));
    painter.drawLine(QPointF(x, y - 12), QPointF(x, y + 12));
}

}  // namespace qfw
