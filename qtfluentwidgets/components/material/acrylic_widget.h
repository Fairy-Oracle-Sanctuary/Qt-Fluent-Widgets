#pragma once

#include <QFrame>
#include "components/widgets/acrylic_label.h"

namespace qfw {

// C++ counterpart of the Python material mixin. Derived frames can customize
// the clip path and paint their content after drawAcrylic().
class AcrylicWidget : public QFrame {
    Q_OBJECT
public:
    explicit AcrylicWidget(QWidget* parent = nullptr);
    AcrylicBrush& acrylicBrush() { return acrylicBrush_; }
    virtual QPainterPath acrylicClipPath() const;

protected:
    void updateAcrylicColor();
    void drawAcrylic(QPainter* painter);
    void paintEvent(QPaintEvent* event) override;

private:
    AcrylicBrush acrylicBrush_;
};

}  // namespace qfw
