#include "components/widgets/skeleton.h"

#include <QHideEvent>
#include <QLinearGradient>
#include <QPainter>
#include <QShowEvent>
#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "common/config.h"

namespace qfw {
namespace {
void validateRect(const QRectF& r) {
    if (!std::isfinite(r.x()) || !std::isfinite(r.y()) || !std::isfinite(r.width()) ||
        !std::isfinite(r.height()) || r.width() <= 0 || r.height() <= 0) {
        throw std::invalid_argument("Skeleton geometry must be finite with positive dimensions");
    }
}
}  // namespace
SkeletonWidget::SkeletonWidget(QWidget* parent) : QWidget(parent), animation_(this) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setAccessibleName(tr("Loading"));
    animation_.setStartValue(0.0);
    animation_.setEndValue(1.0);
    animation_.setDuration(1500);
    animation_.setLoopCount(-1);
    connect(&animation_, &QVariantAnimation::valueChanged, this, [this](const QVariant& value) {
        phase_ = value.toReal();
        update();
    });
    connect(&QConfig::instance(), &QConfig::themeChanged, this, [this] { update(); });
}
void SkeletonWidget::addRect(const QRectF& rect, qreal radius, bool relative) {
    validateRect(rect);
    if (!std::isfinite(radius) || radius < 0) {
        throw std::invalid_argument("Invalid skeleton radius");
    }
    shapes_.append({rect, radius, false, relative});
    update();
}
void SkeletonWidget::addEllipse(const QRectF& rect, bool relative) {
    validateRect(rect);
    shapes_.append({rect, 0, true, relative});
    update();
}
void SkeletonWidget::clear() {
    shapes_.clear();
    update();
}
QPainterPath SkeletonWidget::skeletonPath() const {
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    for (const auto& shape : shapes_) {
        QRectF r = shape.rect;
        if (shape.relative) {
            r = QRectF(r.x() * width(), r.y() * height(), r.width() * width(),
                       r.height() * height());
        }
        if (shape.ellipse) {
            path.addEllipse(r);
        } else {
            const qreal radius = std::min({shape.radius, r.width() / 2, r.height() / 2});
            path.addRoundedRect(r, radius, radius);
        }
    }
    return path;
}
QColor SkeletonWidget::baseColor() const { return isDarkTheme() ? baseDark_ : baseLight_; }
QColor SkeletonWidget::highlightColor() const {
    return isDarkTheme() ? highlightDark_ : highlightLight_;
}
void SkeletonWidget::setColors(const QColor& light, const QColor& dark, const QColor& hl,
                               const QColor& hd) {
    if (!light.isValid() || !dark.isValid()) {
        throw std::invalid_argument("Invalid skeleton color");
    }
    baseLight_ = light;
    baseDark_ = dark;
    if (hl.isValid()) {
        highlightLight_ = hl;
    }
    if (hd.isValid()) {
        highlightDark_ = hd;
    }
    update();
}
bool SkeletonWidget::isAnimationEnabled() const { return animationEnabled_; }
void SkeletonWidget::setAnimationEnabled(bool enabled) {
    animationEnabled_ = enabled;
    if (!enabled) {
        animation_.stop();
        phase_ = 0;
    } else if (isVisible()) {
        if (animation_.state() == QAbstractAnimation::Paused) {
            animation_.resume();
        } else if (animation_.state() == QAbstractAnimation::Stopped) {
            animation_.start();
        }
    }
    update();
}
int SkeletonWidget::animationDuration() const { return animation_.duration(); }
void SkeletonWidget::setAnimationDuration(int duration) {
    if (duration <= 0) {
        throw std::invalid_argument("Skeleton duration must be positive");
    }
    animation_.setDuration(duration);
}
QSize SkeletonWidget::sizeHint() const { return QSize(520, 100); }
QSize SkeletonWidget::minimumSizeHint() const { return QSize(0, sizeHint().height()); }
void SkeletonWidget::showEvent(QShowEvent* e) {
    QWidget::showEvent(e);
    setAnimationEnabled(animationEnabled_);
}
void SkeletonWidget::hideEvent(QHideEvent* e) {
    if (animation_.state() == QAbstractAnimation::Running) {
        animation_.pause();
    }
    QWidget::hideEvent(e);
}
void SkeletonWidget::paintEvent(QPaintEvent*) {
    const QPainterPath path = skeletonPath();
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    if (!isEnabled()) {
        painter.setOpacity(0.43);
    }
    painter.fillPath(path, baseColor());
    if (animationEnabled_ && !path.isEmpty()) {
        constexpr qreal angle = 3.14159265358979323846 / 6;
        const qreal band = std::max(40.0, width() * 0.3);
        const qreal margin = band + height() * std::tan(angle) / 2;
        const qreal center = -margin + phase_ * (width() + 2 * margin);
        const qreal dx = band * std::cos(angle) / 2, dy = band * std::sin(angle) / 2;
        QLinearGradient gradient(center - dx, height() / 2.0 - dy, center + dx,
                                 height() / 2.0 + dy);
        QColor transparent = highlightColor();
        transparent.setAlpha(0);
        gradient.setColorAt(0, transparent);
        gradient.setColorAt(0.5, highlightColor());
        gradient.setColorAt(1, transparent);
        painter.fillPath(path, gradient);
    }
}
QPainterPath SkeletonWidget::personalInfoPath(bool circular) const {
    auto path = SkeletonWidget::skeletonPath();
    const qreal avatar = std::min({100.0, qreal(height()), std::max(0.0, width() / 4.0)});
    const qreal left = avatar + std::min(20.0, std::max(0.0, width() - avatar));
    const qreal textWidth = std::max(0.0, std::min(400.0, width() - left));
    const QRectF rect(0, (height() - avatar) / 2, avatar, avatar);
    if (circular) {
        path.addEllipse(rect);
    } else {
        path.addRoundedRect(rect, 6, 6);
    }
    const qreal y = (height() - 100) / 2.0;
    path.addRoundedRect(QRectF(left, y + 10, textWidth, 30), 6, 6);
    path.addRoundedRect(QRectF(left, y + 60, textWidth / 2, 30), 6, 6);
    return path;
}
QPainterPath CirclePersonalInfoSkeleton::skeletonPath() const { return personalInfoPath(true); }
QPainterPath RectanglePersonalInfoSkeleton::skeletonPath() const { return personalInfoPath(false); }
QSize ArticleSkeleton::sizeHint() const { return QSize(544, 112); }
QPainterPath ArticleSkeleton::skeletonPath() const {
    auto path = SkeletonWidget::skeletonPath();
    const qreal image = std::min({112.0, qreal(height()), std::max(0.0, width() / 4.0)});
    const qreal left = image + std::min(20.0, std::max(0.0, width() - image));
    const qreal textWidth = std::max(0.0, std::min(412.0, width() - left));
    const qreal y = (height() - 112) / 2.0;
    path.addRoundedRect(QRectF(0, (height() - image) / 2, image, image), 4, 4);
    const int tops[] = {4, 28, 64, 90};
    for (int i = 0; i < 4; ++i) {
        path.addRoundedRect(QRectF(left, y + tops[i], textWidth * (i % 2 ? 0.55 : 1), 18), 4, 4);
    }
    return path;
}
}  // namespace qfw
