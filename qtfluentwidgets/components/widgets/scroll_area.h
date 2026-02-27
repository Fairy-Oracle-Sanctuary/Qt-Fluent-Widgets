#pragma once

#include <QEasingCurve>
#include <QScrollArea>

#include "common/smooth_scroll.h"

namespace qfw {

class SmoothScrollDelegate;
class SmoothScrollBar;

class ScrollArea : public QScrollArea {
    Q_OBJECT

public:
    explicit ScrollArea(QWidget* parent = nullptr);

    void setSmoothMode(SmoothMode mode, Qt::Orientation orientation);
    void enableTransparentBackground();
    void setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy policy);
    void setVerticalScrollBarPolicy(Qt::ScrollBarPolicy policy);

private:
    SmoothScrollDelegate* scrollDelegate_ = nullptr;
};

class SingleDirectionScrollArea : public QScrollArea {
    Q_OBJECT

public:
    explicit SingleDirectionScrollArea(QWidget* parent = nullptr,
                                       Qt::Orientation orient = Qt::Vertical);

    void setVerticalScrollBarPolicy(Qt::ScrollBarPolicy policy);
    void setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy policy);

    void setSmoothMode(SmoothMode mode);
    void enableTransparentBackground();

protected:
    void keyPressEvent(QKeyEvent* e) override;
    void wheelEvent(QWheelEvent* e) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    Qt::Orientation orient_;
    SmoothScroll* smoothScroll_ = nullptr;
    SmoothScrollBar* vScrollBar_ = nullptr;
    SmoothScrollBar* hScrollBar_ = nullptr;
};

class SmoothScrollArea : public QScrollArea {
    Q_OBJECT

public:
    explicit SmoothScrollArea(QWidget* parent = nullptr);

    void setScrollAnimation(Qt::Orientation orient, int duration,
                            QEasingCurve::Type easing = QEasingCurve::OutCubic);
    void enableTransparentBackground();

private:
    SmoothScrollDelegate* delegate_ = nullptr;
};

}  // namespace qfw
