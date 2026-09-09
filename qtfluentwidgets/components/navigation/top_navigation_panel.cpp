#include "components/navigation/top_navigation_panel.h"

#include <QFontMetrics>
#include <QPainter>
#include <QScrollBar>

#include "common/color.h"
#include "common/icon.h"
#include "common/router.h"
#include "common/style_sheet.h"
#include "components/widgets/info_badge.h"
#include "components/widgets/scroll_area.h"
#include "components/widgets/tool_tip.h"

namespace qfw {

// ============================================================================
// TopNavigationPushButton
// ============================================================================

TopNavigationPushButton::TopNavigationPushButton(const QVariant& icon, const QString& text,
                                                 bool isSelectable, QWidget* parent)
    : NavigationPushButton(icon, text, isSelectable, parent) {
    setFixedSize(40, 36);
}

void TopNavigationPushButton::setExpanded(bool expanded) {
    if (isExpanded_ == expanded) {
        return;
    }

    isExpanded_ = expanded;
    // When expanded, set isCompacted to False so paintEvent draws text
    if (expanded) {
        isCompacted_ = false;
    }
    updateSize();
    update();
}

void TopNavigationPushButton::setCompacted(bool compacted) {
    // Skip if expanded - expanded items always show text
    if (isExpanded_) {
        return;
    }

    if (isCompacted_ == compacted) {
        return;
    }

    isCompacted_ = compacted;
    updateSize();
    update();
}

void TopNavigationPushButton::updateSize() {
    if (isExpanded_ || !isCompacted_) {
        // Calculate width based on text
        QFontMetrics fm(font());
        int textWidth = fm.horizontalAdvance(text());
        int width = qMax(80, 44 + textWidth + 16);  // icon + text + padding
        setFixedSize(width, 36);
    } else {
        setFixedSize(40, 36);
    }
}

// ============================================================================
// TopNavigationPanel
// ============================================================================

TopNavigationPanel::TopNavigationPanel(QWidget* parent)
    : QFrame(parent),
      slideAni_(new ScaleSlideAnimation(this, Qt::Horizontal)),
      history_(new Router(this)) {
    initWidget();
    initLayout();
}

void TopNavigationPanel::initWidget() {
    setFixedHeight(48);
    setAttribute(Qt::WA_StyledBackground);

    // Create widgets
    scrollArea_ = new SingleDirectionScrollArea(this, Qt::Horizontal);
    scrollWidget_ = new QWidget();
    returnButton_ = new NavigationToolButton(QVariant::fromValue(static_cast<const FluentIconBase*>(
                                                 new FluentIcon(FluentIconEnum::Return))),
                                             this);

    // Create layouts
    hBoxLayout_ = new QHBoxLayout(this);
    leftLayout_ = new QHBoxLayout();
    centerLayout_ = new QHBoxLayout();
    rightLayout_ = new QHBoxLayout();
    scrollLayout_ = new QHBoxLayout(scrollWidget_);

    // Configure return button
    returnButton_->hide();
    returnButton_->setDisabled(true);
    returnButton_->installEventFilter(new ToolTipFilter(returnButton_, 1000));
    returnButton_->setToolTip(tr("Back"));

    // Configure scroll area
    scrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea_->setWidget(scrollWidget_);
    scrollArea_->setWidgetResizable(true);
    scrollArea_->enableTransparentBackground();

    connect(scrollArea_->horizontalScrollBar(), &QScrollBar::valueChanged, this,
            &TopNavigationPanel::onScrollChanged);

    scrollWidget_->setObjectName("scrollWidget");

    // Connect signals - use own Router instance
    connect(history_, &Router::emptyChanged, returnButton_, &NavigationToolButton::setDisabled);
    connect(returnButton_, &NavigationToolButton::clicked, history_, &Router::pop);
    connect(slideAni_, &ScaleSlideAnimation::valueChanged, this, [this]() { update(); });
    connect(slideAni_, &ScaleSlideAnimation::finished, this,
            &TopNavigationPanel::onIndicatorAniFinished);

    // Apply style sheet
    qfw::setStyleSheet(this, FluentStyleSheet::NavigationInterface);
    qfw::setStyleSheet(scrollWidget_, FluentStyleSheet::NavigationInterface);
}

void TopNavigationPanel::initLayout() {
    hBoxLayout_->setContentsMargins(12, 0, 12, 0);
    hBoxLayout_->setSpacing(0);

    leftLayout_->setContentsMargins(0, 0, 0, 0);
    centerLayout_->setContentsMargins(0, 0, 0, 0);
    rightLayout_->setContentsMargins(0, 0, 0, 0);
    scrollLayout_->setContentsMargins(0, 0, 0, 0);

    leftLayout_->setSpacing(4);
    centerLayout_->setSpacing(4);
    rightLayout_->setSpacing(4);
    scrollLayout_->setSpacing(4);

    hBoxLayout_->addLayout(leftLayout_);
    hBoxLayout_->addWidget(scrollArea_, 1);
    hBoxLayout_->addLayout(centerLayout_);
    hBoxLayout_->addLayout(rightLayout_);

    leftLayout_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    centerLayout_->setAlignment(Qt::AlignCenter);
    rightLayout_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    scrollLayout_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    leftLayout_->addWidget(returnButton_);
}

void TopNavigationPanel::setDisplayMode(TopNavigationDisplayMode mode) {
    if (displayMode_ == mode) {
        return;
    }

    displayMode_ = mode;
    emit displayModeChanged(mode);

    for (auto* item : items_) {
        item->setCompacted(mode == TopNavigationDisplayMode::Compact);
    }
}

void TopNavigationPanel::toggle() {
    if (isCompact()) {
        expand();
    } else {
        collapse();
    }
}

NavigationWidget* TopNavigationPanel::widget(const QString& routeKey) const {
    if (!items_.contains(routeKey)) {
        return nullptr;
    }
    return items_[routeKey];
}

NavigationWidget* TopNavigationPanel::currentItem() const {
    if (currentRouteKey_.isEmpty()) {
        return nullptr;
    }
    return widget(currentRouteKey_);
}

void TopNavigationPanel::setCurrentItem(const QString& routeKey) {
    if (!items_.contains(routeKey) || routeKey == currentRouteKey_) {
        return;
    }

    // Stop current animation and reset position
    adjustIndicatorPos();

    currentRouteKey_ = routeKey;

    // Update indicator color
    auto* newItem = currentItem();
    if (newItem) {
        lightIndicatorColor_ = newItem->lightIndicatorColor();
        darkIndicatorColor_ = newItem->darkIndicatorColor();

        if (newItem->parentWidget() == scrollWidget_) {
            scrollArea_->ensureWidgetVisible(newItem, 50, 0);
        }
    }

    // Start animation
    if (isIndicatorAnimationEnabled_) {
        slideAni_->startAnimation(currentIndicatorGeometry());
    }

    // Update selection state
    for (auto it = items_.constBegin(); it != items_.constEnd(); ++it) {
        it.value()->setSelected(it.key() == routeKey);
    }
}

void TopNavigationPanel::removeWidget(const QString& routeKey) {
    if (!items_.contains(routeKey)) {
        return;
    }

    auto* w = items_.take(routeKey);
    history_->remove(routeKey);
    w->deleteLater();
}

TopNavigationPushButton* TopNavigationPanel::addItem(const QString& routeKey, const QVariant& icon,
                                                     const QString& text,
                                                     std::function<void()> onClick, bool selectable,
                                                     TopNavigationItemPosition position,
                                                     const QString& tooltip, bool expanded) {
    return insertItem(-1, routeKey, icon, text, onClick, selectable, position, tooltip, expanded);
}

TopNavigationPushButton* TopNavigationPanel::insertItem(int index, const QString& routeKey,
                                                        const QVariant& icon, const QString& text,
                                                        std::function<void()> onClick,
                                                        bool selectable,
                                                        TopNavigationItemPosition position,
                                                        const QString& tooltip, bool expanded) {
    if (items_.contains(routeKey)) {
        return qobject_cast<TopNavigationPushButton*>(items_[routeKey]);
    }

    auto* w = new TopNavigationPushButton(icon, text, selectable, this);
    w->setCompacted(isCompact());
    if (expanded) {
        w->setExpanded(true);
    }

    insertWidget(index, routeKey, w, onClick, position, tooltip);
    return w;
}

void TopNavigationPanel::addWidget(const QString& routeKey, NavigationWidget* widget,
                                   std::function<void()> onClick,
                                   TopNavigationItemPosition position, const QString& tooltip) {
    insertWidget(-1, routeKey, widget, onClick, position, tooltip);
}

void TopNavigationPanel::insertWidget(int index, const QString& routeKey, NavigationWidget* widget,
                                      std::function<void()> onClick,
                                      TopNavigationItemPosition position, const QString& tooltip) {
    if (items_.contains(routeKey)) {
        return;
    }

    registerWidget(routeKey, widget, onClick, tooltip);
    insertWidgetToLayout(index, widget, position);
}

void TopNavigationPanel::registerWidget(const QString& routeKey, NavigationWidget* widget,
                                        std::function<void()> onClick, const QString& tooltip) {
    connect(widget, &NavigationWidget::clicked, this, &TopNavigationPanel::onWidgetClicked);

    if (onClick) {
        connect(widget, &NavigationWidget::clicked, [onClick](bool) { onClick(); });
    }

    widget->setProperty("routeKey", routeKey);
    items_[routeKey] = widget;

    if (!tooltip.isEmpty()) {
        widget->setToolTip(tooltip);
        widget->installEventFilter(new ToolTipFilter(widget, 1000));
    }
}

void TopNavigationPanel::insertWidgetToLayout(int index, NavigationWidget* widget,
                                              TopNavigationItemPosition position) {
    if (position == TopNavigationItemPosition::Left) {
        widget->setParent(scrollWidget_);
        scrollLayout_->insertWidget(index, widget, 0, Qt::AlignLeft | Qt::AlignVCenter);
    } else if (position == TopNavigationItemPosition::Center) {
        widget->setParent(this);
        centerLayout_->insertWidget(index, widget, 0, Qt::AlignCenter);
    } else if (position == TopNavigationItemPosition::Right) {
        widget->setParent(this);
        if (index < 0) {
            index = rightLayout_->count();
        }
        rightLayout_->insertWidget(index, widget, 0, Qt::AlignRight | Qt::AlignVCenter);
    } else {
        widget->setParent(scrollWidget_);
        scrollLayout_->insertWidget(index, widget, 0, Qt::AlignLeft | Qt::AlignVCenter);
    }

    widget->show();
}

void TopNavigationPanel::setItemExpanded(const QString& routeKey, bool expanded) {
    if (!items_.contains(routeKey)) {
        return;
    }

    auto* w = qobject_cast<TopNavigationPushButton*>(items_[routeKey]);
    if (w) {
        w->setExpanded(expanded);
    }
}

void TopNavigationPanel::setReturnButtonVisible(bool visible) {
    isReturnButtonVisible_ = visible;
    returnButton_->setVisible(visible);
}

int TopNavigationPanel::layoutMinWidth() const {
    int width = 24;  // margins
    for (auto* item : items_) {
        width += item->width() + 4;
    }
    return width;
}

void TopNavigationPanel::setIndicatorColor(const QColor& light, const QColor& dark) {
    lightIndicatorColor_ = light;
    darkIndicatorColor_ = dark;
    update();
}

void TopNavigationPanel::setIndicatorAnimationEnabled(bool enabled) {
    isIndicatorAnimationEnabled_ = enabled;
}

QRectF TopNavigationPanel::currentIndicatorGeometry() const {
    auto* item = currentItem();
    if (!item || !item->isVisible()) {
        return QRectF(0, height() - 6, 16, 3);
    }

    QPoint topLeft = item->mapTo(this, QPoint(0, 0));
    QRectF rect(topLeft.x(), topLeft.y(), item->width(), item->height());

    return QRectF(rect.x() - 8 + rect.width() / 2, height() - 9, 16, 3);
}

void TopNavigationPanel::onWidgetClicked(bool triggeredByUser) {
    Q_UNUSED(triggeredByUser);
    auto* w = qobject_cast<NavigationWidget*>(sender());
    if (w && w->isSelectable()) {
        setCurrentItem(w->property("routeKey").toString());
    }
}

void TopNavigationPanel::onIndicatorAniFinished() {
    // Empty implementation, matching Python version
}

void TopNavigationPanel::onScrollChanged() {
    adjustIndicatorPos();

    for (auto* item : items_) {
        InfoBadgeManager::updateForTarget(item);
    }
}

void TopNavigationPanel::adjustIndicatorPos() {
    auto* item = currentItem();
    if (item) {
        slideAni_->stopAnimation();
        slideAni_->setGeometry(currentIndicatorGeometry());
    }
}

void TopNavigationPanel::showEvent(QShowEvent* e) {
    QFrame::showEvent(e);
    adjustIndicatorPos();
}

void TopNavigationPanel::resizeEvent(QResizeEvent* e) {
    QFrame::resizeEvent(e);
    adjustIndicatorPos();
}

void TopNavigationPanel::paintEvent(QPaintEvent* e) {
    QFrame::paintEvent(e);

    if (!currentItem()) {
        return;
    }

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    // Use ThemeColorLight1 in dark mode for better visibility
    QColor indicatorColor = autoFallbackThemeColor(lightIndicatorColor_, darkIndicatorColor_);
    if (isDarkTheme() && !darkIndicatorColor_.isValid()) {
        indicatorColor = themedColor(indicatorColor, true, QStringLiteral("ThemeColorLight1"));
    }
    painter.setBrush(indicatorColor);
    painter.drawRoundedRect(slideAni_->getGeometry(), 1.5, 1.5);
}

}  // namespace qfw
