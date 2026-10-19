#include "flow_layout.h"

#include <QEvent>
#include <QVector>
#include <QWidget>
#include <QWidgetItem>
#include <QStyle>
#include <stdexcept>

namespace qfw {

FlowLayout::FlowLayout(QWidget* parent, bool needAni, bool isTight)
    : QLayout(parent),
      needAni_(needAni),
      isTight_(isTight),
      aniGroup_(new QParallelAnimationGroup(this)),
      debounceTimer_(new QTimer(this)) {
    debounceTimer_->setSingleShot(true);
    connect(debounceTimer_, &QTimer::timeout, this, &FlowLayout::onDebounceTimeout);
}

FlowLayout::~FlowLayout() {
    while (auto* item = takeAt(0)) {
        delete item;
    }
    for (auto* ani : anis_) {
        delete ani;
    }
}

void FlowLayout::addItem(QLayoutItem* item) {
    items_.append(item);
    invalidate();
}

void FlowLayout::insertItem(int index, QLayoutItem* item) {
    items_.insert(index, item);
    invalidate();
}

void FlowLayout::addWidget(QWidget* w) {
    QLayout::addWidget(w);
    onWidgetAdded(w);
    if (!needAni_) {
        w->show();
        if (auto* p = parentWidget()) {
            p->adjustSize();
            p->updateGeometry();
        }
        invalidate();
        activate();
        doLayout(geometry(), true);
    }
}

void FlowLayout::insertWidget(int index, QWidget* w) {
    insertItem(index, new QWidgetItem(w));
    addChildWidget(w);
    onWidgetAdded(w, index);
}

void FlowLayout::onWidgetAdded(QWidget* w, int index) {
    if (!isInstalledEventFilter_) {
        if (w->parentWidget()) {
            wParent_ = w->parentWidget();
            wParent_->installEventFilter(this);
            isInstalledEventFilter_ = true;
        } else {
            w->installEventFilter(this);
        }
    }

    if (!needAni_) {
        return;
    }

    auto* ani = new QPropertyAnimation(w, "geometry", this);
    ani->setEndValue(QRect(QPoint(0, 0), w->size()));
    ani->setDuration(duration_);
    ani->setEasingCurve(ease_);
    w->setProperty("flowAni", QVariant::fromValue(ani));
    aniGroup_->addAnimation(ani);

    if (index == -1) {
        anis_.append(ani);
    } else {
        anis_.insert(qMin(index, anis_.size()), ani);
    }
}

void FlowLayout::setAnimation(int duration, QEasingCurve::Type ease) {
    if (!needAni_) {
        return;
    }

    duration_ = duration;
    ease_ = ease;

    for (auto* ani : anis_) {
        ani->setDuration(duration);
        ani->setEasingCurve(ease);
    }
}

int FlowLayout::count() const { return items_.count(); }

QLayoutItem* FlowLayout::itemAt(int index) const {
    if (index >= 0 && index < items_.count()) {
        return items_[index];
    }
    return nullptr;
}

QLayoutItem* FlowLayout::takeAt(int index) {
    if (index >= 0 && index < items_.count()) {
        auto* item = items_[index];
        if (auto* w = item->widget()) {
            auto* ani = w->property("flowAni").value<QPropertyAnimation*>();
            if (ani) {
                anis_.removeAll(ani);
                aniGroup_->removeAnimation(ani);
                ani->stop();
                w->setProperty("flowAni", QVariant());
                ani->deleteLater();
            }
        }
        items_.removeAt(index);
        invalidate();
        return item;
    }
    return nullptr;
}

void FlowLayout::removeWidget(QWidget* widget) {
    for (int i = 0; i < items_.count(); ++i) {
        if (items_[i]->widget() == widget) {
            auto* item = takeAt(i);
            if (item) {
                if (auto* w = item->widget()) {
                    w->hide();
                }
                delete item;
            }
            invalidate();
            activate();
            return;
        }
    }
}

void FlowLayout::removeAllWidgets() {
    while (!items_.isEmpty()) {
        auto* item = takeAt(0);
        if (item) {
            if (auto* w = item->widget()) {
                w->hide();
            }
            delete item;
        }
    }
    invalidate();
    activate();
}

void FlowLayout::takeAllWidgets() {
    while (!items_.isEmpty()) {
        auto* item = takeAt(0);
        if (item && item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    invalidate();
    activate();
}

Qt::Orientations FlowLayout::expandingDirections() const { return Qt::Orientations(); }

bool FlowLayout::hasHeightForWidth() const { return true; }

int FlowLayout::heightForWidth(int width) const { return doLayout(QRect(0, 0, width, 0), false); }

void FlowLayout::setGeometry(const QRect& rect) {
    const bool geometryChanged = (rect != geometry());
    QLayout::setGeometry(rect);

    if (needAni_) {
        if (!geometryChanged && debounceTimer_->isActive()) {
            return;
        }
        debounceTimer_->start(80);
    } else {
        doLayout(rect, true);
    }
}

void FlowLayout::onDebounceTimeout() { doLayout(geometry(), true); }

QSize FlowLayout::sizeHint() const { return minimumSize(); }

QSize FlowLayout::minimumSize() const {
    QSize size(0, 0);

    for (auto* item : items_) {
        size = size.expandedTo(item->minimumSize());
    }

    const auto m = contentsMargins();
    size += QSize(m.left() + m.right(), m.top() + m.bottom());

    return size;
}

void FlowLayout::setVerticalSpacing(int spacing) {
    if (verticalSpacing_ == spacing) return;
    verticalSpacing_ = spacing;
    invalidate();
}

int FlowLayout::verticalSpacing() const { return verticalSpacing_; }

void FlowLayout::setHorizontalSpacing(int spacing) {
    if (horizontalSpacing_ == spacing) return;
    horizontalSpacing_ = spacing;
    invalidate();
}

int FlowLayout::horizontalSpacing() const { return horizontalSpacing_; }

bool FlowLayout::eventFilter(QObject* obj, QEvent* event) {
    if (event->type() == QEvent::ParentChange) {
        for (auto* item : items_) {
            if (item->widget() != obj) {
                continue;
            }

            wParent_ = obj->parent() ? qobject_cast<QWidget*>(obj->parent()) : nullptr;
            if (wParent_) {
                obj->removeEventFilter(this);
                wParent_->installEventFilter(this);
                isInstalledEventFilter_ = true;
            }
            break;
        }
    }

    if (obj == wParent_ && event->type() == QEvent::Show) {
        doLayout(geometry(), true);
        isInstalledEventFilter_ = true;
    }
    if (obj == wParent_ && event->type() == QEvent::Hide) {
        debounceTimer_->stop();
        aniGroup_->stop();
    }

    return QLayout::eventFilter(obj, event);
}

int FlowLayout::doLayout(const QRect& rect, bool move) const {
    struct AnimationTarget {
        QPropertyAnimation* animation;
        QLayoutItem* item;
        QRect geometry;
    };

    bool aniRestart = false;
    QVector<AnimationTarget> animationTargets;
    if (move && needAni_) {
        animationTargets.reserve(qMin(items_.count(), anis_.count()));
    }

    QVector<QPair<int, QRect>> placements;
    const int height = calculate(rect, move ? &placements : nullptr);
    for (const auto& placement : placements) {
        auto* item = items_.at(placement.first);
        const QRect target = placement.second;
        auto* widget = item->widget();
        auto* ani = widget ? widget->property("flowAni").value<QPropertyAnimation*>() : nullptr;
        if (needAni_ && ani && aniGroup_->indexOfAnimation(ani) >= 0) {
            animationTargets.append({ani, item, target});
            if (target != ani->endValue().toRect() ||
                (ani->state() != QAbstractAnimation::Running && item->geometry() != target))
                aniRestart = true;
        } else if (item->geometry() != target) {
            item->setGeometry(target);
        }
    }
    if (needAni_ && aniRestart) {
        aniGroup_->stop();
        for (const AnimationTarget& target : animationTargets) {
            target.animation->setStartValue(target.item->geometry());
            target.animation->setEndValue(target.geometry);
        }
        aniGroup_->start();
    }
    return height;
}

int FlowLayout::calculate(const QRect& rect, QVector<QPair<int, QRect>>* placements) const {
    const auto m = contentsMargins();
    int x = rect.x() + m.left();
    int y = rect.y() + m.top();
    int rowHeight = 0;
    int spaceX = horizontalSpacing();
    int spaceY = verticalSpacing();

    for (int i = 0; i < items_.count(); ++i) {
        auto* item = items_[i];
        QWidget* widget = item->widget();
        if (widget && !widget->isVisible() && isTight_) {
            continue;
        }

        const QSize itemSize = item->sizeHint();
        int nextX = x + itemSize.width() + spaceX;

        if (nextX - spaceX > rect.right() - m.right() && rowHeight > 0) {
            x = rect.x() + m.left();
            y = y + rowHeight + spaceY;
            nextX = x + itemSize.width() + spaceX;
            rowHeight = 0;
        }

        if (placements) placements->append(qMakePair(i, QRect(QPoint(x, y), itemSize)));

        x = nextX;
        rowHeight = qMax(rowHeight, itemSize.height());
    }

    return y + rowHeight + m.bottom() - rect.y();
}

AdaptiveFlowLayout::AdaptiveFlowLayout(QWidget* parent, bool needAni, bool isTight)
    : FlowLayout(parent, needAni, isTight) {}
void AdaptiveFlowLayout::setWidgetMinimumWidth(int width) {
    if (width <= 0) throw std::invalid_argument("Minimum widget width must be positive");
    if (minimumWidth_ == width) return;
    minimumWidth_ = width;
    invalidate();
}
void AdaptiveFlowLayout::setWidgetMaximumWidth(int width) {
    if (width != -1 && width <= 0) throw std::invalid_argument("Maximum widget width must be positive or -1");
    if (maximumWidth_ == width) return;
    maximumWidth_ = width;
    invalidate();
}
int AdaptiveFlowLayout::calculate(const QRect& rect, QVector<QPair<int, QRect>>* placements) const {
    const auto m = contentsMargins();
    const int available = qMax(0, rect.width() - m.left() - m.right());
    const int sx = horizontalSpacing(), sy = verticalSpacing();
    const qint64 divisor = qint64(minimumWidth_) + sx;
    const int columns = divisor > 0 ? int(qMax<qint64>(1, (qint64(available) + sx) / divisor)) : 1;
    int width = columns > 1 ? int((qint64(available) - qint64(columns - 1) * sx) / columns) : available;
    if (maximumWidth_ != -1) width = qMin(width, maximumWidth_);
    width = qMax(0, width);
    int x = rect.x() + m.left(), y = rect.y() + m.top();
    int column = 0, rowHeight = 0;
    const QRect content(rect.x() + m.left(), rect.y() + m.top(), available,
                        qMax(0, rect.height() - m.top() - m.bottom()));
    for (int i = 0; i < count(); ++i) {
        auto* item = itemAt(i);
        auto* widget = item->widget();
        if (widget && !widget->isVisible() && isTight()) continue;
        if (column >= columns) {
            x = rect.x() + m.left();
            y += rowHeight + sy;
            rowHeight = 0;
            column = 0;
        }
        const int height = item->sizeHint().height();
        const QRect target(x, y, width, height);
        if (placements) {
            const auto direction = parentWidget() ? parentWidget()->layoutDirection() : Qt::LeftToRight;
            placements->append(qMakePair(i, QStyle::visualRect(direction, content, target)));
        }
        x += width + sx;
        rowHeight = qMax(rowHeight, height);
        ++column;
    }
    return y + rowHeight + m.bottom() - rect.y();
}

}  // namespace qfw
