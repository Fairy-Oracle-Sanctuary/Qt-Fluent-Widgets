#pragma once

#include <QColor>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QPointer>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollArea>
#include <QVBoxLayout>

#include "../widgets/button.h"
#include "../widgets/line_edit.h"
#include "../widgets/scroll_area.h"
#include "../widgets/slider.h"
#include "mask_dialog_base.h"

namespace qfw {

class HuePanel : public QWidget {
    Q_OBJECT

public:
    explicit HuePanel(const QColor& color = QColor(255, 0, 0), QWidget* parent = nullptr);

    void setColor(const QColor& color);

    int hue() const;
    int saturation() const;

signals:
    void colorChanged(const QColor& color);

protected:
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

private:
    void setPickerPosition(const QPoint& pos);

    QPixmap huePixmap_;
    QColor color_;
    QPoint pickerPos_;
};

class BrightnessSlider : public ClickableSlider {
    Q_OBJECT

public:
    explicit BrightnessSlider(const QColor& color, QWidget* parent = nullptr);

    void setColor(const QColor& color);

signals:
    void colorChanged(const QColor& color);

private slots:
    void onValueChanged(int value);

private:
    QColor color_;
};

class ColorCard : public QWidget {
    Q_OBJECT

public:
    explicit ColorCard(const QColor& color, QWidget* parent = nullptr, bool enableAlpha = false);

    void setColor(const QColor& color);

protected:
    void paintEvent(QPaintEvent* e) override;

private:
    QPixmap createTiledBackground();

    QColor color_;
    bool enableAlpha_;
    QPixmap tiledPixmap_;
};

class ColorLineEdit : public LineEdit {
    Q_OBJECT

public:
    explicit ColorLineEdit(int value, QWidget* parent = nullptr);

signals:
    void valueChanged(const QString& value);

private slots:
    void onTextEdited(const QString& text);
};

class HexColorLineEdit : public ColorLineEdit {
    Q_OBJECT

public:
    explicit HexColorLineEdit(const QColor& color, QWidget* parent = nullptr,
                              bool enableAlpha = false);

    void setColor(const QColor& color);

private:
    QColor::NameFormat colorFormat_;
    QLabel* prefixLabel_;
};

class ColorDialog : public MaskDialogBase {
    Q_OBJECT

public:
    explicit ColorDialog(const QColor& color, const QString& title, QWidget* parent = nullptr,
                         bool enableAlpha = false);

    void setColor(const QColor& color, bool movePicker = true);

    PrimaryPushButton* yesButton;
    QPushButton* cancelButton;

signals:
    void colorChanged(const QColor& color);

protected:
    void showEvent(QShowEvent* e) override;

private slots:
    void onHueChanged(const QColor& color);
    void onBrightnessChanged(const QColor& color);
    void onRedChanged(const QString& red);
    void onGreenChanged(const QString& green);
    void onBlueChanged(const QString& blue);
    void onHexColorChanged(const QString& color);
    void onYesButtonClicked();

private:
    void initWidget();
    void initLayout();
    void setQss();
    void connectSignalToSlot();
    void updateStyle();

    bool enableAlpha_;
    QColor oldColor_;
    QColor color_;

    SingleDirectionScrollArea* scrollArea_;
    QWidget* scrollWidget_;

    QFrame* buttonGroup_;

    PrimaryPushButton* yesButton_;
    QPushButton* cancelButton_;

    QLabel* titleLabel_;
    HuePanel* huePanel_;
    ColorCard* newColorCard_;
    ColorCard* oldColorCard_;
    BrightnessSlider* brightSlider_;

    QLabel* editLabel_;
    QLabel* redLabel_;
    QLabel* greenLabel_;
    QLabel* blueLabel_;

    HexColorLineEdit* hexLineEdit_;
    ColorLineEdit* redLineEdit_;
    ColorLineEdit* greenLineEdit_;
    ColorLineEdit* blueLineEdit_;

    QVBoxLayout* vBoxLayout_;
};

}  // namespace qfw
