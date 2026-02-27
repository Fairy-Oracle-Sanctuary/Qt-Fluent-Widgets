#include "view/basic_input_interface.h"

#include <QButtonGroup>
#include <QSize>
#include <QUrl>
#include <QVBoxLayout>

#include "common/translator.h"
#include "components/widgets/button.h"
#include "components/widgets/check_box.h"
#include "components/widgets/combo_box.h"
#include "components/widgets/menu.h"
#include "components/widgets/slider.h"
#include "components/widgets/switch_button.h"

namespace qfw {

BasicInputInterface::BasicInputInterface(QWidget* parent)
    : GalleryInterface(Translator().basicInput(),
                       QStringLiteral("qtfluentwidgets.components.widgets"), parent) {
    setObjectName(QStringLiteral("basicInputInterface"));

    // simple push button
    addExampleCard(tr("A simple button with text content"),
                   new PushButton(tr("Standard push button"), this),
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/basic_input/button/demo.py"));

    // tool button
    auto* toolButton = new ToolButton(QIcon(QStringLiteral(":/gallery/images/kunkun.png")), this);
    toolButton->setIconSize(QSize(40, 40));
    toolButton->resize(70, 70);
    addExampleCard(tr("A button with graphical content"), toolButton,
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/basic_input/button/demo.py"));

    // primary color push button
    addExampleCard(tr("Accent style applied to push button"),
                   new PrimaryPushButton(tr("Accent style button"), this),
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/basic_input/button/demo.py"));

    // primary color tool button
    addExampleCard(tr("Accent style applied to tool button"),
                   new PrimaryToolButton(FluentIcon(FluentIconEnum::BasketBall), this),
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/basic_input/button/demo.py"));

    // pill push button
    addExampleCard(tr("Pill push button"),
                   new PillPushButton(FluentIcon(FluentIconEnum::Tag), tr("Tag"), this),
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/master/"
                                  "examples/basic_input/button/demo.py"));

    // pill tool button
    addExampleCard(tr("Pill tool button"),
                   new PillToolButton(FluentIcon(FluentIconEnum::BasketBall), this),
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/master/"
                                  "examples/basic_input/button/demo.py"));

    // transparent push button
    addExampleCard(tr("A transparent push button"),
                   new TransparentPushButton(FluentIcon(FluentIconEnum::BookShelf),
                                             tr("Transparent push button"), this),
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/master/"
                                  "examples/basic_input/button/demo.py"));

    // transparent tool button
    addExampleCard(tr("A transparent tool button"),
                   new TransparentToolButton(FluentIcon(FluentIconEnum::BookShelf), this),
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/basic_input/button/demo.py"));

    // 2-state check box
    addExampleCard(tr("A 2-state CheckBox"), new CheckBox(tr("Two-state CheckBox"), this),
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/basic_input/check_box/demo.py"));

    // 3-state check box
    auto* checkBox = new CheckBox(tr("Three-state CheckBox"), this);
    checkBox->setTristate(true);
    addExampleCard(tr("A 3-state CheckBox"), checkBox,
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/basic_input/check_box/demo.py"));

    // combo box
    auto* comboBox = new ComboBox(this);
    comboBox->addItems({tr("shoko 🥰"), tr("西宫硝子 😊"), tr("一级棒卡哇伊的硝子酱 😘")});
    comboBox->setCurrentIndex(0);
    comboBox->setMinimumWidth(210);
    addExampleCard(tr("A ComboBox with items"), comboBox,
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/basic_input/combo_box/demo.py"));

    // editable combo box
    auto* editableComboBox = new EditableComboBox(this);
    editableComboBox->addItems({
        tr("Star Platinum"),
        tr("Crazy Diamond"),
        tr("Gold Experience"),
        tr("Sticky Fingers"),
    });
    editableComboBox->setPlaceholderText(tr("Choose your stand"));
    editableComboBox->setMinimumWidth(210);
    addExampleCard(tr("An editable ComboBox"), editableComboBox,
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/basic_input/combo_box/demo.py"));

    // email menu (shared)
    auto* emailMenu = createEmailMenu();

    // drop down push button
    auto* dropDownPushButton =
        new DropDownPushButton(FluentIcon(FluentIconEnum::Mail), tr("Email"), this);
    dropDownPushButton->setMenu(emailMenu);
    addExampleCard(tr("A push button with drop down menu"), dropDownPushButton,
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/basic_input/button/demo.py"));

    // drop down tool button
    auto* dropDownToolButton = new DropDownToolButton(FluentIcon(FluentIconEnum::Mail), this);
    dropDownToolButton->setMenu(emailMenu);
    addExampleCard(tr("A tool button with drop down menu"), dropDownToolButton,
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/basic_input/button/demo.py"));

    // primary color drop down push button
    auto* primaryDropDownPushButton =
        new PrimaryDropDownPushButton(FluentIcon(FluentIconEnum::Mail), tr("Email"), this);
    primaryDropDownPushButton->setMenu(emailMenu);
    addExampleCard(tr("A primary color push button with drop down menu"), primaryDropDownPushButton,
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/basic_input/button/demo.py"));

    // transparent drop down push button
    auto* transparentDropDownPushButton =
        new TransparentDropDownPushButton(FluentIcon(FluentIconEnum::Mail), tr("Email"), this);
    transparentDropDownPushButton->setMenu(emailMenu);
    addExampleCard(tr("A transparent push button with drop down menu"),
                   transparentDropDownPushButton,
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/master/"
                                  "examples/basic_input/button/demo.py"));

    // transparent drop down tool button
    auto* transparentDropDownToolButton =
        new TransparentDropDownToolButton(FluentIcon(FluentIconEnum::Mail), this);
    transparentDropDownToolButton->setMenu(emailMenu);
    addExampleCard(tr("A transparent tool button with drop down menu"),
                   transparentDropDownToolButton,
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/master/"
                                  "examples/basic_input/button/demo.py"));

    // hyperlink button
    addExampleCard(tr("A hyperlink button that navigates to a URI"),
                   new HyperlinkButton(QStringLiteral("https://qfluentwidgets.com"), tr("GitHub"),
                                       this, QVariant::fromValue(FluentIconEnum::Link)),
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/master/"
                                  "examples/basic_input/button/demo.py"));

    // radio button
    auto* radioWidget = new QWidget(this);
    auto* radioLayout = new QVBoxLayout(radioWidget);
    radioLayout->setContentsMargins(2, 0, 0, 0);
    radioLayout->setSpacing(15);

    auto* radioButton1 = new RadioButton(tr("Star Platinum"), radioWidget);
    auto* radioButton2 = new RadioButton(tr("Crazy Diamond"), radioWidget);
    auto* radioButton3 = new RadioButton(tr("Soft and Wet"), radioWidget);

    auto* buttonGroup = new QButtonGroup(radioWidget);
    buttonGroup->addButton(radioButton1);
    buttonGroup->addButton(radioButton2);
    buttonGroup->addButton(radioButton3);

    radioLayout->addWidget(radioButton1);
    radioLayout->addWidget(radioButton2);
    radioLayout->addWidget(radioButton3);
    radioButton1->click();

    addExampleCard(tr("A group of RadioButton controls in a button group"), radioWidget,
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/basic_input/radio_button/demo.py"));

    // horizontal slider
    auto* slider = new Slider(Qt::Horizontal, this);
    slider->setRange(0, 100);
    slider->setValue(30);
    slider->setMinimumWidth(200);
    addExampleCard(tr("A simple horizontal slider"), slider,
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/basic_input/slider/demo.py"));

    // ikun menu (shared)
    auto* ikunMenu = createIkunMenu();

    // split push button
    auto* splitPushButton = new SplitPushButton(tr("Choose your stand"), this,
                                                QVariant::fromValue(FluentIconEnum::BasketBall));
    splitPushButton->setFlyout(createStandMenu(splitPushButton));
    addExampleCard(tr("A split push button with drop down menu"), splitPushButton,
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/basic_input/button/demo.py"));

    // split tool button
    auto* splitToolButton =
        new SplitToolButton(QVariant(QStringLiteral(":/gallery/images/kunkun.png")), this);
    splitToolButton->setIconSize(QSize(30, 30));
    splitToolButton->setFlyout(ikunMenu);
    addExampleCard(tr("A split tool button with drop down menu"), splitToolButton,
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/basic_input/button/demo.py"));

    // primary color split push button
    auto* primarySplitPushButton = new PrimarySplitPushButton(
        tr("Choose your stand"), this, QVariant::fromValue(FluentIconEnum::BasketBall));
    primarySplitPushButton->setFlyout(createStandMenu(primarySplitPushButton));
    addExampleCard(tr("A primary color split push button with drop down menu"),
                   primarySplitPushButton,
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/basic_input/button/demo.py"));

    // primary color split tool button
    auto* primarySplitToolButton =
        new PrimarySplitToolButton(FluentIcon(FluentIconEnum::BasketBall), this);
    primarySplitToolButton->setFlyout(ikunMenu);
    addExampleCard(tr("A primary color split tool button with drop down menu"),
                   primarySplitToolButton,
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/basic_input/button/demo.py"));

    // switch button
    switchButton_ = new SwitchButton(tr("Off"), this);
    connect(switchButton_, &SwitchButton::checkedChanged, this,
            &BasicInputInterface::onSwitchCheckedChanged);
    addExampleCard(tr("A simple switch button"), switchButton_,
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/basic_input/switch_button/demo.py"));

    // toggle button
    addExampleCard(
        tr("A simple toggle push button"),
        new ToggleButton(FluentIcon(FluentIconEnum::BasketBall), tr("Start practicing"), this),
        QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/examples/"
                       "basic_input/button/demo.py"));

    // toggle tool button
    addExampleCard(
        tr("A simple toggle tool button"),
        new ToggleToolButton(FluentIcon(FluentIconEnum::BasketBall), this),
        QStringLiteral(
            "https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/master/examples/button/demo.py"));

    // transparent toggle push button
    addExampleCard(tr("A transparent toggle push button"),
                   new TransparentTogglePushButton(FluentIcon(FluentIconEnum::BasketBall),
                                                   tr("Start practicing"), this),
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/master/"
                                  "examples/basic_input/button/demo.py"));

    // transparent toggle tool button
    addExampleCard(tr("A transparent toggle tool button"),
                   new TransparentToggleToolButton(FluentIcon(FluentIconEnum::BasketBall), this),
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/master/"
                                  "examples/basic_input/button/demo.py"));
}

void BasicInputInterface::onSwitchCheckedChanged(bool isChecked) {
    if (isChecked) {
        switchButton_->setText(tr("On"));
    } else {
        switchButton_->setText(tr("Off"));
    }
}

RoundMenu* BasicInputInterface::createStandMenu(QWidget* button) {
    auto* menu = new RoundMenu(QString(), this);
    auto* act1 = new QAction(tr("Star Platinum"), this);
    auto* act2 = new QAction(tr("Crazy Diamond"), this);
    auto* act3 = new QAction(tr("Gold Experience"), this);
    auto* act4 = new QAction(tr("Sticky Fingers"), this);
    menu->addActions({act1, act2, act3, act4});

    // Connect actions to update button text
    auto* pushButton = qobject_cast<PushButton*>(button);
    if (pushButton) {
        connect(act1, &QAction::triggered, pushButton,
                [pushButton, this]() { pushButton->setText(tr("Star Platinum")); });
        connect(act2, &QAction::triggered, pushButton,
                [pushButton, this]() { pushButton->setText(tr("Crazy Diamond")); });
        connect(act3, &QAction::triggered, pushButton,
                [pushButton, this]() { pushButton->setText(tr("Gold Experience")); });
        connect(act4, &QAction::triggered, pushButton,
                [pushButton, this]() { pushButton->setText(tr("Sticky Fingers")); });
    }

    return menu;
}

RoundMenu* BasicInputInterface::createEmailMenu() {
    auto* menu = new RoundMenu(QString(), this);
    auto* act1 = new QAction(tr("Send"), this);
    auto* act2 = new QAction(tr("Save"), this);
    menu->addActions({act1, act2});
    return menu;
}

RoundMenu* BasicInputInterface::createIkunMenu() {
    auto* menu = new RoundMenu(QString(), this);
    auto* act1 = new QAction(tr("Sing"), this);
    auto* act2 = new QAction(tr("Jump"), this);
    auto* act3 = new QAction(tr("Rap"), this);
    auto* act4 = new QAction(tr("Music"), this);
    menu->addActions({act1, act2, act3, act4});
    return menu;
}

}  // namespace qfw
