#include "common/router.h"

#include <QWidget>

namespace qfw {

// ============================================================================
// RouteItem
// ============================================================================

RouteItem::RouteItem(QStackedWidget* stacked, const QString& routeKey)
    : stacked_(stacked), routeKey_(routeKey) {}

bool RouteItem::operator==(const RouteItem& other) const {
    return other.stacked_ == stacked_ && other.routeKey_ == routeKey_;
}

// ============================================================================
// StackedHistory
// ============================================================================

StackedHistory::StackedHistory(QStackedWidget* stacked)
    : stacked_(stacked), defaultRouteKey_(QString()), history_({defaultRouteKey_}) {}

bool StackedHistory::push(const QString& routeKey) {
    if (history_.last() == routeKey) {
        return false;
    }

    history_.append(routeKey);
    return true;
}

void StackedHistory::pop() {
    if (isEmpty()) {
        return;
    }

    history_.removeLast();
    goToTop();
}

void StackedHistory::remove(const QString& routeKey) {
    if (!history_.contains(routeKey)) {
        return;
    }

    history_.removeAll(routeKey);
    goToTop();
}

void StackedHistory::setDefaultRouteKey(const QString& routeKey) {
    defaultRouteKey_ = routeKey;
    if (!history_.isEmpty()) {
        history_[0] = routeKey;
    }
}

void StackedHistory::goToTop() {
    QWidget* w = stacked_->findChild<QWidget*>(history_.last());
    if (w) {
        stacked_->setCurrentWidget(w);
    }
}

// ============================================================================
// Router
// ============================================================================

Router* Router::instance() {
    static Router* router = new Router();
    return router;
}

Router::Router(QObject* parent) : QObject(parent) {}

void Router::setDefaultRouteKey(QStackedWidget* stacked, const QString& routeKey) {
    if (!stackHistories_.contains(stacked)) {
        stackHistories_[stacked] = new StackedHistory(stacked);
    }
    stackHistories_[stacked]->setDefaultRouteKey(routeKey);
}

bool Router::push(QStackedWidget* stacked, const QString& routeKey) {
    RouteItem item(stacked, routeKey);

    if (!stackHistories_.contains(stacked)) {
        stackHistories_[stacked] = new StackedHistory(stacked);
    }

    bool success = stackHistories_[stacked]->push(routeKey);
    if (success) {
        history_.append(item);
    }

    emit emptyChanged(isEmpty());
    return success;
}

void Router::pop() {
    if (history_.isEmpty()) {
        return;
    }

    RouteItem item = history_.takeLast();
    stackHistories_[item.stackedWidget()]->pop();
    emit emptyChanged(isEmpty());
}

void Router::remove(const QString& routeKey) {
    // Remove items with matching routeKey
    history_.erase(
        std::remove_if(history_.begin(), history_.end(),
                       [&routeKey](const RouteItem& item) { return item.routeKey() == routeKey; }),
        history_.end());

    // Group by stacked widget and remove from each history
    QMap<QStackedWidget*, QStringList> grouped;
    for (const auto& item : history_) {
        grouped[item.stackedWidget()].append(item.routeKey());
    }

    for (auto it = grouped.begin(); it != grouped.end(); ++it) {
        if (stackHistories_.contains(it.key())) {
            stackHistories_[it.key()]->remove(routeKey);
        }
    }

    emit emptyChanged(isEmpty());
}

}  // namespace qfw
