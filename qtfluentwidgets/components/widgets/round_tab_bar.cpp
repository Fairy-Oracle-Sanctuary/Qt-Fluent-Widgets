#include "round_tab_bar.h"

#include <QWheelEvent>

#include "common/config.h"
#include "common/icon.h"

namespace qfw {
namespace {
class RoundTabLayout : public QHBoxLayout {
public:
    int overlapWidth() const {
        int visible = 0;
        for (int i = 0; i < count(); ++i)
            if (!itemAt(i)->isEmpty()) ++visible;
        return qMax(0, visible - 1) * 5;
    }
    QSize compact(QSize size) const {
        size.setWidth(qMax(0, size.width() - overlapWidth()));
        return size;
    }
    QSize sizeHint() const override { return compact(QHBoxLayout::sizeHint()); }
    QSize minimumSize() const override { return compact(QHBoxLayout::minimumSize()); }
    QSize maximumSize() const override { return compact(QHBoxLayout::maximumSize()); }
    void setGeometry(const QRect& rect) override {
        QRect expanded = rect;
        expanded.setWidth(rect.width() + overlapWidth());
        QHBoxLayout::setGeometry(expanded);
        const bool rtl = parentWidget() && parentWidget()->isRightToLeft();
        int x = rtl ? rect.right() + 1 - contentsMargins().right()
                    : rect.x() + contentsMargins().left();
        for (int i = 0; i < count(); ++i) {
            auto* item = itemAt(i);
            if (item->isEmpty()) continue;
            QRect geometry = item->geometry();
            geometry.moveLeft(rtl ? x - geometry.width() : x);
            item->setGeometry(geometry);
            x += (rtl ? -1 : 1) * (geometry.width() - 5);
        }
    }
};
}  // namespace

RoundTabItem::RoundTabItem(QWidget* parent) : TabItem(parent) { initRound(); }
RoundTabItem::RoundTabItem(const QString& text, QWidget* parent) : TabItem(text, parent) {
    initRound();
}
RoundTabItem::RoundTabItem(FluentIconEnum icon, const QString& text, QWidget* parent)
    : TabItem(icon, text, parent) {
    initRound();
}
void RoundTabItem::initRound() {
    setFixedHeight(32);
    setBorderRadius(8);
    setShadowEnabled(false);
}
QSize RoundTabItem::sizeHint() const { return QSize(maximumWidth(), 32); }
void RoundTabItem::clearIcon() {
    hasIcon_ = false;
    FluentPushButton::setIcon(QIcon());
    update();
}
void RoundTabItem::setShadowEnabled(bool enabled) {
    TabItem::setShadowEnabled(enabled);
    shadowEffect_->setEnabled(enabled);
}
QPainterPath RoundTabItem::selectedPath(bool outline, qreal bottom) const {
    const qreal h = bottom < 0 ? height() : bottom;
    const qreal edge = outline ? .5 : 0;
    const qreal w = width() - 2 * edge;
    const qreal radius = qMax(qreal(0), qMin(qreal(borderRadius()), qMin(h / 2, w / 4)));
    const qreal wing = qMin(qreal(5), radius);
    QPainterPath path;
    path.moveTo(edge, h);
    if (wing) path.arcTo(QRectF(edge - wing, h - 2 * wing, 2 * wing, 2 * wing), 270, 90);
    path.lineTo(edge + wing, edge + radius);
    path.quadTo(edge + wing, edge, edge + wing + radius, edge);
    path.lineTo(edge + w - wing - radius, edge);
    path.quadTo(edge + w - wing, edge, edge + w - wing, edge + radius);
    path.lineTo(edge + w - wing, h - wing);
    if (wing)
        path.arcTo(QRectF(edge + w - wing, h - 2 * wing, 2 * wing, 2 * wing), 180, 90);
    else
        path.lineTo(edge + w, h);
    if (!outline) path.closeSubpath();
    return path;
}
void RoundTabItem::drawRoundSelectedBackground(QPainter* painter) {
    painter->setPen(Qt::NoPen);
    painter->setBrush(isDarkTheme() ? darkSelectedBackgroundColor_ : lightSelectedBackgroundColor_);
    painter->drawPath(selectedPath());
    painter->setBrush(Qt::NoBrush);
    painter->setPen(QPen(isDarkTheme() ? QColor(255, 255, 255, 18) : QColor(0, 0, 0, 15), 1));
    painter->drawPath(selectedPath(true));
}
void RoundTabItem::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    if (isSelected())
        drawRoundSelectedBackground(&painter);
    else if (isHover || isPressed) {
        const int shade = isDarkTheme() ? 255 : 0;
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(shade, shade, shade, isPressed ? 12 : 10));
        painter.drawRoundedRect(QRectF(0, 0, width(), height() - 2), borderRadius(),
                                borderRadius());
    }
    if (!isEnabled())
        painter.setOpacity(.4);
    else if (!isSelected())
        painter.setOpacity(isDarkTheme() ? .79 : .61);
    if (hasIcon_) FluentIcon(iconEnum_).render(&painter, QRect(10, 8, 16, 16));
    drawText(&painter);
}

RoundTabBar::RoundTabBar(QWidget* parent) : TabBar(parent) {
    hBoxLayout_->removeItem(itemLayout_);
    delete itemLayout_;
    itemLayout_ = new RoundTabLayout;
    hBoxLayout_->insertLayout(0, itemLayout_);
    itemLayout_->setSizeConstraint(QLayout::SetMinAndMaxSize);
    itemLayout_->setContentsMargins(16, 6, 0, 0);
    itemLayout_->setSpacing(0);
    itemLayout_->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    setFixedHeight(38);
    setTabMaximumWidth(200);
    setTabShadowEnabled(false);
    connect(&QConfig::instance(), &QConfig::themeChangedFinished, this, [this]() {
        viewport()->update();
        for (auto* item : items_) item->update();
    });
}
TabItem* RoundTabBar::createTabItem(const QString& text, FluentIconEnum icon) {
    return new RoundTabItem(icon, text, view_);
}
TabItem* RoundTabBar::addTab(const QString& routeKey, const QString& text) {
    return insertTab(-1, routeKey, text);
}
TabItem* RoundTabBar::addTab(const QString& routeKey, const QString& text, FluentIconEnum icon) {
    return insertTab(-1, routeKey, text, icon);
}
TabItem* RoundTabBar::insertTab(int index, const QString& routeKey, const QString& text) {
    auto* item = static_cast<RoundTabItem*>(TabBar::insertTab(index, routeKey, text));
    if (item) item->clearIcon();
    return item;
}
TabItem* RoundTabBar::insertTab(int index, const QString& routeKey, const QString& text,
                                FluentIconEnum icon) {
    return TabBar::insertTab(index, routeKey, text, icon);
}
void RoundTabBar::setTabMaximumWidth(int width) {
    TabBar::setTabMaximumWidth(qMax(1, width));
    if (isScrollable()) setScrollable(true);
}
bool RoundTabBar::isTabVisible(int index) const {
    auto* item = tabItem(index);
    return item && !item->isHidden();
}
void RoundTabBar::setTabVisible(int index, bool visible) {
    auto* item = tabItem(index);
    if (!item || isTabVisible(index) == visible) return;
    item->setVisible(visible);
    if (!visible && currentIndex() == index) {
        int next = -1;
        for (int i = 1; i < count(); ++i) {
            const int candidate = (index + i) % count();
            if (isTabVisible(candidate) && isTabEnabled(candidate)) {
                next = candidate;
                break;
            }
        }
        setCurrentIndex(next);
        emit currentChanged(next);
    } else if (visible && currentIndex() < 0) {
        setCurrentIndex(index);
        emit currentChanged(index);
    }
    update();
}
QRect RoundTabBar::tabRect(int index) const {
    auto* item = tabItem(index);
    if (!item) return QRect();
    int x = isRightToLeft() ? view_->width() - itemLayout_->contentsMargins().right()
                            : itemLayout_->contentsMargins().left();
    for (int i = 0; i < index; ++i)
        if (isTabVisible(i)) x += (isRightToLeft() ? -1 : 1) * (tabItem(i)->width() - 5);
    QRect rect = item->geometry();
    rect.moveLeft(isRightToLeft() ? x - item->width() : x);
    return rect;
}
void RoundTabBar::paintEvent(QPaintEvent*) {
    QPainter painter(viewport());
    painter.setPen(isDarkTheme() ? QColor(255, 255, 255, 21) : QColor(0, 0, 0, 15));
    QList<TabItem*> visible;
    for (auto* item : items_)
        if (!item->isHidden()) visible.append(item);
    for (int i = 0; i < visible.size(); ++i) {
        auto* item = visible[i];
        auto* next = i + 1 < visible.size() ? visible[i + 1] : nullptr;
        if (item->isHovered() || item->isSelected() ||
            (next && (next->isHovered() || next->isSelected())))
            continue;
        const int x =
            item->mapTo(viewport(), QPoint(isRightToLeft() ? 0 : item->width() - 1, 0)).x();
        painter.drawLine(x, height() / 2 - 8, x, height() / 2 + 8);
    }
}
void RoundTabBar::mouseMoveEvent(QMouseEvent* event) {
    if (!(event->buttons() & Qt::LeftButton)) {
        event->ignore();
        return;
    }
    if (!isMovable() || count() < 2 || !itemLayout_->geometry().contains(event->pos())) return;
    auto* item = currentTab();
    if (!item || item->isHidden()) return;
    const int dx = event->pos().x() - dragPos_.x();
    dragPos_ = event->pos();
    if (!dx) return;
    QRect region;
    for (int i = 0; i < count(); ++i)
        if (isTabVisible(i)) region = region.united(tabRect(i));
    item->move(qBound(region.left(), item->x() + dx, region.right() + 1 - item->width()),
               item->y());
    isDraging_ = true;
    const int step = (dx > 0) != isRightToLeft() ? 1 : -1;
    int sibling = currentIndex() + step;
    while (isIndexValid(sibling, count()) && !isTabVisible(sibling)) sibling += step;
    if (!isIndexValid(sibling, count())) return;
    const int middle = tabItem(sibling)->geometry().center().x();
    if ((dx < 0 && item->x() < middle) || (dx > 0 && item->geometry().right() > middle))
        swapItem(sibling);
}
void RoundTabBar::wheelEvent(QWheelEvent* event) {
    if (isScrollable()) {
        TabBar::wheelEvent(event);
        event->accept();
    } else {
        event->ignore();
    }
}
bool RoundTabBar::eventFilter(QObject* watched, QEvent* event) {
    // SingleDirectionScrollArea's filter otherwise bypasses wheelEvent overrides.
    if (watched == viewport() && event->type() == QEvent::Wheel) {
        wheelEvent(static_cast<QWheelEvent*>(event));
        return true;
    }
    return TabBar::eventFilter(watched, event);
}
}  // namespace qfw
