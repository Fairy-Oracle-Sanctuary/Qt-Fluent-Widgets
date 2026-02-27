#include "components/widgets/menu.h"

#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QCursor>
#include <QFontMetrics>
#include <QGraphicsEffect>
#include <QHoverEvent>
#include <QKeySequence>
#include <QListWidgetItem>
#include <QMimeData>
#include <QPainter>
#include <QProxyStyle>
#include <QRegion>
#include <QScreen>
#include <QStyleFactory>

#include "common/color.h"
#include "common/font.h"
#include "common/screen.h"
#include "common/style_sheet.h"

namespace qfw {

// ============================================================================
// CustomMenuStyle
// ============================================================================
CustomMenuStyle::CustomMenuStyle(int iconSize) : QProxyStyle(), iconSize_(iconSize) {}

int CustomMenuStyle::pixelMetric(PixelMetric metric, const QStyleOption* option,
                                 const QWidget* widget) const {
    if (metric == QStyle::PM_SmallIconSize) {
        return iconSize_;
    }
    return QProxyStyle::pixelMetric(metric, option, widget);
}

// ============================================================================
// SubMenuItemWidget
// ============================================================================
SubMenuItemWidget::SubMenuItemWidget(RoundMenu* menu, QListWidgetItem* item, QWidget* parent)
    : QWidget(parent), menu_(menu), item_(item) {
    setMouseTracking(true);
}

void SubMenuItemWidget::enterEvent(QEnterEvent* e) {
    QWidget::enterEvent(e);
    emit showMenuSig(item_);
}

// ============================================================================
// MenuItemDelegate
// ============================================================================
MenuItemDelegate::MenuItemDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

bool MenuItemDelegate::isSeparator(const QModelIndex& index) const {
    return index.model()->data(index, Qt::DecorationRole).toString() == QStringLiteral("seperator");
}

void MenuItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                             const QModelIndex& index) const {
    if (!isSeparator(index)) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    if (!painter) {
        return;
    }

    painter->save();

    const int c = isDarkTheme() ? 255 : 0;
    QPen pen(QColor(c, c, c, 25), 1);
    pen.setCosmetic(true);
    painter->setPen(pen);

    const QRect rect = option.rect;
    painter->drawLine(0, rect.y() + 4, rect.width() + 12, rect.y() + 4);

    painter->restore();
}

bool MenuItemDelegate::helpEvent(QHelpEvent* event, QAbstractItemView* view,
                                 const QStyleOptionViewItem& option, const QModelIndex& index) {
    if (!tooltipDelegate_) {
        tooltipDelegate_ = new ItemViewToolTipDelegate(view, 100, ItemViewToolTipType::List);
    }
    return tooltipDelegate_->helpEvent(event, view, option, index);
}

// ============================================================================
// ShortcutMenuItemDelegate
// ============================================================================
void ShortcutMenuItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                     const QModelIndex& index) const {
    MenuItemDelegate::paint(painter, option, index);

    if (isSeparator(index) || !painter) {
        return;
    }

    QAction* action = reinterpret_cast<QAction*>(index.data(Qt::UserRole).value<quintptr>());
    if (!action || action->shortcut().isEmpty()) {
        return;
    }

    painter->save();

    if (!(option.state & QStyle::State_Enabled)) {
        painter->setOpacity(isDarkTheme() ? 0.5 : 0.6);
    }

    const QFont font = getFont(12);
    painter->setFont(font);
    painter->setPen(isDarkTheme() ? QColor(255, 255, 255, 200) : QColor(0, 0, 0, 153));

    const QFontMetrics fm(font);
    const QString shortcut = action->shortcut().toString(QKeySequence::NativeText);
    const int sw = fm.horizontalAdvance(shortcut);

    painter->translate(option.rect.width() - sw - 20, 0);

    const QRectF rect(0, option.rect.y(), sw, option.rect.height());
    painter->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, shortcut);

    painter->restore();
}

// ============================================================================
// IndicatorMenuItemDelegate
// ============================================================================
void IndicatorMenuItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                      const QModelIndex& index) const {
    MenuItemDelegate::paint(painter, option, index);

    if (!(option.state & QStyle::State_Selected) || !painter) {
        return;
    }

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform |
                            QPainter::TextAntialiasing);

    painter->setPen(Qt::NoPen);
    // Use ThemeColorLight1 in dark mode for better visibility
    QColor indicatorColor =
        isDarkTheme() ? themedColor(themeColor(), true, QStringLiteral("ThemeColorLight1"))
                      : themeColor();
    painter->setBrush(indicatorColor);
    painter->drawRoundedRect(6, 11 + option.rect.y(), 3, 15, 1.5, 1.5);

    painter->restore();
}

// ============================================================================
// CheckableMenuItemDelegate
// ============================================================================
void CheckableMenuItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                      const QModelIndex& index) const {
    ShortcutMenuItemDelegate::paint(painter, option, index);

    if (isSeparator(index) || !painter) {
        return;
    }

    // Draw indicator
    QAction* action = reinterpret_cast<QAction*>(index.data(Qt::UserRole).value<quintptr>());
    if (!action || !action->isChecked()) {
        return;
    }

    painter->save();
    drawIndicator(painter, option, index);
    painter->restore();
}

// ============================================================================
// RadioIndicatorMenuItemDelegate
// ============================================================================
void RadioIndicatorMenuItemDelegate::drawIndicator(QPainter* painter,
                                                   const QStyleOptionViewItem& option,
                                                   const QModelIndex& index) const {
    Q_UNUSED(index);
    const QRect rect = option.rect;
    const int r = 5;
    const int x = rect.x() + 22;
    const int y = rect.center().y() - r / 2;

    painter->setRenderHints(QPainter::Antialiasing);
    if (!(option.state & QStyle::State_MouseOver)) {
        painter->setOpacity(isDarkTheme() ? 0.75 : 0.65);
    }

    painter->setPen(Qt::NoPen);
    painter->setBrush(isDarkTheme() ? Qt::white : Qt::black);
    painter->drawEllipse(QRectF(x, y, r, r));
}

// ============================================================================
// CheckIndicatorMenuItemDelegate
// ============================================================================
void CheckIndicatorMenuItemDelegate::drawIndicator(QPainter* painter,
                                                   const QStyleOptionViewItem& option,
                                                   const QModelIndex& index) const {
    Q_UNUSED(index);
    const QRect rect = option.rect;
    const int s = 11;
    const int x = rect.x() + 19;
    const int y = rect.center().y() - s / 2;

    painter->setRenderHints(QPainter::Antialiasing);
    if (!(option.state & QStyle::State_MouseOver)) {
        painter->setOpacity(0.75);
    }

    // Use theme color to draw check mark
    painter->setPen(QPen(themeColor(), 2));
    painter->setBrush(Qt::NoBrush);

    // Draw a simple check mark
    const QPointF points[3] = {QPointF(x + 2, y + s / 2), QPointF(x + s / 3, y + s - 3),
                               QPointF(x + s - 2, y + 3)};
    painter->drawPolyline(points, 3);
}

// ============================================================================
// createCheckableMenuItemDelegate
// ============================================================================
MenuItemDelegate* createCheckableMenuItemDelegate(MenuIndicatorType style, QObject* parent) {
    switch (style) {
        case MenuIndicatorType::Radio:
            return new RadioIndicatorMenuItemDelegate(parent);
        case MenuIndicatorType::Check:
            return new CheckIndicatorMenuItemDelegate(parent);
    }
    return nullptr;
}

// ============================================================================
// MenuActionListWidget
// ============================================================================
MenuActionListWidget::MenuActionListWidget(QWidget* parent) : QListWidget(parent) {
    setProperty("qssClass", "MenuActionListWidget");

    viewportMargins_ = QMargins(0, 6, 0, 6);
    setViewportMargins(viewportMargins_);
    setTextElideMode(Qt::ElideNone);
    setDragEnabled(false);
    setMouseTracking(true);
    setVerticalScrollMode(ScrollMode::ScrollPerPixel);
    setIconSize(QSize(14, 14));

    setItemDelegate(new ShortcutMenuItemDelegate(this));

    scrollDelegate_ = new SmoothScrollDelegate(this);

    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void MenuActionListWidget::setViewportMargins(int left, int top, int right, int bottom) {
    setViewportMargins(QMargins(left, top, right, bottom));
}

void MenuActionListWidget::setViewportMargins(const QMargins& margins) {
    viewportMargins_ = margins;
    QListWidget::setViewportMargins(margins);
}

void MenuActionListWidget::insertItem(int row, QListWidgetItem* item) {
    QListWidget::insertItem(row, item);
    adjustSizeForMenu();
}

void MenuActionListWidget::addItem(QListWidgetItem* item) {
    QListWidget::addItem(item);
    adjustSizeForMenu();
}

QListWidgetItem* MenuActionListWidget::takeItem(int row) {
    QListWidgetItem* item = QListWidget::takeItem(row);
    adjustSizeForMenu();
    return item;
}

void MenuActionListWidget::setItemHeight(int height) {
    if (height == itemHeight_) {
        return;
    }

    for (int i = 0; i < count(); ++i) {
        QListWidgetItem* item = this->item(i);
        if (!itemWidget(item)) {
            item->setSizeHint(QSize(item->sizeHint().width(), height));
        }
    }

    itemHeight_ = height;
    adjustSizeForMenu();
}

void MenuActionListWidget::setMaxVisibleItems(int num) {
    maxVisibleItems_ = num;
    adjustSizeForMenu();
}

int MenuActionListWidget::maxVisibleItems() const { return maxVisibleItems_; }

int MenuActionListWidget::itemsHeight() const {
    const int n = (maxVisibleItems_ < 0) ? count() : qMin(maxVisibleItems_, count());
    int h = 0;
    for (int i = 0; i < n; ++i) {
        h += item(i)->sizeHint().height();
    }
    const QMargins m = viewportMargins_;
    return h + m.top() + m.bottom();
}

int MenuActionListWidget::heightForAnimation(const QPoint& pos, MenuAnimationType aniType) const {
    Q_UNUSED(aniType);

    const int ih = itemsHeight();

    QSize avail = QSize();
    const QRect ss = getCurrentScreenGeometry(true);
    int w = ss.width() - 100;
    int h = ss.height() - 100;

    if (aniType == MenuAnimationType::DropDown || aniType == MenuAnimationType::FadeInDropDown) {
        h = qMax(ss.bottom() - pos.y() - 10, 1);
    } else if (aniType == MenuAnimationType::PullUp || aniType == MenuAnimationType::FadeInPullUp) {
        h = qMax(pos.y() - ss.top() - 28, 1);
    }

    avail = QSize(w, h);
    return qMin(ih, avail.height());
}

void MenuActionListWidget::adjustSizeForMenu(const QPoint& pos, MenuAnimationType aniType) {
    QSize size;
    for (int i = 0; i < count(); ++i) {
        const QSize s = item(i)->sizeHint();
        size.setWidth(qMax(qMax(s.width(), size.width()), 1));
        size.setHeight(qMax(1, size.height() + s.height()));
    }

    const QRect ss = getCurrentScreenGeometry(true);
    int availW = ss.width() - 100;
    int availH = ss.height() - 100;

    if (aniType == MenuAnimationType::DropDown || aniType == MenuAnimationType::FadeInDropDown) {
        availH = qMax(ss.bottom() - pos.y() - 10, 1);
    } else if (aniType == MenuAnimationType::PullUp || aniType == MenuAnimationType::FadeInPullUp) {
        availH = qMax(pos.y() - ss.top() - 28, 1);
    }

    const QMargins m = viewportMargins_;
    size += QSize(m.left() + m.right() + 2, m.top() + m.bottom());
    size.setHeight(qMin(availH, size.height() + 3));
    size.setWidth(qMax(qMin(availW, size.width()), minimumWidth()));

    if (maxVisibleItems_ > 0) {
        size.setHeight(
            qMin(size.height(), maxVisibleItems_ * itemHeight_ + m.top() + m.bottom() + 3));
    }

    setFixedSize(size);
}

// ============================================================================
// RoundMenu
// ============================================================================
RoundMenu::RoundMenu(const QString& title, QWidget* parent) : QMenu(parent) {
    setTitle(title);
    initWidgets();
}

QSize RoundMenu::sizeHint() const {
    if (!view_ || !layout()) {
        return QMenu::sizeHint();
    }
    const QMargins m = layout()->contentsMargins();
    return QSize(view_->width() + m.left() + m.right(), view_->height() + m.top() + m.bottom());
}

void RoundMenu::popup(const QPoint& pos, QAction* at) {
    Q_UNUSED(at);
    execAt(pos, true, MenuAnimationType::DropDown);
}

void RoundMenu::initWidgets() {
    setProperty("qssClass", "RoundMenu");

    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);

    setStyle(QStyleFactory::create(QStringLiteral("fusion")));

    timer_ = new QTimer(this);
    timer_->setSingleShot(true);
    timer_->setInterval(400);
    connect(timer_, &QTimer::timeout, this, &RoundMenu::onShowMenuTimeout);

    hBoxLayout_ = new QHBoxLayout(this);
    panel_ = new QWidget(this);
    panel_->setObjectName(QStringLiteral("menuPanel"));
    panelLayout_ = new QHBoxLayout(panel_);
    panelLayout_->setContentsMargins(0, 0, 0, 0);

    view_ = new MenuActionListWidget(panel_);
    view_->setProperty("transparent", true);

    setShadowEffect();

    panelLayout_->addWidget(view_, 1);
    hBoxLayout_->addWidget(panel_, 1, Qt::AlignCenter);
    hBoxLayout_->setContentsMargins(12, 8, 12, 20);

    qfw::setStyleSheet(this, qfw::FluentStyleSheet::Menu);

    connect(view_, &QListWidget::itemClicked, this, &RoundMenu::onItemClicked);
    connect(view_, &QListWidget::itemEntered, this, &RoundMenu::onItemEntered);
}

void RoundMenu::setMaxVisibleItems(int num) {
    if (view_) {
        view_->setMaxVisibleItems(num);
    }
    adjustSize();
}

void RoundMenu::setItemHeight(int height) {
    if (height == itemHeight_) {
        return;
    }
    itemHeight_ = height;
    if (view_) {
        view_->setItemHeight(height);
    }
}

void RoundMenu::setShadowEffect(int blurRadius, const QPoint& offset, const QColor& color) {
    if (!panel_) {
        return;
    }

    shadowEffect_ = new QGraphicsDropShadowEffect(panel_);
    shadowEffect_->setBlurRadius(blurRadius);
    shadowEffect_->setOffset(offset);
    shadowEffect_->setColor(color);

    panel_->setGraphicsEffect(nullptr);
    panel_->setGraphicsEffect(shadowEffect_);
}

void RoundMenu::adjustSize() {
    if (!view_) {
        return;
    }
    const QMargins m = layout()->contentsMargins();
    const int w = view_->width() + m.left() + m.right();
    const int h = view_->height() + m.top() + m.bottom();
    setFixedSize(w, h);
}

void RoundMenu::clear() {
    while (!actions_.isEmpty()) {
        QAction* a = actions_.last();
        removeAction(a);
    }

    while (!subMenus_.isEmpty()) {
        RoundMenu* m = subMenus_.last();
        removeMenu(m);
    }
}

void RoundMenu::setIcon(const QIcon& icon) { icon_ = icon; }

QIcon RoundMenu::icon() const { return icon_; }

void RoundMenu::setTitle(const QString& title) {
    title_ = title;
    QMenu::setTitle(title);
}

QString RoundMenu::title() const { return title_; }

QList<QAction*> RoundMenu::menuActions() const {
    QList<QAction*> out;
    out.reserve(actions_.size());
    for (auto a : actions_) {
        if (a) {
            out.push_back(a);
        }
    }
    return out;
}

bool RoundMenu::hasItemIcon() const {
    for (auto a : actions_) {
        if (a && !a->icon().isNull()) {
            return true;
        }
    }
    for (auto m : subMenus_) {
        if (m && !m->icon().isNull()) {
            return true;
        }
    }
    return false;
}

QIcon RoundMenu::createItemIcon(QObject* w) const {
    const bool hasIcon = hasItemIcon();

    QIcon icon;
    if (auto* act = qobject_cast<QAction*>(w)) {
        icon = act->icon();
    } else if (auto* menu = qobject_cast<RoundMenu*>(w)) {
        icon = menu->icon();
    }

    if (hasIcon && icon.isNull()) {
        QPixmap pm(view_ ? view_->iconSize() : QSize(14, 14));
        pm.fill(Qt::transparent);
        return QIcon(pm);
    }

    if (!hasIcon) {
        return QIcon();
    }

    return icon;
}

int RoundMenu::longestShortcutWidth() const {
    const QFontMetrics fm(getFont(12));

    int maxW = 0;
    for (auto a : actions_) {
        if (!a) {
            continue;
        }
        const int w = fm.horizontalAdvance(a->shortcut().toString(QKeySequence::NativeText));
        maxW = qMax(maxW, w);
    }
    return maxW;
}

int RoundMenu::adjustItemText(QListWidgetItem* item, QAction* action) {
    if (!item || !action || !view_) {
        return 0;
    }

    int sw = 0;
    if (qobject_cast<ShortcutMenuItemDelegate*>(view_->itemDelegate())) {
        sw = longestShortcutWidth();
        if (sw) {
            sw += 22;
        }
    }

    int w = 0;
    if (!hasItemIcon()) {
        item->setText(action->text());
        w = 40 + view_->fontMetrics().boundingRect(action->text()).width() + sw;
    } else {
        item->setText(QStringLiteral(" ") + action->text());
        const int space = 4 - view_->fontMetrics().boundingRect(QStringLiteral(" ")).width();
        w = 60 + view_->fontMetrics().boundingRect(item->text()).width() + sw + space;
    }

    item->setSizeHint(QSize(w, itemHeight_));
    return w;
}

QListWidgetItem* RoundMenu::createActionItem(QAction* action, QAction* before) {
    if (!action || !view_) {
        return nullptr;
    }

    if (!before) {
        actions_.append(action);
    } else {
        const int idx = actions_.indexOf(before);
        if (idx < 0) {
            return nullptr;
        }
        actions_.insert(idx, action);
    }

    auto* item = new QListWidgetItem(createItemIcon(action), action->text());
    adjustItemText(item, action);

    if (!action->isEnabled()) {
        item->setFlags(Qt::NoItemFlags);
    }

    if (action->text() != action->toolTip()) {
        item->setToolTip(action->toolTip());
    }

    item->setData(Qt::UserRole, QVariant::fromValue<quintptr>(reinterpret_cast<quintptr>(action)));
    action->setProperty("item", QVariant::fromValue<quintptr>(reinterpret_cast<quintptr>(item)));

    connect(action, &QAction::changed, this, &RoundMenu::onActionChanged);

    return item;
}

void RoundMenu::addAction(QAction* action) {
    QListWidgetItem* item = createActionItem(action);
    if (!item) {
        return;
    }

    view_->addItem(item);
    adjustSize();
}

void RoundMenu::insertAction(QAction* before, QAction* action) {
    if (!before || !action || !view_) {
        return;
    }

    const int idx = actions_.indexOf(before);
    if (idx < 0) {
        return;
    }

    auto* beforeItem =
        reinterpret_cast<QListWidgetItem*>(before->property("item").value<quintptr>());
    if (!beforeItem) {
        return;
    }

    QListWidgetItem* item = createActionItem(action, before);
    if (!item) {
        return;
    }

    view_->insertItem(view_->row(beforeItem), item);
    adjustSize();
}

void RoundMenu::addActions(const QList<QAction*>& actions) {
    for (QAction* a : actions) {
        addAction(a);
    }
}

void RoundMenu::insertActions(QAction* before, const QList<QAction*>& actions) {
    for (QAction* a : actions) {
        insertAction(before, a);
    }
}

void RoundMenu::removeItem(QListWidgetItem* item) {
    if (!item || !view_) {
        return;
    }

    const int row = view_->row(item);
    QListWidgetItem* taken = view_->takeItem(row);
    if (taken) {
        taken->setData(Qt::UserRole, QVariant());

        if (QWidget* w = view_->itemWidget(taken)) {
            w->deleteLater();
        }

        delete taken;
    }
}

void RoundMenu::removeAction(QAction* action) {
    if (!action) {
        return;
    }

    const int idx = actions_.indexOf(action);
    if (idx < 0) {
        return;
    }

    auto* item = reinterpret_cast<QListWidgetItem*>(action->property("item").value<quintptr>());

    actions_.removeAt(idx);
    action->setProperty("item", QVariant());

    if (item) {
        removeItem(item);
    }
    adjustSize();
}

void RoundMenu::addWidget(QWidget* widget, bool selectable, const std::function<void()>& onClick) {
    if (!widget || !view_) {
        return;
    }

    auto* action = new QAction(this);
    action->setProperty("selectable", selectable);

    QListWidgetItem* item = createActionItem(action);
    if (!item) {
        return;
    }

    item->setSizeHint(widget->size());
    view_->addItem(item);
    view_->setItemWidget(item, widget);

    if (!selectable) {
        item->setFlags(Qt::NoItemFlags);
    }

    if (onClick) {
        connect(action, &QAction::triggered, this, [onClick]() { onClick(); });
    }

    adjustSize();
}

void RoundMenu::setParentMenu(RoundMenu* parent, QListWidgetItem* item) {
    parentMenu_ = parent;
    menuItem_ = item;
    isSubMenu_ = (parent != nullptr);
}

QPair<QListWidgetItem*, QWidget*> RoundMenu::createSubMenuItem(RoundMenu* menu) {
    if (!menu || !view_) {
        return {nullptr, nullptr};
    }

    subMenus_.append(menu);

    auto* item = new QListWidgetItem(createItemIcon(menu), menu->title());

    int w = 0;
    if (!hasItemIcon()) {
        w = 60 + view_->fontMetrics().boundingRect(menu->title()).width();
    } else {
        item->setText(QStringLiteral(" ") + item->text());
        w = 72 + view_->fontMetrics().boundingRect(item->text()).width();
    }

    menu->setParentMenu(this, item);
    item->setSizeHint(QSize(w, itemHeight_));
    item->setData(Qt::UserRole, QVariant::fromValue<quintptr>(reinterpret_cast<quintptr>(menu)));

    auto* wgt = new SubMenuItemWidget(menu, item, this);
    connect(wgt, &SubMenuItemWidget::showMenuSig, this, &RoundMenu::showSubMenu);
    wgt->resize(item->sizeHint());

    return {item, wgt};
}

void RoundMenu::addMenu(RoundMenu* menu) {
    if (!menu || !view_) {
        return;
    }

    auto pair = createSubMenuItem(menu);
    if (!pair.first || !pair.second) {
        return;
    }

    view_->addItem(pair.first);
    view_->setItemWidget(pair.first, pair.second);
    adjustSize();
}

void RoundMenu::insertMenu(QAction* before, RoundMenu* menu) {
    if (!before || !menu || !view_) {
        return;
    }

    if (!actions_.contains(before)) {
        return;
    }

    auto* beforeItem =
        reinterpret_cast<QListWidgetItem*>(before->property("item").value<quintptr>());
    if (!beforeItem) {
        return;
    }

    auto pair = createSubMenuItem(menu);
    if (!pair.first || !pair.second) {
        return;
    }

    view_->insertItem(view_->row(beforeItem), pair.first);
    view_->setItemWidget(pair.first, pair.second);
    adjustSize();
}

void RoundMenu::removeMenu(RoundMenu* menu) {
    if (!menu || !view_) {
        return;
    }

    const int idx = subMenus_.indexOf(menu);
    if (idx < 0) {
        return;
    }

    QListWidgetItem* item = menu->menuItem_;
    subMenus_.removeAt(idx);

    if (item) {
        removeItem(item);
    }

    adjustSize();
}

void RoundMenu::addSeparator() {
    if (!view_) {
        return;
    }

    const QMargins m = view_->menuViewportMargins();
    const int w = view_->width() - m.left() - m.right();

    auto* item = new QListWidgetItem();
    item->setFlags(Qt::NoItemFlags);
    item->setSizeHint(QSize(w, 9));
    view_->addItem(item);
    item->setData(Qt::DecorationRole, QStringLiteral("seperator"));

    adjustSize();
}

void RoundMenu::onItemClicked(QListWidgetItem* item) {
    if (!item || !view_) {
        return;
    }

    QAction* action = reinterpret_cast<QAction*>(item->data(Qt::UserRole).value<quintptr>());
    if (!action || !actions_.contains(action) || !action->isEnabled()) {
        return;
    }

    if (view_->itemWidget(item) && !action->property("selectable").toBool()) {
        return;
    }

    hideMenu(false);

    if (!isSubMenu_) {
        action->trigger();
        return;
    }

    closeParentMenu();
    action->trigger();
}

void RoundMenu::closeParentMenu() {
    RoundMenu* menu = this;
    while (menu) {
        menu->close();
        menu = menu->parentMenu_;
    }
}

void RoundMenu::onItemEntered(QListWidgetItem* item) {
    lastHoverItem_ = item;

    if (!item) {
        return;
    }

    auto* subMenu = reinterpret_cast<RoundMenu*>(item->data(Qt::UserRole).value<quintptr>());
    if (!subMenu) {
        return;
    }

    showSubMenu(item);
}

void RoundMenu::showSubMenu(QListWidgetItem* item) {
    lastHoverItem_ = item;
    lastHoverSubMenuItem_ = item;

    if (timer_) {
        timer_->stop();
        timer_->start();
    }
}

void RoundMenu::onShowMenuTimeout() {
    if (!lastHoverSubMenuItem_ || lastHoverItem_ != lastHoverSubMenuItem_) {
        return;
    }

    if (!view_) {
        return;
    }

    QWidget* w = view_->itemWidget(lastHoverSubMenuItem_);
    auto* subItem = qobject_cast<SubMenuItemWidget*>(w);
    if (!subItem) {
        return;
    }

    RoundMenu* menu = subItem->menu();
    if (!menu) {
        return;
    }

    if (menu->parentMenu_ && menu->parentMenu_->isHidden()) {
        return;
    }

    const QRect itemRect(QPoint(w->mapToGlobal(w->rect().topLeft())), w->size());

    int x = itemRect.right() + 5;
    int y = itemRect.y() - 5;

    const QRect screenRect = getCurrentScreenGeometry(true);
    const QSize subMenuSize = menu->sizeHint();

    if ((x + subMenuSize.width()) > screenRect.right()) {
        x = qMax(itemRect.left() - subMenuSize.width() - 5, screenRect.left());
    }

    if ((y + subMenuSize.height()) > screenRect.bottom()) {
        y = screenRect.bottom() - subMenuSize.height();
    }

    y = qMax(y, screenRect.top());

    menu->execAt(QPoint(x, y), true, MenuAnimationType::DropDown);
}

void RoundMenu::hideMenu(bool isHideBySystem) {
    isHideBySystem_ = isHideBySystem;
    if (view_) {
        view_->clearSelection();
    }

    if (isSubMenu_) {
        hide();
    } else {
        close();
    }
}

void RoundMenu::hideEvent(QHideEvent* e) {
    if (isHideBySystem_ && isSubMenu_) {
        closeParentMenu();
    }

    isHideBySystem_ = true;
    e->accept();
}

void RoundMenu::closeEvent(QCloseEvent* e) {
    e->accept();
    emit closedSignal();
    if (view_) {
        view_->clearSelection();
    }
}

void RoundMenu::mousePressEvent(QMouseEvent* e) {
    QWidget* w = childAt(e->pos());
    if ((w != view_) && view_ && (!view_->isAncestorOf(w))) {
        hideMenu(true);
    }
}

void RoundMenu::mouseMoveEvent(QMouseEvent* e) {
    if (!isSubMenu_ || !parentMenu_ || !parentMenu_->view_ || !menuItem_) {
        return;
    }

    const QPoint pos = e->globalPos();

    MenuActionListWidget* view = parentMenu_->view_;

    const QMargins margin = view->menuViewportMargins();
    QRect rect = view->visualItemRect(menuItem_);
    rect.translate(view->mapToGlobal(QPoint()));
    rect.translate(margin.left(), margin.top() + 2);

    if (parentMenu_->geometry().contains(pos) && !rect.contains(pos) && !geometry().contains(pos)) {
        view->clearSelection();
        hideMenu(false);
    }
}

void RoundMenu::onActionChanged() {
    QAction* action = qobject_cast<QAction*>(sender());
    if (!action || !view_) {
        return;
    }

    auto* item = reinterpret_cast<QListWidgetItem*>(action->property("item").value<quintptr>());
    if (!item) {
        return;
    }

    item->setIcon(createItemIcon(action));

    if (action->text() != action->toolTip()) {
        item->setToolTip(action->toolTip());
    }

    adjustItemText(item, action);

    if (action->isEnabled()) {
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    } else {
        item->setFlags(Qt::NoItemFlags);
    }

    view_->adjustSizeForMenu();
    adjustSize();
}

void RoundMenu::execAt(const QPoint& pos, bool ani, MenuAnimationType aniType) {
    if (!view_) {
        return;
    }

    view_->adjustSizeForMenu(pos, aniType);
    adjustSize();

    delete aniManager_;
    aniManager_ = nullptr;

    if (!ani) {
        aniType = MenuAnimationType::None;
    }

    switch (aniType) {
        case MenuAnimationType::None:
            aniManager_ = new DummyMenuAnimationManager(this);
            break;
        case MenuAnimationType::DropDown:
            aniManager_ = new DropDownMenuAnimationManager(this);
            break;
        case MenuAnimationType::PullUp:
            aniManager_ = new PullUpMenuAnimationManager(this);
            break;
        case MenuAnimationType::FadeInDropDown:
            aniManager_ = new FadeInDropDownMenuAnimationManager(this);
            break;
        case MenuAnimationType::FadeInPullUp:
            aniManager_ = new FadeInPullUpMenuAnimationManager(this);
            break;
    }

    auto* m = qobject_cast<MenuAnimationManager*>(aniManager_);
    if (m) {
        m->exec(pos);
    }

    show();

    if (isSubMenu_ && menuItem_) {
        menuItem_->setSelected(true);
    }
}

// ============================================================================
// MenuAnimationManager
// ============================================================================
MenuAnimationManager::MenuAnimationManager(RoundMenu* menu) : QObject(menu), menu_(menu) {
    posAni_ = new QPropertyAnimation(menu, QByteArrayLiteral("pos"), menu);
    posAni_->setDuration(250);
    posAni_->setEasingCurve(QEasingCurve::OutQuad);
    connect(posAni_, &QPropertyAnimation::valueChanged, this,
            &MenuAnimationManager::onValueChanged);
    connect(posAni_, &QPropertyAnimation::valueChanged, this,
            &MenuAnimationManager::updateMenuViewport);
}

void MenuAnimationManager::onValueChanged() {}

QSize MenuAnimationManager::availableViewSize(const QPoint& pos) const {
    Q_UNUSED(pos);
    const QRect ss = getCurrentScreenGeometry(true);
    return QSize(ss.width() - 100, ss.height() - 100);
}

void MenuAnimationManager::updateMenuViewport() {
    if (!menu_ || !menu_->view_) {
        return;
    }

    menu_->view_->viewport()->update();
    menu_->view_->setAttribute(Qt::WA_UnderMouse, true);

    QHoverEvent e(QEvent::HoverEnter, QPoint(), QPoint(1, 1));
    QApplication::sendEvent(menu_->view_, &e);
}

QPoint MenuAnimationManager::endPosition(const QPoint& pos) const {
    if (!menu_) {
        return pos;
    }

    const QRect rect = getCurrentScreenGeometry(true);
    const int w = menu_->width() + 5;
    const int h = menu_->height();

    const int left = menu_->layout() ? menu_->layout()->contentsMargins().left() : 0;

    const int x = qMin(pos.x() - left, rect.right() - w);
    const int y = qMin(pos.y() - 4, rect.bottom() - h + 10);

    return QPoint(x, y);
}

QSize MenuAnimationManager::menuSize() const {
    if (!menu_ || !menu_->view_) {
        return QSize();
    }

    const QMargins m = menu_->layout() ? menu_->layout()->contentsMargins() : QMargins();
    const int w = menu_->view_->width() + m.left() + m.right() + 120;
    const int h = menu_->view_->height() + m.top() + m.bottom() + 20;
    return QSize(w, h);
}

// ============================================================================
// DummyMenuAnimationManager
// ============================================================================
void DummyMenuAnimationManager::exec(const QPoint& pos) {
    if (!menu_) {
        return;
    }
    menu_->move(endPosition(pos));
}

// ============================================================================
// DropDownMenuAnimationManager
// ============================================================================
void DropDownMenuAnimationManager::exec(const QPoint& pos) {
    if (!menu_ || !posAni_) {
        return;
    }

    const QPoint end = endPosition(pos);
    const int h = menu_->height() + 5;

    posAni_->setStartValue(end - QPoint(0, h / 2));
    posAni_->setEndValue(end);
    posAni_->start();
}

QSize DropDownMenuAnimationManager::availableViewSize(const QPoint& pos) const {
    const QRect ss = getCurrentScreenGeometry(true);
    return QSize(ss.width() - 100, qMax(ss.bottom() - pos.y() - 10, 1));
}

void DropDownMenuAnimationManager::onValueChanged() {
    if (!menu_) {
        return;
    }

    const QSize s = menuSize();
    const int y = posAni_->endValue().toPoint().y() - posAni_->currentValue().toPoint().y();
    menu_->setMask(QRegion(0, y, s.width(), s.height()));
}

// ============================================================================
// PullUpMenuAnimationManager
// ============================================================================
QPoint PullUpMenuAnimationManager::endPositionPullUp(const QPoint& pos) const {
    if (!menu_) {
        return pos;
    }

    const QRect rect = getCurrentScreenGeometry(true);
    const int w = menu_->width() + 5;
    const int h = menu_->height();

    const int left = menu_->layout() ? menu_->layout()->contentsMargins().left() : 0;

    const int x = qMin(pos.x() - left, rect.right() - w);
    const int y = qMax(pos.y() - h + 10, rect.top() + 4);

    return QPoint(x, y);
}

void PullUpMenuAnimationManager::exec(const QPoint& pos) {
    if (!menu_ || !posAni_) {
        return;
    }

    const QPoint end = endPositionPullUp(pos);
    const int h = menu_->height() + 5;

    posAni_->setStartValue(end + QPoint(0, h / 2));
    posAni_->setEndValue(end);
    posAni_->start();
}

QSize PullUpMenuAnimationManager::availableViewSize(const QPoint& pos) const {
    const QRect ss = getCurrentScreenGeometry(true);
    return QSize(ss.width() - 100, qMax(pos.y() - ss.top() - 28, 1));
}

void PullUpMenuAnimationManager::onValueChanged() {
    if (!menu_) {
        return;
    }

    const QSize s = menuSize();
    const int y = posAni_->endValue().toPoint().y() - posAni_->currentValue().toPoint().y();
    menu_->setMask(QRegion(0, y, s.width(), s.height() - 28));
}

// ============================================================================
// FadeInDropDownMenuAnimationManager
// ============================================================================
FadeInDropDownMenuAnimationManager::FadeInDropDownMenuAnimationManager(RoundMenu* menu)
    : DropDownMenuAnimationManager(menu) {
    opacityAni_ = new QPropertyAnimation(menu, QByteArrayLiteral("windowOpacity"), this);
    group_ = new QParallelAnimationGroup(this);
    group_->addAnimation(posAni_);
    group_->addAnimation(opacityAni_);
}

void FadeInDropDownMenuAnimationManager::exec(const QPoint& pos) {
    if (!menu_ || !group_) {
        return;
    }

    const QPoint end = endPosition(pos);

    opacityAni_->setStartValue(0.0);
    opacityAni_->setEndValue(1.0);
    opacityAni_->setDuration(150);
    opacityAni_->setEasingCurve(QEasingCurve::OutQuad);

    posAni_->setStartValue(end - QPoint(0, 8));
    posAni_->setEndValue(end);
    posAni_->setDuration(150);
    posAni_->setEasingCurve(QEasingCurve::OutQuad);

    group_->start();
}

// ============================================================================
// FadeInPullUpMenuAnimationManager
// ============================================================================
FadeInPullUpMenuAnimationManager::FadeInPullUpMenuAnimationManager(RoundMenu* menu)
    : PullUpMenuAnimationManager(menu) {
    opacityAni_ = new QPropertyAnimation(menu, QByteArrayLiteral("windowOpacity"), this);
    group_ = new QParallelAnimationGroup(this);
    group_->addAnimation(posAni_);
    group_->addAnimation(opacityAni_);
}

void FadeInPullUpMenuAnimationManager::exec(const QPoint& pos) {
    if (!menu_ || !group_) {
        return;
    }

    const QPoint end = endPositionPullUp(pos);

    opacityAni_->setStartValue(0.0);
    opacityAni_->setEndValue(1.0);
    opacityAni_->setDuration(150);
    opacityAni_->setEasingCurve(QEasingCurve::OutQuad);

    posAni_->setStartValue(end + QPoint(0, 8));
    posAni_->setEndValue(end);
    posAni_->setDuration(200);
    posAni_->setEasingCurve(QEasingCurve::OutQuad);

    group_->start();
}

// ============================================================================
// EditMenu
// ============================================================================
EditMenu::EditMenu(QWidget* parent) : RoundMenu(QString(), parent) { createActions(); }

void EditMenu::createActions() {
    undoAct_ = new QAction(tr("Undo"), this);
    undoAct_->setShortcut(QKeySequence::Undo);
    addAction(undoAct_);

    redoAct_ = new QAction(tr("Redo"), this);
    redoAct_->setShortcut(QKeySequence::Redo);
    addAction(redoAct_);

    addSeparator();

    cutAct_ = new QAction(tr("Cut"), this);
    cutAct_->setShortcut(QKeySequence::Cut);
    addAction(cutAct_);

    copyAct_ = new QAction(tr("Copy"), this);
    copyAct_->setShortcut(QKeySequence::Copy);
    addAction(copyAct_);

    pasteAct_ = new QAction(tr("Paste"), this);
    pasteAct_->setShortcut(QKeySequence::Paste);
    addAction(pasteAct_);

    deleteAct_ = new QAction(tr("Delete"), this);
    deleteAct_->setShortcut(QKeySequence::Delete);
    addAction(deleteAct_);

    addSeparator();

    selectAllAct_ = new QAction(tr("Select all"), this);
    selectAllAct_->setShortcut(QKeySequence::SelectAll);
    addAction(selectAllAct_);
}

// ============================================================================
// LineEditMenu
// ============================================================================
LineEditMenu::LineEditMenu(::QLineEdit* parent) : EditMenu(parent), edit_(parent) {
    if (edit_) {
        connect(undoAct_, &QAction::triggered, edit_, &QLineEdit::undo);
        connect(redoAct_, &QAction::triggered, edit_, &QLineEdit::redo);
        connect(cutAct_, &QAction::triggered, edit_, &QLineEdit::cut);
        connect(copyAct_, &QAction::triggered, edit_, &QLineEdit::copy);
        connect(pasteAct_, &QAction::triggered, edit_, &QLineEdit::paste);
        connect(deleteAct_, &QAction::triggered, this, [this]() {
            if (!edit_ || edit_->isReadOnly()) {
                return;
            }
            edit_->del();
        });
        connect(selectAllAct_, &QAction::triggered, edit_, &QLineEdit::selectAll);

        connect(edit_, &QLineEdit::selectionChanged, this, &LineEditMenu::updateActions);
        connect(edit_, &QLineEdit::textChanged, this, &LineEditMenu::updateActions);

        // QLineEdit doesn't provide undoAvailable/redoAvailable signals consistently across Qt
        // versions/configs; we conservatively keep them disabled unless we can query/track.
    }

    updateActions();
}

void LineEditMenu::updateActions() {
    if (!edit_) {
        return;
    }

    const bool ro = edit_->isReadOnly();
    const bool hasSel = edit_->hasSelectedText();

    if (undoAct_) {
        undoAct_->setEnabled(!ro && undoAvailable_);
    }
    if (redoAct_) {
        redoAct_->setEnabled(!ro && redoAvailable_);
    }

    if (cutAct_) {
        cutAct_->setEnabled(!ro && hasSel);
    }
    if (copyAct_) {
        copyAct_->setEnabled(hasSel);
    }
    if (pasteAct_) {
        const QClipboard* cb = QApplication::clipboard();
        const bool canPaste = cb && cb->mimeData() && cb->mimeData()->hasText();
        pasteAct_->setEnabled(!ro && canPaste);
    }
    if (deleteAct_) {
        deleteAct_->setEnabled(!ro && hasSel);
    }
    if (selectAllAct_) {
        selectAllAct_->setEnabled(!edit_->text().isEmpty());
    }
}

// ============================================================================
// TextEditMenu
// ============================================================================
TextEditMenu::TextEditMenu(::QTextEdit* parent) : EditMenu(parent), edit_(parent) {
    if (edit_) {
        connect(undoAct_, &QAction::triggered, edit_, &QTextEdit::undo);
        connect(redoAct_, &QAction::triggered, edit_, &QTextEdit::redo);
        connect(cutAct_, &QAction::triggered, edit_, &QTextEdit::cut);
        connect(copyAct_, &QAction::triggered, edit_, &QTextEdit::copy);
        connect(pasteAct_, &QAction::triggered, edit_, &QTextEdit::paste);
        connect(deleteAct_, &QAction::triggered, this, [this]() {
            if (!edit_ || edit_->isReadOnly()) {
                return;
            }
            QTextCursor c = edit_->textCursor();
            if (!c.hasSelection()) {
                return;
            }
            c.removeSelectedText();
            edit_->setTextCursor(c);
        });
        connect(selectAllAct_, &QAction::triggered, edit_, &QTextEdit::selectAll);

        connect(edit_, &QTextEdit::copyAvailable, this, &TextEditMenu::updateActions);
        connect(edit_, &QTextEdit::undoAvailable, this, &TextEditMenu::updateActions);
        connect(edit_, &QTextEdit::redoAvailable, this, &TextEditMenu::updateActions);
        connect(edit_, &QTextEdit::textChanged, this, &TextEditMenu::updateActions);
        connect(edit_, &QTextEdit::cursorPositionChanged, this, &TextEditMenu::updateActions);
    }

    updateActions();
}

void TextEditMenu::updateActions() {
    if (!edit_) {
        return;
    }

    const bool ro = edit_->isReadOnly();
    const QTextCursor c = edit_->textCursor();
    const bool hasSel = c.hasSelection();

    if (undoAct_) {
        undoAct_->setEnabled(edit_->document() && edit_->document()->isUndoAvailable());
    }
    if (redoAct_) {
        redoAct_->setEnabled(edit_->document() && edit_->document()->isRedoAvailable());
    }

    if (cutAct_) {
        cutAct_->setEnabled(!ro && hasSel);
    }
    if (copyAct_) {
        copyAct_->setEnabled(hasSel);
    }
    if (pasteAct_) {
        pasteAct_->setEnabled(!ro && edit_->canPaste());
    }
    if (deleteAct_) {
        deleteAct_->setEnabled(!ro && hasSel);
    }
    if (selectAllAct_) {
        selectAllAct_->setEnabled(!edit_->toPlainText().isEmpty());
    }
}

// ============================================================================
// PlainTextEditMenu
// ============================================================================
PlainTextEditMenu::PlainTextEditMenu(::QPlainTextEdit* parent) : EditMenu(parent), edit_(parent) {
    if (edit_) {
        connect(undoAct_, &QAction::triggered, edit_, &QPlainTextEdit::undo);
        connect(redoAct_, &QAction::triggered, edit_, &QPlainTextEdit::redo);
        connect(cutAct_, &QAction::triggered, edit_, &QPlainTextEdit::cut);
        connect(copyAct_, &QAction::triggered, edit_, &QPlainTextEdit::copy);
        connect(pasteAct_, &QAction::triggered, edit_, &QPlainTextEdit::paste);
        connect(deleteAct_, &QAction::triggered, this, [this]() {
            if (!edit_ || edit_->isReadOnly()) {
                return;
            }
            QTextCursor c = edit_->textCursor();
            if (!c.hasSelection()) {
                return;
            }
            c.removeSelectedText();
            edit_->setTextCursor(c);
        });
        connect(selectAllAct_, &QAction::triggered, edit_, &QPlainTextEdit::selectAll);

        connect(edit_, &QPlainTextEdit::copyAvailable, this, &PlainTextEditMenu::updateActions);
        connect(edit_, &QPlainTextEdit::undoAvailable, this, &PlainTextEditMenu::updateActions);
        connect(edit_, &QPlainTextEdit::redoAvailable, this, &PlainTextEditMenu::updateActions);
        connect(edit_, &QPlainTextEdit::textChanged, this, &PlainTextEditMenu::updateActions);
        connect(edit_, &QPlainTextEdit::cursorPositionChanged, this,
                &PlainTextEditMenu::updateActions);
    }

    updateActions();
}

void PlainTextEditMenu::updateActions() {
    if (!edit_) {
        return;
    }

    const bool ro = edit_->isReadOnly();
    const QTextCursor c = edit_->textCursor();
    const bool hasSel = c.hasSelection();

    if (undoAct_) {
        undoAct_->setEnabled(edit_->document() && edit_->document()->isUndoAvailable());
    }
    if (redoAct_) {
        redoAct_->setEnabled(edit_->document() && edit_->document()->isRedoAvailable());
    }

    if (cutAct_) {
        cutAct_->setEnabled(!ro && hasSel);
    }
    if (copyAct_) {
        copyAct_->setEnabled(hasSel);
    }
    if (pasteAct_) {
        pasteAct_->setEnabled(!ro && edit_->canPaste());
    }
    if (deleteAct_) {
        deleteAct_->setEnabled(!ro && hasSel);
    }
    if (selectAllAct_) {
        selectAllAct_->setEnabled(!edit_->toPlainText().isEmpty());
    }
}

// ============================================================================
// CheckableMenu
// ============================================================================
CheckableMenu::CheckableMenu(const QString& title, QWidget* parent, MenuIndicatorType indicatorType)
    : RoundMenu(title, parent) {
    if (view_) {
        view_->setItemDelegate(createCheckableMenuItemDelegate(indicatorType, view_));
        view_->setObjectName(QStringLiteral("checkableListWidget"));
        // Add extra right padding for checkable indicator and avoid text being too close to edge
        view_->setViewportMargins(QMargins(0, 6, 10, 6));
    }
}

void CheckableMenu::addAction(QAction* action) {
    if (!action || !view_) {
        return;
    }
    QListWidgetItem* item = createActionItem(action);
    if (!item) {
        return;
    }
    // Adjust item width for checkable indicator
    item->setSizeHint(QSize(item->sizeHint().width() + 26, itemHeight_));
    view_->addItem(item);
    adjustSize();
}

void CheckableMenu::addActions(const QList<QAction*>& actions) {
    for (auto* action : actions) {
        addAction(action);
    }
}

int CheckableMenu::adjustItemText(QListWidgetItem* item, QAction* action) {
    if (!item || !action || !view_) {
        return 0;
    }
    // Base implementation from RoundMenu
    int sw = 0;
    if (qobject_cast<ShortcutMenuItemDelegate*>(view_->itemDelegate())) {
        sw = longestShortcutWidth();
        if (sw) {
            sw += 22;
        }
    }

    int w = 0;
    if (!hasItemIcon()) {
        item->setText(action->text());
        w = 40 + view_->fontMetrics().boundingRect(action->text()).width() + sw;
    } else {
        item->setText(QStringLiteral(" ") + action->text());
        const int space = 4 - view_->fontMetrics().boundingRect(QStringLiteral(" ")).width();
        w = 60 + view_->fontMetrics().boundingRect(item->text()).width() + sw + space;
    }

    item->setSizeHint(QSize(w + 26, itemHeight_));  // Extra space for check indicator
    return w + 26;
}

void CheckableMenu::execAt(const QPoint& pos, bool ani, MenuAnimationType aniType) {
    RoundMenu::execAt(pos, ani, aniType);
}

// ============================================================================
// SystemTrayMenu
// ============================================================================
SystemTrayMenu::SystemTrayMenu(const QString& title, QWidget* parent) : RoundMenu(title, parent) {}

QSize SystemTrayMenu::sizeHint() const {
    if (!layout()) {
        return RoundMenu::sizeHint();
    }
    const QMargins m = layout()->contentsMargins();
    const QSize s = layout()->sizeHint();
    return QSize(s.width() - m.right() + 5, s.height() - m.bottom());
}

// ============================================================================
// CheckableSystemTrayMenu
// ============================================================================
CheckableSystemTrayMenu::CheckableSystemTrayMenu(const QString& title, QWidget* parent,
                                                 MenuIndicatorType indicatorType)
    : CheckableMenu(title, parent, indicatorType) {}

QSize CheckableSystemTrayMenu::sizeHint() const {
    if (!layout()) {
        return CheckableMenu::sizeHint();
    }
    const QMargins m = layout()->contentsMargins();
    const QSize s = layout()->sizeHint();
    return QSize(s.width() - m.right() + 5, s.height() - m.bottom());
}

// ============================================================================
// LabelContextMenu
// ============================================================================
LabelContextMenu::LabelContextMenu(QLabel* parent) : RoundMenu(QString(), parent) {
    // Store selected text at creation time
    // Note: QLabel needs to have text interaction flags set to support selection
    if (parent) {
        // QLabel doesn't have selectedText() by default, we need to work with what we have
        // If text interaction is enabled, we can try to get selection
        selectedText_ = parent->text();
    }

    copyAct_ = new QAction(tr("Copy"), this);
    copyAct_->setShortcut(QKeySequence::Copy);
    QObject::connect(copyAct_, &QAction::triggered, this, &LabelContextMenu::onCopy);

    selectAllAct_ = new QAction(tr("Select all"), this);
    selectAllAct_->setShortcut(QKeySequence::SelectAll);
    QObject::connect(selectAllAct_, &QAction::triggered, this, &LabelContextMenu::onSelectAll);
}

QLabel* LabelContextMenu::label() const { return qobject_cast<QLabel*>(parentWidget()); }

void LabelContextMenu::onCopy() {
    if (!selectedText_.isEmpty()) {
        QApplication::clipboard()->setText(selectedText_);
    }
}

void LabelContextMenu::onSelectAll() {
    QLabel* lbl = label();
    if (lbl) {
        // QLabel doesn't have setSelection() by default
        // This would require text interaction to be enabled
        // For now, we just select all text conceptually
        selectedText_ = lbl->text();
    }
}

void LabelContextMenu::execAt(const QPoint& pos, bool ani, MenuAnimationType aniType) {
    QLabel* lbl = label();
    if (!lbl) {
        return;
    }

    // Clear previous actions first
    clear();

    // Check if there's selected text (conceptually - QLabel needs text interaction flags)
    if (!lbl->text().isEmpty()) {
        addAction(copyAct_);
        addAction(selectAllAct_);
    } else {
        addAction(selectAllAct_);
    }

    RoundMenu::execAt(pos, ani, aniType);
}

}  // namespace qfw
