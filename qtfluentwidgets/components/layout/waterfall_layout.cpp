#include "components/layout/waterfall_layout.h"

#include <QEvent>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QTimer>
#include <QWidget>
#include <QWidgetItem>
#include <algorithm>
#include <limits>
#include <stdexcept>

namespace qfw {
namespace {
int boundedDimension(qint64 value) {
    return int(std::clamp<qint64>(value, 0, std::numeric_limits<int>::max()));
}
int boundedCoordinate(qint64 value) {
    return int(std::clamp<qint64>(value, std::numeric_limits<int>::min(),
                                  std::numeric_limits<int>::max()));
}
}  // namespace
WaterfallLayout::WaterfallLayout(QWidget* parent, bool needAni, bool isTight)
    : QLayout(parent),
      animations_(new QParallelAnimationGroup(this)),
      debounce_(new QTimer(this)),
      animated_(needAni),
      tight_(isTight) {
    debounce_->setSingleShot(true);
    connect(debounce_, &QTimer::timeout, this, &WaterfallLayout::applyGeometry);
    if (parent) parent->installEventFilter(this);
}
WaterfallLayout::~WaterfallLayout() {
    debounce_->stop();
    animations_->stop();
    while (!entries_.isEmpty()) {
        auto entry = entries_.takeLast();
        if (entry.item->widget()) entry.item->widget()->removeEventFilter(this);
        delete entry.item;
    }
}
void WaterfallLayout::addItem(QLayoutItem* item) { insertItem(count(), item); }
void WaterfallLayout::insertItem(int index, QLayoutItem* item) {
    if (!item) throw std::invalid_argument("WaterfallLayout requires a layout item");
    for (const auto& entry : entries_)
        if (entry.item == item) return;
    Entry entry;
    entry.item = item;
    if (auto* widget = item->widget()) {
        widget->installEventFilter(this);
        if (animated_) {
            auto* animation = new QPropertyAnimation(widget, "geometry", animations_);
            animation->setDuration(duration_);
            animation->setEasingCurve(easing_);
            animations_->addAnimation(animation);
            entry.animation = animation;
        }
    }
    entries_.insert(std::clamp(index, 0, count()), entry);
    invalidate();
}
void WaterfallLayout::addWidget(QWidget* widget) { insertWidget(count(), widget); }
void WaterfallLayout::insertWidget(int index, QWidget* widget) {
    if (!widget) throw std::invalid_argument("WaterfallLayout requires a widget");
    for (const auto& entry : entries_)
        if (entry.item->widget() == widget) return;
    addChildWidget(widget);
    insertItem(index, new QWidgetItem(widget));
}
int WaterfallLayout::count() const { return entries_.size(); }
QLayoutItem* WaterfallLayout::itemAt(int index) const {
    return index >= 0 && index < count() ? entries_[index].item : nullptr;
}
QLayoutItem* WaterfallLayout::takeAt(int index) {
    if (index < 0 || index >= count()) return nullptr;
    auto entry = entries_.takeAt(index);
    if (auto* widget = entry.item->widget()) widget->removeEventFilter(this);
    if (entry.animation) {
        entry.animation->stop();
        animations_->removeAnimation(entry.animation);
        delete entry.animation;
    }
    invalidate();
    return entry.item;
}
void WaterfallLayout::removeWidget(QWidget* widget) {
    for (int i = 0; i < count(); ++i) {
        if (entries_[i].item->widget() != widget) continue;
        delete takeAt(i);
        if (widget) widget->hide();
        return;
    }
}
void WaterfallLayout::removeAllWidgets() {
    while (auto* item = takeAt(0)) {
        if (auto* widget = item->widget()) widget->hide();
        delete item;
    }
}
void WaterfallLayout::takeAllWidgets() {
    while (auto* item = takeAt(0)) {
        if (auto* widget = item->widget()) widget->deleteLater();
        delete item;
    }
}
Qt::Orientations WaterfallLayout::expandingDirections() const { return {}; }
bool WaterfallLayout::hasHeightForWidth() const { return true; }
int WaterfallLayout::heightForWidth(int width) const {
    return calculate(QRect(0, 0, std::max(0, width), 0));
}
QSize WaterfallLayout::sizeHint() const { return minimumSize(); }
QSize WaterfallLayout::minimumSize() const {
    QSize result(0, 0);
    for (const auto& entry : entries_) {
        auto* widget = entry.item->widget();
        if (tight_ && widget && widget->isHidden()) continue;
        auto size = entry.item->minimumSize();
        if (widget) size = widget->minimumSizeHint().expandedTo(widget->minimumSize());
        result = result.expandedTo(size);
    }
    const auto margins = contentsMargins();
    return result + QSize(margins.left() + margins.right(), margins.top() + margins.bottom());
}
void WaterfallLayout::setGeometry(const QRect& rect) {
    const bool changed = geometry() != rect;
    QLayout::setGeometry(rect);
    if (applying_) return;
    if (!animated_)
        applyGeometry();
    else if (changed || !debounce_->isActive())
        scheduleGeometry();
}
void WaterfallLayout::invalidate() {
    QLayout::invalidate();
    if (!applying_ && debounce_) scheduleGeometry();
}
void WaterfallLayout::scheduleGeometry() { debounce_->start(animated_ ? 80 : 0); }
void WaterfallLayout::setVerticalSpacing(int spacing) {
    spacing = std::max(0, spacing);
    if (verticalSpacing_ == spacing) return;
    verticalSpacing_ = spacing;
    invalidate();
}
int WaterfallLayout::verticalSpacing() const { return verticalSpacing_; }
void WaterfallLayout::setHorizontalSpacing(int spacing) {
    spacing = std::max(0, spacing);
    if (horizontalSpacing_ == spacing) return;
    horizontalSpacing_ = spacing;
    invalidate();
}
int WaterfallLayout::horizontalSpacing() const { return horizontalSpacing_; }
void WaterfallLayout::setColumnCount(int count) {
    if (count > 4096) throw std::invalid_argument("Too many waterfall columns");
    count = std::max(0, count);
    if (columns_ == count) return;
    columns_ = count;
    invalidate();
}
int WaterfallLayout::columnCount() const { return columns_; }
void WaterfallLayout::setColumnWidth(int width) {
    width = std::max(0, width);
    if (columnWidth_ == width) return;
    columnWidth_ = width;
    invalidate();
}
int WaterfallLayout::columnWidth() const { return columnWidth_; }
void WaterfallLayout::setAnimation(int duration, QEasingCurve::Type easing) {
    if (duration < 0 || duration > 60000) throw std::invalid_argument("Invalid animation duration");
    duration_ = duration;
    easing_ = easing;
    for (auto& entry : entries_)
        if (entry.animation) {
            entry.animation->setDuration(duration);
            entry.animation->setEasingCurve(easing);
        }
}
bool WaterfallLayout::isAnimationEnabled() const { return animated_; }
bool WaterfallLayout::isTight() const { return tight_; }
int WaterfallLayout::calculate(const QRect& rect, QVector<Placement>* placements) const {
    const auto margins = contentsMargins();
    const int width = boundedDimension(qint64(rect.width()) - margins.left() - margins.right());
    int columns = columns_;
    if (columnWidth_ > 0 || columns == 0) {
        int basis = columnWidth_ > 0 ? columnWidth_ : 150;
        if (columnWidth_ == 0)
            for (const auto& entry : entries_) {
                if (auto* widget = entry.item->widget()) {
                    if (tight_ && widget->isHidden()) continue;
                    basis = std::max(basis, widget->minimumWidth());
                    break;
                }
            }
        columns = std::clamp<int>(
            int((qint64(width) + horizontalSpacing_) / (qint64(basis) + horizontalSpacing_)), 1,
            4096);
    }
    columns = std::max(1, columns);
    const int itemWidth =
        columnWidth_ > 0
            ? columnWidth_
            : boundedDimension((qint64(width) - qint64(horizontalSpacing_) * (columns - 1)) /
                               columns);
    QVector<qint64> heights(columns, 0);
    QVector<bool> used(columns, false);
    const bool rtl = parentWidget() && parentWidget()->layoutDirection() == Qt::RightToLeft;
    for (int i = 0; i < count(); ++i) {
        auto* item = entries_[i].item;
        auto* widget = item->widget();
        if (tight_ && widget && widget->isHidden()) continue;
        const int column =
            int(std::min_element(heights.cbegin(), heights.cend()) - heights.cbegin());
        int height = item->sizeHint().height();
        if (widget) {
            if (widget->minimumHeight() == widget->maximumHeight())
                height = widget->minimumHeight();
            else {
                height = widget->hasHeightForWidth() ? widget->heightForWidth(itemWidth)
                                                     : widget->sizeHint().height();
                height = std::clamp(height, widget->minimumHeight(), widget->maximumHeight());
            }
        } else if (item->hasHeightForWidth())
            height = item->heightForWidth(itemWidth);
        height = std::max(0, height);
        const qint64 offset = qint64(column) * (qint64(itemWidth) + horizontalSpacing_);
        const qint64 x =
            rtl ? qint64(rect.x()) + rect.width() - margins.right() - itemWidth - offset
                : qint64(rect.x()) + margins.left() + offset;
        if (placements)
            placements->append(
                {i, QRect(boundedCoordinate(x),
                          boundedCoordinate(qint64(rect.y()) + margins.top() + heights[column]),
                          itemWidth, height)});
        heights[column] += qint64(height) + verticalSpacing_;
        used[column] = true;
    }
    qint64 tallest = 0;
    for (int i = 0; i < columns; ++i)
        if (used[i]) tallest = std::max(tallest, heights[i] - verticalSpacing_);
    return boundedDimension(tallest + margins.top() + margins.bottom());
}
void WaterfallLayout::applyGeometry() {
    if (applying_) return;
    applying_ = true;
    QVector<Placement> placements;
    calculate(geometry(), &placements);
    if (animated_) animations_->stop();
    for (auto& entry : entries_)
        if (entry.animation && entry.item->widget()) {
            const QRect stationary = entry.item->widget()->geometry();
            entry.animation->setStartValue(stationary);
            entry.animation->setEndValue(stationary);
        }
    bool moving = false;
    for (const auto& place : placements) {
        auto& entry = entries_[place.index];
        auto* widget = entry.item->widget();
        if (entry.animation && widget && widget->isVisible() && duration_ > 0 &&
            widget->geometry() != place.rect) {
            entry.animation->setStartValue(widget->geometry());
            entry.animation->setEndValue(place.rect);
            moving = true;
        } else {
            if (entry.animation) {
                entry.animation->setStartValue(place.rect);
                entry.animation->setEndValue(place.rect);
            }
            entry.item->setGeometry(place.rect);
        }
    }
    if (animated_ && moving) animations_->start();
    applying_ = false;
}
bool WaterfallLayout::eventFilter(QObject* object, QEvent* event) {
    if (event->type() == QEvent::Show || event->type() == QEvent::Hide ||
        event->type() == QEvent::LayoutDirectionChange || event->type() == QEvent::LayoutRequest ||
        event->type() == QEvent::FontChange) {
        if (!applying_) {
            if (event->type() != QEvent::LayoutRequest) QLayout::invalidate();
            scheduleGeometry();
        }
    }
    return QLayout::eventFilter(object, event);
}
}  // namespace qfw
