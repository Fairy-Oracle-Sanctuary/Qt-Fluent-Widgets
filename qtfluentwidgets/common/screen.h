#pragma once

#include <QApplication>
#include <QCursor>
#include <QPoint>
#include <QRect>
#include <QScreen>

namespace qfw {

// Screen utilities
QScreen* getCurrentScreen();
QRect getCurrentScreenGeometry(bool available = true);

}  // namespace qfw
