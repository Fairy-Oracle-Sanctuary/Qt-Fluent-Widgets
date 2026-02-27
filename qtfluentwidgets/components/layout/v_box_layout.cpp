#include "v_box_layout.h"

namespace qfw {

VBoxLayout::VBoxLayout(QWidget* parent) : QVBoxLayout(parent) {}

void VBoxLayout::addWidgets(const QList<QWidget*>& widgets, int stretch,
                            Qt::Alignment alignment) {
    for (QWidget* widget : widgets) {
        addWidget(widget, stretch, alignment);
    }
}

void VBoxLayout::addWidget(QWidget* widget, int stretch, Qt::Alignment alignment) {
    QVBoxLayout::addWidget(widget, stretch, alignment);
    widgets_.append(widget);
    widget->show();
}

void VBoxLayout::removeWidget(QWidget* widget) {
    QVBoxLayout::removeWidget(widget);
    widgets_.removeAll(widget);
}

void VBoxLayout::deleteWidget(QWidget* widget) {
    removeWidget(widget);
    widget->hide();
    widget->deleteLater();
}

void VBoxLayout::removeAllWidgets() {
    for (QWidget* widget : widgets_) {
        QVBoxLayout::removeWidget(widget);
    }
    widgets_.clear();
}

} // namespace qfw
