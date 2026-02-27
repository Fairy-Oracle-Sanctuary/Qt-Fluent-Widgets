#include "icon_widget.h"

#include <QPainter>

namespace qfw {

IconWidget::IconWidget(QWidget* parent) : QWidget(parent) { setIcon(QIcon()); }

IconWidget::IconWidget(const FluentIconBase& icon, QWidget* parent) : IconWidget(parent) {
    setIcon(icon);
}

IconWidget::IconWidget(const FluentIconBase* icon, QWidget* parent) : IconWidget(parent) {
    setIcon(icon);
}

IconWidget::IconWidget(const QIcon& icon, QWidget* parent) : IconWidget(parent) { setIcon(icon); }

IconWidget::IconWidget(const QString& icon, QWidget* parent) : IconWidget(parent) { setIcon(icon); }

QIcon IconWidget::icon() const {
    if (icon_.canConvert<QIcon>()) {
        return icon_.value<QIcon>();
    }

    if (icon_.canConvert<QString>()) {
        return QIcon(icon_.value<QString>());
    }

    if (fluentIcon_) {
        return fluentIcon_->icon();
    }

    return QIcon();
}

void IconWidget::setIcon(const FluentIconBase& icon) {
    fluentIcon_.reset(icon.clone());
    icon_ = QVariant();
    update();
}

void IconWidget::setIcon(const FluentIconBase* icon) {
    fluentIcon_.reset(icon ? icon->clone() : nullptr);
    icon_ = QVariant();
    update();
}

void IconWidget::setIcon(const QIcon& icon) {
    fluentIcon_.reset();
    setIcon(QVariant::fromValue(icon));
}

void IconWidget::setIcon(const QString& icon) {
    fluentIcon_.reset();
    setIcon(QVariant::fromValue(icon));
}

void IconWidget::setIcon(const QVariant& icon) {
    fluentIcon_.reset();
    icon_ = icon;
    update();
}

void IconWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    if (fluentIcon_) {
        fluentIcon_->render(&painter, rect(), Theme::Auto, {});
        return;
    }

    const QIcon qicon = icon();
    if (!qicon.isNull()) {
        qicon.paint(&painter, rect(), Qt::AlignCenter, QIcon::Normal, QIcon::Off);
    }
}

}  // namespace qfw
