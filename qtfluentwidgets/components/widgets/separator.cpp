#include "separator.h"

#include <QPainter>

#include "common/config.h"

namespace qfw {

// ============================================================================
// HorizontalSeparator
// ============================================================================

HorizontalSeparator::HorizontalSeparator(QWidget* parent) : QWidget(parent) {
    setFixedHeight(3);
}

void HorizontalSeparator::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QColor color = isDarkTheme() ? QColor(255, 255, 255, 51) : QColor(0, 0, 0, 22);
    painter.setPen(color);
    painter.drawLine(0, 1, width(), 1);
}

// ============================================================================
// VerticalSeparator
// ============================================================================

VerticalSeparator::VerticalSeparator(QWidget* parent) : QWidget(parent) {
    setFixedWidth(3);
}

void VerticalSeparator::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QColor color = isDarkTheme() ? QColor(255, 255, 255, 51) : QColor(0, 0, 0, 22);
    painter.setPen(color);
    painter.drawLine(1, 0, 1, height());
}

}  // namespace qfw
