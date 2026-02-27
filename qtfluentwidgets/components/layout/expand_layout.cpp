#include "expand_layout.h"

#include <QResizeEvent>
#include <QWidget>

namespace qfw {

ExpandLayout::ExpandLayout(QWidget* parent) : QLayout(parent) {}

ExpandLayout::~ExpandLayout() {
    while (auto* item = takeAt(0)) {
        delete item;
    }
}

void ExpandLayout::addWidget(QWidget* widget) {
    if (!widget || widgets_.contains(widget)) {
        return;
    }

    widgets_.append(widget);
    widget->installEventFilter(this);
}

void ExpandLayout::addItem(QLayoutItem* item) {
    items_.append(item);
}

int ExpandLayout::count() const {
    return items_.count();
}

QLayoutItem* ExpandLayout::itemAt(int index) const {
    if (index >= 0 && index < items_.count()) {
        return items_[index];
    }
    return nullptr;
}

QLayoutItem* ExpandLayout::takeAt(int index) {
    if (index >= 0 && index < items_.count()) {
        widgets_.removeAt(index);
        return items_.takeAt(index);
    }
    return nullptr;
}

Qt::Orientations ExpandLayout::expandingDirections() const {
    return Qt::Vertical;
}

bool ExpandLayout::hasHeightForWidth() const {
    return true;
}

int ExpandLayout::heightForWidth(int width) const {
    return doLayout(QRect(0, 0, width, 0), false);
}

void ExpandLayout::setGeometry(const QRect& rect) {
    QLayout::setGeometry(rect);
    doLayout(rect, true);
}

QSize ExpandLayout::sizeHint() const {
    return minimumSize();
}

QSize ExpandLayout::minimumSize() const {
    QSize size;

    for (QWidget* w : widgets_) {
        size = size.expandedTo(w->minimumSize());
    }

    const auto m = contentsMargins();
    size += QSize(m.left() + m.right(), m.top() + m.bottom());

    return size;
}

int ExpandLayout::doLayout(const QRect& rect, bool move) const {
    const auto m = contentsMargins();
    int x = rect.x() + m.left();
    int y = rect.y() + m.top();
    int width = rect.width() - m.left() - m.right();

    for (int i = 0; i < widgets_.count(); ++i) {
        QWidget* w = widgets_[i];
        if (w->isHidden()) {
            continue;
        }

        if (i > 0) {
            y += spacing();
        }

        if (move) {
            w->setGeometry(QRect(QPoint(x, y), QSize(width, w->height())));
        }

        y += w->height();
    }

    return y - rect.y();
}

bool ExpandLayout::eventFilter(QObject* obj, QEvent* e) {
    if (QWidget* widget = qobject_cast<QWidget*>(obj)) {
        if (widgets_.contains(widget) && e->type() == QEvent::Resize) {
            auto* re = static_cast<QResizeEvent*>(e);
            QSize delta = re->size() - re->oldSize();
            if (delta.height() != 0 && delta.width() == 0) {
                if (QWidget* parent = parentWidget()) {
                    parent->resize(parent->width(), parent->height() + delta.height());
                }
            }
        }
    }

    return QLayout::eventFilter(obj, e);
}

} // namespace qfw
