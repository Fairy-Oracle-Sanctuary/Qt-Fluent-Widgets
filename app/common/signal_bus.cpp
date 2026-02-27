#include "signal_bus.h"

namespace qfw {

SignalBus::SignalBus(QObject* parent) : QObject(parent) {}

SignalBus& SignalBus::instance() {
    static SignalBus instance;
    return instance;
}

SignalBus& signalBus() {
    return SignalBus::instance();
}

}  // namespace qfw
