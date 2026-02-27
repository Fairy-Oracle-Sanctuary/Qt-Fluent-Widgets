#pragma once

#include <QColor>
#include <QFrame>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QVariant>

#include "common/config.h"
#include "components/widgets/icon_widget.h"

class QHBoxLayout;
class QVBoxLayout;

namespace qfw {

class ComboBox;
class HyperlinkButton;
class RangeConfigItem;
class Slider;
class SwitchButton;

class SettingIconWidget : public IconWidget {
    Q_OBJECT

public:
    explicit SettingIconWidget(QWidget* parent = nullptr);
    explicit SettingIconWidget(const QVariant& icon, QWidget* parent = nullptr);

    void setIcon(const QVariant& icon);

protected:
    void paintEvent(QPaintEvent* event) override;
};

class SettingCard : public QFrame {
    Q_OBJECT

public:
    explicit SettingCard(const QVariant& icon, const QString& title, const QString& content = {},
                         QWidget* parent = nullptr);

    explicit SettingCard(FluentIconEnum icon, const QString& title, const QString& content = {},
                         QWidget* parent = nullptr)
        : SettingCard(QVariant::fromValue(FluentIcon(icon).qicon()), title, content, parent) {}

    explicit SettingCard(const FluentIconBase& icon, const QString& title,
                         const QString& content = {}, QWidget* parent = nullptr)
        : SettingCard(QVariant::fromValue(icon.qicon()), title, content, parent) {}

    void setTitle(const QString& title);
    void setContent(const QString& content);

    virtual void setValue(const QVariant& value);

    void setIconSize(int width, int height);

protected:
    void paintEvent(QPaintEvent* event) override;

    SettingIconWidget* iconLabel_;
    QLabel* titleLabel_;
    QLabel* contentLabel_;
    QHBoxLayout* hBoxLayout_;
    QVBoxLayout* vBoxLayout_;
};

class SwitchSettingCard : public SettingCard {
    Q_OBJECT

public:
    explicit SwitchSettingCard(const QVariant& icon, const QString& title,
                               const QString& content = {}, ConfigItem* configItem = nullptr,
                               QWidget* parent = nullptr);

    explicit SwitchSettingCard(FluentIconEnum icon, const QString& title,
                               const QString& content = {}, ConfigItem* configItem = nullptr,
                               QWidget* parent = nullptr)
        : SwitchSettingCard(QVariant::fromValue(FluentIcon(icon).qicon()), title, content,
                            configItem, parent) {}

    explicit SwitchSettingCard(const FluentIconBase& icon, const QString& title,
                               const QString& content = {}, ConfigItem* configItem = nullptr,
                               QWidget* parent = nullptr)
        : SwitchSettingCard(QVariant::fromValue(icon.qicon()), title, content, configItem, parent) {
    }

    void setValue(const QVariant& value) override;

    void setChecked(bool isChecked);
    bool isChecked() const;

signals:
    void checkedChanged(bool checked);

private slots:
    void onCheckedChanged(bool isChecked);

private:
    ConfigItem* configItem_;
    SwitchButton* switchButton_;
};

class RangeSettingCard : public SettingCard {
    Q_OBJECT

public:
    explicit RangeSettingCard(RangeConfigItem* configItem, const QVariant& icon,
                              const QString& title, const QString& content = {},
                              QWidget* parent = nullptr);

    explicit RangeSettingCard(RangeConfigItem* configItem, FluentIconEnum icon,
                              const QString& title, const QString& content = {},
                              QWidget* parent = nullptr)
        : RangeSettingCard(configItem, QVariant::fromValue(FluentIcon(icon).qicon()), title,
                           content, parent) {}

    explicit RangeSettingCard(RangeConfigItem* configItem, const FluentIconBase& icon,
                              const QString& title, const QString& content = {},
                              QWidget* parent = nullptr)
        : RangeSettingCard(configItem, QVariant::fromValue(icon.qicon()), title, content, parent) {}

    void setValue(const QVariant& value) override;

signals:
    void valueChanged(int value);

private slots:
    void onSliderValueChanged(int value);

private:
    RangeConfigItem* configItem_;
    Slider* slider_;
    QLabel* valueLabel_;
};

class PushSettingCard : public SettingCard {
    Q_OBJECT

public:
    explicit PushSettingCard(const QString& text, const QVariant& icon, const QString& title,
                             const QString& content = {}, QWidget* parent = nullptr);

    explicit PushSettingCard(const QString& text, FluentIconEnum icon, const QString& title,
                             const QString& content = {}, QWidget* parent = nullptr)
        : PushSettingCard(text, QVariant::fromValue(FluentIcon(icon).qicon()), title, content,
                          parent) {}

    explicit PushSettingCard(const QString& text, const FluentIconBase& icon, const QString& title,
                             const QString& content = {}, QWidget* parent = nullptr)
        : PushSettingCard(text, QVariant::fromValue(icon.qicon()), title, content, parent) {}

signals:
    void clicked();

protected:
    QPushButton* button_;
};

class PrimaryPushSettingCard : public PushSettingCard {
    Q_OBJECT

public:
    explicit PrimaryPushSettingCard(const QString& text, const QVariant& icon, const QString& title,
                                    const QString& content = {}, QWidget* parent = nullptr);

    explicit PrimaryPushSettingCard(const QString& text, FluentIconEnum icon, const QString& title,
                                    const QString& content = {}, QWidget* parent = nullptr)
        : PrimaryPushSettingCard(text, QVariant::fromValue(FluentIcon(icon).qicon()), title,
                                 content, parent) {}

    explicit PrimaryPushSettingCard(const QString& text, const FluentIconBase& icon,
                                    const QString& title, const QString& content = {},
                                    QWidget* parent = nullptr)
        : PrimaryPushSettingCard(text, QVariant::fromValue(icon.qicon()), title, content, parent) {}
};

class HyperlinkCard : public SettingCard {
    Q_OBJECT

public:
    explicit HyperlinkCard(const QString& url, const QString& text, const QVariant& icon,
                           const QString& title, const QString& content = {},
                           QWidget* parent = nullptr);

    explicit HyperlinkCard(const QString& url, const QString& text, FluentIconEnum icon,
                           const QString& title, const QString& content = {},
                           QWidget* parent = nullptr)
        : HyperlinkCard(url, text, QVariant::fromValue(FluentIcon(icon).qicon()), title, content,
                        parent) {}

    explicit HyperlinkCard(const QString& url, const QString& text, const FluentIconBase& icon,
                           const QString& title, const QString& content = {},
                           QWidget* parent = nullptr)
        : HyperlinkCard(url, text, QVariant::fromValue(icon.qicon()), title, content, parent) {}

private:
    HyperlinkButton* linkButton_;
};

class ColorPickerButton : public QToolButton {
    Q_OBJECT

public:
    explicit ColorPickerButton(const QColor& color, const QString& title, QWidget* parent = nullptr,
                               bool enableAlpha = false);

    void setColor(const QColor& color);

signals:
    void colorChanged(const QColor& color);

protected:
    void paintEvent(QPaintEvent* event) override;

private slots:
    void showColorDialog();
    void onColorChanged(const QColor& color);

private:
    QColor color_;
    QString title_;
    bool enableAlpha_;
};

class ColorSettingCard : public SettingCard {
    Q_OBJECT

public:
    explicit ColorSettingCard(ConfigItem* configItem, const QVariant& icon, const QString& title,
                              const QString& content = {}, QWidget* parent = nullptr,
                              bool enableAlpha = false);

    explicit ColorSettingCard(ConfigItem* configItem, FluentIconEnum icon, const QString& title,
                              const QString& content = {}, QWidget* parent = nullptr,
                              bool enableAlpha = false)
        : ColorSettingCard(configItem, QVariant::fromValue(FluentIcon(icon).qicon()), title,
                           content, parent, enableAlpha) {}

    explicit ColorSettingCard(ConfigItem* configItem, const FluentIconBase& icon,
                              const QString& title, const QString& content = {},
                              QWidget* parent = nullptr, bool enableAlpha = false)
        : ColorSettingCard(configItem, QVariant::fromValue(icon.qicon()), title, content, parent,
                           enableAlpha) {}

    void setValue(const QVariant& value) override;

signals:
    void colorChanged(const QColor& color);

private slots:
    void onColorPicked(const QColor& color);

private:
    ConfigItem* configItem_;
    ColorPickerButton* colorPicker_;
};

class ComboBoxSettingCard : public SettingCard {
    Q_OBJECT

public:
    explicit ComboBoxSettingCard(OptionsConfigItem* configItem, const QVariant& icon,
                                 const QString& title, const QString& content = {},
                                 const QStringList& texts = {}, QWidget* parent = nullptr);

    explicit ComboBoxSettingCard(OptionsConfigItem* configItem, FluentIconEnum icon,
                                 const QString& title, const QString& content = {},
                                 const QStringList& texts = {}, QWidget* parent = nullptr)
        : ComboBoxSettingCard(configItem, QVariant::fromValue(FluentIcon(icon).qicon()), title,
                              content, texts, parent) {}

    explicit ComboBoxSettingCard(OptionsConfigItem* configItem, const FluentIconBase& icon,
                                 const QString& title, const QString& content = {},
                                 const QStringList& texts = {}, QWidget* parent = nullptr)
        : ComboBoxSettingCard(configItem, QVariant::fromValue(icon.qicon()), title, content, texts,
                              parent) {}

    void setValue(const QVariant& value) override;

private slots:
    void onCurrentIndexChanged(int index);

private:
    OptionsConfigItem* configItem_;
    ComboBox* comboBox_;
    QList<QVariant> options_;
    QStringList texts_;
};

}  // namespace qfw
