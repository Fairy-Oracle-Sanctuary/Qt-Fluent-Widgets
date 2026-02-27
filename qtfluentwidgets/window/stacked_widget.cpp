#include "window/stacked_widget.h"

#include <QAbstractScrollArea>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QStackedWidget>

#include "components/widgets/stacked_widget.h"

namespace qfw {

StackedWidget::StackedWidget(QWidget* parent) : QFrame(parent) {
    setProperty("qssClass", "StackedWidget");

    hBoxLayout_ = new QHBoxLayout(this);
    view_ = new PopUpAniStackedWidget(this);

    hBoxLayout_->setContentsMargins(0, 0, 0, 0);
    hBoxLayout_->addWidget(view_);

    connect(view_, &QStackedWidget::currentChanged, this, &StackedWidget::currentChanged);

    setAttribute(Qt::WA_StyledBackground);
}

bool StackedWidget::isAnimationEnabled() const {
    return view_ ? view_->isAnimationEnabled() : true;
}

void StackedWidget::setAnimationEnabled(bool enabled) {
    if (view_) {
        view_->setAnimationEnabled(enabled);
    }
}

void StackedWidget::addWidget(QWidget* widget) {
    if (view_) {
        view_->addWidget(widget);
    }
}

void StackedWidget::removeWidget(QWidget* widget) {
    if (view_) {
        view_->removeWidget(widget);
    }
}

QWidget* StackedWidget::widget(int index) const { return view_ ? view_->widget(index) : nullptr; }

void StackedWidget::setCurrentWidget(QWidget* widget, bool popOut) {
    if (!view_ || !widget) {
        return;
    }

    if (auto* area = qobject_cast<QAbstractScrollArea*>(widget)) {
        if (area->verticalScrollBar()) {
            area->verticalScrollBar()->setValue(0);
        }
    }

    if (!popOut) {
        view_->setCurrentWidget(widget, false, true, 300);
    } else {
        view_->setCurrentWidget(widget, true, false, 300, QEasingCurve(QEasingCurve::InQuad));
    }
}

void StackedWidget::setCurrentIndex(int index, bool popOut) {
    setCurrentWidget(widget(index), popOut);
}

int StackedWidget::currentIndex() const { return view_ ? view_->currentIndex() : -1; }

QWidget* StackedWidget::currentWidget() const { return view_ ? view_->currentWidget() : nullptr; }

int StackedWidget::indexOf(QWidget* widget) const { return view_ ? view_->indexOf(widget) : -1; }

int StackedWidget::count() const { return view_ ? view_->count() : 0; }

QStackedWidget* StackedWidget::view() const { return view_; }

}  // namespace qfw
