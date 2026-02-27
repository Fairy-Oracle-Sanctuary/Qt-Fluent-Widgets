#include "common/font.h"

#include <QWidget>

namespace qfw {

QFont getFont(int fontSize, QFont::Weight weight) {
    QFont font;
    font.setFamilies({QStringLiteral("Segoe UI"), QStringLiteral("Microsoft YaHei"),
                      QStringLiteral("PingFang SC")});
    font.setPixelSize(fontSize);
    font.setWeight(weight);
    return font;
}

void setFont(QWidget* widget, int fontSize, QFont::Weight weight) {
    if (!widget) {
        return;
    }

    widget->setFont(getFont(fontSize, weight));
}

}  // namespace qfw
