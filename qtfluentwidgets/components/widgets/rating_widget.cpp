#include "components/widgets/rating_widget.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "common/config.h"
#include "common/font.h"

namespace qfw {
namespace {
QPainterPath starPath(qreal cx, qreal cy, qreal radius = 8) {
    QPainterPath path;
    constexpr qreal pi = 3.14159265358979323846;
    for (int i = 0; i < 10; ++i) {
        const qreal angle = -pi / 2 + i * pi / 5;
        const qreal r = i % 2 ? radius * 0.475 : radius;
        const QPointF point(cx + r * std::cos(angle), cy + r * std::sin(angle));
        if (!i) {
            path.moveTo(point);
        } else {
            path.lineTo(point);
        }
    }
    path.closeSubpath();
    return path;
}
}  // namespace
RatingWidget::RatingWidget(QWidget* parent) : RatingWidget(0, parent) {}
RatingWidget::RatingWidget(qreal value, QWidget* parent)
    : QWidget(parent), value_(validateValue(value)) {
    qfw::setFont(this, 20, QFont::DemiBold);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setAccessibleName(tr("Rating"));
    setAccessibleDescription(text());
    connect(&QConfig::instance(), &QConfig::themeChanged, this, [this] { update(); });
}
qreal RatingWidget::validateValue(qreal value) {
    if (!std::isfinite(value)) {
        throw std::invalid_argument("Rating requires a finite score");
    }
    return value;
}
qreal RatingWidget::value() const { return value_; }
void RatingWidget::setValue(qreal value) {
    validateValue(value);
    if (value == value_) {
        return;
    }
    value_ = value;
    setAccessibleDescription(text());
    updateGeometry();
    update();
    emit valueChanged(value);
}
int RatingWidget::decimals() const { return decimals_; }
void RatingWidget::setDecimals(int decimals) {
    if (decimals < 0 || decimals > 6) {
        throw std::invalid_argument("Rating precision must be between 0 and 6");
    }
    decimals_ = decimals;
    setAccessibleDescription(text());
    updateGeometry();
    update();
}
QString RatingWidget::text() const {
    QString result = QString::number(value_, 'f', decimals_);
    if (decimals_) {
        while (result.endsWith('0')) {
            result.chop(1);
        }
        if (result.endsWith('.')) {
            result.chop(1);
        }
    }
    return result == QStringLiteral("-0") ? QStringLiteral("0") : result;
}
void RatingWidget::setStarColor(const QColor& light, const QColor& dark) {
    if (!light.isValid()) {
        throw std::invalid_argument("Invalid star color");
    }
    light_ = light;
    dark_ = dark.isValid() ? dark : light;
    update();
}
QColor RatingWidget::starColor() const { return isDarkTheme() ? dark_ : light_; }
QSize RatingWidget::sizeHint() const {
    return QSize(fontMetrics().horizontalAdvance(text()) + 24,
                 std::max(24, fontMetrics().height()));
}
QSize RatingWidget::minimumSizeHint() const { return sizeHint(); }
void RatingWidget::changeEvent(QEvent* event) {
    QWidget::changeEvent(event);
    if (event->type() == QEvent::FontChange) {
        updateGeometry();
        update();
    }
}
void RatingWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    if (!isEnabled()) {
        painter.setOpacity(0.43);
    }
    const bool rtl = layoutDirection() == Qt::RightToLeft;
    const int tw = fontMetrics().horizontalAdvance(text());
    painter.setPen(isDarkTheme() ? Qt::white : Qt::black);
    painter.drawText(QRectF(rtl ? 24 : 0, 0, tw, height()), Qt::AlignVCenter | Qt::AlignLeft,
                     text());
    painter.setPen(QPen(starColor(), 1.2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(starColor());
    painter.drawPath(starPath(rtl ? 10 : tw + 14, height() / 2.0));
}
InteractiveRatingWidget::InteractiveRatingWidget(QWidget* parent)
    : InteractiveRatingWidget(0, parent) {}
InteractiveRatingWidget::InteractiveRatingWidget(qreal value, QWidget* parent)
    : RatingWidget(std::clamp(validateValue(value), 0.0, 5.0), parent) {
    setMouseTracking(true);
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::StrongFocus);
}
QSize InteractiveRatingWidget::sizeHint() const { return QSize(116, 24); }
void InteractiveRatingWidget::setValue(qreal value) {
    value = std::clamp(validateValue(value), 0.0, 5.0);
    hover_.reset();
    RatingWidget::setValue(value);
    update();
}
qreal InteractiveRatingWidget::displayValue() const { return hover_.value_or(value()); }
bool InteractiveRatingWidget::isReadOnly() const { return readOnly_; }
void InteractiveRatingWidget::setReadOnly(bool readOnly) {
    readOnly_ = readOnly;
    pressed_ = false;
    clearPreview();
    setCursor(readOnly ? Qt::ArrowCursor : Qt::PointingHandCursor);
}
qreal InteractiveRatingWidget::valueAt(qreal x) const {
    if (layoutDirection() == Qt::RightToLeft) {
        x = sizeHint().width() - x;
    }
    x = std::clamp(x, 0.0, qreal(sizeHint().width()));
    const int index = int(x / 24);
    return std::min(5.0, index + std::min(1.0, (x - index * 24) / 20));
}
qreal InteractiveRatingWidget::fillPosition(qreal value) const {
    const int whole = int(value);
    return whole >= 5 ? sizeHint().width() : whole * 24 + (value - whole) * 20;
}
void InteractiveRatingWidget::preview(qreal x) {
    const qreal value = valueAt(x);
    if (hover_ && *hover_ == value) {
        return;
    }
    hover_ = value;
    update();
    emit hovered(value);
}
void InteractiveRatingWidget::clearPreview() {
    if (hover_) {
        hover_.reset();
        update();
        emit hovered(value());
    }
}
void InteractiveRatingWidget::mouseMoveEvent(QMouseEvent* event) {
    if (isEnabled() && !readOnly_) {
        preview(event->pos().x());
    }
    RatingWidget::mouseMoveEvent(event);
}
void InteractiveRatingWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && isEnabled() && !readOnly_) {
        pressed_ = true;
        setFocus(Qt::MouseFocusReason);
        preview(event->pos().x());
        event->accept();
    } else {
        RatingWidget::mousePressEvent(event);
    }
}
void InteractiveRatingWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && pressed_) {
        pressed_ = false;
        if (isEnabled() && !readOnly_ && rect().contains(event->pos())) {
            setValue(valueAt(event->pos().x()));
        }
        event->accept();
    } else {
        RatingWidget::mouseReleaseEvent(event);
    }
}
void InteractiveRatingWidget::leaveEvent(QEvent* event) {
    clearPreview();
    RatingWidget::leaveEvent(event);
}
void InteractiveRatingWidget::keyPressEvent(QKeyEvent* event) {
    if (!isEnabled() || readOnly_) {
        RatingWidget::keyPressEvent(event);
        return;
    }
    int direction = 0;
    switch (event->key()) {
        case Qt::Key_Left:
            direction = layoutDirection() == Qt::RightToLeft ? 1 : -1;
            break;
        case Qt::Key_Right:
            direction = layoutDirection() == Qt::RightToLeft ? -1 : 1;
            break;
        case Qt::Key_Down:
            direction = -1;
            break;
        case Qt::Key_Up:
            direction = 1;
            break;
        case Qt::Key_Home:
            setValue(0);
            event->accept();
            return;
        case Qt::Key_End:
            setValue(5);
            event->accept();
            return;
        default:
            RatingWidget::keyPressEvent(event);
            return;
    }
    setValue(value() + direction * 0.1);
    event->accept();
}
void InteractiveRatingWidget::changeEvent(QEvent* event) {
    RatingWidget::changeEvent(event);
    if (event->type() == QEvent::EnabledChange && !isEnabled()) {
        pressed_ = false;
        clearPreview();
    }
}
void InteractiveRatingWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    if (!isEnabled()) {
        painter.setOpacity(0.43);
    }
    if (layoutDirection() == Qt::RightToLeft) {
        painter.translate(sizeHint().width(), 0);
        painter.scale(-1, 1);
    }
    QPainterPath stars;
    for (int i = 0; i < 5; ++i) {
        stars.addPath(starPath(i * 24 + 10, height() / 2.0));
    }
    painter.save();
    painter.setClipRect(QRectF(0, 0, fillPosition(displayValue()), height()));
    painter.fillPath(stars, starColor());
    painter.restore();
    painter.setPen(QPen(starColor(), 1.2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(stars);
    // Intentionally no focus border, including after mouse or keyboard activation.
}
}  // namespace qfw
