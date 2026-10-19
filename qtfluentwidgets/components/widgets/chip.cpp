#include "components/widgets/chip.h"

#include <QHideEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QShowEvent>
#include <QStyle>
#include <QVariantAnimation>
#include <algorithm>

#include "common/config.h"
#include "common/font.h"
#include "common/style_sheet.h"

namespace qfw {

Chip::Chip(QWidget* parent) : QAbstractButton(parent) {
    setCheckable(true);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::PointingHandCursor);
    setFixedHeight(33);
    setIconSize(QSize(16, 16));
    qfw::setFont(this);
    closeButton_ = new RoundToolButton(FluentIcon(FluentIconEnum::CancelMedium), this);
    closeButton_->setFixedSize(20, 20);
    closeButton_->setIconSize(QSize(10, 10));
    closeButton_->setHasBorder(false);
    closeButton_->setToolTip(tr("Remove"));
    closeButton_->setAccessibleName(tr("Remove"));
    connect(closeButton_, &QAbstractButton::clicked, this, &Chip::closed);
    backgroundAnimation_ = new QVariantAnimation(this);
    backgroundAnimation_->setDuration(120);
    connect(backgroundAnimation_, &QVariantAnimation::valueChanged, this,
            [this](const QVariant& value) {
                background_ = value.value<QColor>();
                update();
            });
    connect(this, &QAbstractButton::pressed, this, [this] { refreshBackground(); });
    connect(this, &QAbstractButton::released, this, [this] { refreshBackground(); });
    connect(this, &QAbstractButton::toggled, this, [this] {
        updateCloseIcon();
        refreshBackground();
    });
    connect(&QConfig::instance(), &QConfig::themeChanged, this, [this] {
        updateCloseIcon();
        refreshBackground(false);
        update();
    });
    connect(&QConfig::instance(), &QConfig::themeColorChanged, this, [this] {
        refreshBackground(false);
        update();
    });
    updateCloseIcon();
    refreshBackground(false);
    positionCloseButton();
}
Chip::Chip(const QString& text, QWidget* parent) : Chip(parent) { setText(text); }
Chip::Chip(const QIcon& icon, const QString& text, QWidget* parent) : Chip(text, parent) {
    setIcon(icon);
}
Chip::Chip(const FluentIconBase& icon, const QString& text, QWidget* parent) : Chip(text, parent) {
    setIcon(icon);
}
Chip::~Chip() = default;
void Chip::setIcon(const QIcon& icon) {
    fluentIcon_.reset();
    QAbstractButton::setIcon(icon);
    updateGeometry();
    update();
}
void Chip::setIcon(const FluentIconBase& icon) {
    fluentIcon_.reset(icon.clone());
    QAbstractButton::setIcon(fluentIcon_->qicon());
    updateGeometry();
    update();
}
void Chip::setIcon(const QVariant& icon) {
    if (icon.canConvert<QIcon>()) {
        setIcon(icon.value<QIcon>());
    } else if (icon.canConvert<const FluentIconBase*>()) {
        const auto* fluent = icon.value<const FluentIconBase*>();
        if (fluent) {
            setIcon(*fluent);
        } else {
            setIcon(QIcon());
        }
    } else if (icon.canConvert<QString>()) {
        setIcon(QIcon(icon.toString()));
    } else if (!icon.isValid()) {
        setIcon(QIcon());
    }
}
QIcon Chip::icon() const { return fluentIcon_ ? fluentIcon_->icon() : QAbstractButton::icon(); }
RoundToolButton* Chip::closeButton() const { return closeButton_; }
QColor Chip::backgroundColor() const { return background_; }
QSize Chip::sizeHint() const {
    return QSize(48 + fontMetrics().horizontalAdvance(text()) + (icon().isNull() ? 0 : 24), 33);
}
QSize Chip::minimumSizeHint() const { return sizeHint(); }
void Chip::positionCloseButton() {
    if (closeButton_) {
        const QRect logical(width() - closeButton_->width() - 6,
                            (height() - closeButton_->height()) / 2, 20, 20);
        closeButton_->setGeometry(QStyle::visualRect(layoutDirection(), rect(), logical));
    }
}
void Chip::resizeEvent(QResizeEvent* event) {
    QAbstractButton::resizeEvent(event);
    positionCloseButton();
}
void Chip::updateCloseIcon() {
    const bool lightIcon = isChecked() ? !isDarkTheme() : isDarkTheme();
    closeButton_->setIcon(
        FluentIcon(FluentIconEnum::CancelMedium).icon(lightIcon ? Theme::Dark : Theme::Light));
}
void Chip::refreshBackground(bool animate) {
    if (!backgroundAnimation_) {
        return;
    }
    backgroundAnimation_->stop();
    QColor color;
    if (isChecked()) {
        color = themeColor();
    } else if (isDarkTheme()) {
        color = QColor(255, 255, 255, !isEnabled() ? 8 : isDown() ? 8 : hovered_ ? 17 : 13);
    } else {
        color = QColor(249, 249, 249, !isEnabled() || isDown() ? 76 : hovered_ ? 127 : 255);
    }
    if (animate && isVisible()) {
        backgroundAnimation_->setStartValue(background_);
        backgroundAnimation_->setEndValue(color);
        backgroundAnimation_->start();
    } else {
        background_ = color;
        update();
    }
}
void Chip::enterEvent(enterEvent_QEnterEvent* event) {
    hovered_ = true;
    refreshBackground();
    QAbstractButton::enterEvent(event);
}
void Chip::leaveEvent(QEvent* event) {
    hovered_ = false;
    refreshBackground();
    QAbstractButton::leaveEvent(event);
}
void Chip::changeEvent(QEvent* event) {
    QAbstractButton::changeEvent(event);
    if (event->type() == QEvent::EnabledChange || event->type() == QEvent::PaletteChange) {
        if (closeButton_) {
            updateCloseIcon();
        }
        refreshBackground(false);
    } else if (event->type() == QEvent::FontChange) {
        updateGeometry();
    } else if (event->type() == QEvent::LayoutDirectionChange) {
        positionCloseButton();
        update();
    }
}
void Chip::hideEvent(QHideEvent* event) {
    backgroundAnimation_->stop();
    QAbstractButton::hideEvent(event);
}
void Chip::showEvent(QShowEvent* event) {
    QAbstractButton::showEvent(event);
    refreshBackground(false);
}
void Chip::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing |
                           QPainter::SmoothPixmapTransform);
    painter.setFont(font());
    const bool dark = isDarkTheme();
    const QColor textColor = isChecked() ? (dark ? QColor(Qt::black) : QColor(Qt::white))
                                         : (dark ? QColor(Qt::white) : QColor(Qt::black));
    const QColor border = isChecked() ? themeColor()
                          : dark      ? QColor(255, 255, 255, 20)
                                      : QColor(0, 0, 0, 19);
    painter.setPen(QPen(border, 1));
    painter.setBrush(background_);
    painter.drawRoundedRect(QRectF(rect()).adjusted(1, 1, -1, -1), 5, 5);
    if (!isEnabled()) {
        painter.setOpacity(0.36);
    }
    const bool hasIcon = !icon().isNull();
    int textStart = 12;
    if (hasIcon) {
        const QRect logical(12, (height() - 16) / 2, 16, 16);
        const QIcon drawn = fluentIcon_ ? fluentIcon_->icon(Theme::Auto, textColor) : icon();
        drawn.paint(&painter, QStyle::visualRect(layoutDirection(), rect(), logical),
                    Qt::AlignCenter);
        textStart += 24;
    }
    const int textWidth = std::max(0, width() - textStart - 36);
    const QRect textRect =
        QStyle::visualRect(layoutDirection(), rect(), QRect(textStart, 0, textWidth, height()));
    painter.setPen(textColor);
    painter.drawText(textRect, Qt::AlignLeading | Qt::AlignVCenter,
                     fontMetrics().elidedText(text(), Qt::ElideRight, textWidth));
    if (hasFocus() && testAttribute(Qt::WA_KeyboardFocusChange)) {
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(textColor, 1, Qt::DashLine));
        painter.drawRoundedRect(QRectF(rect()).adjusted(4, 4, -4, -4), 3, 3);
    }
}

}  // namespace qfw
