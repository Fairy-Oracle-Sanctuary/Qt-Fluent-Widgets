#include "common/theme_listener.h"

#include <QApplication>
#include <QPalette>

namespace qfw {

SystemThemeListener::SystemThemeListener(QObject* parent)
    : QThread(parent), checkTimer_(new QTimer(this)) {
    // Connect timer to check system theme periodically
    connect(checkTimer_, &QTimer::timeout, this, &SystemThemeListener::onThemeChanged);

    // Check every 5 seconds (similar to darkdetect polling)
    checkTimer_->start(5000);
}

void SystemThemeListener::run() {
    // Start the timer when thread runs
    checkTimer_->start();
    exec();  // Run event loop
}

void SystemThemeListener::onThemeChanged() { checkSystemTheme(); }

void SystemThemeListener::checkSystemTheme() {
    // Simple system theme detection using Qt palette
    // This is a simplified version - Python version uses darkdetect library
    QPalette palette = QApplication::palette();
    QColor windowColor = palette.color(QPalette::Window);

    // Determine if system is in dark mode based on window color
    Theme newTheme = (windowColor.lightnessF() < 0.5) ? Theme::Dark : Theme::Light;

    // Only emit if theme actually changed and app is in AUTO mode
    if (newTheme != currentSystemTheme_) {
        currentSystemTheme_ = newTheme;

        // Only emit if app is in AUTO mode and theme differs from current
        if (QConfig::instance().theme() == Theme::Auto) {
            emit systemThemeChanged(newTheme);
        }
    }
}

}  // namespace qfw
