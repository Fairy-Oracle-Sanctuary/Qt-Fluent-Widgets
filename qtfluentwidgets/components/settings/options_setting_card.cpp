#include "components/settings/options_setting_card.h"

#include <QApplication>
#include <QFont>

#include "common/config.h"
#include "common/style_sheet.h"
#include "components/widgets/button.h"
namespace qfw {

OptionsSettingCard::OptionsSettingCard(OptionsConfigItem* configItem, const QVariant& icon,
                                       const QString& title, const QString& content,
                                       const QStringList& texts, QWidget* parent)
    : ExpandSettingCard(icon, title, content, parent),
      configItem_(configItem),
      configName_(configItem->name()),
      texts_(texts) {
    choiceLabel_ = new QLabel(this);
    buttonGroup_ = new QButtonGroup(this);

    choiceLabel_->setObjectName("titleLabel");
    choiceLabel_->setFont(QFont("Microsoft YaHei", 10, QFont::Normal));
    addWidget(choiceLabel_);

    initWidget();
}

void OptionsSettingCard::initWidget() {
    // Create buttons
    viewLayout_->setSpacing(19);
    viewLayout_->setContentsMargins(48, 18, 0, 18);

    const QList<QVariant>& options = configItem_->optionsValidator()->options();
    for (int i = 0; i < texts_.size() && i < options.size(); ++i) {
        const QString& text = texts_[i];
        const QVariant& option = options[i];

        auto* button = new RadioButton(text, view_);
        buttonGroup_->addButton(button);
        viewLayout_->addWidget(button);
        button->setProperty(configName_.toUtf8().constData(), option);
    }

    adjustViewSize();
    setValue(QConfig::instance().get(*configItem_));

    connect(configItem_, &ConfigItem::valueChanged, this,
            [this](const QVariant& value) { setValue(value); });
    connect(buttonGroup_, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), this,
            &OptionsSettingCard::onButtonClicked);

    qfw::setStyleSheet(this, FluentStyleSheet::ExpandSettingCard);
}

void OptionsSettingCard::onButtonClicked(QAbstractButton* button) {
    if (button->text() == choiceLabel_->text()) {
        return;
    }

    QVariant value = button->property(configName_.toUtf8().constData());
    QConfig::instance().set(*configItem_, value, true, true,
                            true);  // save=true, emitSignal=true, lazyUpdate=true

    choiceLabel_->setText(button->text());
    choiceLabel_->adjustSize();
    emit optionChanged(configItem_);
}

void OptionsSettingCard::setValue(const QVariant& value) {
    QConfig::instance().set(*configItem_, value, false, true,
                            true);  // save=false, emitSignal=true, lazyUpdate=true

    const QList<QAbstractButton*>& buttons = buttonGroup_->buttons();
    for (QAbstractButton* button : buttons) {
        bool isChecked = button->property(configName_.toUtf8().constData()) == value;
        button->setChecked(isChecked);

        if (isChecked) {
            choiceLabel_->setText(button->text());
            choiceLabel_->adjustSize();
        }
    }
}

}  // namespace qfw
