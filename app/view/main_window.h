#pragma once

#include <QApplication>
#include <QScreen>

#include "window/fluent_window.h"

namespace qfw {

// MainWindow - Main application window (minimal version)
class MainWindow : public FluentWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    void initWindow();
    void initNavigation();
    void connectSignalToSlot();

private slots:
    void onSwitchToSample(const QString& routeKey, int index);
    void onMicaEnableChanged(bool enabled);
};

}  // namespace qfw
