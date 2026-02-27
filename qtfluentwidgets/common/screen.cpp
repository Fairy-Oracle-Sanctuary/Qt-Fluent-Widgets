#include "common/screen.h"

#include <QApplication>
#include <QCursor>
#include <QScreen>

namespace qfw {

QScreen* getCurrentScreen() {
    QPoint cursorPos = QCursor::pos();
    
    const auto screens = QApplication::screens();
    for (QScreen* screen : screens) {
        if (screen->geometry().contains(cursorPos)) {
            return screen;
        }
    }
    
    return nullptr;
}

QRect getCurrentScreenGeometry(bool available) {
    QScreen* screen = getCurrentScreen();
    
    // This should not happen
    if (!screen) {
        return QRect(0, 0, 1920, 1080);
    }
    
    return available ? screen->availableGeometry() : screen->geometry();
}

}  // namespace qfw
