#pragma once

#include <QButtonGroup>
#include <QColor>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "common/config.h"
#include "components/settings/expand_setting_card.h"
#include "components/widgets/button.h"

namespace qfw {

class CustomColorSettingCard : public ExpandGroupSettingCard {
    Q_OBJECT

public:
    explicit CustomColorSettingCard(ColorConfigItem* configItem, const QVariant& icon,
                                    const QString& title, const QString& content = QString(),
                                    QWidget* parent = nullptr, bool enableAlpha = false);

    explicit CustomColorSettingCard(ColorConfigItem* configItem, FluentIconEnum icon,
                                    const QString& title, const QString& content = QString(),
                                    QWidget* parent = nullptr, bool enableAlpha = false)
        : CustomColorSettingCard(configItem, QVariant::fromValue(FluentIcon(icon).qicon()), title,
                                 content, parent, enableAlpha) {}

    explicit CustomColorSettingCard(ColorConfigItem* configItem, const FluentIconBase& icon,
                                    const QString& title, const QString& content = QString(),
                                    QWidget* parent = nullptr, bool enableAlpha = false)
        : CustomColorSettingCard(configItem, QVariant::fromValue(icon.qicon()), title, content,
                                 parent, enableAlpha) {}

    bool isEnableAlpha() const;
    void setEnableAlpha(bool enable);

signals:
    void colorChanged(const QColor& color);

private slots:
    void onRadioButtonClicked(QAbstractButton* button);
    void showColorDialog();
    void onCustomColorChanged(const QColor& color);

private:
    void initWidget();
    void initLayout();

    bool enableAlpha_;
    ColorConfigItem* configItem_;
    QColor defaultColor_;
    QColor customColor_;

    QLabel* choiceLabel_;

    QWidget* radioWidget_;
    QVBoxLayout* radioLayout_;
    RadioButton* defaultRadioButton_;
    RadioButton* customRadioButton_;
    QButtonGroup* buttonGroup_;

    QWidget* customColorWidget_;
    QHBoxLayout* customColorLayout_;
    QLabel* customLabel_;
    QPushButton* chooseColorButton_;
};

}  // namespace qfw
