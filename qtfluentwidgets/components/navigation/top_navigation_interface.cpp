#include "components/navigation/top_navigation_interface.h"

#include <QEvent>
#include <QResizeEvent>

namespace qfw {

TopNavigationInterface::TopNavigationInterface(QWidget* parent, bool showReturnButton)
    : QWidget(parent),
      panel_(new TopNavigationPanel(this)),
      hBoxLayout_(new QHBoxLayout(this)) {
    panel_->setReturnButtonVisible(showReturnButton);
    panel_->installEventFilter(this);

    connect(panel_, &TopNavigationPanel::displayModeChanged,
            this, &TopNavigationInterface::displayModeChanged);

    hBoxLayout_->setContentsMargins(0, 0, 0, 0);
    hBoxLayout_->setSpacing(0);
    hBoxLayout_->addWidget(panel_);

    setAttribute(Qt::WA_TranslucentBackground);
}

TopNavigationPushButton* TopNavigationInterface::addItem(
    const QString& routeKey, const QVariant& icon, const QString& text,
    std::function<void()> onClick, bool selectable,
    TopNavigationItemPosition position, const QString& tooltip, bool expanded) {
    return insertItem(-1, routeKey, icon, text, onClick, selectable, position, tooltip, expanded);
}

TopNavigationPushButton* TopNavigationInterface::insertItem(
    int index, const QString& routeKey, const QVariant& icon, const QString& text,
    std::function<void()> onClick, bool selectable,
    TopNavigationItemPosition position, const QString& tooltip, bool expanded) {
    auto* w = panel_->insertItem(index, routeKey, icon, text, onClick, selectable, position, tooltip, expanded);
    setMinimumWidth(panel_->layoutMinWidth());
    return w;
}

void TopNavigationInterface::addWidget(const QString& routeKey, NavigationWidget* widget,
                                       std::function<void()> onClick,
                                       TopNavigationItemPosition position,
                                       const QString& tooltip) {
    insertWidget(-1, routeKey, widget, onClick, position, tooltip);
}

void TopNavigationInterface::insertWidget(int index, const QString& routeKey, NavigationWidget* widget,
                                          std::function<void()> onClick,
                                          TopNavigationItemPosition position,
                                          const QString& tooltip) {
    panel_->insertWidget(index, routeKey, widget, onClick, position, tooltip);
    setMinimumWidth(panel_->layoutMinWidth());
}

void TopNavigationInterface::removeWidget(const QString& routeKey) {
    panel_->removeWidget(routeKey);
}

void TopNavigationInterface::setCurrentItem(const QString& name) {
    panel_->setCurrentItem(name);
}

bool TopNavigationInterface::eventFilter(QObject* obj, QEvent* e) {
    if (obj != panel_ || e->type() != QEvent::Resize) {
        return QWidget::eventFilter(obj, e);
    }

    if (panel_->displayMode() != TopNavigationDisplayMode::Menu) {
        auto* event = static_cast<QResizeEvent*>(e);
        if (event->oldSize().height() != event->size().height()) {
            setFixedHeight(event->size().height());
        }
    }

    return QWidget::eventFilter(obj, e);
}

void TopNavigationInterface::resizeEvent(QResizeEvent* e) {
    if (e->oldSize().width() != width()) {
        panel_->setFixedWidth(width());
    }
}

}  // namespace qfw
