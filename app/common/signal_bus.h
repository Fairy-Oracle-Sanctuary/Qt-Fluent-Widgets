#pragma once

#include <QObject>
#include <QString>

namespace qfw {

// Signal bus for gallery app - centralized signal handling
class SignalBus : public QObject {
    Q_OBJECT

public:
    static SignalBus& instance();

signals:
    void switchToSampleCard(const QString& routeKey, int index);
    void micaEnableChanged(bool enabled);
    void supportSignal();

private:
    explicit SignalBus(QObject* parent = nullptr);
};

// Global accessor
SignalBus& signalBus();

}  // namespace qfw
