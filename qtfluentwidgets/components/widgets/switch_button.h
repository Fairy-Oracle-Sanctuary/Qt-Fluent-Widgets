#pragma once

#include <QColor>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QPaintEvent>
#include <QPropertyAnimation>
#include <QToolButton>
#include <QWidget>

#include "components/widgets/button.h"

namespace qfw {

class Indicator : public ToolButton {
    Q_OBJECT
    Q_PROPERTY(qreal sliderX READ sliderX WRITE setSliderX)

public:
    explicit Indicator(QWidget* parent = nullptr);

    void toggle();
    void setDown(bool isDown);
    void setHover(bool isHover);
    void setCheckedColor(const QColor& light, const QColor& dark);

    qreal sliderX() const;
    void setSliderX(qreal x);

signals:
    void checkedChanged(bool checked);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    void drawBackground(QPainter* painter);
    void drawCircle(QPainter* painter);
    QColor backgroundColor() const;
    QColor borderColor() const;
    QColor sliderColor() const;
    void toggleSlider();

    qreal sliderX_ = 5;
    QPropertyAnimation* slideAni_ = nullptr;
    QColor lightCheckedColor_;
    QColor darkCheckedColor_;
};

enum class IndicatorPosition {
    LEFT = 0,
    RIGHT = 1
};

class SwitchButton : public QWidget {
    Q_OBJECT
    Q_PROPERTY(int spacing READ spacing WRITE setSpacing)
    Q_PROPERTY(bool checked READ isChecked WRITE setChecked)
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(QString onText READ onText WRITE setOnText)
    Q_PROPERTY(QString offText READ offText WRITE setOffText)

public:
    explicit SwitchButton(QWidget* parent = nullptr, IndicatorPosition indicatorPos = IndicatorPosition::LEFT);
    explicit SwitchButton(const QString& text, QWidget* parent = nullptr,
                          IndicatorPosition indicatorPos = IndicatorPosition::LEFT);

    bool isChecked() const;
    void setChecked(bool isChecked);
    void toggleChecked();

    void setTextColor(const QColor& light, const QColor& dark);
    void setCheckedIndicatorColor(const QColor& light, const QColor& dark);

    QString text() const;
    void setText(const QString& text);

    QString onText() const;
    void setOnText(const QString& text);

    QString offText() const;
    void setOffText(const QString& text);

    int spacing() const;
    void setSpacing(int spacing);

signals:
    void checkedChanged(bool checked);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    void initWidget();
    void updateText();

    QString text_;
    QString offText_;
    QString onText_;
    int spacing_ = 12;
    IndicatorPosition indicatorPos_;
    QHBoxLayout* hBox_ = nullptr;
    Indicator* indicator_ = nullptr;
    QLabel* label_ = nullptr;
    QColor lightTextColor_;
    QColor darkTextColor_;
};

}  // namespace qfw
