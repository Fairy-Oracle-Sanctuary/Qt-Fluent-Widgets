#include "components/navigation/navigation_panel.h"

#include <QApplication>
#include <QCursor>
#include <QEvent>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>
#include <QScreen>
#include <QScrollBar>

#include "common/config.h"
#include "common/icon.h"
#include "common/style_sheet.h"
#include "components/navigation/navigation_widget.h"
#include "components/widgets/flyout.h"
#include "components/widgets/tool_tip.h"

namespace qfw {

namespace {

class NavigationToolTipFilter : public ToolTipFilter {
public:
    explicit NavigationToolTipFilter(NavigationWidget* parent, int showDelay = 300,
                                     ToolTipPosition position = ToolTipPosition::Top)
        : ToolTipFilter(parent, showDelay, position), navWidget_(parent) {}

protected:
    bool canShowToolTip() override {
        return ToolTipFilter::canShowToolTip() && navWidget_ && navWidget_->isCompacted();
    }

private:
    QPointer<NavigationWidget> navWidget_;
};

class NavigationPanelFlyoutView final : public FlyoutViewBase {
public:
    explicit NavigationPanelFlyoutView(QWidget* parent = nullptr) : FlyoutViewBase(parent) {
        hBoxLayout_ = new QHBoxLayout(this);
        hBoxLayout_->setContentsMargins(0, 0, 0, 0);
        hBoxLayout_->setSpacing(0);
    }

    void addWidget(QWidget* widget, int stretch = 0, Qt::Alignment align = Qt::AlignLeft) override {
        if (!widget || !hBoxLayout_) {
            return;
        }

        hBoxLayout_->addWidget(widget, stretch, align);
    }

private:
    QHBoxLayout* hBoxLayout_ = nullptr;
};

}  // namespace

// ============================================================================
// NavigationItemLayout
// ============================================================================

void NavigationItemLayout::setGeometry(const QRect& rect) {
    QVBoxLayout::setGeometry(rect);

    for (int i = 0; i < count(); ++i) {
        QLayoutItem* item = itemAt(i);
        if (!item) {
            continue;
        }

        QWidget* w = item->widget();
        if (qobject_cast<NavigationSeparator*>(w)) {
            const QRect g = item->geometry();
            w->setGeometry(0, g.y(), rect.width(), g.height());
        }
    }
}

// ============================================================================
// NavigationPanel
// ============================================================================

NavigationPanel::NavigationPanel(QWidget* parent, bool isMinimalEnabled)
    : QFrame(parent),
      parentWidget_(parent),
      indicator_(new NavigationIndicator(this)),
      acrylicBrush_(this, 30),
      scrollArea_(new ScrollArea(this)),
      scrollWidget_(new QWidget()),
      menuButton_(new NavigationToolButton(QVariant::fromValue(static_cast<const FluentIconBase*>(
                                               new FluentIcon(FluentIconEnum::Menu))),
                                           this)),
      returnButton_(new NavigationToolButton(QVariant::fromValue(static_cast<const FluentIconBase*>(
                                                 new FluentIcon(FluentIconEnum::Return))),
                                             this)),
      vBoxLayout_(new NavigationItemLayout(this)),
      topLayout_(new NavigationItemLayout()),
      bottomLayout_(new NavigationItemLayout()),
      scrollLayout_(new NavigationItemLayout(scrollWidget_)),
      history_(new Router(this)),
      expandAni_(new QPropertyAnimation(this, QByteArrayLiteral("geometry"), this)),
      isMinimalEnabled_(isMinimalEnabled) {
    displayMode_ =
        isMinimalEnabled_ ? NavigationDisplayMode::Minimal : NavigationDisplayMode::Compact;

    initWidget();
}

void NavigationPanel::initWidget() {
    resize(48, height());
    setAttribute(Qt::WA_StyledBackground);

    if (window()) {
        window()->installEventFilter(this);
    }

    returnButton_->hide();
    returnButton_->setDisabled(true);

    scrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea_->setFrameShape(QFrame::NoFrame);
    scrollArea_->setWidget(scrollWidget_);
    scrollArea_->setWidgetResizable(true);

    expandAni_->setEasingCurve(QEasingCurve::OutQuad);
    expandAni_->setDuration(150);

    connect(menuButton_, &NavigationWidget::clicked, this, [this](bool) { toggle(); });
    connect(expandAni_, &QPropertyAnimation::finished, this, &NavigationPanel::onExpandAniFinished);

    connect(history_, &Router::emptyChanged, returnButton_, &QWidget::setDisabled);
    connect(returnButton_, &NavigationWidget::clicked, this, [this](bool) { history_->pop(); });

    connect(indicator_, &NavigationIndicator::aniFinished, this,
            &NavigationPanel::onIndicatorAniFinished);

    // add tool tip
    returnButton_->installEventFilter(new ToolTipFilter(returnButton_, 1000));
    returnButton_->setToolTip(tr("Back"));

    menuButton_->installEventFilter(new ToolTipFilter(menuButton_, 1000));
    menuButton_->setToolTip(tr("Open Navigation"));

    scrollWidget_->setObjectName(QStringLiteral("scrollWidget"));
    setProperty("menu", false);
    qfw::setStyleSheet(this, qfw::FluentStyleSheet::NavigationInterface);
    qfw::setStyleSheet(scrollWidget_, qfw::FluentStyleSheet::NavigationInterface);

    initLayout();
}

void NavigationPanel::initLayout() {
    vBoxLayout_->setContentsMargins(0, 5, 0, 5);
    topLayout_->setContentsMargins(4, 0, 4, 0);
    bottomLayout_->setContentsMargins(4, 0, 4, 0);
    scrollLayout_->setContentsMargins(4, 0, 4, 0);

    vBoxLayout_->setSpacing(4);
    topLayout_->setSpacing(4);
    bottomLayout_->setSpacing(4);
    scrollLayout_->setSpacing(4);

    vBoxLayout_->addLayout(topLayout_, 0);
    vBoxLayout_->addWidget(scrollArea_, 1);
    vBoxLayout_->addLayout(bottomLayout_, 0);

    vBoxLayout_->setAlignment(Qt::AlignTop);
    topLayout_->setAlignment(Qt::AlignTop);
    scrollLayout_->setAlignment(Qt::AlignTop);
    bottomLayout_->setAlignment(Qt::AlignBottom);

    topLayout_->addWidget(returnButton_, 0, Qt::AlignTop);
    topLayout_->addWidget(menuButton_, 0, Qt::AlignTop);

    if (scrollWidget_ && scrollWidget_->layout() == nullptr) {
        scrollWidget_->setLayout(scrollLayout_);
    }
}

void NavigationPanel::setIndicatorAnimationEnabled(bool enabled) {
    isIndicatorAnimationEnabled_ = enabled;
}

void NavigationPanel::setUpdateIndicatorPosOnCollapseFinished(bool update) {
    isUpdateIndicatorPosOnCollapseFinished_ = update;
}

void NavigationPanel::setMenuButtonVisible(bool visible) {
    isMenuButtonVisible_ = visible;
    if (menuButton_) {
        menuButton_->setVisible(visible);
    }
}

void NavigationPanel::setReturnButtonVisible(bool visible) {
    isReturnButtonVisible_ = visible;
    if (returnButton_) {
        returnButton_->setVisible(visible);
    }
}

void NavigationPanel::setCollapsible(bool on) {
    isCollapsible_ = on;
    if (!on && displayMode_ != NavigationDisplayMode::Expand) {
        expand(false);
    }
}

void NavigationPanel::setExpandWidth(int width) {
    if (width <= 42) {
        return;
    }

    expandWidth_ = width;
}

void NavigationPanel::setMinimumExpandWidth(int width) { minimumExpandWidth_ = width; }

void NavigationPanel::setAcrylicEnabled(bool enabled) {
    if (enabled == isAcrylicEnabled_) {
        return;
    }

    isAcrylicEnabled_ = enabled;
    setProperty("transparent", canDrawAcrylic());
    setStyle(QApplication::style());
    update();
}

NavigationWidget* NavigationPanel::widget(const QString& routeKey) const {
    if (!items_.contains(routeKey)) {
        return nullptr;
    }

    return items_.value(routeKey).widget;
}

NavigationTreeWidget* NavigationPanel::addItem(const QString& routeKey, const QVariant& icon,
                                               const QString& text,
                                               const std::function<void()>& onClick,
                                               bool selectable, NavigationItemPosition position,
                                               const QString& tooltip,
                                               const QString& parentRouteKey) {
    return insertItem(-1, routeKey, icon, text, onClick, selectable, position, tooltip,
                      parentRouteKey);
}

void NavigationPanel::addWidget(const QString& routeKey, NavigationWidget* widget,
                                const std::function<void()>& onClick,
                                NavigationItemPosition position, const QString& tooltip,
                                const QString& parentRouteKey) {
    insertWidget(-1, routeKey, widget, onClick, position, tooltip, parentRouteKey);
}

NavigationTreeWidget* NavigationPanel::insertItem(int index, const QString& routeKey,
                                                  const QVariant& icon, const QString& text,
                                                  const std::function<void()>& onClick,
                                                  bool selectable, NavigationItemPosition position,
                                                  const QString& tooltip,
                                                  const QString& parentRouteKey) {
    if (items_.contains(routeKey)) {
        return nullptr;
    }

    auto* w = new NavigationTreeWidget(icon, text, selectable, this);
    insertWidget(index, routeKey, w, onClick, position, tooltip, parentRouteKey);
    return w;
}

void NavigationPanel::insertWidget(int index, const QString& routeKey, NavigationWidget* widget,
                                   const std::function<void()>& onClick,
                                   NavigationItemPosition position, const QString& tooltip,
                                   const QString& parentRouteKey) {
    if (!widget || items_.contains(routeKey)) {
        return;
    }

    registerWidget(routeKey, parentRouteKey, widget, onClick, tooltip);

    if (!parentRouteKey.isEmpty()) {
        if (auto* p = qobject_cast<NavigationTreeWidget*>(this->widget(parentRouteKey))) {
            p->insertChild(index, qobject_cast<NavigationTreeWidgetBase*>(widget));
        }
    } else {
        insertWidgetToLayout(index, widget, position);
    }
}

void NavigationPanel::addSeparator(NavigationItemPosition position) {
    insertSeparator(-1, position);
}

void NavigationPanel::insertSeparator(int index, NavigationItemPosition position) {
    auto* separator = new NavigationSeparator(this);
    insertWidgetToLayout(index, separator, position);
}

NavigationItemHeader* NavigationPanel::addItemHeader(const QString& text,
                                                     NavigationItemPosition position) {
    return insertItemHeader(-1, text, position);
}

NavigationItemHeader* NavigationPanel::insertItemHeader(int index, const QString& text,
                                                        NavigationItemPosition position) {
    auto* header = new NavigationItemHeader(text, this);
    insertWidgetToLayout(index, header, position);

    const bool compacted = !(displayMode_ == NavigationDisplayMode::Expand ||
                             displayMode_ == NavigationDisplayMode::Menu);
    header->setCompacted(compacted);

    return header;
}

void NavigationPanel::registerWidget(const QString& routeKey, const QString& parentRouteKey,
                                     NavigationWidget* widget, const std::function<void()>& onClick,
                                     const QString& tooltip) {
    connect(widget, &NavigationWidget::clicked, this, &NavigationPanel::onWidgetClicked);

    if (onClick) {
        connect(widget, &NavigationWidget::clicked, this, [onClick](bool) { onClick(); });
    }

    widget->setProperty("routeKey", routeKey);
    widget->setProperty("parentRouteKey", parentRouteKey);

    NavigationItem item;
    item.routeKey = routeKey;
    item.parentRouteKey = parentRouteKey;
    item.widget = widget;
    items_.insert(routeKey, item);

    if (displayMode_ == NavigationDisplayMode::Expand ||
        displayMode_ == NavigationDisplayMode::Menu) {
        widget->setCompacted(false);
    }

    if (!tooltip.isEmpty()) {
        widget->setToolTip(tooltip);
        widget->installEventFilter(new NavigationToolTipFilter(widget, 1000));
    }
}

void NavigationPanel::insertWidgetToLayout(int index, NavigationWidget* widget,
                                           NavigationItemPosition position) {
    if (!widget) {
        return;
    }

    if (position == NavigationItemPosition::Top) {
        widget->setParent(this);
        topLayout_->insertWidget(index, widget, 0, Qt::AlignTop);
    } else if (position == NavigationItemPosition::Scroll) {
        widget->setParent(scrollWidget_);
        scrollLayout_->insertWidget(index, widget, 0, Qt::AlignTop);
    } else {
        widget->setParent(this);
        bottomLayout_->insertWidget(index, widget, 0, Qt::AlignBottom);
    }

    widget->show();
}

void NavigationPanel::removeWidget(const QString& routeKey) {
    if (!items_.contains(routeKey)) {
        return;
    }

    if (currentRouteKey_ == routeKey) {
        currentRouteKey_.clear();
    }

    NavigationItem item = items_.take(routeKey);

    if (!item.parentRouteKey.isEmpty()) {
        if (auto* p = qobject_cast<NavigationTreeWidget*>(widget(item.parentRouteKey))) {
            p->removeChild(qobject_cast<NavigationTreeWidgetBase*>(item.widget.data()));
        }
    }

    if (auto* tree = qobject_cast<NavigationTreeWidgetBase*>(item.widget.data())) {
        if (tree->isRoot()) {
            const auto children =
                tree->findChildren<NavigationWidget*>(QString(), Qt::FindChildrenRecursively);
            for (auto* child : children) {
                const QVariant keyVar = child->property("routeKey");
                if (keyVar.isValid()) {
                    items_.remove(keyVar.toString());
                }
                child->deleteLater();
            }
        }
    }

    if (item.widget) {
        item.widget->deleteLater();
    }
}

void NavigationPanel::expand(bool useAni) {
    stopIndicatorAnimation();
    restoreTreeExpandState(useAni);

    expandAni_->setProperty("expand", true);
    menuButton_->setToolTip(tr("Close Navigation"));

    // determine the display mode according to the width of window
    const int expandWidth = minimumExpandWidth_ + expandWidth_ - 322;
    const bool canExpandInline =
        ((window() && window()->width() >= expandWidth) && !isMinimalEnabled_) || !isCollapsible_;

    if (canExpandInline || !isMenuModeEnabledWhenNarrow_) {
        displayMode_ = NavigationDisplayMode::Expand;
    } else {
        setProperty("menu", true);
        setStyle(QApplication::style());
        displayMode_ = NavigationDisplayMode::Menu;

        if (canDrawAcrylic()) {
            acrylicBrush_.grabImage(QRect(mapToGlobal(QPoint()), QSize(expandWidth_, height())));
        }

        if (parentWidget_ && !parentWidget_->isWindow()) {
            const QPoint pos = parentWidget_->pos();
            setParent(window());
            move(pos);
        }
        show();
    }

    setWidgetCompacted(false);

    if (useAni) {
        emit displayModeChanged(displayMode_);
        expandAni_->setStartValue(QRect(pos(), QSize(48, height())));
        expandAni_->setEndValue(QRect(pos(), QSize(expandWidth_, height())));
        expandAni_->start();
    } else {
        resize(expandWidth_, height());
        onExpandAniFinished();
    }
}

void NavigationPanel::collapse() {
    if (currentItem() && !currentItem()->property("parentRouteKey").toString().isEmpty()) {
        stopIndicatorAnimation();
    }

    if (expandAni_->state() == QAbstractAnimation::Running) {
        return;
    }

    for (auto it = items_.begin(); it != items_.end(); ++it) {
        auto* w = it.value().widget.data();
        if (auto* tree = qobject_cast<NavigationTreeWidgetBase*>(w)) {
            if (tree->isRoot()) {
                tree->saveExpandState();
                tree->setExpanded(false);
            }
        }
    }

    expandAni_->setStartValue(QRect(pos(), QSize(width(), height())));
    expandAni_->setEndValue(QRect(pos(), QSize(48, height())));
    expandAni_->setProperty("expand", false);

    expandAni_->start();

    menuButton_->setToolTip(tr("Open Navigation"));
}

void NavigationPanel::toggle() {
    if (displayMode_ == NavigationDisplayMode::Compact ||
        displayMode_ == NavigationDisplayMode::Minimal) {
        expand();
    } else {
        collapse();
    }
}

void NavigationPanel::stopIndicatorAnimation() {
    if (indicator_) {
        indicator_->stopAnimation();
    }
    onIndicatorAniFinished();
}

void NavigationPanel::restoreTreeExpandState(bool useAni) {
    for (auto it = items_.begin(); it != items_.end(); ++it) {
        auto* w = it.value().widget.data();
        if (auto* tree = qobject_cast<NavigationTreeWidgetBase*>(w)) {
            if (tree->isRoot()) {
                tree->restoreExpandState(useAni);
            }
        }
    }
}

void NavigationPanel::setWidgetCompacted(bool compacted) {
    const auto children = findChildren<NavigationWidget*>();
    for (auto* item : children) {
        if (item) {
            item->setCompacted(compacted);
        }
    }
}

void NavigationPanel::setCurrentItem(const QString& routeKey) {
    if (!items_.contains(routeKey) || routeKey == currentRouteKey_) {
        return;
    }

    NavigationWidget* prevItem = currentItem();
    currentRouteKey_ = routeKey;

    NavigationWidget* newItem = currentItem();
    NavigationWidget* newIndicatorItem = findIndicatorItem(newItem);
    NavigationWidget* prevIndicatorItem = findIndicatorItem(prevItem);

    if (!(isIndicatorAnimationEnabled_ && prevItem && prevIndicatorItem && newIndicatorItem)) {
        for (auto it = items_.begin(); it != items_.end(); ++it) {
            if (it.value().widget) {
                it.value().widget->setSelected(it.key() == routeKey);
            }
        }
        return;
    }

    const QRectF preIndicatorRect = getIndicatorRect(prevIndicatorItem);
    const QRectF newIndicatorRect = getIndicatorRect(newIndicatorItem);

    // Mark previous items as not selected
    if (prevItem) {
        prevItem->setSelected(false);
    }
    if (prevIndicatorItem && prevIndicatorItem != prevItem) {
        prevIndicatorItem->setSelected(false);
    }

    // Mark new item as "about to be selected" during animation
    // This prevents it from drawing its own indicator while animation runs
    newIndicatorItem->setAboutSelected(true);

    if (newItem && newItem != newIndicatorItem) {
        newItem->setAboutSelected(true);
    }

    if (indicator_) {
        indicator_->setIndicatorColor(newItem->lightIndicatorColor(),
                                      newItem->darkIndicatorColor());
        indicator_->startAnimation(preIndicatorRect, newIndicatorRect);
    }
}

NavigationWidget* NavigationPanel::currentItem() const {
    if (currentRouteKey_.isEmpty()) {
        return nullptr;
    }
    return widget(currentRouteKey_);
}

int NavigationPanel::layoutMinHeight() const {
    const int th = topLayout_ ? topLayout_->minimumSize().height() : 0;
    const int bh = bottomLayout_ ? bottomLayout_->minimumSize().height() : 0;

    int sh = 0;
    const auto seps = findChildren<NavigationSeparator*>();
    for (auto* s : seps) {
        if (s) {
            sh += s->height();
        }
    }

    int spacing = 0;
    if (topLayout_) {
        spacing += topLayout_->count() * topLayout_->spacing();
    }
    if (bottomLayout_) {
        spacing += bottomLayout_->count() * bottomLayout_->spacing();
    }

    // 36 is the fixed height of each navigation item
    return 36 + th + bh + sh + spacing;
}

NavigationWidget* NavigationPanel::findIndicatorItem(NavigationWidget* item) const {
    QWidget* parent = item;
    while (parent) {
        if (auto* w = qobject_cast<NavigationWidget*>(parent)) {
            if (w->isVisible()) {
                return w;
            }
        }
        parent = parent->parentWidget();
    }
    return nullptr;
}

QRectF NavigationPanel::getIndicatorRect(NavigationWidget* item) const {
    if (!item) {
        return QRectF();
    }

    const QPoint pos = item->mapTo(const_cast<NavigationPanel*>(this), QPoint(0, 0));
    const QRectF rect = item->indicatorRect();
    return rect.translated(pos);
}

void NavigationPanel::onIndicatorAniFinished() {
    NavigationWidget* item = currentItem();
    if (!item) {
        return;
    }

    item->setSelected(true);

    if (auto* indicatorItem = findIndicatorItem(item)) {
        indicatorItem->setAboutSelected(false);
    }

    if (indicator_) {
        indicator_->hide();
    }
}

void NavigationPanel::onWidgetClicked(bool) {
    auto* w = qobject_cast<NavigationWidget*>(sender());
    if (!w) {
        return;
    }

    if (!w->isSelectable()) {
        showFlyoutNavigationMenu(qobject_cast<NavigationTreeWidget*>(w));
        return;
    }

    const QString routeKey = w->property("routeKey").toString();
    setCurrentItem(routeKey);

    const bool isLeaf = !qobject_cast<NavigationTreeWidgetBase*>(w) ||
                        qobject_cast<NavigationTreeWidgetBase*>(w)->isLeaf();

    if (displayMode_ == NavigationDisplayMode::Menu && isLeaf) {
        collapse();
    } else if (isCollapsed()) {
        showFlyoutNavigationMenu(qobject_cast<NavigationTreeWidget*>(w));
    }
}

void NavigationPanel::showFlyoutNavigationMenu(NavigationTreeWidget* widget) {
    if (!(isCollapsed() && widget)) {
        return;
    }

    if (!widget->isRoot() || widget->isLeaf()) {
        return;
    }

    auto* view = new NavigationPanelFlyoutView();

    auto* menu = new NavigationFlyoutMenu(widget, view);
    view->addWidget(menu);

    auto* flyout = new Flyout(view, window());
    flyout->resize(flyout->sizeHint());

    SlideRightFlyoutAnimationManager manager(flyout);
    const QPoint pos = manager.position(widget);
    flyout->exec(pos, FlyoutAnimationType::SlideRight);

    connect(menu, &NavigationFlyoutMenu::expanded, flyout, [flyout, widget, menu]() {
        if (!flyout || !widget || !menu) {
            return;
        }

        flyout->setFixedSize(menu->size());

        SlideRightFlyoutAnimationManager mgr(flyout);
        const QPoint p = mgr.position(widget);

        const QRect rect = flyout->window() ? flyout->window()->geometry() : QRect();
        const int w = flyout->sizeHint().width() + 5;
        const int h = flyout->sizeHint().height();
        const int x = qMax(rect.left(), qMin(p.x(), rect.right() - w));
        const int y = qMax(rect.top() + 42, qMin(p.y() - 4, rect.bottom() - h + 5));
        flyout->move(x, y);
    });
}

bool NavigationPanel::eventFilter(QObject* obj, QEvent* e) {
    if (obj != window() || !isCollapsible_) {
        return QFrame::eventFilter(obj, e);
    }

    if (e->type() == QEvent::MouseButtonRelease) {
        auto* me = static_cast<QMouseEvent*>(e);
        if (!geometry().contains(me->pos()) && displayMode_ == NavigationDisplayMode::Menu) {
            collapse();
        }
    } else if (e->type() == QEvent::Resize) {
        const int w = static_cast<QResizeEvent*>(e)->size().width();
        if (w < minimumExpandWidth_ && displayMode_ == NavigationDisplayMode::Expand) {
            collapse();
        } else if (w >= minimumExpandWidth_ && displayMode_ == NavigationDisplayMode::Compact &&
                   !isMenuButtonVisible_) {
            expand();
        }
    }

    return QFrame::eventFilter(obj, e);
}

void NavigationPanel::onExpandAniFinished() {
    const bool expanding = expandAni_->property("expand").toBool();

    if (!expanding) {
        displayMode_ =
            isMinimalEnabled_ ? NavigationDisplayMode::Minimal : NavigationDisplayMode::Compact;
        emit displayModeChanged(displayMode_);
    }

    if (displayMode_ == NavigationDisplayMode::Minimal) {
        hide();
        setProperty("menu", false);
        setStyle(QApplication::style());
    } else if (displayMode_ == NavigationDisplayMode::Compact) {
        setProperty("menu", false);
        setStyle(QApplication::style());

        setWidgetCompacted(true);

        if (isUpdateIndicatorPosOnCollapseFinished_) {
            stopIndicatorAnimation();
        }

        if (parentWidget_ && !parentWidget_->isWindow()) {
            setParent(parentWidget_);
            move(0, 0);
            show();
        }
    }
}

void NavigationPanel::updateAcrylicColor() {
    if (isDarkTheme()) {
        acrylicBrush_.setTintColor(QColor(32, 32, 32, 200));
        acrylicBrush_.setLuminosityColor(QColor(0, 0, 0, 0));
    } else {
        acrylicBrush_.setTintColor(QColor(255, 255, 255, 180));
        acrylicBrush_.setLuminosityColor(QColor(255, 255, 255, 0));
    }
}

bool NavigationPanel::canDrawAcrylic() const {
    return acrylicBrush_.isAvailable() && isAcrylicEnabled_;
}

void NavigationPanel::paintEvent(QPaintEvent* e) {
    if (!canDrawAcrylic() || displayMode_ != NavigationDisplayMode::Menu) {
        QFrame::paintEvent(e);
        return;
    }

    QPainter painter(this);

    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    path.addRoundedRect(0, 1, width() - 1, height() - 1, 7, 7);
    path.addRect(0, 1, 8, height() - 1);
    acrylicBrush_.setClipPath(path);

    updateAcrylicColor();
    acrylicBrush_.paint(&painter);
    QFrame::paintEvent(e);
}

}  // namespace qfw
