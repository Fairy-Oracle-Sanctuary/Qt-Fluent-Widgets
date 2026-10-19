#pragma once
#include "acrylic_line_edit.h"
#include "components/widgets/combo_box.h"
#include "components/settings/setting_card.h"

namespace qfw {
class AcrylicComboBoxMenu : public AcrylicMenuAdapter<ComboBoxMenu> {
    Q_OBJECT
public:
    explicit AcrylicComboBoxMenu(QWidget* parent=nullptr);
};
class AcrylicComboBox : public ComboBox {
    Q_OBJECT
public:
    using ComboBox::ComboBox;
protected:
    ComboBoxMenu* createComboMenu() override;
};
class AcrylicEditableComboBox : public AcrylicLineEditBase<EditableComboBox> {
    Q_OBJECT
public:
    using AcrylicLineEditBase::AcrylicLineEditBase;
protected:
    ComboBoxMenu* createComboMenu() override;
};
class AcrylicComboBoxSettingCard : public SettingCard {
    Q_OBJECT
public:
    explicit AcrylicComboBoxSettingCard(OptionsConfigItem* configItem,const QVariant& icon,
        const QString& title,const QString& content={},const QStringList& texts={},QWidget* parent=nullptr);
    explicit AcrylicComboBoxSettingCard(OptionsConfigItem* configItem,FluentIconEnum icon,
        const QString& title,const QString& content={},const QStringList& texts={},QWidget* parent=nullptr)
        : AcrylicComboBoxSettingCard(configItem,QVariant::fromValue(FluentIcon(icon).qicon()),title,content,texts,parent) {}
    explicit AcrylicComboBoxSettingCard(OptionsConfigItem* configItem,const FluentIconBase& icon,
        const QString& title,const QString& content={},const QStringList& texts={},QWidget* parent=nullptr)
        : AcrylicComboBoxSettingCard(configItem,QVariant::fromValue(icon.qicon()),title,content,texts,parent) {}
    AcrylicComboBox* comboBox() const { return comboBox_; }
    void setValue(const QVariant& value) override;
protected:
    void changeEvent(QEvent* event) override;
private:
    QPointer<OptionsConfigItem> configItem_;
    AcrylicComboBox* comboBox_;
};
}
