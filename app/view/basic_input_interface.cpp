#include "view/basic_input_interface.h"
#include "view/pro_input_examples.h"
#include "view/pro_picker_examples.h"

#include <QButtonGroup>
#include <QSize>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>

#include "common/translator.h"
#include "components/layout/flow_layout.h"
#include "components/widgets/button.h"
#include "components/widgets/extended_button.h"
#include "components/widgets/progress_button.h"
#include "components/widgets/chip.h"
#include "components/widgets/sub_clip.h"
#include "components/widgets/tag.h"
#include "components/widgets/label.h"
#include "components/widgets/flyout.h"
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
    ProInputExamples::addBasicInput(this);
    ProPickerExamples::addBasicInput(this);
    auto* primaryTools = new QWidget(this);
    auto* primaryToolsLayout = new QVBoxLayout(primaryTools);
    primaryToolsLayout->setContentsMargins(0, 0, 0, 0);
    auto* primaryTool = new PrimaryDropDownToolButton(FluentIcon(FluentIconEnum::Add), primaryTools);
    primaryTool->setAccessibleName(tr("Add item"));
    primaryTool->setToolTip(tr("Add item"));
    auto* primaryMenu = new RoundMenu(QString(), primaryTool);
    auto* primaryState = new BodyLabel(tr("Use click, Space, Enter or Down to open the menu."), primaryTools);
    primaryState->setWordWrap(true);
    for (const QString& value : QStringList{tr("Design"), tr("Development"), tr("Documentation")}) {
        auto* action = new QAction(value, primaryMenu);
        primaryMenu->addAction(action);
        connect(action, &QAction::triggered, primaryState, [primaryState, value, this] {
            primaryState->setText(tr("Selected item: %1").arg(value));
        });
    }
    primaryTool->setMenu(primaryMenu);
    primaryToolsLayout->addWidget(primaryTool, 0, Qt::AlignLeft);
    primaryToolsLayout->addWidget(primaryState);
    addExampleCard(tr("Primary drop-down tool button"), primaryTools,
        QStringLiteral("https://github.com/Fairy-Oracle-Sanctuary/Qt-Fluent-Widgets"));
    auto* customSplit = new PrimarySplitPushButton(tr("Preview"),this);
    customSplit->setDropIcon(FluentIcon(FluentIconEnum::Info));
    customSplit->setDropIconSize(QSize(16,16));
    auto* preview = new Flyout(new FlyoutView(tr("Widget flyout"),tr("A split button can open a Fluent widget flyout.")),this,false);
    customSplit->setFlyout(preview,[preview](const QPoint& pos) { preview->exec(pos,FlyoutAnimationType::DropDown); });
    addExampleCard(tr("Split button with a custom drop icon and widget flyout"),customSplit,
        QStringLiteral("https://github.com/Fairy-Oracle-Sanctuary/Qt-Fluent-Widgets"));

    auto* clipHost = new QWidget(this);
    auto* clipLayout = new QVBoxLayout(clipHost);
    clipLayout->setContentsMargins(0, 0, 0, 0);
    clipLayout->setSpacing(12);
    auto* clipRow = new QWidget(clipHost);
    auto* clips = new FlowLayout(clipRow, false, true);
    clips->setContentsMargins(0, 0, 0, 0);
    clips->setHorizontalSpacing(8);
    clips->setVerticalSpacing(8);
    auto* clipState = new BodyLabel(tr("Close a token to remove it; restore brings it back."), clipHost);
    clipState->setWordWrap(true);
    QList<SubClip*> tokens;
    for (const auto& text : QStringList{tr("Design"), tr("Development"), tr("Documentation")}) {
        auto* token = new SubClip(text, clipRow);
        clips->addWidget(token);
        tokens.append(token);
        connect(token, &SubClip::closed, clipState, [token, clipState, this](const QString& value) {
            token->hide();
            clipState->setText(tr("Removed token: %1").arg(value));
        });
    }
    auto* disabledToken = new SubClip(tr("Disabled token"), clipRow);
    disabledToken->setEnabled(false);
    clips->addWidget(disabledToken);
    auto* restore = new PushButton(tr("Restore tokens"), clipHost);
    connect(restore, &QPushButton::clicked, clipHost, [tokens, clipState, this] {
        for (auto* token : tokens) token->show();
        clipState->setText(tr("Close a token to remove it; restore brings it back."));
    });
    clipLayout->addWidget(clipRow);
    clipLayout->addWidget(clipState);
    clipLayout->addWidget(restore, 0, Qt::AlignLeft);
    addExampleCard(tr("Compact removable selection tokens"), clipHost,
                   QStringLiteral("https://github.com/Fairy-Oracle-Sanctuary/PySide6-Fluent-Widgets-Pro"));

    const QString extendedSource = QStringLiteral(
        "https://github.com/Fairy-Oracle-Sanctuary/Qt-Fluent-Widgets/blob/main/"
        "app/view/basic_input_interface.cpp");
    const auto makeButtonRow = [this]() {
        auto* host = new QWidget(this);
        host->setMaximumWidth(760);
        auto* layout = new FlowLayout(host);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setHorizontalSpacing(12);
        layout->setVerticalSpacing(12);
        return host;
    };
    const auto addButton = [](QWidget* host, QAbstractButton* button) {
        button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        static_cast<FlowLayout*>(host->layout())->addWidget(button);
    };

    auto* filledRow = makeButtonRow();
    auto* filled = new FilledPushButton(FluentIcon(FluentIconEnum::Add), tr("Filled button"), filledRow);
    auto* filledTool = new FilledToolButton(FluentIcon(FluentIconEnum::Add), filledRow);
    filledTool->setToolTip(tr("Filled tool button"));
    auto* textButton = new TextPushButton(FluentIcon(FluentIconEnum::Add), tr("Text button"), filledRow);
    auto* textTool = new TextToolButton(FluentIcon(FluentIconEnum::Add), filledRow);
    textTool->setToolTip(tr("Text tool button"));
    auto* disabledFilled = new FilledPushButton(tr("Disabled"), filledRow);
    disabledFilled->setEnabled(false);
    auto* scheme = new ComboBox(filledRow);
    scheme->addItems({tr("Information"), tr("Success"), tr("Attention"), tr("Warning"), tr("Error")});
    scheme->setCurrentIndex(2);
    scheme->setFixedWidth(150);
    scheme->setToolTip(tr("Color scheme"));
    addButton(filledRow, filled);
    addButton(filledRow, filledTool);
    addButton(filledRow, textButton);
    addButton(filledRow, textTool);
    addButton(filledRow, disabledFilled);
    static_cast<FlowLayout*>(filledRow->layout())->addWidget(scheme);
    connect(scheme, &ComboBox::currentIndexChanged, this,
            [filled, filledTool, textButton, textTool](int index) {
                const QStringList schemes = {QStringLiteral("information"), QStringLiteral("success"),
                    QStringLiteral("attention"), QStringLiteral("warning"), QStringLiteral("error")};
                if (index >= 0 && index < schemes.size()) {
                    filled->setColorScheme(schemes.at(index));
                    filledTool->setColorScheme(schemes.at(index));
                    textButton->setColorScheme(schemes.at(index));
                    textTool->setColorScheme(schemes.at(index));
                }
            });
    addExampleCard(tr("Filled and text buttons with five color schemes"), filledRow, extendedSource, 1);

    auto* roundRow = makeButtonRow();
    auto* outlined = new OutlinedPushButton(FluentIcon(FluentIconEnum::Add), tr("Outlined button"), roundRow);
    auto* outlinedTool = new OutlinedToolButton(FluentIcon(FluentIconEnum::Add), roundRow);
    outlinedTool->setToolTip(tr("Outlined tool button"));
    addButton(roundRow, outlined);
    addButton(roundRow, outlinedTool);
    addButton(roundRow, new RoundPushButton(FluentIcon(FluentIconEnum::Add), tr("Round button"), roundRow));
    auto* roundTool = new RoundToolButton(FluentIcon(FluentIconEnum::Add), roundRow);
    roundTool->setToolTip(tr("Round tool button"));
    addButton(roundRow, roundTool);
    auto* borderless = new RoundToolButton(FluentIcon(FluentIconEnum::Add), roundRow);
    borderless->setHasBorder(false);
    borderless->setToolTip(tr("Round tool button without a resting border"));
    addButton(roundRow, borderless);
    auto* link = new HyperlinkToolButton(FluentIcon(FluentIconEnum::Link),
        QStringLiteral("https://github.com/Fairy-Oracle-Sanctuary/Qt-Fluent-Widgets"), roundRow);
    link->setToolTip(tr("View source on GitHub"));
    addButton(roundRow, link);
    addExampleCard(tr("Outlined, round and hyperlink buttons"), roundRow, extendedSource, 1);

    // The glow is drawn outside the button: reserve its maximum 45px extent.
    auto* luminaHost = new QWidget(this);
    auto* luminaLayout = new QVBoxLayout(luminaHost);
    luminaLayout->setContentsMargins(48, 48, 48, 48);
    luminaLayout->addWidget(new LuminaPushButton(tr("Lumina button"), luminaHost), 0, Qt::AlignLeft);
    addExampleCard(tr("Lumina button with an animated glow"), luminaHost, extendedSource);

    auto* progressHost = new QWidget(this);
    progressHost->setMaximumWidth(500);
    auto* progressLayout = new QVBoxLayout(progressHost);
    progressLayout->setContentsMargins(0, 0, 0, 0);
    progressLayout->setSpacing(12);
    auto* indeterminate = new IndeterminateProgressPushButton(tr("Download"), progressHost);
    indeterminate->setFixedWidth(180);
    auto* spinnerControl = new PushButton(tr("Stop animation"), progressHost);
    connect(spinnerControl, &QAbstractButton::clicked, this, [this, indeterminate, spinnerControl] {
        if (indeterminate->isSpinning()) {
            indeterminate->stop();
            spinnerControl->setText(tr("Start animation"));
        } else {
            indeterminate->start();
            spinnerControl->setText(tr("Stop animation"));
        }
    });
    auto* spinnerRow = new QHBoxLayout;
    spinnerRow->addWidget(indeterminate);
    spinnerRow->addWidget(spinnerControl);
    spinnerRow->addStretch();
    progressLayout->addLayout(spinnerRow);
    auto* progress = new ProgressPushButton(tr("Download"), progressHost);
    progress->setFixedWidth(180);
    progress->setAutoProgressEnabled(true);
    progress->setToolTip(tr("Click to start; click again to stop"));
    auto* progressStatus = new BodyLabel(tr("Ready"), progressHost);
    auto* progressRow = new QHBoxLayout;
    progressRow->addWidget(progress);
    progressRow->addWidget(progressStatus);
    progressRow->addStretch();
    progressLayout->addLayout(progressRow);
    auto* progressTimer = new QTimer(progress);
    progressTimer->setInterval(50);
    connect(progressTimer, &QTimer::timeout, progress, [progress] {
        progress->setValue(progress->value() + 1);
        if (progress->value() == 100) { progress->setProgressing(false); }
    });
    connect(progress, &ProgressPushButton::progressChanged, this,
            [this, progress, progressTimer, progressStatus](bool active) {
        if (active) {
            progress->setValue(0);
            progressStatus->setText(tr("Downloading: %1%").arg(0));
            progressTimer->start();
        } else {
            progressTimer->stop();
            progressStatus->setText(progress->value() == 100 ? tr("Completed")
                : tr("Stopped at %1%").arg(progress->value()));
        }
    });
    connect(progress, &ProgressPushButton::valueChanged, this, [this, progressStatus](int value) {
        progressStatus->setText(tr("Downloading: %1%").arg(value));
    });
    connect(progress, &ProgressPushButton::stopRequested, progress, [progress] {
        progress->setProgressing(false);
    });
    addExampleCard(tr("Determinate and indeterminate progress buttons"), progressHost, extendedSource);

    auto* chipRow = makeButtonRow();
    auto* chip = new Chip(FluentIcon(FluentIconEnum::Tag), tr("Selectable chip"), chipRow);
    auto* selectedChip = new Chip(tr("Selected chip"), chipRow);
    selectedChip->setChecked(true);
    auto* disabledChip = new Chip(tr("Disabled"), chipRow);
    disabledChip->setEnabled(false);
    auto* restoreChips = new PushButton(tr("Restore chips"), chipRow);
    for (Chip* token : {chip, selectedChip}) {
        connect(token, &Chip::closed, token, &QWidget::hide);
    }
    connect(restoreChips, &QAbstractButton::clicked, this, [chip, selectedChip] {
        chip->setChecked(false);
        selectedChip->setChecked(true);
        chip->show();
        selectedChip->show();
    });
    addButton(chipRow, chip);
    addButton(chipRow, selectedChip);
    addButton(chipRow, disabledChip);
    addButton(chipRow, restoreChips);
    addExampleCard(tr("Selectable chips with independent close buttons"), chipRow, extendedSource, 1);

    auto* tagRow = makeButtonRow();
    const QStringList tagTypes = {QStringLiteral("information"), QStringLiteral("success"),
        QStringLiteral("warning"), QStringLiteral("error"), QStringLiteral("progress")};
    const QStringList tagLabels = {tr("Information"), tr("Success"), tr("Warning"), tr("Error"), tr("In progress")};
    for (int i = 0; i < tagTypes.size(); ++i) {
        auto* tag = new Tag(FluentIcon(FluentIconEnum::Tag), tagLabels.at(i), tagRow);
        tag->setType(tagTypes.at(i));
        tag->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        static_cast<FlowLayout*>(tagRow->layout())->addWidget(tag);
    }
    addExampleCard(tr("Decorative status tags with five types"), tagRow, extendedSource, 1);

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
