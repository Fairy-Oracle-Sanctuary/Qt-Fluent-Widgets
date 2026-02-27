#pragma once

#include <QList>
#include <QMap>
#include <QObject>
#include <QStackedWidget>
#include <QString>

namespace qfw {

class RouteItem {
public:
    RouteItem(QStackedWidget* stacked, const QString& routeKey);

    QStackedWidget* stackedWidget() const { return stacked_; }
    const QString& routeKey() const { return routeKey_; }

    bool operator==(const RouteItem& other) const;

private:
    QStackedWidget* stacked_;
    QString routeKey_;
};

class StackedHistory {
public:
    explicit StackedHistory(QStackedWidget* stacked);

    int length() const { return history_.length(); }
    bool isEmpty() const { return history_.length() <= 1; }

    bool push(const QString& routeKey);
    void pop();
    void remove(const QString& routeKey);
    const QString& top() const { return history_.last(); }
    void setDefaultRouteKey(const QString& routeKey);
    void goToTop();

private:
    QStackedWidget* stacked_;
    QString defaultRouteKey_;
    QStringList history_;
};

class Router : public QObject {
    Q_OBJECT

public:
    explicit Router(QObject* parent = nullptr);

    static Router* instance();

    void setDefaultRouteKey(QStackedWidget* stacked, const QString& routeKey);
    bool push(QStackedWidget* stacked, const QString& routeKey);
    void pop();
    void remove(const QString& routeKey);

    bool isEmpty() const { return history_.isEmpty(); }

signals:
    void emptyChanged(bool empty);

private:
    QList<RouteItem> history_;
    QMap<QStackedWidget*, StackedHistory*> stackHistories_;
};

}  // namespace qfw
