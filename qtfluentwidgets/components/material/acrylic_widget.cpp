#include "acrylic_widget.h"

#include <QPainter>
#include "common/config.h"
#include "common/style_sheet.h"

namespace qfw {

AcrylicWidget::AcrylicWidget(QWidget* parent) : QFrame(parent), acrylicBrush_(this, 30) {
    updateAcrylicColor();
    connect(&QConfig::instance(), &QConfig::themeChanged, this, [this] {
        updateAcrylicColor();
        update();
    });
}

QPainterPath AcrylicWidget::acrylicClipPath() const { return {}; }

void AcrylicWidget::updateAcrylicColor() {
    acrylicBrush_.setTintColor(isDarkTheme() ? QColor(32, 32, 32, 200)
                                           : QColor(255, 255, 255, 180));
    acrylicBrush_.setLuminosityColor(isDarkTheme() ? QColor(0, 0, 0, 0)
                                                 : QColor(255, 255, 255, 0));
}

void AcrylicWidget::drawAcrylic(QPainter* painter) {
    // Clear a previous clip when a derived widget switches back to an empty path.
    acrylicBrush_.setClipPath(acrylicClipPath());
    updateAcrylicColor();
    acrylicBrush_.paint(painter);
}

void AcrylicWidget::paintEvent(QPaintEvent* event) {
    {
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        drawAcrylic(&painter);
    }
    QFrame::paintEvent(event);
}

}  // namespace qfw
