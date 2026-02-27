#include "flow_layout.h"

#include <QEvent>
#include <QWidget>
#include <QWidgetItem>

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

void FlowLayout::addItem(QLayoutItem* item) { items_.append(item); }

void FlowLayout::insertItem(int index, QLayoutItem* item) { items_.insert(index, item); }

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
            w->parentWidget()->installEventFilter(this);
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
        anis_.insert(index, ani);
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
                ani->deleteLater();
            }
        }
        items_.removeAt(index);
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
    QLayout::setGeometry(rect);

    if (needAni_) {
        debounceTimer_->start(80);
    } else {
        doLayout(rect, true);
    }
}

void FlowLayout::onDebounceTimeout() { doLayout(geometry(), true); }

QSize FlowLayout::sizeHint() const { return minimumSize(); }

QSize FlowLayout::minimumSize() const {
    QSize size;

    for (auto* item : items_) {
        size = size.expandedTo(item->minimumSize());
    }

    const auto m = contentsMargins();
    size += QSize(m.left() + m.right(), m.top() + m.bottom());

    return size;
}

void FlowLayout::setVerticalSpacing(int spacing) { verticalSpacing_ = spacing; }

int FlowLayout::verticalSpacing() const { return verticalSpacing_; }

void FlowLayout::setHorizontalSpacing(int spacing) { horizontalSpacing_ = spacing; }

int FlowLayout::horizontalSpacing() const { return horizontalSpacing_; }

bool FlowLayout::eventFilter(QObject* obj, QEvent* event) {
    for (auto* item : items_) {
        if (item->widget() == obj && event->type() == QEvent::ParentChange) {
            wParent_ = obj->parent() ? qobject_cast<QWidget*>(obj->parent()) : nullptr;
            if (wParent_) {
                wParent_->installEventFilter(this);
            }
            isInstalledEventFilter_ = true;
            break;
        }
    }

    if (obj == wParent_ && event->type() == QEvent::Show) {
        doLayout(geometry(), true);
        isInstalledEventFilter_ = true;
    }

    return QLayout::eventFilter(obj, event);
}

int FlowLayout::doLayout(const QRect& rect, bool move) const {
    bool aniRestart = false;
    const auto m = contentsMargins();
    int x = rect.x() + m.left();
    int y = rect.y() + m.top();
    int rowHeight = 0;
    int spaceX = horizontalSpacing();
    int spaceY = verticalSpacing();

    for (int i = 0; i < items_.count(); ++i) {
        auto* item = items_[i];
        if (item->widget() && !item->widget()->isVisible() && isTight_) {
            continue;
        }

        int nextX = x + item->sizeHint().width() + spaceX;

        if (nextX - spaceX > rect.right() - m.right() && rowHeight > 0) {
            x = rect.x() + m.left();
            y = y + rowHeight + spaceY;
            nextX = x + item->sizeHint().width() + spaceX;
            rowHeight = 0;
        }

        if (move) {
            QRect target(QPoint(x, y), item->sizeHint());
            if (!needAni_) {
                item->setGeometry(target);
            } else if (i < anis_.count()) {
                auto* ani = anis_[i];
                if (target != ani->endValue().toRect()) {
                    ani->stop();
                    ani->setEndValue(target);
                    aniRestart = true;
                }
            }
        }

        x = nextX;
        rowHeight = qMax(rowHeight, item->sizeHint().height());
    }

    if (needAni_ && aniRestart) {
        aniGroup_->stop();
        aniGroup_->start();
    }

    return y + rowHeight + m.bottom() - rect.y();
}

}  // namespace qfw
