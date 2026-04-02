#pragma once

#include <QApplication>
#include <QScreen>

#include "window/fluent_window.h"

namespace qfw {

// TopMainWindow - Main application window with top navigation bar
class TopMainWindow : public TopFluentWindow {
    Q_OBJECT

public:
    explicit TopMainWindow(QWidget* parent = nullptr);

private:
    void initWindow();
    void initNavigation();
    void connectSignalToSlot();

private slots:
    void onSwitchToSample(const QString& routeKey, int index);
    void onMicaEnableChanged(bool enabled);
};

}  // namespace qfw
