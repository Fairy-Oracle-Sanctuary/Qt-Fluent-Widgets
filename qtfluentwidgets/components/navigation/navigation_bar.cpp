#include "components/navigation/navigation_bar.h"

#include <QEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QScrollBar>

#include "common/color.h"
#include "common/config.h"
#include "common/font.h"
#include "common/icon.h"
#include "common/style_sheet.h"
#include "components/navigation/navigation_panel.h"  // for NavigationItemPosition
#include "components/navigation/navigation_widget.h"

namespace qfw {

namespace {

static void drawIconVariant(const QVariant& icon, QPainter* painter, const QRectF& rect,
                            const QColor& fill = QColor()) {
    if (!painter || !icon.isValid()) {
        return;
    }

    if (icon.canConvert<QIcon>()) {
        QIcon qicon = icon.value<QIcon>();
        if (!qicon.isNull()) {
            if (fill.isValid()) {
                const QSize sz = rect.size().toSize();
                if (!sz.isEmpty()) {
                    QPixmap pix = qicon.pixmap(sz);
                    if (!pix.isNull()) {
                        QPainter p(&pix);
                        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
                        p.fillRect(pix.rect(), fill);
                        p.end();

                        painter->drawPixmap(rect.topLeft(), pix);
                        return;
                    }
                }
            }

            qicon.paint(painter, rect.toRect());
        }
        return;
    }

    if (icon.canConvert<const FluentIconBase*>()) {
        const FluentIconBase* fluentIcon = icon.value<const FluentIconBase*>();
        if (!fluentIcon) {
            return;
        }

        if (fill.isValid()) {
            fluentIcon->render(painter, rect.toRect(), Theme::Auto,
                               QVariantMap{{QStringLiteral("fill"), fill.name()}});
        } else {
            fluentIcon->render(painter, rect.toRect());
        }
    }
}

}  // namespace

// ============================================================================
// IconSlideAnimation
// ============================================================================

IconSlideAnimation::IconSlideAnimation(QWidget* parent)
    : QPropertyAnimation(parent), offset_(0), maxOffset_(6) {
    setTargetObject(this);
    setPropertyName(QByteArrayLiteral("offset"));
}

void IconSlideAnimation::setOffset(qreal value) {
    offset_ = value;
    if (auto* w = qobject_cast<QWidget*>(parent())) {
        w->update();
    }
}

void IconSlideAnimation::slideDown() {
    setEndValue(maxOffset_);
    setDuration(100);
    start();
}

void IconSlideAnimation::slideUp() {
    setEndValue(0);
    setDuration(100);
    start();
}

// ============================================================================
// NavigationBarPushButton
// ============================================================================

NavigationBarPushButton::NavigationBarPushButton(const QVariant& icon, const QString& text,
                                                 bool isSelectable, const QVariant& selectedIcon,
                                                 QWidget* parent)
    : NavigationPushButton(icon, text, isSelectable, parent), selectedIcon_(selectedIcon) {
    iconAni_ = new IconSlideAnimation(this);

    setFixedSize(64, 58);
    qfw::setFont(this, 11);
}

void NavigationBarPushButton::setSelectedColor(const QColor& light, const QColor& dark) {
    lightSelectedColor_ = light;
    darkSelectedColor_ = dark;
    update();
}

void NavigationBarPushButton::setSelectedIcon(const QVariant& icon) {
    selectedIcon_ = icon;
    update();
}

void NavigationBarPushButton::setSelectedTextVisible(bool visible) {
    isSelectedTextVisible_ = visible;
    update();
}

QRectF NavigationBarPushButton::indicatorRect() const { return QRectF(0, 16, 4, 24); }

void NavigationBarPushButton::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e)

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing |
                           QPainter::SmoothPixmapTransform);
    painter.setPen(Qt::NoPen);

    drawBackground(&painter);
    drawIcon(&painter);
    drawText(&painter);
}

void NavigationBarPushButton::drawBackground(QPainter* painter) {
    if (!painter) {
        return;
    }

    if (isSelected_ || isAboutSelected_) {
        painter->setBrush(isDarkTheme() ? QColor(255, 255, 255, 42) : QColor(Qt::white));
        painter->drawRoundedRect(rect(), 5, 5);

        if (!isAboutSelected_) {
            painter->setBrush(autoFallbackThemeColor(lightSelectedColor_, darkSelectedColor_));
            if (!isPressed_) {
                painter->drawRoundedRect(QRectF(0, 16, 4, 24), 2, 2);
            } else {
                painter->drawRoundedRect(QRectF(0, 19, 4, 18), 2, 2);
            }
        }
    } else if (isPressed_ || isEnter_) {
        const int c = isDarkTheme() ? 255 : 0;
        const int alpha = isEnter_ ? 9 : 6;
        painter->setBrush(QColor(c, c, c, alpha));
        painter->drawRoundedRect(rect(), 5, 5);
    }
}

void NavigationBarPushButton::drawIcon(QPainter* painter) {
    if (!painter) {
        return;
    }

    if ((isPressed_ || !isEnter_) && !(isSelected_ || isAboutSelected_)) {
        painter->setOpacity(0.6);
    }

    if (!isEnabled()) {
        painter->setOpacity(0.4);
    }

    QRectF rectF(22, 13, 20, 20);
    if (!isSelectedTextVisible_) {
        rectF.translate(0, iconAni_ ? iconAni_->offset() : 0);
    }

    QVariant effectiveSelectedIcon = selectedIcon_.isValid() ? selectedIcon_ : iconVariant();

    if (isSelected_ || isAboutSelected_) {
        const QColor color = autoFallbackThemeColor(lightSelectedColor_, darkSelectedColor_);
        drawIconVariant(effectiveSelectedIcon, painter, rectF, color);
    } else {
        drawIconVariant(iconVariant(), painter, rectF);
    }

    painter->setOpacity(1.0);
}

void NavigationBarPushButton::drawText(QPainter* painter) {
    if (!painter) {
        return;
    }

    if (isSelected_ && !isSelectedTextVisible_) {
        return;
    }

    if (isSelected_ || isAboutSelected_) {
        painter->setPen(autoFallbackThemeColor(lightSelectedColor_, darkSelectedColor_));
    } else {
        painter->setPen(isDarkTheme() ? QColor(Qt::white) : QColor(Qt::black));
    }

    painter->setFont(font());
    painter->drawText(QRect(0, 32, width(), 26), Qt::AlignCenter, text());
}

void NavigationBarPushButton::setSelected(bool selected) {
    if (selected == isSelected_) {
        return;
    }

    isSelected_ = selected;
    isAboutSelected_ = false;

    if (iconAni_) {
        if (selected) {
            iconAni_->slideDown();
        } else {
            iconAni_->slideUp();
        }
    }

    update();
    emit selectedChanged(isSelected_);
}

// ============================================================================
// NavigationBar
// ============================================================================

NavigationBar::NavigationBar(QWidget* parent)
    : QWidget(parent),
      indicator_(new NavigationIndicator(this)),
      scrollArea_(new ScrollArea(this)),
      scrollWidget_(new QWidget()),
      vBoxLayout_(new QVBoxLayout(this)),
      topLayout_(new QVBoxLayout()),
      bottomLayout_(new QVBoxLayout()),
      scrollLayout_(new QVBoxLayout(scrollWidget_)),
      history_(new Router(this)) {
    initWidget();
}

void NavigationBar::initWidget() {
    resize(48, height());
    setAttribute(Qt::WA_StyledBackground);

    scrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    if (scrollArea_->horizontalScrollBar()) {
        scrollArea_->horizontalScrollBar()->setEnabled(false);
    }

    scrollArea_->setWidget(scrollWidget_);
    scrollArea_->setWidgetResizable(true);

    scrollWidget_->setObjectName(QStringLiteral("scrollWidget"));
    qfw::setStyleSheet(this, qfw::FluentStyleSheet::NavigationInterface);
    qfw::setStyleSheet(scrollWidget_, qfw::FluentStyleSheet::NavigationInterface);

    initLayout();

    connect(indicator_, &NavigationIndicator::aniFinished, this,
            &NavigationBar::onIndicatorAniFinished);
}

void NavigationBar::initLayout() {
    vBoxLayout_->setContentsMargins(0, 5, 0, 5);
    topLayout_->setContentsMargins(4, 0, 4, 0);
    bottomLayout_->setContentsMargins(4, 0, 4, 0);
    scrollLayout_->setContentsMargins(4, 0, 4, 0);

    vBoxLayout_->setSpacing(4);
    topLayout_->setSpacing(4);
    bottomLayout_->setSpacing(4);
    scrollLayout_->setSpacing(4);

    vBoxLayout_->addLayout(topLayout_, 0);
    vBoxLayout_->addWidget(scrollArea_, 0);
    vBoxLayout_->addLayout(bottomLayout_, 0);

    vBoxLayout_->setAlignment(Qt::AlignTop);
    topLayout_->setAlignment(Qt::AlignTop);
    scrollLayout_->setAlignment(Qt::AlignTop);
    bottomLayout_->setAlignment(Qt::AlignBottom);

    if (!scrollWidget_->layout()) {
        scrollWidget_->setLayout(scrollLayout_);
    }
}

NavigationWidget* NavigationBar::widget(const QString& routeKey) const {
    if (!items_.contains(routeKey)) {
        return nullptr;
    }
    return items_.value(routeKey);
}

NavigationBarPushButton* NavigationBar::addItem(const QString& routeKey, const QVariant& icon,
                                                const QString& text,
                                                const std::function<void()>& onClick,
                                                bool selectable, const QVariant& selectedIcon,
                                                qfw::NavigationItemPosition position) {
    return insertItem(-1, routeKey, icon, text, onClick, selectable, selectedIcon, position);
}

void NavigationBar::addWidget(const QString& routeKey, NavigationWidget* widget,
                              const std::function<void()>& onClick,
                              qfw::NavigationItemPosition position) {
    insertWidget(-1, routeKey, widget, onClick, position);
}

NavigationBarPushButton* NavigationBar::insertItem(int index, const QString& routeKey,
                                                   const QVariant& icon, const QString& text,
                                                   const std::function<void()>& onClick,
                                                   bool selectable, const QVariant& selectedIcon,
                                                   qfw::NavigationItemPosition position) {
    if (items_.contains(routeKey)) {
        return nullptr;
    }

    auto* w = new NavigationBarPushButton(icon, text, selectable, selectedIcon, this);
    w->setSelectedColor(lightSelectedColor_, darkSelectedColor_);
    w->setSelectedTextVisible(isSelectedTextVisible_);

    insertWidget(index, routeKey, w, onClick, position);
    return w;
}

void NavigationBar::insertWidget(int index, const QString& routeKey, NavigationWidget* widget,
                                 const std::function<void()>& onClick,
                                 qfw::NavigationItemPosition position) {
    if (!widget || items_.contains(routeKey)) {
        return;
    }

    connect(widget, &NavigationWidget::clicked, this, &NavigationBar::onWidgetClicked);
    if (onClick) {
        connect(widget, &NavigationWidget::clicked, this, [onClick](bool) { onClick(); });
    }

    widget->setProperty("routeKey", routeKey);
    items_.insert(routeKey, widget);

    if (position == qfw::NavigationItemPosition::Top) {
        widget->setParent(this);
        topLayout_->insertWidget(index, widget, 0, Qt::AlignTop | Qt::AlignHCenter);
    } else if (position == qfw::NavigationItemPosition::Scroll) {
        widget->setParent(scrollWidget_);
        scrollLayout_->insertWidget(index, widget, 0, Qt::AlignTop | Qt::AlignHCenter);
    } else {
        widget->setParent(this);
        bottomLayout_->insertWidget(index, widget, 0, Qt::AlignBottom | Qt::AlignHCenter);
    }

    widget->show();
}

void NavigationBar::removeWidget(const QString& routeKey) {
    if (!items_.contains(routeKey)) {
        return;
    }

    auto w = items_.take(routeKey);
    if (w) {
        w->deleteLater();
    }

    if (history_) {
        history_->remove(routeKey);
    }
}

NavigationWidget* NavigationBar::currentItem() const {
    if (currentRouteKey_.isEmpty()) {
        return nullptr;
    }

    return widget(currentRouteKey_);
}

void NavigationBar::setCurrentItem(const QString& routeKey) {
    if (!items_.contains(routeKey) || routeKey == currentRouteKey_) {
        return;
    }

    stopIndicatorAnimation();

    NavigationWidget* prevItem = currentItem();
    currentRouteKey_ = routeKey;

    if (!isIndicatorAnimationEnabled_ || !prevItem) {
        for (auto it = items_.begin(); it != items_.end(); ++it) {
            if (it.value()) {
                it.value()->setSelected(it.key() == routeKey);
            }
        }
        return;
    }

    NavigationWidget* newItem = currentItem();
    const QRectF preIndicatorRect = getIndicatorRect(prevItem);
    const QRectF newIndicatorRect = getIndicatorRect(newItem);

    prevItem->setSelected(false);
    newItem->setAboutSelected(true);

    indicator_->raise();
    indicator_->setIndicatorColor(newItem->lightIndicatorColor(), newItem->darkIndicatorColor());
    indicator_->startAnimation(preIndicatorRect, newIndicatorRect);
}

void NavigationBar::setSelectedTextVisible(bool visible) {
    if (visible == isSelectedTextVisible_) {
        return;
    }

    isSelectedTextVisible_ = visible;
    for (auto it = items_.begin(); it != items_.end(); ++it) {
        if (auto* btn = qobject_cast<NavigationBarPushButton*>(it.value().data())) {
            btn->setSelectedTextVisible(visible);
        }
    }
}

void NavigationBar::setSelectedColor(const QColor& light, const QColor& dark) {
    lightSelectedColor_ = light;
    darkSelectedColor_ = dark;

    for (auto it = items_.begin(); it != items_.end(); ++it) {
        if (auto* btn = qobject_cast<NavigationBarPushButton*>(it.value().data())) {
            btn->setSelectedColor(light, dark);
        }
    }
}

QRectF NavigationBar::getIndicatorRect(NavigationWidget* item) const {
    if (!item) {
        return QRectF();
    }

    const QPoint pos = item->mapTo(const_cast<NavigationBar*>(this), QPoint(0, 0));
    const QRectF rect = item->indicatorRect();
    return rect.translated(pos);
}

void NavigationBar::stopIndicatorAnimation() {
    if (!isIndicatorAnimationEnabled_) {
        return;
    }

    indicator_->stopAnimation();
    onIndicatorAniFinished();
}

void NavigationBar::onWidgetClicked(bool) {
    auto* w = qobject_cast<NavigationWidget*>(sender());
    if (!w) {
        return;
    }

    if (w->isSelectable()) {
        setCurrentItem(w->property("routeKey").toString());
    }
}

void NavigationBar::onIndicatorAniFinished() {
    NavigationWidget* item = currentItem();
    if (!item) {
        return;
    }

    item->setAboutSelected(false);
    item->setSelected(true);
    indicator_->hide();
}

}  // namespace qfw
