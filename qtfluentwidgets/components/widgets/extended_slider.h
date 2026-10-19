#pragma once
#include <QPointer>
#include <QTimer>

#include "components/widgets/slider.h"

namespace qfw {
class ToolTip;
class ToolTipSlider : public Slider {
    Q_OBJECT
public:
    explicit ToolTipSlider(QWidget* parent = nullptr);
    explicit ToolTipSlider(Qt::Orientation orientation, QWidget* parent = nullptr);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void enterEvent(enterEvent_QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    void showToolTip();
    void hideToolTip();
    void moveFromHandle(QMouseEvent* event);
    void finishDrag();
    QPointer<SliderHandle> handle_;
    QPointer<ToolTip> tip_;
    QTimer hideTimer_;
    bool dragging_ = false;
    QPoint pressOffset_;
};

class RangeSlider : public QWidget {
    Q_OBJECT
    Q_PROPERTY(int minimum READ minimum WRITE setMinimum)
    Q_PROPERTY(int maximum READ maximum WRITE setMaximum)
    Q_PROPERTY(int minValue READ minValue WRITE setMinValue NOTIFY minValueChanged)
    Q_PROPERTY(int maxValue READ maxValue WRITE setMaxValue NOTIFY maxValueChanged)
public:
    explicit RangeSlider(QWidget* parent = nullptr);
    explicit RangeSlider(Qt::Orientation orientation, QWidget* parent = nullptr);
    Qt::Orientation orientation() const;
    void setOrientation(Qt::Orientation orientation);
    int minimum() const;
    int maximum() const;
    void setMinimum(int value);
    void setMaximum(int value);
    void setRange(int minimum, int maximum);
    int minValue() const;
    int maxValue() const;
    void setMinValue(int value);
    void setMaxValue(int value);
    void setValues(int minimumValue, int maximumValue);
    int minimumGap() const;
    void setMinimumGap(int gap);
    int singleStep() const;
    void setSingleStep(int step);
    void setThemeColor(const QColor& light, const QColor& dark);
    bool isToolTipEnabled() const;
    void setToolTipEnabled(bool enabled);
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
signals:
    void minValueChanged(int value);
    void maxValueChanged(int value);
    void rangeChanged(int minimumValue, int maximumValue);
    void sliderPressed(int handle);
    void sliderReleased(int handle);

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void changeEvent(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    qint64 effectiveGap() const;
    void commitValues(int minimumValue, int maximumValue);
    QPointF handleCenter(int value) const;
    int valueAt(const QPoint& pos) const;
    void dragTo(const QPoint& pos);
    void stepActive(qint64 step);
    void showToolTips();
    void hideToolTips();
    Qt::Orientation orientation_;
    int minimum_ = 0, maximum_ = 100, minValue_ = 0, maxValue_ = 100;
    int gap_ = 1, step_ = 1;
    int pressed_ = -1, active_ = 0;
    QColor light_, dark_;
    bool toolTipEnabled_ = true;
    QPointer<ToolTip> tips_[2];
    QTimer hideTimer_;
};
}  // namespace qfw
