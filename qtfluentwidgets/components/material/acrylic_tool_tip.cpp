#include "acrylic_tool_tip.h"

namespace qfw {

AcrylicToolTipContainer::AcrylicToolTipContainer(QWidget* parent) : AcrylicWidget(parent) {
    setProperty("transparent", true);
}

QPainterPath AcrylicToolTipContainer::acrylicClipPath() const {
    QPainterPath path;
    path.addRoundedRect(QRectF(rect().adjusted(1, 1, -1, -1)), 3, 3);
    return path;
}

AcrylicToolTip::AcrylicToolTip(const QString& text, QWidget* parent)
    : ToolTip(text, parent, [](ToolTip* tip) { return new AcrylicToolTipContainer(tip); }) {}

AcrylicToolTipContainer* AcrylicToolTip::acrylicContainer() const {
    return static_cast<AcrylicToolTipContainer*>(container());
}

void AcrylicToolTip::captureBackground(const QRect& globalRect) {
    acrylicContainer()->acrylicBrush().grabImage(globalRect);
}

void AcrylicToolTip::showEvent(QShowEvent* event) {
    // mapToGlobal also handles the child-window positioning used on Wayland.
    captureBackground(QRect(acrylicContainer()->mapToGlobal(QPoint()), acrylicContainer()->size()));
    ToolTip::showEvent(event);
}

AcrylicToolTipFilter::AcrylicToolTipFilter(QWidget* parent, int showDelay, ToolTipPosition position)
    : ToolTipFilter(parent, showDelay, position) {}

AcrylicToolTipFilter::~AcrylicToolTipFilter() {
    delete ownedToolTip_.data();
    tooltip_ = nullptr;
}

ToolTip* AcrylicToolTipFilter::createToolTip() {
    auto* widget = qobject_cast<QWidget*>(parent());
    if (!widget) return nullptr;
    ownedToolTip_ = new AcrylicToolTip(widget->toolTip(), widget->window());
    connect(ownedToolTip_, &QObject::destroyed, this, [this] { tooltip_ = nullptr; });
    return ownedToolTip_;
}

}  // namespace qfw
