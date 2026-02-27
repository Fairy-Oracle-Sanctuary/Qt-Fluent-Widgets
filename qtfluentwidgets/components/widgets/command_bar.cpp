#include "components/widgets/command_bar.h"

#include <QApplication>
#include <QDebug>
#include <QFontMetrics>
#include <QHoverEvent>
#include <QLayout>
#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>

#include "common/font.h"
#include "common/icon.h"
#include "common/style_sheet.h"

namespace qfw {

// ============================================================================
// CommandButton
// ============================================================================

CommandButton::CommandButton(QWidget* parent) : TransparentToggleToolButton(parent) {
    setCheckable(false);
    setToolButtonStyle(Qt::ToolButtonIconOnly);
    qfw::setFont(this, 12);
}

void CommandButton::setTight(bool tight) {
    if (tight_ == tight) {
        return;
    }

    tight_ = tight;
    update();
}

bool CommandButton::isTight() const { return tight_; }

bool CommandButton::isIconOnly() const {
    if (text_.isEmpty()) {
        return true;
    }

    const auto style = toolButtonStyle();
    return style == Qt::ToolButtonIconOnly || style == Qt::ToolButtonFollowStyle;
}

QSize CommandButton::sizeHint() const {
    if (isIconOnly()) {
        return tight_ ? QSize(36, 34) : QSize(48, 34);
    }

    const int tw = fontMetrics().boundingRect(text()).width();

    const auto style = toolButtonStyle();
    if (style == Qt::ToolButtonTextBesideIcon) {
        return QSize(tw + 47, 34);
    }
    if (style == Qt::ToolButtonTextOnly) {
        return QSize(tw + 32, 34);
    }

    return QSize(tw + 32, 50);
}

QString CommandButton::text() const { return text_; }

void CommandButton::setText(const QString& text) {
    if (text_ == text) {
        return;
    }

    text_ = text;
    update();
}

void CommandButton::setAction(QAction* action) {
    action_ = action;
    onActionChanged();

    if (!action) {
        return;
    }

    connect(this, &QToolButton::clicked, action, &QAction::trigger);
    connect(action, &QAction::toggled, this, &QToolButton::setChecked);
    connect(action, &QAction::changed, this, &CommandButton::onActionChanged);

    installEventFilter(new CommandToolTipFilter(this, 700));
}

QAction* CommandButton::action() const { return action_; }

void CommandButton::drawIcon(QPainter* painter, const QRectF& rect, QIcon::State state) {
    // Empty implementation - CommandButton handles icon drawing in paintEvent
    // This prevents parent class from drawing the icon
    Q_UNUSED(painter);
    Q_UNUSED(rect);
    Q_UNUSED(state);
}

void CommandButton::onActionChanged() {
    if (!action_) {
        return;
    }

    setIcon(action_->icon());
    setText(action_->text());
    setToolTip(action_->toolTip());
    setEnabled(action_->isEnabled());
    setCheckable(action_->isCheckable());
    setChecked(action_->isChecked());
}

void CommandButton::paintEvent(QPaintEvent* e) {
    QToolButton::paintEvent(e);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    if (!isChecked()) {
        painter.setPen(isDarkTheme() ? Qt::white : Qt::black);
    } else {
        painter.setPen(isDarkTheme() ? Qt::black : Qt::white);
    }

    if (!isEnabled()) {
        painter.setOpacity(0.43);
    } else if (isPressed) {
        painter.setOpacity(0.63);
    }

    const auto style = toolButtonStyle();
    const int iw = iconSize().width();
    const int ih = iconSize().height();

    if (isIconOnly()) {
        const qreal y = (height() - ih) / 2.0;
        const qreal x = (width() - iw) / 2.0;
        painter.drawPixmap(QRect(x, y, iw, ih), icon().pixmap(iconSize()));
        return;
    }

    if (style == Qt::ToolButtonTextOnly) {
        painter.drawText(rect(), Qt::AlignCenter, text());
        return;
    }

    if (style == Qt::ToolButtonTextBesideIcon) {
        const qreal y = (height() - ih) / 2.0;
        ToggleToolButton::drawIcon(&painter, QRectF(11, y, iw, ih),
                                   isChecked() ? QIcon::On : QIcon::Off);
        const QRectF r(26, 0, width() - 26, height());
        painter.drawText(r, Qt::AlignCenter, text());
        return;
    }

    if (style == Qt::ToolButtonTextUnderIcon) {
        const qreal x = (width() - iw) / 2.0;
        ToggleToolButton::drawIcon(&painter, QRectF(x, 9, iw, ih),
                                   isChecked() ? QIcon::On : QIcon::Off);
        const QRectF r(0, ih + 13, width(), height() - ih - 13);
        painter.drawText(r, Qt::AlignHCenter | Qt::AlignTop, text());
        return;
    }
}

// ============================================================================
// CommandToolTipFilter
// ============================================================================

bool CommandToolTipFilter::canShowToolTip() {
    auto* btn = qobject_cast<CommandButton*>(parent());
    return ToolTipFilter::canShowToolTip() && btn && btn->isIconOnly();
}

// ============================================================================
// MoreActionsButton
// ============================================================================

MoreActionsButton::MoreActionsButton(QWidget* parent) : CommandButton(parent) {
    setIcon(FluentIcon(FluentIconEnum::More));
}

QSize MoreActionsButton::sizeHint() const { return QSize(40, 34); }

void MoreActionsButton::clearState() {
    setAttribute(Qt::WA_UnderMouse, false);
    QHoverEvent e(QEvent::HoverLeave, QPointF(-1, -1), QPointF());
    QApplication::sendEvent(this, &e);
}

// ============================================================================
// CommandSeparator
// ============================================================================

CommandSeparator::CommandSeparator(QWidget* parent) : QWidget(parent) { setFixedSize(9, 34); }

void CommandSeparator::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);

    QPainter painter(this);
    painter.setPen(isDarkTheme() ? QColor(255, 255, 255, 21) : QColor(0, 0, 0, 15));
    painter.drawLine(5, 2, 5, height() - 2);
}

// ============================================================================
// CommandMenu
// ============================================================================

CommandMenu::CommandMenu(QWidget* parent) : RoundMenu(QString(), parent) {
    setItemHeight(32);
    if (view()) {
        view()->setIconSize(QSize(16, 16));
    }
}

// ============================================================================
// CommandBar
// ============================================================================

CommandBar::CommandBar(QWidget* parent) : QFrame(parent) {
    moreButton_ = new MoreActionsButton(this);
    connect(moreButton_, &QToolButton::clicked, this, &CommandBar::showMoreActionsMenu);
    moreButton_->hide();

    qfw::setFont(this, 12);
    setAttribute(Qt::WA_TranslucentBackground);
}

void CommandBar::setSpacing(int spacing) {
    if (spacing_ == spacing) {
        return;
    }

    spacing_ = spacing;
    updateGeometry();
}

int CommandBar::spacing() const { return spacing_; }

CommandButton* CommandBar::addAction(QAction* action) {
    if (!action) {
        return nullptr;
    }

    if (actions().contains(action)) {
        return nullptr;
    }

    CommandButton* button = createButton(action);
    insertWidgetToLayout(-1, button);
    QFrame::addAction(action);
    return button;
}

void CommandBar::addActions(const QList<QAction*>& actions) {
    for (auto* a : actions) {
        addAction(a);
    }
}

void CommandBar::addHiddenAction(QAction* action) {
    if (!action) {
        return;
    }

    if (actions().contains(action)) {
        return;
    }

    hiddenActions_.append(action);
    updateGeometry();
    QFrame::addAction(action);
}

void CommandBar::addHiddenActions(const QList<QAction*>& actions) {
    for (auto* a : actions) {
        addHiddenAction(a);
    }
}

CommandButton* CommandBar::insertAction(QAction* before, QAction* action) {
    if (!before || !action) {
        return nullptr;
    }

    if (!actions().contains(before)) {
        return nullptr;
    }

    const int index = actions().indexOf(before);
    CommandButton* button = createButton(action);
    insertWidgetToLayout(index, button);
    QFrame::insertAction(before, action);
    return button;
}

void CommandBar::addSeparator() { insertSeparator(-1); }

void CommandBar::insertSeparator(int index) {
    insertWidgetToLayout(index, new CommandSeparator(this));
}

void CommandBar::addWidget(QWidget* widget) { insertWidgetToLayout(-1, widget); }

void CommandBar::removeAction(QAction* action) {
    if (!action) {
        return;
    }

    if (!actions().contains(action)) {
        return;
    }

    for (QWidget* w : std::as_const(widgets_)) {
        auto* btn = qobject_cast<CommandButton*>(w);
        if (btn && btn->action() == action) {
            widgets_.removeOne(w);
            w->hide();
            w->deleteLater();
            break;
        }
    }

    updateGeometry();
}

void CommandBar::removeWidget(QWidget* widget) {
    if (!widget) {
        return;
    }

    if (!widgets_.contains(widget)) {
        return;
    }

    widgets_.removeOne(widget);
    updateGeometry();
}

void CommandBar::removeHiddenAction(QAction* action) {
    hiddenActions_.removeOne(action);
    updateGeometry();
}

void CommandBar::setToolButtonStyle(Qt::ToolButtonStyle style) {
    if (toolButtonStyle_ == style) {
        return;
    }

    toolButtonStyle_ = style;
    for (auto* b : commandButtons()) {
        b->setToolButtonStyle(style);
    }
    updateGeometry();
}

Qt::ToolButtonStyle CommandBar::toolButtonStyle() const { return toolButtonStyle_; }

void CommandBar::setButtonTight(bool tight) {
    if (buttonTight_ == tight) {
        return;
    }

    buttonTight_ = tight;
    for (auto* b : commandButtons()) {
        b->setTight(tight);
    }

    updateGeometry();
}

bool CommandBar::isButtonTight() const { return buttonTight_; }

void CommandBar::setIconSize(const QSize& size) {
    if (iconSize_ == size) {
        return;
    }

    iconSize_ = size;
    for (auto* b : commandButtons()) {
        b->setIconSize(size);
    }

    updateGeometry();
}

QSize CommandBar::iconSize() const { return iconSize_; }

void CommandBar::resizeEvent(QResizeEvent* e) {
    QFrame::resizeEvent(e);
    updateGeometry();
}

CommandButton* CommandBar::createButton(QAction* action) {
    auto* button = new CommandButton(this);
    button->setAction(action);
    button->setToolButtonStyle(toolButtonStyle());
    button->setTight(isButtonTight());
    button->setIconSize(iconSize());
    button->setFont(font());
    return button;
}

void CommandBar::insertWidgetToLayout(int index, QWidget* widget) {
    if (!widget) {
        return;
    }

    widget->setParent(this);
    widget->show();

    if (index < 0 || index >= widgets_.size()) {
        widgets_.append(widget);
    } else {
        widgets_.insert(index, widget);
    }

    int maxH = 0;
    for (auto* w : std::as_const(widgets_)) {
        maxH = qMax(maxH, w->height());
    }
    if (maxH > 0) {
        setFixedHeight(maxH);
    }

    updateGeometry();
}

QSize CommandBar::minimumSizeHint() const { return moreButton_->size(); }

QList<CommandButton*> CommandBar::commandButtons() const {
    QList<CommandButton*> btns;
    for (auto* w : widgets_) {
        if (auto* b = qobject_cast<CommandButton*>(w)) {
            btns.append(b);
        }
    }
    return btns;
}

QList<QWidget*> CommandBar::visibleWidgets() const {
    if (suitableWidth() <= width()) {
        return widgets_;
    }

    int w = moreButton_->width();
    int index = 0;
    for (index = 0; index < widgets_.size(); ++index) {
        w += widgets_[index]->width();
        if (index > 0) {
            w += spacing();
        }

        if (w > width()) {
            break;
        }
    }

    return widgets_.mid(0, index);
}

void CommandBar::updateGeometry() {
    hiddenWidgets_.clear();
    const QList<QWidget*> visibles = visibleWidgets();
    for (int i = 0; i < widgets_.size(); ++i) {
        if (!visibles.contains(widgets_[i])) {
            hiddenWidgets_.append(widgets_[i]);
        }
    }
    moreButton_->hide();

    int x = contentsMargins().left();
    const int h = height();

    for (auto* widget : visibles) {
        if (widget) {
            widget->show();
            widget->move(x, (h - widget->height()) / 2);
            x += widget->width() + spacing();
        }
    }

    if (!hiddenActions_.isEmpty() || !hiddenWidgets_.isEmpty()) {
        moreButton_->show();
        moreButton_->move(x, (h - moreButton_->height()) / 2);
    }

    for (auto* w : std::as_const(hiddenWidgets_)) {
        if (w) w->hide();
    }
}

int CommandBar::suitableWidth() const {
    QList<int> widths;
    widths.reserve(widgets_.size() + 1);
    for (auto* w : widgets_) {
        widths.append(w->width());
    }

    if (!hiddenActions_.isEmpty()) {
        widths.append(moreButton_->width());
    }

    int sum = 0;
    for (int v : widths) {
        sum += v;
    }

    return sum + spacing() * qMax(widths.size() - 1, 0);
}

void CommandBar::resizeToSuitableWidth() { setFixedWidth(suitableWidth()); }

void CommandBar::setMenuDropDown(bool down) {
    menuAnimation_ = down ? MenuAnimationType::DropDown : MenuAnimationType::PullUp;
}

bool CommandBar::isMenuDropDown() const { return menuAnimation_ == MenuAnimationType::DropDown; }

void CommandBar::showMoreActionsMenu() {
    moreButton_->clearState();

    QList<QAction*> actions;
    for (const auto& ap : std::as_const(hiddenActions_)) {
        QAction* a = ap.data();
        if (a) {
            actions.append(a);
        }
    }

    for (auto it = hiddenWidgets_.crbegin(); it != hiddenWidgets_.crend(); ++it) {
        if (auto* b = qobject_cast<CommandButton*>(*it)) {
            if (b->action()) {
                actions.prepend(b->action());
            }
        }
    }

    auto* menu = new CommandMenu(this);
    menu->addActions(actions);

    int x = -menu->width() + menu->layout()->contentsMargins().right() + moreButton_->width() + 18;
    int y = (menuAnimation_ == MenuAnimationType::DropDown) ? moreButton_->height() : -5;

    const QPoint pos = moreButton_->mapToGlobal(QPoint(x, y));
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->execAt(pos, true, menuAnimation_);
}

// ============================================================================
// CommandViewMenu
// ============================================================================

CommandViewMenu::CommandViewMenu(QWidget* parent) : CommandMenu(parent) {
    if (view()) {
        view()->setObjectName(QStringLiteral("commandListWidget"));
    }
}

void CommandViewMenu::setDropDown(bool down, bool isLong) {
    if (!view()) {
        return;
    }

    view()->setProperty("dropDown", down);
    view()->setProperty("long", isLong);
    updateDynamicStyle(view());
}

// ============================================================================
// CommandViewBar
// ============================================================================

CommandViewBar::CommandViewBar(QWidget* parent) : CommandBar(parent) { setMenuDropDown(true); }

void CommandViewBar::setMenuDropDown(bool down) {
    menuAnimation_ = down ? MenuAnimationType::FadeInDropDown : MenuAnimationType::FadeInPullUp;
}

bool CommandViewBar::isMenuDropDown() const {
    return menuAnimation_ == MenuAnimationType::FadeInDropDown;
}

void CommandViewBar::showMoreActionsMenu() {
    moreButton_->clearState();

    QList<QAction*> actions;
    for (const auto& ap : std::as_const(hiddenActions_)) {
        QAction* a = ap.data();
        if (a) {
            actions.append(a);
        }
    }

    for (auto it = hiddenWidgets_.crbegin(); it != hiddenWidgets_.crend(); ++it) {
        if (auto* b = qobject_cast<CommandButton*>(*it)) {
            if (b->action()) {
                actions.prepend(b->action());
            }
        }
    }

    auto* menu = new CommandViewMenu(this);
    menu->addActions(actions);

    auto* view = qobject_cast<CommandBarView*>(parentWidget());
    if (view) {
        view->setMenuVisible(true);
        connect(menu, &RoundMenu::closedSignal, view, [view]() { view->setMenuVisible(false); });
        const bool isLong =
            menu->view() && view->width() > 0 && menu->view()->width() > view->width() + 5;
        menu->setDropDown(isMenuDropDown(), isLong);

        if (menu->view() && menu->view()->width() < view->width()) {
            menu->view()->setFixedWidth(view->width());
            menu->adjustSize();
        }
    }

    int x = -menu->width() + menu->layout()->contentsMargins().right() + moreButton_->width() + 18;

    int y = 0;
    if (isMenuDropDown()) {
        y = moreButton_->height();
    } else {
        y = -13;
        menu->setShadowEffect(0, QPoint(0, 0), QColor(0, 0, 0, 0));
        menu->layout()->setContentsMargins(12, 20, 12, 8);
    }

    const QPoint pos = moreButton_->mapToGlobal(QPoint(x, y));
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->execAt(pos, true, menuAnimation_);
}

// ============================================================================
// CommandBarView
// ============================================================================

CommandBarView::CommandBarView(QWidget* parent) : FlyoutViewBase(parent) {
    bar_ = new CommandViewBar(this);

    hBoxLayout_ = new QHBoxLayout(this);
    hBoxLayout_->setContentsMargins(6, 6, 6, 6);
    hBoxLayout_->addWidget(bar_);
    hBoxLayout_->setSizeConstraint(QHBoxLayout::SetMinAndMaxSize);

    setButtonTight(true);
    setIconSize(QSize(14, 14));
}

void CommandBarView::setMenuVisible(bool visible) {
    if (menuVisible_ == visible) {
        return;
    }

    menuVisible_ = visible;
    update();
}

void CommandBarView::addWidget(QWidget* widget, int stretch, Qt::Alignment align) {
    Q_UNUSED(stretch);
    Q_UNUSED(align);
    bar_->addWidget(widget);
}

void CommandBarView::setSpacing(int spacing) { bar_->setSpacing(spacing); }

int CommandBarView::spacing() const { return bar_->spacing(); }

CommandButton* CommandBarView::addAction(QAction* action) { return bar_->addAction(action); }

void CommandBarView::addActions(const QList<QAction*>& actions) { bar_->addActions(actions); }

void CommandBarView::addHiddenAction(QAction* action) { bar_->addHiddenAction(action); }

void CommandBarView::addHiddenActions(const QList<QAction*>& actions) {
    bar_->addHiddenActions(actions);
}

CommandButton* CommandBarView::insertAction(QAction* before, QAction* action) {
    return bar_->insertAction(before, action);
}

void CommandBarView::addSeparator() { bar_->addSeparator(); }

void CommandBarView::insertSeparator(int index) { bar_->insertSeparator(index); }

void CommandBarView::removeAction(QAction* action) { bar_->removeAction(action); }

void CommandBarView::removeWidget(QWidget* widget) { bar_->removeWidget(widget); }

void CommandBarView::removeHiddenAction(QAction* action) { bar_->removeHiddenAction(action); }

void CommandBarView::setToolButtonStyle(Qt::ToolButtonStyle style) {
    bar_->setToolButtonStyle(style);
}

Qt::ToolButtonStyle CommandBarView::toolButtonStyle() const { return bar_->toolButtonStyle(); }

void CommandBarView::setButtonTight(bool tight) { bar_->setButtonTight(tight); }

bool CommandBarView::isButtonTight() const { return bar_->isButtonTight(); }

void CommandBarView::setIconSize(const QSize& size) { bar_->setIconSize(size); }

QSize CommandBarView::iconSize() const { return bar_->iconSize(); }

int CommandBarView::suitableWidth() const {
    const QMargins m = contentsMargins();
    return m.left() + m.right() + bar_->suitableWidth();
}

void CommandBarView::resizeToSuitableWidth() {
    bar_->resizeToSuitableWidth();
    setFixedWidth(suitableWidth());
}

QList<QAction*> CommandBarView::actions() const { return bar_->actions(); }

void CommandBarView::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    path.addRoundedRect(QRectF(rect().adjusted(1, 1, -1, -1)), 8, 8);

    if (menuVisible_) {
        const int y = bar_->isMenuDropDown() ? height() - 10 : 1;
        path.addRect(1, y, width() - 2, 9);
    }

    painter.setBrush(isDarkTheme() ? QColor(40, 40, 40) : QColor(248, 248, 248));
    painter.setPen(isDarkTheme() ? QColor(56, 56, 56) : QColor(233, 233, 233));
    painter.drawPath(path.simplified());
}

}  // namespace qfw
