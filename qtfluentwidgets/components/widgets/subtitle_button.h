#pragma once

#include "components/widgets/button.h"
#include "components/widgets/check_box.h"

namespace qfw {

/** Two-line checkbox. Subtitle text is literal and wraps to the available width. */
class SubtitleCheckBox : public CheckBox {
    Q_OBJECT
    Q_PROPERTY(QString subText READ subText WRITE setSubText NOTIFY subTextChanged)
    Q_PROPERTY(int margin READ margin WRITE setMargin)
public:
    explicit SubtitleCheckBox(QWidget* parent = nullptr);
    SubtitleCheckBox(const QString& text, const QString& subText, QWidget* parent = nullptr);
    QString subText() const;
    void setSubText(const QString& text);
    void setText(const QString& text);
    int margin() const;
    void setMargin(int margin);
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    int heightForWidth(int width) const override;
signals:
    void subTextChanged(const QString& text);

protected:
    void paintEvent(QPaintEvent* event) override;
    bool hitButton(const QPoint& position) const override;
    void changeEvent(QEvent* event) override;

private:
    QString subText_;
    int margin_ = 2;
};

/** Native radio-button grouping and keyboard behavior with a descriptive second line. */
class SubtitleRadioButton : public RadioButton {
    Q_OBJECT
    Q_PROPERTY(QString subText READ subText WRITE setSubText NOTIFY subTextChanged)
    Q_PROPERTY(int margin READ margin WRITE setMargin)
public:
    explicit SubtitleRadioButton(QWidget* parent = nullptr);
    SubtitleRadioButton(const QString& text, const QString& subText, QWidget* parent = nullptr);
    QString subText() const;
    void setSubText(const QString& text);
    void setText(const QString& text);
    int margin() const;
    void setMargin(int margin);
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    int heightForWidth(int width) const override;
signals:
    void subTextChanged(const QString& text);

protected:
    void paintEvent(QPaintEvent* event) override;
    bool hitButton(const QPoint& position) const override;
    void changeEvent(QEvent* event) override;

private:
    QString subText_;
    int margin_ = 6;
};

}  // namespace qfw
