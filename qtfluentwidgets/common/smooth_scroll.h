#pragma once

#include <QAbstractScrollArea>
#include <QApplication>
#include <QDateTime>
#include <QLocale>
#include <QPoint>
#include <QRect>
#include <QScreen>
#include <QScrollArea>
#include <QTimer>
#include <QWheelEvent>

namespace qfw {

enum class SmoothMode { NoSmooth = 0, Constant = 1, Linear = 2, Quadratic = 3, Cosine = 4 };

class SmoothScroll : public QObject {
    Q_OBJECT

public:
    explicit SmoothScroll(QAbstractScrollArea* widget, Qt::Orientation orient = Qt::Vertical,
                          QObject* parent = nullptr);

    void setSmoothMode(SmoothMode mode);
    void wheelEvent(QWheelEvent* e);

private slots:
    void smoothMove();

private:
    void handleWheelEvent(QWheelEvent* e);
    double subDelta(double delta, int stepsLeft, int stepsTotal) const;

    QAbstractScrollArea* widget_;
    Qt::Orientation orient_;
    int fps_ = 60;
    int duration_ = 400;
    int stepsTotal_ = 0;
    double stepRatio_ = 1.5;
    int acceleration_ = 1;

    QPoint lastWheelPos_;
    QPoint lastWheelGlobalPos_;
    QList<double> scrollStamps_;
    QList<QPair<double, int>> stepsLeftQueue_;

    SmoothMode smoothMode_ = SmoothMode::Linear;
    QTimer smoothMoveTimer_;
};

}  // namespace qfw
