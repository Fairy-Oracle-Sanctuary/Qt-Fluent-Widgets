#pragma once

#include <QFont>

class QWidget;

namespace qfw {

QFont getFont(int fontSize = 14, QFont::Weight weight = QFont::Normal);
void setFont(QWidget* widget, int fontSize = 14, QFont::Weight weight = QFont::Normal);

}  // namespace qfw
