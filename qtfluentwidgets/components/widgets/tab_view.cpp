#include "components/widgets/tab_view.h"

#include <QApplication>
#include <QBrush>
#include <QFontMetrics>
#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QUuid>

#include "common/font.h"
#include "common/icon.h"
#include "common/style_sheet.h"

namespace qfw {

// ============================================================================
// TabToolButton
// ============================================================================

TabToolButton::TabToolButton(QWidget* parent) : TransparentToolButton(parent) { init(); }

TabToolButton::TabToolButton(FluentIconEnum icon, QWidget* parent)
    : TransparentToolButton(parent), iconEnum_(icon) {
    init();
}

void TabToolButton::init() {
    setFixedSize(32, 24);
    setIconSize(QSize(12, 12));
}

void TabToolButton::paintEvent(QPaintEvent* event) {
    // Call parent to draw background (hover/pressed states via QSS)
    TransparentToolButton::paintEvent(event);

    // Draw icon on top
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    if (!isEnabled()) {
        painter.setOpacity(0.43);
    } else if (isPressed) {
        painter.setOpacity(0.63);
    } else if (isHover) {
        painter.setOpacity(0.7);
    }

    const int w = iconSize().width();
    const int h = iconSize().height();
    const qreal y = (height() - h) / 2.0;
    const qreal x = (width() - w) / 2.0;
    const QRectF rect(x, y, w, h);
    drawIcon(&painter, rect, QIcon::Off);
}

void TabToolButton::drawIcon(QPainter* painter, const QRectF& rect, QIcon::State state) {
    QColor color = isDarkTheme() ? QColor(0xea, 0xea, 0xea) : QColor(0x48, 0x48, 0x48);

    FluentIcon icon(iconEnum_);
    icon.render(painter, rect.toRect(), Theme::Auto, QVariantMap{{"fill", color.name()}});
}

// ============================================================================
// TabItem
// ============================================================================

TabItem::TabItem(QWidget* parent) : FluentPushButton(parent) { init(); }

TabItem::TabItem(const QString& text, QWidget* parent) : FluentPushButton(text, parent) { init(); }

TabItem::TabItem(FluentIconEnum icon, const QString& text, QWidget* parent)
    : FluentPushButton(parent), iconEnum_(icon), hasIcon_(true) {
    setText(text);
    init();
}

void TabItem::init() {
    borderRadius_ = 5;
    isSelected_ = false;
    isShadowEnabled_ = true;
    closeButtonDisplayMode_ = TabCloseButtonDisplayMode::Always;

    lightSelectedBackgroundColor_ = QColor(249, 249, 249);
    darkSelectedBackgroundColor_ = QColor(40, 40, 40);

    closeButton_ = new TabToolButton(FluentIconEnum::Close, this);
    shadowEffect_ = new QGraphicsDropShadowEffect(this);
    slideAni_ = new QPropertyAnimation(this, "pos", this);

    qfw::setFont(this, 12);
    setFixedHeight(36);
    setMaximumWidth(240);
    setMinimumWidth(64);

    closeButton_->setIconSize(QSize(10, 10));
    closeButton_->show();  // Ensure close button is visible

    shadowEffect_->setBlurRadius(5);
    shadowEffect_->setOffset(0, 1);
    setGraphicsEffect(shadowEffect_);
    setSelected(false);

    connect(closeButton_, &QToolButton::clicked, this, &TabItem::closed);
}

void TabItem::setBorderRadius(int radius) {
    borderRadius_ = radius;
    update();
}

void TabItem::setSelected(bool isSelected) {
    isSelected_ = isSelected;

    shadowEffect_->setColor(QColor(0, 0, 0, 50 * (canShowShadow() ? 1 : 0)));
    update();

    if (isSelected) {
        raise();
    }

    if (closeButtonDisplayMode_ == TabCloseButtonDisplayMode::OnHover) {
        closeButton_->setVisible(isSelected);
    }
}

void TabItem::setShadowEnabled(bool isEnabled) {
    if (isEnabled == isShadowEnabled_) return;

    isShadowEnabled_ = isEnabled;
    shadowEffect_->setColor(QColor(0, 0, 0, 50 * (canShowShadow() ? 1 : 0)));
}

bool TabItem::canShowShadow() const { return isSelected_ && isShadowEnabled_; }

void TabItem::setCloseButtonDisplayMode(TabCloseButtonDisplayMode mode) {
    if (mode == closeButtonDisplayMode_) return;

    closeButtonDisplayMode_ = mode;

    if (mode == TabCloseButtonDisplayMode::Never) {
        closeButton_->hide();
    } else if (mode == TabCloseButtonDisplayMode::Always) {
        closeButton_->show();
    } else {
        closeButton_->setVisible(isHovered() || isSelected_);
    }
}

void TabItem::setRouteKey(const QString& key) { routeKey_ = key; }

void TabItem::setTextColor(const QColor& color) {
    textColor_ = color;
    update();
}

void TabItem::setSelectedBackgroundColor(const QColor& light, const QColor& dark) {
    lightSelectedBackgroundColor_ = light;
    darkSelectedBackgroundColor_ = dark;
    update();
}

void TabItem::setIconEnum(FluentIconEnum icon) {
    iconEnum_ = icon;
    hasIcon_ = true;
    update();
}

void TabItem::slideTo(int x, int duration) {
    slideAni_->setStartValue(pos());
    slideAni_->setEndValue(QPoint(x, y()));
    slideAni_->setDuration(duration);
    slideAni_->setEasingCurve(QEasingCurve::InOutQuad);
    slideAni_->start();
}

QSize TabItem::sizeHint() const { return QSize(maximumWidth(), 36); }

void TabItem::resizeEvent(QResizeEvent* event) {
    FluentPushButton::resizeEvent(event);
    closeButton_->move(width() - 6 - closeButton_->width(),
                       height() / 2 - closeButton_->height() / 2);
}

void TabItem::enterEvent(QEnterEvent* event) {
    FluentPushButton::enterEvent(event);
    if (closeButtonDisplayMode_ == TabCloseButtonDisplayMode::OnHover) {
        closeButton_->show();
    }
}

void TabItem::leaveEvent(QEvent* event) {
    FluentPushButton::leaveEvent(event);
    if (closeButtonDisplayMode_ == TabCloseButtonDisplayMode::OnHover && !isSelected_) {
        closeButton_->hide();
    }
}

void TabItem::mousePressEvent(QMouseEvent* event) {
    FluentPushButton::mousePressEvent(event);
    forwardMouseEvent(event);
}

void TabItem::mouseMoveEvent(QMouseEvent* event) {
    FluentPushButton::mouseMoveEvent(event);
    forwardMouseEvent(event);
}

void TabItem::mouseReleaseEvent(QMouseEvent* event) {
    FluentPushButton::mouseReleaseEvent(event);
    forwardMouseEvent(event);
}

void TabItem::mouseDoubleClickEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        emit doubleClicked();
    }
    FluentPushButton::mouseDoubleClickEvent(event);
}

void TabItem::forwardMouseEvent(QMouseEvent* event) {
    QPoint pos = mapToParent(event->pos());
    QMouseEvent* newEvent =
        new QMouseEvent(event->type(), pos, event->button(), event->buttons(), event->modifiers());
    QApplication::sendEvent(parent(), newEvent);
}

void TabItem::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    if (isSelected_) {
        drawSelectedBackground(&painter);
    } else {
        drawNotSelectedBackground(&painter);
    }

    // draw icon
    if (!isSelected_) {
        painter.setOpacity(isDarkTheme() ? 0.79 : 0.61);
    }

    if (hasIcon_) {
        FluentIcon icon(iconEnum_);
        icon.render(&painter, QRect(10, 10, 16, 16));
    }

    // draw text
    drawText(&painter);
}

void TabItem::drawSelectedBackground(QPainter* painter) {
    int w = width();
    int h = height();
    int r = borderRadius_;
    int d = 2 * r;

    bool isDark = isDarkTheme();

    // draw top border
    QPainterPath path;
    path.arcMoveTo(1, h - d - 1, d, d, 225);
    path.arcTo(1, h - d - 1, d, d, 225, -45);
    path.lineTo(1, r);
    path.arcTo(1, 1, d, d, -180, -90);
    path.lineTo(w - r, 1);
    path.arcTo(w - d - 1, 1, d, d, 90, -90);
    path.lineTo(w - 1, h - r);
    path.arcTo(w - d - 1, h - d - 1, d, d, 0, -45);

    QColor topBorderColor(0, 0, 0, 20);
    if (isDark) {
        if (isPressed) {
            topBorderColor = QColor(255, 255, 255, 18);
        } else if (isHover) {
            topBorderColor = QColor(255, 255, 255, 13);
        }
    } else {
        topBorderColor = QColor(0, 0, 0, 16);
    }

    painter->strokePath(path, topBorderColor);

    // draw bottom border
    path = QPainterPath();
    path.arcMoveTo(1, h - d - 1, d, d, 225);
    path.arcTo(1, h - d - 1, d, d, 225, 45);
    path.lineTo(w - r - 1, h - 1);
    path.arcTo(w - d - 1, h - d - 1, d, d, 270, 45);

    QColor bottomBorderColor = topBorderColor;
    if (!isDark) {
        bottomBorderColor = QColor(0, 0, 0, 63);
    }

    painter->strokePath(path, bottomBorderColor);

    // draw background
    painter->setPen(Qt::NoPen);
    QRect rect = this->rect().adjusted(1, 1, -1, -1);
    painter->setBrush(isDark ? darkSelectedBackgroundColor_ : lightSelectedBackgroundColor_);
    painter->drawRoundedRect(rect, r, r);
}

void TabItem::drawNotSelectedBackground(QPainter* painter) {
    if (!isPressed && !isHover) return;

    bool isDark = isDarkTheme();

    QColor color;
    if (isPressed) {
        color = isDark ? QColor(255, 255, 255, 12) : QColor(0, 0, 0, 7);
    } else {
        color = isDark ? QColor(255, 255, 255, 15) : QColor(0, 0, 0, 10);
    }

    painter->setBrush(color);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(rect().adjusted(1, 1, -1, -1), borderRadius_, borderRadius_);
}

void TabItem::drawText(QPainter* painter) {
    QFontMetrics fm(font());
    int tw = fm.boundingRect(text()).width();

    int dw;
    QRectF rect;

    if (hasIcon_) {
        dw = closeButton_->isVisible() ? 70 : 45;
        rect = QRectF(33, 0, width() - dw, height());
    } else {
        dw = closeButton_->isVisible() ? 47 : 20;
        rect = QRectF(10, 0, width() - dw, height());
    }

    QPen pen;
    QColor color = isDarkTheme() ? Qt::white : Qt::black;
    color = textColor_.isValid() ? textColor_ : color;
    int rw = static_cast<int>(rect.width());

    if (tw > rw) {
        QLinearGradient gradient(rect.x(), 0, tw + rect.x(), 0);
        gradient.setColorAt(0, color);
        gradient.setColorAt(qMax(0.0, (rw - 10.0) / tw), color);
        gradient.setColorAt(qMax(0.0, rw * 1.0 / tw), Qt::transparent);
        gradient.setColorAt(1, Qt::transparent);
        pen.setBrush(QBrush(gradient));
    } else {
        pen.setColor(color);
    }

    painter->setPen(pen);
    painter->setFont(font());
    painter->drawText(rect, Qt::AlignVCenter | Qt::AlignLeft, text());
}

// ============================================================================
// TabBar
// ============================================================================

TabBar::TabBar(QWidget* parent) : SingleDirectionScrollArea(parent, Qt::Horizontal) { init(); }

void TabBar::init() {
    currentIndex_ = -1;
    isMovable_ = false;
    isScrollable_ = false;
    isTabShadowEnabled_ = true;
    isDraging_ = false;
    tabMaxWidth_ = 240;
    tabMinWidth_ = 64;

    lightSelectedBackgroundColor_ = QColor(249, 249, 249);
    darkSelectedBackgroundColor_ = QColor(40, 40, 40);
    closeButtonDisplayMode_ = TabCloseButtonDisplayMode::Always;

    view_ = new QWidget(this);
    hBoxLayout_ = new QHBoxLayout(view_);
    itemLayout_ = new QHBoxLayout();
    widgetLayout_ = new QHBoxLayout();
    addButton_ = new TabToolButton(FluentIconEnum::Add, this);

    setFixedHeight(46);
    setWidget(view_);
    setWidgetResizable(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    hBoxLayout_->setSizeConstraint(QHBoxLayout::SetMaximumSize);

    connect(addButton_, &QToolButton::clicked, this, &TabBar::tabAddRequested);

    view_->setObjectName("view");
    qfw::setStyleSheet(this, FluentStyleSheet::TabView);
    qfw::setStyleSheet(view_, FluentStyleSheet::TabView);

    initLayout();
}

void TabBar::initLayout() {
    hBoxLayout_->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    itemLayout_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    widgetLayout_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    itemLayout_->setContentsMargins(5, 5, 5, 5);
    widgetLayout_->setContentsMargins(0, 0, 0, 0);
    hBoxLayout_->setContentsMargins(0, 0, 0, 0);

    itemLayout_->setSizeConstraint(QHBoxLayout::SetMinAndMaxSize);

    hBoxLayout_->setSpacing(0);
    itemLayout_->setSpacing(0);

    hBoxLayout_->addLayout(itemLayout_);
    hBoxLayout_->addSpacing(3);

    widgetLayout_->addWidget(addButton_, 0, Qt::AlignLeft);
    hBoxLayout_->addLayout(widgetLayout_);
    hBoxLayout_->addStretch(1);
}

void TabBar::setAddButtonVisible(bool visible) { addButton_->setVisible(visible); }

TabItem* TabBar::addTab(const QString& routeKey, const QString& text, FluentIconEnum icon) {
    return insertTab(-1, routeKey, text, icon);
}

TabItem* TabBar::insertTab(int index, const QString& routeKey, const QString& text,
                           FluentIconEnum icon) {
    if (itemMap_.contains(routeKey)) {
        qWarning("The route key `%s` is duplicated.", qPrintable(routeKey));
        return nullptr;
    }

    if (index == -1) {
        index = items_.size();
    }

    // adjust current index
    if (index <= currentIndex_ && currentIndex_ >= 0) {
        currentIndex_++;
    }

    TabItem* item = new TabItem(icon, text, view_);
    item->setRouteKey(routeKey);

    // set the size of tab
    int w = isScrollable_ ? tabMaxWidth_ : tabMinWidth_;
    item->setMinimumWidth(w);
    item->setMaximumWidth(tabMaxWidth_);

    item->setShadowEnabled(isTabShadowEnabled_);
    item->setCloseButtonDisplayMode(closeButtonDisplayMode_);
    item->setSelectedBackgroundColor(lightSelectedBackgroundColor_, darkSelectedBackgroundColor_);

    connect(item, &QPushButton::pressed, this, &TabBar::onItemPressed);
    connect(item, &TabItem::doubleClicked, this, [this, item]() {
        int idx = items_.indexOf(item);
        if (idx >= 0) emit tabBarDoubleClicked(idx);
    });
    connect(item, &TabItem::closed, this, [this, item]() {
        int idx = items_.indexOf(item);
        if (idx >= 0) emit tabCloseRequested(idx);
    });

    itemLayout_->insertWidget(index, item, 1);
    items_.insert(index, item);
    itemMap_[routeKey] = item;

    if (items_.size() == 1) {
        setCurrentIndex(0);
    }

    return item;
}

void TabBar::removeTab(int index) {
    if (!isIndexValid(index, items_.size())) {
        return;
    }

    // adjust current index
    if (index < currentIndex_) {
        currentIndex_--;
    } else if (index == currentIndex_) {
        if (currentIndex_ > 0) {
            setCurrentIndex(currentIndex_ - 1);
            emit currentChanged(currentIndex_);
        } else if (items_.size() == 1) {
            currentIndex_ = -1;
        } else {
            setCurrentIndex(1);
            currentIndex_ = 0;
            emit currentChanged(0);
        }
    }

    // remove tab
    TabItem* item = items_.takeAt(index);
    itemMap_.remove(item->routeKey());
    hBoxLayout_->removeWidget(item);
    item->deleteLater();

    // remove shadow
    update();
}

void TabBar::removeTabByKey(const QString& routeKey) {
    if (!itemMap_.contains(routeKey)) {
        return;
    }

    removeTab(items_.indexOf(itemMap_[routeKey]));
}

void TabBar::setCurrentIndex(int index) {
    if (index == currentIndex_) {
        return;
    }

    if (currentIndex_ >= 0 && currentIndex_ < items_.size()) {
        items_[currentIndex_]->setSelected(false);
    }

    currentIndex_ = index;
    if (index >= 0 && index < items_.size()) {
        items_[index]->setSelected(true);
    }
}

void TabBar::setCurrentTab(const QString& routeKey) {
    if (!itemMap_.contains(routeKey)) {
        return;
    }

    setCurrentIndex(items_.indexOf(itemMap_[routeKey]));
}

TabItem* TabBar::currentTab() const { return tabItem(currentIndex_); }

void TabBar::setCloseButtonDisplayMode(TabCloseButtonDisplayMode mode) {
    if (mode == closeButtonDisplayMode_) {
        return;
    }

    closeButtonDisplayMode_ = mode;
    for (TabItem* item : items_) {
        item->setCloseButtonDisplayMode(mode);
    }
}

TabItem* TabBar::tabItem(int index) const {
    if (!isIndexValid(index, items_.size())) {
        return nullptr;
    }
    return items_[index];
}

TabItem* TabBar::tab(const QString& routeKey) const { return itemMap_.value(routeKey, nullptr); }

QRect TabBar::tabRegion() const { return itemLayout_->geometry(); }

QRect TabBar::tabRect(int index) const {
    if (!isIndexValid(index, items_.size())) {
        return QRect();
    }

    int x = 0;
    for (int i = 0; i < index; ++i) {
        x += items_[i]->width();
    }

    QRect rect = items_[index]->geometry();
    rect.moveLeft(x);
    return rect;
}

QVariant TabBar::tabData(int index) const {
    TabItem* item = tabItem(index);
    return item ? item->property("data") : QVariant();
}

void TabBar::setTabData(int index, const QVariant& data) {
    TabItem* item = tabItem(index);
    if (item) {
        item->setProperty("data", data);
    }
}

QString TabBar::tabText(int index) const {
    TabItem* item = tabItem(index);
    return item ? item->text() : QString();
}

QIcon TabBar::tabIcon(int index) const {
    TabItem* item = tabItem(index);
    return item ? item->icon() : QIcon();
}

bool TabBar::isTabEnabled(int index) const {
    TabItem* item = tabItem(index);
    return item ? item->isEnabled() : false;
}

void TabBar::setTabEnabled(int index, bool enabled) {
    TabItem* item = tabItem(index);
    if (item) {
        item->setEnabled(enabled);
    }
}

void TabBar::setTabsClosable(bool closable) {
    if (closable) {
        setCloseButtonDisplayMode(TabCloseButtonDisplayMode::Always);
    } else {
        setCloseButtonDisplayMode(TabCloseButtonDisplayMode::Never);
    }
}

bool TabBar::tabsClosable() const {
    return closeButtonDisplayMode_ != TabCloseButtonDisplayMode::Never;
}

void TabBar::setTabIcon(int index, FluentIconEnum icon) {
    TabItem* item = tabItem(index);
    if (item) {
        item->setIconEnum(icon);
    }
}

void TabBar::setTabText(int index, const QString& text) {
    TabItem* item = tabItem(index);
    if (item) {
        item->setText(text);
    }
}

bool TabBar::isTabVisible(int index) const {
    TabItem* item = tabItem(index);
    return item ? item->isVisible() : false;
}

void TabBar::setTabVisible(int index, bool visible) {
    TabItem* item = tabItem(index);
    if (!item) return;

    item->setVisible(visible);

    if (visible && currentIndex_ < 0) {
        setCurrentIndex(0);
    } else if (!visible) {
        if (currentIndex_ > 0) {
            setCurrentIndex(currentIndex_ - 1);
            emit currentChanged(currentIndex_);
        } else if (items_.size() == 1) {
            currentIndex_ = -1;
        } else {
            setCurrentIndex(1);
            currentIndex_ = 0;
            emit currentChanged(0);
        }
    }
}

void TabBar::setTabTextColor(int index, const QColor& color) {
    TabItem* item = tabItem(index);
    if (item) {
        item->setTextColor(color);
    }
}

void TabBar::setTabToolTip(int index, const QString& toolTip) {
    TabItem* item = tabItem(index);
    if (item) {
        item->setToolTip(toolTip);
    }
}

QString TabBar::tabToolTip(int index) const {
    TabItem* item = tabItem(index);
    return item ? item->toolTip() : QString();
}

void TabBar::setTabSelectedBackgroundColor(const QColor& light, const QColor& dark) {
    lightSelectedBackgroundColor_ = light;
    darkSelectedBackgroundColor_ = dark;

    for (TabItem* item : items_) {
        item->setSelectedBackgroundColor(light, dark);
    }
}

void TabBar::setTabShadowEnabled(bool enabled) {
    if (enabled == isTabShadowEnabled_) {
        return;
    }

    isTabShadowEnabled_ = enabled;
    for (TabItem* item : items_) {
        item->setShadowEnabled(enabled);
    }
}

void TabBar::setMovable(bool movable) { isMovable_ = movable; }

void TabBar::setScrollable(bool scrollable) {
    isScrollable_ = scrollable;
    int w = scrollable ? tabMaxWidth_ : tabMinWidth_;
    for (TabItem* item : items_) {
        item->setMinimumWidth(w);
    }
}

void TabBar::setTabMaximumWidth(int width) {
    if (width == tabMaxWidth_) {
        return;
    }

    tabMaxWidth_ = width;
    for (TabItem* item : items_) {
        item->setMaximumWidth(width);
    }
}

void TabBar::setTabMinimumWidth(int width) {
    if (width == tabMinWidth_) {
        return;
    }

    tabMinWidth_ = width;

    if (!isScrollable_) {
        for (TabItem* item : items_) {
            item->setMinimumWidth(width);
        }
    }
}

void TabBar::clear() {
    while (count() > 0) {
        removeTab(count() - 1);
    }
}

void TabBar::onItemPressed() {
    TabItem* senderItem = qobject_cast<TabItem*>(sender());
    if (!senderItem) return;

    for (TabItem* item : items_) {
        item->setSelected(item == senderItem);
    }

    int index = items_.indexOf(senderItem);
    emit tabBarClicked(index);

    if (index != currentIndex_) {
        setCurrentIndex(index);
        emit currentChanged(index);
    }
}

void TabBar::adjustLayout() {
    QPropertyAnimation* ani = qobject_cast<QPropertyAnimation*>(sender());
    if (ani) {
        disconnect(ani, &QPropertyAnimation::finished, this, &TabBar::adjustLayout);
    }

    for (TabItem* item : items_) {
        itemLayout_->removeWidget(item);
    }

    for (TabItem* item : items_) {
        itemLayout_->addWidget(item);
    }
}

void TabBar::paintEvent(QPaintEvent* event) {
    SingleDirectionScrollArea::paintEvent(event);

    QPainter painter(viewport());
    painter.setRenderHints(QPainter::Antialiasing);

    // draw separators
    QColor color;
    if (isDarkTheme()) {
        color = QColor(255, 255, 255, 21);
    } else {
        color = QColor(0, 0, 0, 15);
    }

    painter.setPen(color);

    for (int i = 0; i < items_.size(); ++i) {
        TabItem* item = items_[i];
        bool canDraw = !(item->isHovered() || item->isSelected());
        if (i < items_.size() - 1) {
            TabItem* nextItem = items_[i + 1];
            if (nextItem->isHovered() || nextItem->isSelected()) {
                canDraw = false;
            }
        }

        if (canDraw) {
            int x = item->geometry().right();
            int y = height() / 2 - 8;
            painter.drawLine(x, y, x, y + 16);
        }
    }
}

void TabBar::mousePressEvent(QMouseEvent* event) {
    SingleDirectionScrollArea::mousePressEvent(event);

    if (!isMovable_ || event->button() != Qt::LeftButton ||
        !itemLayout_->geometry().contains(event->pos())) {
        return;
    }

    dragPos_ = event->pos();
}

void TabBar::mouseMoveEvent(QMouseEvent* event) {
    SingleDirectionScrollArea::mouseMoveEvent(event);

    if (!isMovable_ || count() <= 1 || !itemLayout_->geometry().contains(event->pos())) {
        return;
    }

    int index = currentIndex_;
    TabItem* item = tabItem(index);
    if (!item) return;

    int dx = event->pos().x() - dragPos_.x();
    dragPos_ = event->pos();

    // first tab can't move left
    if (index == 0 && dx < 0 && item->x() <= 0) {
        return;
    }

    // last tab can't move right
    if (index == count() - 1 && dx > 0 &&
        item->geometry().right() >= itemLayout_->sizeHint().width()) {
        return;
    }

    item->move(item->x() + dx, item->y());
    isDraging_ = true;

    // move the left sibling item to right
    if (dx < 0 && index > 0) {
        int siblingIndex = index - 1;
        if (item->x() < tabItem(siblingIndex)->geometry().center().x()) {
            swapItem(siblingIndex);
        }
    }

    // move the right sibling item to left
    if (dx > 0 && index < count() - 1) {
        int siblingIndex = index + 1;
        if (item->geometry().right() > tabItem(siblingIndex)->geometry().center().x()) {
            swapItem(siblingIndex);
        }
    }
}

void TabBar::mouseReleaseEvent(QMouseEvent* event) {
    SingleDirectionScrollArea::mouseReleaseEvent(event);

    if (!isMovable_ || !isDraging_) {
        return;
    }

    isDraging_ = false;

    TabItem* item = tabItem(currentIndex_);
    if (!item) return;

    int x = tabRect(currentIndex_).x();
    int duration = qAbs(item->x() - x) * 250 / item->width();
    item->slideTo(x, duration);
    connect(item->slideAni(), &QPropertyAnimation::finished, this, &TabBar::adjustLayout);
}

void TabBar::swapItem(int index) {
    TabItem* swappedItem = tabItem(index);
    if (!swappedItem) return;

    int x = tabRect(currentIndex_).x();

    int oldIndex = currentIndex_;
    items_.swapItemsAt(oldIndex, index);
    currentIndex_ = index;
    swappedItem->slideTo(x);

    emit tabMoved(oldIndex, index);
}

// ============================================================================
// TabWidget
// ============================================================================

TabWidget::TabWidget(QWidget* parent) : QWidget(parent) { init(); }

void TabWidget::init() {
    tabBar_ = new TabBar(this);
    stackedWidget_ = new QStackedWidget(this);
    vBoxLayout_ = new QVBoxLayout(this);

    vBoxLayout_->addWidget(tabBar_, 0, Qt::AlignTop);
    vBoxLayout_->addWidget(stackedWidget_, 1);
    vBoxLayout_->setSpacing(1);
    vBoxLayout_->setContentsMargins(0, 0, 0, 0);

    connectTabBarSignalToSlot();
}

void TabWidget::connectTabBarSignalToSlot() {
    connect(tabBar_, &TabBar::tabCloseRequested, this, &TabWidget::tabCloseRequested);
    connect(tabBar_, &TabBar::tabBarClicked, this, &TabWidget::tabBarClicked);
    connect(tabBar_, &TabBar::tabBarDoubleClicked, this, &TabWidget::tabBarDoubleClicked);
    connect(tabBar_, &TabBar::tabAddRequested, this, &TabWidget::tabAddRequested);
    connect(tabBar_, &TabBar::currentChanged, this, &TabWidget::onCurrentTabChanged);
    connect(tabBar_, &TabBar::tabMoved, this, &TabWidget::onTabMoved);
}

int TabWidget::addTab(QWidget* w, const QString& label, FluentIconEnum icon,
                      const QString& routeKey) {
    return insertTab(-1, w, label, icon, routeKey);
}

int TabWidget::insertTab(int index, QWidget* w, const QString& label, FluentIconEnum icon,
                         const QString& routeKey) {
    if (stackedWidget_->indexOf(w) >= 0) {
        return -1;
    }

    // generate unique route key
    QString key =
        routeKey.isEmpty() ? QUuid::createUuid().toString(QUuid::WithoutBraces) : routeKey;
    w->setProperty("routeKey", key);

    // create a new tab
    tabBar_->insertTab(index, key, label, icon);
    stackedWidget_->insertWidget(index, w);

    return stackedWidget_->indexOf(w);
}

void TabWidget::removeTab(int index) {
    if (!isIndexValid(index, stackedWidget_->count())) {
        return;
    }

    stackedWidget_->removeWidget(stackedWidget_->widget(index));
    tabBar_->removeTab(index);
}

void TabWidget::clear() {
    while (stackedWidget_->count()) {
        stackedWidget_->removeWidget(stackedWidget_->widget(0));
    }

    tabBar_->clear();
}

QWidget* TabWidget::widget(int index) const { return stackedWidget_->widget(index); }

QWidget* TabWidget::currentWidget() const { return stackedWidget_->currentWidget(); }

int TabWidget::currentIndex() const { return stackedWidget_->currentIndex(); }

void TabWidget::setTabBar(TabBar* tabBar) {
    if (tabBar == tabBar_) {
        return;
    }

    if (tabBar_) {
        vBoxLayout_->removeWidget(tabBar_);
        tabBar_->deleteLater();
        tabBar_->hide();
    }

    tabBar_ = tabBar;
    vBoxLayout_->insertWidget(0, tabBar_);
    connectTabBarSignalToSlot();
}

bool TabWidget::isMovable() const { return tabBar_->isMovable(); }

void TabWidget::setMovable(bool movable) { tabBar_->setMovable(movable); }

bool TabWidget::isTabEnabled(int index) const { return tabBar_->isTabEnabled(index); }

void TabWidget::setTabEnabled(int index, bool enabled) { tabBar_->setTabEnabled(index, enabled); }

bool TabWidget::isTabVisible(int index) const { return tabBar_->isTabVisible(index); }

void TabWidget::setTabVisible(int index, bool visible) { tabBar_->setTabVisible(index, visible); }

QString TabWidget::tabText(int index) const { return tabBar_->tabText(index); }

QIcon TabWidget::tabIcon(int index) const { return tabBar_->tabIcon(index); }

QString TabWidget::tabToolTip(int index) const { return tabBar_->tabToolTip(index); }

void TabWidget::setTabsClosable(bool closable) { tabBar_->setTabsClosable(closable); }

bool TabWidget::tabsClosable() const { return tabBar_->tabsClosable(); }

void TabWidget::setTabIcon(int index, FluentIconEnum icon) { tabBar_->setTabIcon(index, icon); }

void TabWidget::setTabText(int index, const QString& text) { tabBar_->setTabText(index, text); }

void TabWidget::setTabToolTip(int index, const QString& tip) { tabBar_->setTabToolTip(index, tip); }

void TabWidget::setTabTextColor(int index, const QColor& color) {
    tabBar_->setTabTextColor(index, color);
}

void TabWidget::setTabSelectedBackgroundColor(const QColor& light, const QColor& dark) {
    tabBar_->setTabSelectedBackgroundColor(light, dark);
}

void TabWidget::setTabShadowEnabled(bool enabled) { tabBar_->setTabShadowEnabled(enabled); }

void TabWidget::setScrollable(bool scrollable) { tabBar_->setScrollable(scrollable); }

bool TabWidget::isScrollable() const { return tabBar_->isScrollable(); }

void TabWidget::setTabMaximumWidth(int width) { tabBar_->setTabMaximumWidth(width); }

void TabWidget::setTabMinimumWidth(int width) { tabBar_->setTabMinimumWidth(width); }

int TabWidget::tabMaximumWidth() const { return tabBar_->tabMaximumWidth(); }

int TabWidget::tabMinimumWidth() const { return tabBar_->tabMinimumWidth(); }

QVariant TabWidget::tabData(int index) const { return tabBar_->tabData(index); }

void TabWidget::setTabData(int index, const QVariant& data) { tabBar_->setTabData(index, data); }

int TabWidget::count() const { return stackedWidget_->count(); }

void TabWidget::setCurrentIndex(int index) {
    tabBar_->setCurrentIndex(index);
    stackedWidget_->setCurrentIndex(index);
}

void TabWidget::setCurrentWidget(QWidget* w) {
    int index = stackedWidget_->indexOf(w);
    if (index != -1) {
        setCurrentIndex(index);
    }
}

void TabWidget::setCloseButtonDisplayMode(TabCloseButtonDisplayMode mode) {
    tabBar_->setCloseButtonDisplayMode(mode);
}

void TabWidget::onCurrentTabChanged(int index) {
    stackedWidget_->setCurrentIndex(index);
    emit currentChanged(index);
}

void TabWidget::onTabMoved(int fromIndex, int toIndex) {
    QWidget* w = stackedWidget_->widget(fromIndex);
    stackedWidget_->removeWidget(w);
    stackedWidget_->insertWidget(toIndex, w);
    stackedWidget_->setCurrentIndex(toIndex);
}

}  // namespace qfw
