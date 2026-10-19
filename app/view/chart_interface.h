#pragma once
#include "gallery_interface.h"
#include <QTimer>

namespace qfw {
class ChartWidget;
class PushButton;
class ChartRealtimeDemo : public QWidget {
    Q_OBJECT
public:
    explicit ChartRealtimeDemo(QWidget* parent = nullptr);
protected:
    void hideEvent(QHideEvent* event) override;
private:
    void updateOption();
    void stop();
    ChartWidget* chart_;
    PushButton* start_;
    PushButton* stop_;
    QTimer timer_;
    QList<int> values_;
    bool animation_ = true;
};
class ChartInterface : public GalleryInterface {
    Q_OBJECT
public:
    explicit ChartInterface(QWidget* parent = nullptr);
};
}
