#pragma once
#include <QColor>
#include <QImage>
#include <QString>

namespace qfw {
// Offline MathText layout. Invalid/unsupported input throws std::invalid_argument;
// ChatMarkdownBlock catches it and retains the user's original formula.
QImage renderFormula(const QString& expression, const QColor& color=QColor("#202020"), int pointSize=12);
}
