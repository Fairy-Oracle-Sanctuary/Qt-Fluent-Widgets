#include "components/widgets/watermark.h"

#include <QEvent>
#include <QFontMetricsF>
#include <QPainter>
#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "common/config.h"
#include "common/font.h"

namespace qfw {
Watermark::Watermark(const QString& text, QWidget* target)
    : QWidget(nullptr), text_(text), syncTimer_(this) {
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_NoSystemBackground);
    setAutoFillBackground(false);
    setFocusPolicy(Qt::NoFocus);
    qfw::setFont(this, 18);
    syncTimer_.setSingleShot(true);
    connect(&syncTimer_, &QTimer::timeout, this, &Watermark::syncTarget);
    connect(&QConfig::instance(), &QConfig::themeChanged, this, &Watermark::invalidateTile);
    if (target) {
        setTargetWidget(target);
    }
}
QWidget* Watermark::targetWidget() const { return target_; }
void Watermark::setTargetWidget(QWidget* target) {
    if (target == this || (target && isAncestorOf(target))) {
        throw std::invalid_argument("Invalid watermark target");
    }
    if (target == target_) {
        return;
    }
    const bool visible = !target_ || !isHidden();
    if (target_) {
        target_->removeEventFilter(this);
    }
    for (const auto& sibling : siblings_) {
        if (sibling) {
            sibling->removeEventFilter(this);
        }
    }
    siblings_.clear();
    syncTimer_.stop();
    hide();
    target_ = target;
    setParent(target);
    if (target) {
        target->installEventFilter(this);
        syncTarget();
        setVisible(visible);
    }
}
void Watermark::syncTarget() {
    if (!target_) {
        return;
    }
    setGeometry(target_->rect());
    for (QWidget* child : target_->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly)) {
        if (child != this) {
            child->installEventFilter(this);
            if (!siblings_.contains(child)) {
                siblings_.append(child);
            }
        }
    }
    raise();
}
bool Watermark::eventFilter(QObject* watched, QEvent* event) {
    const auto kind = event->type();
    if (watched == target_) {
        if (kind == QEvent::Resize || kind == QEvent::Show) {
            syncTarget();
        } else if (kind == QEvent::ChildAdded || kind == QEvent::ChildPolished ||
                   kind == QEvent::ChildRemoved) {
            syncTimer_.start(0);
        }
    } else if (auto* widget = qobject_cast<QWidget*>(watched)) {
        if (kind == QEvent::ParentChange && widget->parentWidget() != target_) {
            widget->removeEventFilter(this);
            siblings_.removeAll(widget);
        } else if (widget->parentWidget() == target_ &&
                   (kind == QEvent::ZOrderChange || kind == QEvent::Show)) {
            syncTimer_.start(0);
        }
    }
    return QWidget::eventFilter(watched, event);
}
QString Watermark::text() const { return text_; }
void Watermark::setText(const QString& text) {
    if (text_ != text) {
        text_ = text;
        invalidateTile();
    }
}
qreal Watermark::angle() const { return angle_; }
void Watermark::setAngle(qreal angle) {
    if (!std::isfinite(angle)) {
        throw std::invalid_argument("Watermark angle must be finite");
    }
    angle_ = std::fmod(std::fmod(angle + 180, 360) + 360, 360) - 180;
    invalidateTile();
}
qreal Watermark::opacity() const { return opacity_; }
void Watermark::setOpacity(qreal opacity) {
    if (!std::isfinite(opacity) || opacity < 0 || opacity > 1) {
        throw std::invalid_argument("Invalid watermark opacity");
    }
    opacity_ = opacity;
    update();
}
QSizeF Watermark::spacing() const { return spacing_; }
void Watermark::setSpacing(qreal h, qreal v) {
    if (!std::isfinite(h) || !std::isfinite(v) || h < 0 || v < 0) {
        throw std::invalid_argument("Invalid watermark spacing");
    }
    spacing_ = QSizeF(h, v);
    invalidateTile();
}
void Watermark::setSpacing(qreal value) { setSpacing(value, value); }
QColor Watermark::color() const { return isDarkTheme() ? dark_ : light_; }
void Watermark::setColor(const QColor& light, const QColor& dark) {
    if (!light.isValid()) {
        throw std::invalid_argument("Invalid watermark color");
    }
    light_ = light;
    dark_ = dark.isValid() ? dark : light;
    invalidateTile();
}
QSize Watermark::sizeHint() const { return QSize(640, 240); }
void Watermark::invalidateTile() {
    tile_ = QPixmap();
    update();
}
void Watermark::changeEvent(QEvent* event) {
    QWidget::changeEvent(event);
    if (event->type() == QEvent::FontChange || event->type() == QEvent::PaletteChange) {
        invalidateTile();
    }
}
void Watermark::createTile() {
    const QFontMetricsF metrics(font());
    const auto lines = text_.split('\n');
    qreal width = 0;
    for (const auto& line : lines) {
        width =
            std::max({width, metrics.horizontalAdvance(line), metrics.boundingRect(line).width()});
    }
    width += 4;
    const qreal height = metrics.height() * lines.size() + 4;
    const qreal radians = angle_ * 3.14159265358979323846 / 180;
    const qreal c = std::abs(std::cos(radians)), s = std::abs(std::sin(radians));
    const int tw = std::max(1, int(std::ceil(width * c + height * s + spacing_.width())));
    const int th = std::max(1, int(std::ceil(width * s + height * c + spacing_.height())));
    tileDpr_ = devicePixelRatioF();
    tile_ = QPixmap(int(std::ceil(tw * tileDpr_)), int(std::ceil(th * tileDpr_)));
    tile_.setDevicePixelRatio(tileDpr_);
    tile_.fill(Qt::transparent);
    if (!tile_.isNull()) {
        QPainter painter(&tile_);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
        painter.setFont(font());
        painter.setPen(color());
        painter.translate(tw / 2.0, th / 2.0);
        painter.rotate(angle_);
        painter.drawText(QRectF(-width / 2, -height / 2, width, height), Qt::AlignCenter, text_);
    }
}
void Watermark::paintEvent(QPaintEvent*) {
    if (text_.isEmpty() || opacity_ == 0) {
        return;
    }
    if (tile_.isNull() || tileDpr_ != devicePixelRatioF()) {
        createTile();
    }
    QPainter painter(this);
    painter.setOpacity(opacity_);
    painter.drawTiledPixmap(rect(), tile_);
}
}  // namespace qfw
