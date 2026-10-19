#pragma once
#include "components/widgets/button.h"

namespace qfw {
class ColorPickerView;
class Flyout;

class DropDownColorPicker : public DropDownPushButton {
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(bool alphaEnabled READ isAlphaEnabled WRITE setAlphaEnabled)
public:
    explicit DropDownColorPicker(QWidget* parent = nullptr);
    explicit DropDownColorPicker(const QColor& color, QWidget* parent = nullptr, bool enableAlpha = true);
    ~DropDownColorPicker() override;
    QColor color() const;
    void setColor(const QColor& color);
    void setAlphaEnabled(bool enabled);
    bool isAlphaEnabled() const;
    void showPicker();
    void closePicker();
    bool isPickerVisible() const;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
signals:
    void colorChanged(const QColor& color);
    void colorSelected(const QColor& color);
    void colorPreviewed(const QColor& color);
    void pickingCanceled();
protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void changeEvent(QEvent* event) override;
private:
    QColor color_;
    bool alphaEnabled_ = true;
    bool confirmed_ = false;
    QPointer<Flyout> popup_;
    QPointer<ColorPickerView> view_;
};
}
