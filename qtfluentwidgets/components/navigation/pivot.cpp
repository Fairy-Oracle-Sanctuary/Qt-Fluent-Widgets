#include "components/navigation/pivot.h"

#include <QApplication>
#include <QPainter>
#include <QResizeEvent>
#include <QSizePolicy>

#include "common/color.h"
#include "common/font.h"
#include "common/style_sheet.h"

namespace qfw {

PivotItem::PivotItem(QWidget* parent) : PushButton(parent) {
    setProperty("qssClass", "PivotItem");
    setProperty("isSelected", false);
    setProperty("hasIcon", false);
    setSelected(false);

    connect(this, &QPushButton::clicked, this, [this]() { emit itemClicked(true); });
    setAttribute(Qt::WA_LayoutUsesWidgetRect);

    qfw::setStyleSheet(this, FluentStyleSheet::Pivot);
    qfw::setFont(this, 18);
}

PivotItem::PivotItem(const QString& text, QWidget* parent) : PushButton(text, parent) {
    setProperty("qssClass", "PivotItem");
    setProperty("isSelected", false);
    setProperty("hasIcon", false);
    setSelected(false);

    connect(this, &QPushButton::clicked, this, [this]() { emit itemClicked(true); });
    setAttribute(Qt::WA_LayoutUsesWidgetRect);

    qfw::setStyleSheet(this, FluentStyleSheet::Pivot);
    qfw::setFont(this, 18);
}

void PivotItem::setSelected(bool isSelected) {
    if (isSelected_ == isSelected) {
        return;
    }

    isSelected_ = isSelected;
    setProperty("isSelected", isSelected);
    setStyle(QApplication::style());
    update();
}

Pivot::Pivot(QWidget* parent) : QWidget(parent) {
    setProperty("qssClass", "Pivot");
    hBoxLayout_ = new QHBoxLayout(this);
    slideAni_ = new ScaleSlideAnimation(this, Qt::Horizontal);

    qfw::setStyleSheet(this, FluentStyleSheet::Pivot);

    hBoxLayout_->setSpacing(0);
    hBoxLayout_->setAlignment(Qt::AlignLeft);
    hBoxLayout_->setContentsMargins(0, 0, 0, 0);
    hBoxLayout_->setSizeConstraint(QHBoxLayout::SetMinimumSize);

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    connect(slideAni_, &ScaleSlideAnimation::valueChanged, this,
            [this](const QRectF&) { update(); });
}

PivotItem* Pivot::addItem(const QString& routeKey, const QString& text,
                          const std::function<void(bool)>& onClick, const QVariant& icon) {
    return insertItem(-1, routeKey, text, onClick, icon);
}

void Pivot::addWidget(const QString& routeKey, PivotItem* widget,
                      const std::function<void(bool)>& onClick) {
    insertWidget(-1, routeKey, widget, onClick);
}

PivotItem* Pivot::insertItem(int index, const QString& routeKey, const QString& text,
                             const std::function<void(bool)>& onClick, const QVariant& icon) {
    if (items_.contains(routeKey)) {
        return nullptr;
    }

    auto* item = new PivotItem(text, this);
    if (icon.isValid()) {
        item->setIcon(icon);
    }

    insertWidget(index, routeKey, item, onClick);
    return item;
}

void Pivot::insertWidget(int index, const QString& routeKey, PivotItem* widget,
                         const std::function<void(bool)>& onClick) {
    if (!widget || items_.contains(routeKey)) {
        return;
    }

    widget->setProperty("routeKey", routeKey);
    connect(widget, &PivotItem::itemClicked, this, &Pivot::onItemClicked);
    if (onClick) {
        connect(widget, &PivotItem::itemClicked, this, onClick);
    }

    items_.insert(routeKey, widget);
    hBoxLayout_->insertWidget(index, widget, 0);
}

void Pivot::removeWidget(const QString& routeKey) {
    if (!items_.contains(routeKey)) {
        return;
    }

    auto item = items_.take(routeKey);
    if (item) {
        hBoxLayout_->removeWidget(item);
        item->deleteLater();
    }

    if (items_.isEmpty()) {
        currentRouteKey_.clear();
    }
}

void Pivot::clear() {
    for (auto it = items_.begin(); it != items_.end(); ++it) {
        if (it.value()) {
            hBoxLayout_->removeWidget(it.value());
            it.value()->deleteLater();
        }
    }

    items_.clear();
    currentRouteKey_.clear();
}

PivotItem* Pivot::currentItem() const {
    if (currentRouteKey_.isEmpty()) {
        return nullptr;
    }

    return widget(currentRouteKey_);
}

void Pivot::setCurrentItem(const QString& routeKey) {
    if (!items_.contains(routeKey) || routeKey == currentRouteKey()) {
        return;
    }

    adjustIndicatorPos();

    currentRouteKey_ = routeKey;
    startIndicatorAnimation();

    for (auto it = items_.begin(); it != items_.end(); ++it) {
        if (it.value()) {
            it.value()->setSelected(it.key() == routeKey);
        }
    }

    emit currentItemChanged(routeKey);
}

void Pivot::startIndicatorAnimation() {
    if (slideAni_) {
        slideAni_->startAnimation(currentIndicatorGeometry());
    }
}

void Pivot::showEvent(QShowEvent* e) {
    QWidget::showEvent(e);
    adjustIndicatorPos();
}

void Pivot::setIndicatorLength(int len) {
    indicatorLength_ = len;
    adjustIndicatorPos();
}

void Pivot::setItemFontSize(int size) {
    for (auto it = items_.begin(); it != items_.end(); ++it) {
        if (!it.value()) {
            continue;
        }

        auto font = it.value()->font();
        font.setPixelSize(size);
        it.value()->setFont(font);
        it.value()->adjustSize();
    }
}

void Pivot::setItemText(const QString& routeKey, const QString& text) {
    if (auto* item = widget(routeKey)) {
        item->setText(text);
    }
}

void Pivot::setIndicatorColor(const QColor& light, const QColor& dark) {
    lightIndicatorColor_ = light;
    darkIndicatorColor_ = dark;
    update();
}

PivotItem* Pivot::widget(const QString& routeKey) const {
    if (!items_.contains(routeKey)) {
        return nullptr;
    }

    return items_.value(routeKey);
}

void Pivot::resizeEvent(QResizeEvent* e) {
    QWidget::resizeEvent(e);
    adjustIndicatorPos();
}

void Pivot::onItemClicked(bool checked) {
    Q_UNUSED(checked);
    auto* item = qobject_cast<PivotItem*>(sender());
    if (!item) {
        return;
    }

    const QString routeKey = item->property("routeKey").toString();
    setCurrentItem(routeKey);
}

void Pivot::adjustIndicatorPos() {
    auto* item = currentItem();
    if (item) {
        slideAni_->stopAnimation();
        slideAni_->setGeometry(currentIndicatorGeometry());
    }
}

QRectF Pivot::currentIndicatorGeometry() const {
    auto* item = currentItem();
    if (!item) {
        return QRectF(0, height() - 3, indicatorLength_, 3);
    }

    const QRect rect = item->geometry();
    return QRectF(rect.x() - 8 + rect.width() / 2.0, height() - 3, indicatorLength_, 3);
}

void Pivot::paintEvent(QPaintEvent* e) {
    QWidget::paintEvent(e);

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
    painter.drawRoundedRect(slideAni_->getGeometry(), 1.5, 1.5);
}

}  // namespace qfw
