#pragma once

#include "components/widgets/progress_bar.h"

namespace qfw {

class ProgressRing : public ProgressBar {
    Q_OBJECT
    Q_PROPERTY(int strokeWidth READ strokeWidth WRITE setStrokeWidth)

public:
    explicit ProgressRing(QWidget* parent = nullptr, bool useAni = true);

    int strokeWidth() const;
    void setStrokeWidth(int w);

protected:
    void paintEvent(QPaintEvent* e) override;
    void drawText(QPainter* painter, const QString& text);

private:
    int _strokeWidth = 6;
};

class IndeterminateProgressRing : public QProgressBar {
    Q_OBJECT
    Q_PROPERTY(int startAngle READ startAngle WRITE setStartAngle)
    Q_PROPERTY(int spanAngle READ spanAngle WRITE setSpanAngle)
    Q_PROPERTY(int strokeWidth READ strokeWidth WRITE setStrokeWidth)

public:
    explicit IndeterminateProgressRing(QWidget* parent = nullptr, bool start = true);

    int startAngle() const;
    void setStartAngle(int angle);

    int spanAngle() const;
    void setSpanAngle(int angle);

    int strokeWidth() const;
    void setStrokeWidth(int w);

    QColor lightBarColor() const;
    QColor darkBarColor() const;
    void setCustomBarColor(const QColor& light, const QColor& dark);

    void setCustomBackgroundColor(const QColor& light, const QColor& dark);

    void start();
    void stop();

protected:
    void paintEvent(QPaintEvent* e) override;

private:
    int _startAngle = -180;
    int _spanAngle = 0;
    int _strokeWidth = 6;

    QColor lightBackgroundColor = QColor(0, 0, 0, 0);
    QColor darkBackgroundColor = QColor(255, 255, 255, 0);
    QColor _lightBarColor;
    QColor _darkBarColor;

    QPropertyAnimation* startAngleAni1 = nullptr;
    QPropertyAnimation* startAngleAni2 = nullptr;
    QPropertyAnimation* spanAngleAni1 = nullptr;
    QPropertyAnimation* spanAngleAni2 = nullptr;

    QSequentialAnimationGroup* startAngleAniGroup = nullptr;
    QSequentialAnimationGroup* spanAngleAniGroup = nullptr;
    QParallelAnimationGroup* aniGroup = nullptr;
};

} // namespace qfw
