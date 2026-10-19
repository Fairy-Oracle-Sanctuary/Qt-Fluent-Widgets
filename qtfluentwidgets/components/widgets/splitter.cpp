#include "components/widgets/splitter.h"

#include <QPainter>
#include <algorithm>

#include "common/config.h"
#include "common/style_sheet.h"

namespace qfw {
SplitterHandle::SplitterHandle(Qt::Orientation orientation, QSplitter* parent)
    : QSplitterHandle(orientation, parent) {
    setAttribute(Qt::WA_Hover);
    connect(&QConfig::instance(), &QConfig::themeChanged, this, [this] { update(); });
}
QSize SplitterHandle::sizeHint() const {
    return orientation() == Qt::Horizontal ? QSize(14, 0) : QSize(0, 14);
}
void SplitterHandle::enterEvent(enterEvent_QEnterEvent* event) {
    hovered_ = true;
    update();
    QSplitterHandle::enterEvent(event);
}
void SplitterHandle::leaveEvent(QEvent* event) {
    hovered_ = false;
    update();
    QSplitterHandle::leaveEvent(event);
}
void SplitterHandle::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    if (!isEnabled()) painter.setOpacity(0.36);
    const QRectF bounds(rect());
    if (hovered_ && isEnabled()) {
        const int fill = isDarkTheme() ? 42 : 229;
        painter.setBrush(QColor(fill, fill, fill));
        painter.drawRoundedRect(bounds, 7, 7);
    }
    const int shade = isDarkTheme() ? 161 : 129;
    painter.setBrush(QColor(shade, shade, shade));
    // Match Python's 32px grip, but keep it inside very short/narrow handles.
    const auto center = bounds.center();
    QRectF grip;
    if (orientation() == Qt::Horizontal) {
        const qreal height = std::min<qreal>(32, bounds.height());
        const qreal width = std::max<qreal>(0, bounds.width() - 8.4);
        grip = QRectF(center.x() - width / 2, center.y() - height / 2, width, height);
    } else {
        const qreal width = std::min<qreal>(32, bounds.width());
        const qreal height = std::max<qreal>(0, bounds.height() - 8.8);
        grip = QRectF(center.x() - width / 2, center.y() - height / 2, width, height);
    }
    painter.drawRoundedRect(grip, 4, 4);
}
Splitter::Splitter(Qt::Orientation orientation, QWidget* parent) : QSplitter(orientation, parent) {
    setHandleWidth(14);
}
Splitter::Splitter(QWidget* parent) : Splitter(Qt::Horizontal, parent) {}
QSplitterHandle* Splitter::createHandle() { return new SplitterHandle(orientation(), this); }
}  // namespace qfw
