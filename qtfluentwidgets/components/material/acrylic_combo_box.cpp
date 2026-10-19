#include "acrylic_combo_box.h"
#include <QHBoxLayout>
#include <QEvent>
namespace qfw {
AcrylicComboBoxMenu::AcrylicComboBoxMenu(QWidget* parent) : AcrylicMenuAdapter(parent) {
    view()->item(0)->setSizeHint(QSize(1,2));
    view()->setItemHeight(33);
    view()->adjustSizeForMenu(); adjustSize();
}
ComboBoxMenu* AcrylicComboBox::createComboMenu() { return new AcrylicComboBoxMenu(this); }
ComboBoxMenu* AcrylicEditableComboBox::createComboMenu() { return new AcrylicComboBoxMenu(this); }
AcrylicComboBoxSettingCard::AcrylicComboBoxSettingCard(OptionsConfigItem* configItem,const QVariant& icon,
    const QString& title,const QString& content,const QStringList& texts,QWidget* parent)
    : SettingCard(icon,title,content,parent),configItem_(configItem),comboBox_(new AcrylicComboBox(this)) {
    hBoxLayout_->addWidget(comboBox_,0,Qt::AlignRight); hBoxLayout_->addSpacing(16);
    hBoxLayout_->setContentsMargins(isRightToLeft()?0:16,0,isRightToLeft()?16:0,0);
    if(!configItem_ || !configItem_->optionsValidator()) return;
    const auto options=configItem_->optionsValidator()->options();
    for(int i=0;i<qMin(options.size(),texts.size());++i) comboBox_->addItem(texts[i],{},options[i]);
    setValue(QConfig::instance().get(*configItem_));
    connect(comboBox_,&ComboBox::currentIndexChanged,this,[this](int index) {
        if(configItem_ && index>=0 && index<comboBox_->count()) QConfig::instance().set(*configItem_,comboBox_->itemData(index));
    });
    connect(configItem_,&ConfigItem::valueChanged,this,&AcrylicComboBoxSettingCard::setValue);
}
void AcrylicComboBoxSettingCard::setValue(const QVariant& value) {
    if(!configItem_) return;
    const int index=comboBox_->findData(value); if(index<0) return;
    comboBox_->setCurrentIndex(index); QConfig::instance().set(*configItem_,value);
}
void AcrylicComboBoxSettingCard::changeEvent(QEvent* event) {
    SettingCard::changeEvent(event);
    if(event->type()==QEvent::LayoutDirectionChange)
        hBoxLayout_->setContentsMargins(isRightToLeft()?0:16,0,isRightToLeft()?16:0,0);
}
}
