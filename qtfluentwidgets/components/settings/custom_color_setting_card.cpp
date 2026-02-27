#include "components/settings/custom_color_setting_card.h"

#include <QApplication>
#include <QFont>

#include "common/config.h"
#include "common/style_sheet.h"
#include "components/dialog_box/color_dialog.h"

namespace qfw {

CustomColorSettingCard::CustomColorSettingCard(ColorConfigItem* configItem, const QVariant& icon,
                                               const QString& title, const QString& content,
                                               QWidget* parent, bool enableAlpha)
    : ExpandGroupSettingCard(icon, title, content, parent),
      enableAlpha_(enableAlpha),
      configItem_(configItem) {
    defaultColor_ = configItem->defaultValue().value<QColor>();
    customColor_ = QConfig::instance().get(*configItem).value<QColor>();

    choiceLabel_ = new QLabel(this);

    radioWidget_ = new QWidget(view_);
    radioLayout_ = new QVBoxLayout(radioWidget_);
    defaultRadioButton_ = new RadioButton(tr("Default color"), radioWidget_);
    customRadioButton_ = new RadioButton(tr("Custom color"), radioWidget_);
    buttonGroup_ = new QButtonGroup(this);

    customColorWidget_ = new QWidget(view_);
    customColorLayout_ = new QHBoxLayout(customColorWidget_);
    customLabel_ = new QLabel(tr("Custom color"), customColorWidget_);
    chooseColorButton_ = new QPushButton(tr("Choose color"), customColorWidget_);

    initWidget();
}

void CustomColorSettingCard::initWidget() {
    initLayout();

    if (defaultColor_ != customColor_) {
        customRadioButton_->setChecked(true);
        chooseColorButton_->setEnabled(true);
    } else {
        defaultRadioButton_->setChecked(true);
        chooseColorButton_->setEnabled(false);
    }

    choiceLabel_->setText(buttonGroup_->checkedButton()->text());
    choiceLabel_->adjustSize();

    choiceLabel_->setObjectName("titleLabel");
    customLabel_->setObjectName("titleLabel");
    chooseColorButton_->setObjectName("chooseColorButton");

    connect(buttonGroup_, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), this,
            &CustomColorSettingCard::onRadioButtonClicked);
    connect(chooseColorButton_, &QPushButton::clicked, this,
            &CustomColorSettingCard::showColorDialog);

    qfw::setStyleSheet(this, FluentStyleSheet::ExpandSettingCard);
}

void CustomColorSettingCard::initLayout() {
    addWidget(choiceLabel_);

    radioLayout_->setSpacing(19);
    radioLayout_->setAlignment(Qt::AlignTop);
    radioLayout_->setContentsMargins(48, 18, 0, 18);
    buttonGroup_->addButton(customRadioButton_);
    buttonGroup_->addButton(defaultRadioButton_);
    radioLayout_->addWidget(customRadioButton_);
    radioLayout_->addWidget(defaultRadioButton_);
    radioLayout_->setSizeConstraint(QLayout::SetMinimumSize);

    customColorLayout_->setContentsMargins(48, 18, 44, 18);
    customColorLayout_->addWidget(customLabel_, 0, Qt::AlignLeft);
    customColorLayout_->addWidget(chooseColorButton_, 0, Qt::AlignRight);
    customColorLayout_->setSizeConstraint(QLayout::SetMinimumSize);

    viewLayout_->setSpacing(0);
    viewLayout_->setContentsMargins(0, 0, 0, 0);
    addGroupWidget(radioWidget_);
    addGroupWidget(customColorWidget_);
}

void CustomColorSettingCard::onRadioButtonClicked(QAbstractButton* button) {
    if (button->text() == choiceLabel_->text()) {
        return;
    }

    choiceLabel_->setText(button->text());
    choiceLabel_->adjustSize();

    if (button == defaultRadioButton_) {
        chooseColorButton_->setDisabled(true);
        QConfig::instance().set(*configItem_, defaultColor_);
        QConfig::instance().setThemeColor(defaultColor_);
        if (defaultColor_ != customColor_) {
            emit colorChanged(defaultColor_);
        }
    } else {
        chooseColorButton_->setDisabled(false);
        QConfig::instance().set(*configItem_, customColor_);
        QConfig::instance().setThemeColor(customColor_);
        if (defaultColor_ != customColor_) {
            emit colorChanged(customColor_);
        }
    }
}

void CustomColorSettingCard::showColorDialog() {
    ColorDialog w(QConfig::instance().get(*configItem_).value<QColor>(), tr("Choose color"),
                  window(), enableAlpha_);
    connect(&w, &ColorDialog::colorChanged, this, &CustomColorSettingCard::onCustomColorChanged);
    w.exec();
}

void CustomColorSettingCard::onCustomColorChanged(const QColor& color) {
    QConfig::instance().set(*configItem_, color);
    QConfig::instance().setThemeColor(color);
    customColor_ = color;
    emit colorChanged(color);
}

bool CustomColorSettingCard::isEnableAlpha() const { return enableAlpha_; }

void CustomColorSettingCard::setEnableAlpha(bool enable) { enableAlpha_ = enable; }

}  // namespace qfw
