#include "components/navigation/segmented_widget.h"

#include <QApplication>
#include <QEasingCurve>
#include <QPainter>

#include "common/color.h"
#include "common/config.h"
#include "common/font.h"
#include "common/style_sheet.h"

namespace qfw {

SegmentedItem::SegmentedItem(QWidget* parent) : PivotItem(parent) {
    setProperty("qssClass", "SegmentedItem");
    setProperty("isSelected", false);
    setProperty("hasIcon", false);
    setStyle(QApplication::style());
    qfw::setFont(this, 14);
}

SegmentedItem::SegmentedItem(const QString& text, QWidget* parent) : PivotItem(text, parent) {
    setProperty("qssClass", "SegmentedItem");
    setProperty("isSelected", false);
    setProperty("hasIcon", false);
    setStyle(QApplication::style());
    qfw::setFont(this, 14);
}

SegmentedToolItem::SegmentedToolItem(QWidget* parent) : PivotItem(parent) {
    setProperty("qssClass", "SegmentedToolItem");
    setProperty("isSelected", false);
    setProperty("hasIcon", false);
    setStyle(QApplication::style());
    setFixedSize(38, 33);
}

SegmentedToolItem::SegmentedToolItem(const QVariant& icon, QWidget* parent) : PivotItem(parent) {
    setProperty("qssClass", "SegmentedToolItem");
    setProperty("isSelected", false);
    setProperty("hasIcon", false);
    setStyle(QApplication::style());
    setFixedSize(38, 33);
    if (icon.isValid()) {
        setIcon(icon);
        setProperty("hasIcon", true);
    }
}

SegmentedToggleToolItem::SegmentedToggleToolItem(QWidget* parent) : PivotItem(parent) {
    setProperty("qssClass", "SegmentedToggleToolItem");
    setProperty("isSelected", false);
    setProperty("hasIcon", false);
    setStyle(QApplication::style());
    setFixedSize(50, 32);
    setCheckable(true);
}

SegmentedToggleToolItem::SegmentedToggleToolItem(const QVariant& icon, QWidget* parent)
    : PivotItem(parent) {
    setProperty("qssClass", "SegmentedToggleToolItem");
    setProperty("isSelected", false);
    setProperty("hasIcon", false);
    setStyle(QApplication::style());
    setFixedSize(50, 32);
    setCheckable(true);
    if (icon.isValid()) {
        setIcon(icon);
        setProperty("hasIcon", true);
    }
}

void SegmentedToggleToolItem::setSelected(bool isSelected) {
    if (isSelected_ == isSelected) {
        return;
    }

    isSelected_ = isSelected;
    setProperty("isSelected", isSelected);
    setChecked(isSelected);
    setStyle(QApplication::style());
    update();
}

void SegmentedToggleToolItem::drawIcon(QPainter* painter, const QRectF& rect, QIcon::State state) {
    if (!painter) {
        return;
    }

    // Always use reverse theme color for icons in SegmentedToggleToolItem when selected
    // to ensure visibility against the theme-colored background.
    const bool selected = property("isSelected").toBool();
    if (selected) {
        if (icon_.canConvert<const FluentIconBase*>()) {
            const auto* fluentIcon = icon_.value<const FluentIconBase*>();
            if (fluentIcon) {
                // Use theme-reversed QIcon to guarantee inverted (reverse) rendering
                // against the selected background.
                QIcon reversed = fluentIcon->qicon(true);
                reversed.paint(painter, rect.toRect(), Qt::AlignCenter, QIcon::Normal, state);
                return;
            }
        } else if (icon_.canConvert<QIcon>()) {
            const QIcon qicon = icon_.value<QIcon>();
            if (!qicon.isNull()) {
                const QSize size = rect.size().toSize();
                QPixmap pm = qicon.pixmap(size, QIcon::Normal, state);
                if (!pm.isNull()) {
                    QImage img = pm.toImage().convertToFormat(QImage::Format_ARGB32);
                    img.invertPixels(QImage::InvertRgb);
                    painter->drawPixmap(rect.toRect(), QPixmap::fromImage(img));
                    return;
                }
            }
        }
    }

    PivotItem::drawIcon(painter, rect, state);
}

SegmentedWidget::SegmentedWidget(QWidget* parent) : Pivot(parent) {
    setProperty("qssClass", "SegmentedWidget");
    slideAni_ = new QPropertyAnimation(this, "slideValue", this);
    slideAni_->setDuration(150);
    slideAni_->setEasingCurve(QEasingCurve::OutQuad);

    setAttribute(Qt::WA_StyledBackground);
}

void SegmentedWidget::setItemPadding(int padding) {
    if (itemPadding_ == padding) {
        return;
    }

    itemPadding_ = padding;

    // Update padding on all existing items individually to preserve parent styles
    QString style = QString("padding-left: %1px; padding-right: %1px;").arg(padding);
    for (auto it = items_.begin(); it != items_.end(); ++it) {
        if (it.value().data()) {
            it.value()->setStyleSheet(style);
            it.value()->updateGeometry();
        }
    }
}

void SegmentedWidget::setSlideValue(qreal v) {
    slideValue_ = v;
    update();
}

void SegmentedWidget::startSlideAnimation(qreal endValue) {
    if (!slideAni_) {
        return;
    }

    slideAni_->stop();
    slideAni_->setStartValue(slideValue_);
    slideAni_->setEndValue(endValue);
    slideAni_->start();
}

PivotItem* SegmentedWidget::insertItem(int index, const QString& routeKey, const QString& text,
                                       const std::function<void(bool)>& onClick,
                                       const QVariant& icon) {
    if (widget(routeKey)) {
        return nullptr;
    }

    auto* item = new SegmentedItem(text, this);
    if (icon.isValid()) {
        item->setIcon(icon);
    }

    // Apply custom padding if set
    if (itemPadding_ != 12) {
        QString style = QString("padding-left: %1px; padding-right: %1px;").arg(itemPadding_);
        item->setStyleSheet(style);
    }

    insertWidget(index, routeKey, item, onClick);
    return item;
}

QRectF SegmentedWidget::currentIndicatorGeometry() const {
    auto* item = currentItem();
    const qreal x = item ? item->x() : 0;
    return QRectF(x, 0, 0, 0);
}

void SegmentedWidget::adjustIndicatorPos() {
    auto* item = currentItem();
    if (!item) {
        return;
    }

    if (slideAni_) {
        slideAni_->stop();
    }
    setSlideValue(item->x());
}

void SegmentedWidget::startIndicatorAnimation() {
    auto* item = currentItem();
    if (!item) {
        return;
    }

    startSlideAnimation(item->x());
}

void SegmentedWidget::paintEvent(QPaintEvent* e) {
    QWidget::paintEvent(e);

    if (!currentItem()) {
        return;
    }

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    // draw background
    if (isDarkTheme()) {
        painter.setPen(QColor(255, 255, 255, 14));
        painter.setBrush(QColor(255, 255, 255, 15));
    } else {
        painter.setPen(QColor(0, 0, 0, 19));
        painter.setBrush(QColor(255, 255, 255, 179));
    }

    auto* item = currentItem();
    QRectF rect = item->rect().adjusted(1, 1, -1, -1);
    rect.translate(slideValue_, 0);
    painter.drawRoundedRect(rect, 5, 5);

    // draw indicator
    painter.setPen(Qt::NoPen);
    QColor indicatorColor = autoFallbackThemeColor(lightIndicatorColor_, darkIndicatorColor_);
    if (isDarkTheme() && !darkIndicatorColor_.isValid()) {
        indicatorColor = themedColor(indicatorColor, true, QStringLiteral("ThemeColorLight1"));
    }
    painter.setBrush(indicatorColor);

    const int x = int(item->width() / 2.0 - 8 + slideValue_);
    painter.drawRoundedRect(QRectF(x, height() - 3.5, 16, 3), 1.5, 1.5);
}

SegmentedToolWidget::SegmentedToolWidget(QWidget* parent) : SegmentedWidget(parent) {
    setProperty("qssClass", "SegmentedToolWidget");
    setAttribute(Qt::WA_StyledBackground);
}

PivotItem* SegmentedToolWidget::addItem(const QString& routeKey, const QVariant& icon,
                                        const std::function<void(bool)>& onClick) {
    return insertItem(-1, routeKey, QString(), onClick, icon);
}

PivotItem* SegmentedToolWidget::insertItem(int index, const QString& routeKey, const QString& text,
                                           const std::function<void(bool)>& onClick,
                                           const QVariant& icon) {
    if (widget(routeKey)) {
        return nullptr;
    }

    auto* item = createItem(icon);
    if (!item) {
        return nullptr;
    }
    insertWidget(index, routeKey, item, onClick);
    return item;
}

PivotItem* SegmentedToolWidget::createItem(const QVariant& icon) {
    return new SegmentedToolItem(icon, this);
}

SegmentedToggleToolWidget::SegmentedToggleToolWidget(QWidget* parent)
    : SegmentedToolWidget(parent) {
    setProperty("qssClass", "SegmentedToggleToolWidget");
    setAttribute(Qt::WA_StyledBackground);
}

PivotItem* SegmentedToggleToolWidget::createItem(const QVariant& icon) {
    return new SegmentedToggleToolItem(icon, this);
}

void SegmentedToggleToolWidget::paintEvent(QPaintEvent* e) {
    SegmentedWidget::paintEvent(e);

    if (!currentItem()) {
        return;
    }

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    painter.setPen(Qt::NoPen);
    QColor indicatorColor = autoFallbackThemeColor(lightIndicatorColor_, darkIndicatorColor_);
    if (isDarkTheme() && !darkIndicatorColor_.isValid()) {
        indicatorColor = themedColor(indicatorColor, true, QStringLiteral("ThemeColorLight1"));
    }
    painter.setBrush(indicatorColor);

    auto* item = currentItem();
    painter.drawRoundedRect(QRectF(slideValue_, 0, item->width(), item->height()), 4, 4);
}

}  // namespace qfw
