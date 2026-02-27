#pragma once

#include <QColor>
#include <QMap>
#include <QPropertyAnimation>
#include <QProxyStyle>
#include <QSlider>
#include <QVariant>
#include <QWidget>

namespace qfw {

class SliderHandle : public QWidget {
    Q_OBJECT
    Q_PROPERTY(int radius READ radius WRITE setRadius)

public:
    explicit SliderHandle(QSlider* parent);

    int radius() const;
    void setRadius(int r);

    void setHandleColor(const QColor& light, const QColor& dark);

signals:
    void pressed();
    void released();

protected:
    void enterEvent(QEnterEvent* e) override;
    void leaveEvent(QEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

private:
    void startAni(int radius);

    int _radius = 5;
    QColor lightHandleColor;
    QColor darkHandleColor;
    QPropertyAnimation* radiusAni = nullptr;
};

class Slider : public QSlider {
    Q_OBJECT

public:
    explicit Slider(QWidget* parent = nullptr);
    explicit Slider(Qt::Orientation orientation, QWidget* parent = nullptr);

    void setThemeColor(const QColor& light, const QColor& dark);

signals:
    void clicked(int value);

protected:
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void paintEvent(QPaintEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;

    virtual void drawHorizonTick(QPainter* painter);
    virtual void drawVerticalTick(QPainter* painter);

private slots:
    void adjustHandlePos();

private:
    void postInit();
    int posToValue(const QPoint& pos);
    int grooveLength() const;

    void drawHorizonGroove(QPainter* painter);
    void drawVerticalGroove(QPainter* painter);

    SliderHandle* handle = nullptr;
    QPoint _pressedPos;
    QColor lightGrooveColor;
    QColor darkGrooveColor;
};

class ClickableSlider : public QSlider {
    Q_OBJECT

public:
    explicit ClickableSlider(QWidget* parent = nullptr);
    explicit ClickableSlider(Qt::Orientation orientation, QWidget* parent = nullptr);

signals:
    void clicked(int value);

protected:
    void mousePressEvent(QMouseEvent* e) override;
};

class HollowHandleStyle : public QProxyStyle {
    Q_OBJECT

public:
    explicit HollowHandleStyle(const QMap<QString, QVariant>& config = {});

    QRect subControlRect(ComplexControl cc, const QStyleOptionComplex* opt, SubControl sc,
                         const QWidget* widget) const override;
    void drawComplexControl(ComplexControl cc, const QStyleOptionComplex* opt, QPainter* painter,
                            const QWidget* widget) const override;

private:
    QMap<QString, QVariant> config;
};

}  // namespace qfw
