#include "components/navigation/breadcrumb.h"

#include <QAction>
#include <QApplication>
#include <QFont>
#include <QFontMetrics>
#include <QHoverEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>

#include "common/config.h"
#include "common/font.h"
#include "common/icon.h"
#include "common/screen.h"
#include "components/widgets/menu.h"

namespace qfw {

BreadcrumbWidget::BreadcrumbWidget(QWidget* parent) : QWidget(parent) { setMouseTracking(true); }

void BreadcrumbWidget::mousePressEvent(QMouseEvent* e) {
    QWidget::mousePressEvent(e);
    isPressed_ = true;
    update();
}

void BreadcrumbWidget::mouseReleaseEvent(QMouseEvent* e) {
    QWidget::mouseReleaseEvent(e);
    isPressed_ = false;
    update();
    emit clicked();
}

void BreadcrumbWidget::enterEvent(QEnterEvent* e) {
    QWidget::enterEvent(e);
    isHover_ = true;
    update();
}

void BreadcrumbWidget::leaveEvent(QEvent* e) {
    QWidget::leaveEvent(e);
    isHover_ = false;
    update();
}

ElideButton::ElideButton(QWidget* parent) : BreadcrumbWidget(parent) { setFixedSize(16, 16); }

void ElideButton::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    if (isPressed_) {
        painter.setOpacity(0.5);
    } else if (!isHover_) {
        painter.setOpacity(0.61);
    }

    FluentIcon(FluentIconEnum::More).render(&painter, rect());
}

void ElideButton::clearState() {
    setAttribute(Qt::WA_UnderMouse, false);
    isHover_ = false;

    QHoverEvent e(QEvent::HoverLeave, QPoint(-1, -1), QPoint());
    QApplication::sendEvent(this, &e);
}

BreadcrumbItem::BreadcrumbItem(const QString& routeKey, const QString& text, int index,
                               QWidget* parent)
    : BreadcrumbWidget(parent), text_(text), routeKey_(routeKey), index_(index) {
    setText(text);
}

void BreadcrumbItem::setText(const QString& text) {
    text_ = text;

    const QFontMetrics fm(font());
    const QRect r = fm.boundingRect(text);

    int w = r.width() + static_cast<int>(std::ceil(font().pixelSize() / 10.0));
    if (!isRoot()) {
        w += spacing_ * 2;
    }

    setFixedWidth(w);
    setFixedHeight(r.height());
    update();
}

bool BreadcrumbItem::isRoot() const { return index_ == 0; }

void BreadcrumbItem::setSelected(bool selected) {
    isSelected_ = selected;
    update();
}

void BreadcrumbItem::setFont(const QFont& f) {
    QWidget::setFont(f);
    setText(text_);
}

void BreadcrumbItem::setSpacing(int spacing) {
    spacing_ = spacing;
    setText(text_);
}

void BreadcrumbItem::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);

    QPainter painter(this);
    painter.setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    const int sw = spacing_ * 2;

    if (!isRoot()) {
        const int iw = qRound(font().pixelSize() / 14.0 * 8.0);
        const QRectF iconRect((sw - iw) / 2.0, (height() - iw) / 2.0 + 1.0, iw, iw);

        painter.setOpacity(0.61);
        FluentIcon(FluentIconEnum::ChevronRightMed).render(&painter, iconRect.toRect());
    }

    if (isPressed_) {
        const qreal alpha = isDarkTheme() ? 0.54 : 0.45;
        painter.setOpacity(isSelected_ ? 1.0 : alpha);
    } else if (isSelected_ || isHover_) {
        painter.setOpacity(1.0);
    } else {
        painter.setOpacity(isDarkTheme() ? 0.79 : 0.61);
    }

    painter.setFont(font());
    painter.setPen(isDarkTheme() ? Qt::white : Qt::black);

    QRectF textRect;
    if (isRoot()) {
        textRect = rect();
    } else {
        textRect = QRectF(sw, 0, width() - sw, height());
    }

    painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, text_);
}

BreadcrumbBar::BreadcrumbBar(QWidget* parent) : QWidget(parent) {
    qfw::setFont(this, 14);
    setAttribute(Qt::WA_TranslucentBackground);

    elideButton_ = new ElideButton(this);
    elideButton_->hide();

    connect(elideButton_, &ElideButton::clicked, this, &BreadcrumbBar::showHiddenItemsMenu);
}

void BreadcrumbBar::addItem(const QString& routeKey, const QString& text) {
    if (itemMap_.contains(routeKey)) {
        return;
    }

    auto* item = new BreadcrumbItem(routeKey, text, items_.size(), this);
    item->setFont(font());
    item->setSpacing(spacing_);

    connect(item, &BreadcrumbItem::clicked, this, [this, routeKey]() { setCurrentItem(routeKey); });

    itemMap_.insert(routeKey, item);
    items_.append(item);

    int maxH = 0;
    for (auto* i : items_) {
        maxH = qMax(maxH, i->height());
    }
    setFixedHeight(maxH);

    setCurrentItem(routeKey);
    updateGeometryInternal();
}

void BreadcrumbBar::setCurrentIndex(int index) {
    if (index < 0 || index >= items_.size() || index == currentIndex_) {
        return;
    }

    if (currentIndex_ >= 0 && currentIndex_ < items_.size()) {
        if (auto* cur = currentItem()) {
            cur->setSelected(false);
        }
    }

    currentIndex_ = index;
    if (auto* cur = currentItem()) {
        cur->setSelected(true);
    }

    while (items_.size() - 1 > index) {
        BreadcrumbItem* it = items_.takeLast();
        itemMap_.remove(it->routeKey());
        it->deleteLater();
    }

    updateGeometryInternal();

    emit currentIndexChanged(index);
    if (auto* cur = currentItem()) {
        emit currentItemChanged(cur->routeKey());
    }
}

void BreadcrumbBar::setCurrentItem(const QString& routeKey) {
    if (!itemMap_.contains(routeKey)) {
        return;
    }

    setCurrentIndex(items_.indexOf(itemMap_.value(routeKey)));
}

void BreadcrumbBar::setItemText(const QString& routeKey, const QString& text) {
    if (auto* it = item(routeKey)) {
        it->setText(text);
        updateGeometryInternal();
    }
}

BreadcrumbItem* BreadcrumbBar::item(const QString& routeKey) const {
    return itemMap_.value(routeKey, nullptr);
}

BreadcrumbItem* BreadcrumbBar::itemAt(int index) const {
    if (index >= 0 && index < items_.size()) {
        return items_.at(index);
    }
    return nullptr;
}

BreadcrumbItem* BreadcrumbBar::currentItem() const {
    if (currentIndex_ >= 0 && currentIndex_ < items_.size()) {
        return items_.at(currentIndex_);
    }
    return nullptr;
}

void BreadcrumbBar::resizeEvent(QResizeEvent* e) {
    QWidget::resizeEvent(e);
    updateGeometryInternal();
}

void BreadcrumbBar::clear() {
    while (!items_.isEmpty()) {
        BreadcrumbItem* it = items_.takeLast();
        itemMap_.remove(it->routeKey());
        it->deleteLater();
    }

    elideButton_->hide();
    currentIndex_ = -1;
}

void BreadcrumbBar::popItem() {
    if (items_.isEmpty()) {
        return;
    }

    if (count() >= 2) {
        setCurrentIndex(currentIndex_ - 1);
    } else {
        clear();
    }
}

int BreadcrumbBar::count() const { return items_.size(); }

void BreadcrumbBar::updateGeometryInternal() {
    if (items_.isEmpty()) {
        return;
    }

    int x = 0;
    elideButton_->hide();

    hiddenItems_.clear();
    for (int i = 0; i < items_.size() - 1; ++i) {
        hiddenItems_.append(items_.at(i));
    }

    QVector<QWidget*> visibleItems;

    if (!isElideVisible()) {
        visibleItems.reserve(items_.size());
        for (auto* it : items_) {
            visibleItems.append(it);
        }
        hiddenItems_.clear();
    } else {
        visibleItems = {elideButton_, items_.last()};
        int w = 0;
        for (auto* v : visibleItems) {
            w += v->width();
        }

        for (int i = items_.size() - 2; i >= 0; --i) {
            BreadcrumbItem* it = items_.at(i);
            w += it->width();
            if (w > width()) {
                break;
            }

            visibleItems.insert(1, it);
            hiddenItems_.removeOne(it);
        }
    }

    for (auto* it : hiddenItems_) {
        it->hide();
    }

    for (auto* w : visibleItems) {
        w->move(x, (height() - w->height()) / 2);
        w->show();
        x += w->width();
    }
}

bool BreadcrumbBar::isElideVisible() const {
    int w = 0;
    for (auto* it : items_) {
        w += it->width();
    }
    return w > width();
}

void BreadcrumbBar::setFont(const QFont& f) {
    QWidget::setFont(f);

    const int s = qRound(f.pixelSize() / 14.0 * 16.0);
    elideButton_->setFixedSize(s, s);

    for (auto* it : items_) {
        it->setFont(f);
    }

    updateGeometryInternal();
}

void BreadcrumbBar::showHiddenItemsMenu() {
    if (!elideButton_) {
        return;
    }

    elideButton_->clearState();

    auto* menu = new RoundMenu(QString(), this);
    menu->setItemHeight(32);

    for (auto* it : hiddenItems_) {
        auto* act = new QAction(it->text(), menu);
        connect(act, &QAction::triggered, this, [this, it]() { setCurrentItem(it->routeKey()); });
        menu->addAction(act);
    }

    const int left = (menu->layout() ? menu->layout()->contentsMargins().left() : 0);
    const int x = -left;

    const QPoint pd = mapToGlobal(QPoint(x, height()));
    const QPoint pu = mapToGlobal(QPoint(x, 0));

    const QRect ss = getCurrentScreenGeometry(true);
    const int hd = qMax(ss.bottom() - pd.y() - 10, 1);
    const int hu = qMax(pu.y() - ss.top() - 28, 1);

    if (hd >= hu) {
        menu->execAt(pd, true, MenuAnimationType::DropDown);
    } else {
        menu->execAt(pu, true, MenuAnimationType::PullUp);
    }
}

void BreadcrumbBar::setSpacing(int spacing) {
    if (spacing == spacing_) {
        return;
    }

    spacing_ = spacing;
    for (auto* it : items_) {
        it->setSpacing(spacing);
    }

    updateGeometryInternal();
}

}  // namespace qfw
