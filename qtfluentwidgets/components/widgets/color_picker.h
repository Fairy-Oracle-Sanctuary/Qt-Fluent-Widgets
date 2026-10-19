#pragma once

#include <QAbstractButton>
#include <QButtonGroup>
#include <QColor>
#include <QHBoxLayout>
#include <QList>
#include <QPointer>

#include "components/widgets/button.h"

namespace qfw {
class ColorDialog;
class ColorPalettePopup;

class CircleColorPicker : public QWidget {
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
public:
    explicit CircleColorPicker(QWidget* parent = nullptr);
    explicit CircleColorPicker(const QList<QColor>& colors, QWidget* parent = nullptr);
    static QList<QColor> defaultColors();
    QList<QColor> colors() const;
    int count() const;
    QColor color() const;
    int currentIndex() const;
    void setColors(const QList<QColor>& colors);
    void addColors(const QList<QColor>& colors);
    void addColor(const QColor& color);
    void setColor(const QColor& color);
    void setCurrentIndex(int index);
    QAbstractButton* button(int index) const;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
signals:
    void currentIndexChanged(int index);
    void colorChanged(const QColor& color);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    void applySelection(int index);
    void emitChanges(int oldIndex, const QColor& oldColor);
    QList<QColor> colors_;
    QList<QAbstractButton*> buttons_;
    int current_ = -1;
    QButtonGroup* group_;
    QHBoxLayout* layout_;
};

class DropDownColorPalette : public DropDownPushButton {
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(bool alphaEnabled READ isAlphaEnabled WRITE setAlphaEnabled)
public:
    explicit DropDownColorPalette(QWidget* parent = nullptr);
    explicit DropDownColorPalette(const QColor& color, QWidget* parent = nullptr);
    ~DropDownColorPalette() override;
    static QList<QList<QColor>> themeColors();
    static QList<QColor> standardColors();
    QColor color() const;
    void setColor(const QColor& color);
    QColor automaticColor() const;
    void setAutomaticColor(const QColor& color);
    bool isAutomatic() const;
    void setAutomatic();
    void setAlphaEnabled(bool enabled);
    bool isAlphaEnabled() const;
    void showPalette();
    void closePalette();
    void showColorDialog();
    bool isPaletteVisible() const;
    bool isColorDialogVisible() const;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
signals:
    void colorChanged(const QColor& color);
    void colorSelected(const QColor& color);
    void automaticChanged(bool automatic);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    friend class ColorPalettePopup;
    void setState(const QColor& color, bool automatic);
    void chooseColor(const QColor& color, bool automatic);
    QColor color_;
    QColor automaticColor_ = QColor(Qt::black);
    bool automatic_ = false;
    bool alphaEnabled_ = false;
    QPointer<ColorPalettePopup> popup_;
    QPointer<ColorDialog> dialog_;
};
}  // namespace qfw
