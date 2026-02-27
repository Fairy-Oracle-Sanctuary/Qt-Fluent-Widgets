#include "view/status_info_interface.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>

#include "common/translator.h"
#include "components/widgets/button.h"
#include "components/widgets/info_badge.h"
#include "components/widgets/info_bar.h"
#include "components/widgets/label.h"
#include "components/widgets/progress_bar.h"
#include "components/widgets/progress_ring.h"
#include "components/widgets/spin_box.h"
#include "components/widgets/state_tool_tip.h"
#include "components/widgets/tool_tip.h"

namespace qfw {

// ==========================================================================
// ProgressWidget
// ==========================================================================

ProgressWidget::ProgressWidget(QWidget* progressWidget, QWidget* parent)
    : QWidget(parent), progressWidget_(progressWidget) {
    auto* hBoxLayout = new QHBoxLayout(this);

    auto* spinBox = new SpinBox(this);
    spinBox->setRange(0, 100);
    connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this,
            &ProgressWidget::onValueChanged);

    hBoxLayout->addWidget(progressWidget_);
    hBoxLayout->addSpacing(50);
    hBoxLayout->addWidget(new QLabel(tr("Progress"), this));
    hBoxLayout->addSpacing(5);
    hBoxLayout->addWidget(spinBox);
    hBoxLayout->setContentsMargins(0, 0, 0, 0);

    spinBox->setValue(0);
}

void ProgressWidget::onValueChanged(int value) {
    if (auto* bar = qobject_cast<ProgressBar*>(progressWidget_)) {
        bar->setValue(value);
    } else if (auto* ring = qobject_cast<ProgressRing*>(progressWidget_)) {
        ring->setValue(value);
    }
}

// ==========================================================================
// StatusInfoInterface
// ==========================================================================

StatusInfoInterface::StatusInfoInterface(QWidget* parent)
    : GalleryInterface(
          []() {
              Translator t;
              return t.statusInfo();
          }(),
          "qtfluentwidgets.components.widgets", parent) {
    setObjectName("statusInfoInterface");

    // State tool tip
    auto* stateButton = new PushButton(tr("Show StateToolTip"), this);
    connect(stateButton, &QPushButton::clicked, this, &StatusInfoInterface::onStateButtonClicked);
    addExampleCard(tr("State tool tip"), stateButton,
                   "https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/examples/"
                   "status_info/state_tool_tip/demo.py");

    // Tool tip - simple
    auto* toolTipButton = new PushButton(tr("Button with a simple ToolTip"), this);
    toolTipButton->setToolTip(tr("Simple ToolTip"));
    toolTipButton->installEventFilter(new ToolTipFilter(toolTipButton, 0, ToolTipPosition::Top));
    addExampleCard(tr("A button with a simple ToolTip"), toolTipButton,
                   "https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/examples/"
                   "status_info/tool_tip/demo.py");

    // Tool tip - label with pixmap
    auto* pixmapLabel = new ImageLabel(this);
    QPixmap pixmap(":/gallery/images/kunkun.png");
    pixmapLabel->setImage(pixmap.scaled(160, 160, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    pixmapLabel->setToolTip(tr("Label with a ToolTip"));
    pixmapLabel->setToolTipDuration(2000);
    pixmapLabel->installEventFilter(new ToolTipFilter(pixmapLabel, 0, ToolTipPosition::Top));
    pixmapLabel->setFixedSize(160, 160);
    addExampleCard(tr("A label with a ToolTip"), pixmapLabel,
                   "https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/examples/"
                   "status_info/tool_tip/demo.py");

    // Info badge
    auto* badgeWidget = new QWidget(this);
    auto* badgeLayout = new QHBoxLayout(badgeWidget);
    badgeLayout->addWidget(InfoBadge::info("1"));
    badgeLayout->addWidget(InfoBadge::success("10"));
    badgeLayout->addWidget(InfoBadge::attention("100"));
    badgeLayout->addWidget(InfoBadge::warning("1000"));
    badgeLayout->addWidget(InfoBadge::error("10000"));
    badgeLayout->addWidget(InfoBadge::custom("1w+", "#005fb8", "#60cdff"));
    badgeLayout->setSpacing(20);
    badgeLayout->setContentsMargins(0, 10, 0, 10);
    addExampleCard(tr("InfoBadge in different styles"), badgeWidget,
                   "https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/examples/"
                   "status_info/info_bar/demo.py");

    // Short info bar
    auto* infoBar1 = InfoBar::create(InfoBarIconEnum::Success, tr("Success"),
                                     tr("The Anthem of man is the Anthem of courage."),
                                     Qt::Horizontal, true, -1, InfoBarPosition::None, this);
    addExampleCard(tr("A closable InfoBar"), infoBar1,
                   "https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/examples/"
                   "status_info/info_bar/demo.py");

    // Long info bar
    QString longContent =
        tr("My name is kira yoshikake, 33 years old. Living in the villa area northeast of "
           "duwangting, unmarried. I work in Guiyou chain store. Every day I have to work overtime "
           "until 8 p.m. to go home. I don't smoke. The wine is only for a taste. Sleep at 11 p.m. "
           "for 8 hours a day. Before I go to bed, I must drink a cup of warm milk, then do 20 "
           "minutes of soft exercise, get on the bed, and immediately fall asleep. Never leave "
           "fatigue and stress until the next day. Doctors say I'm normal.");
    auto* infoBar2 = InfoBar::create(InfoBarIconEnum::Warning, tr("Warning"), longContent,
                                     Qt::Vertical, true, -1, InfoBarPosition::None, this);
    addExampleCard(tr("A closable InfoBar with long message"), infoBar2,
                   "https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/examples/"
                   "status_info/info_bar/demo.py");

    // Custom info bar
    auto* infoBar3 =
        InfoBar::create(InfoBarIconEnum::Information, tr("GitHub"),
                        tr("When you look long into an abyss, the abyss looks into you."),
                        Qt::Horizontal, true, -1, InfoBarPosition::None, this);
    infoBar3->addWidget(new PushButton(tr("Action"), this));
    infoBar3->setCustomBackgroundColor(QColor("white"), QColor("#2a2a2a"));
    addExampleCard(tr("An InfoBar with custom icon, background color and widget."), infoBar3,
                   "https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/examples/"
                   "status_info/info_bar/demo.py");

    // Different position info bar buttons
    auto* positionWidget = new QWidget(this);
    auto* positionLayout = new QHBoxLayout(positionWidget);
    auto* btn1 = new PushButton(tr("Top right"), positionWidget);
    auto* btn2 = new PushButton(tr("Top"), positionWidget);
    auto* btn3 = new PushButton(tr("Top left"), positionWidget);
    auto* btn4 = new PushButton(tr("Bottom right"), positionWidget);
    auto* btn5 = new PushButton(tr("Bottom"), positionWidget);
    auto* btn6 = new PushButton(tr("Bottom left"), positionWidget);

    connect(btn1, &QPushButton::clicked, this, &StatusInfoInterface::createTopRightInfoBar);
    connect(btn2, &QPushButton::clicked, this, &StatusInfoInterface::createTopInfoBar);
    connect(btn3, &QPushButton::clicked, this, &StatusInfoInterface::createTopLeftInfoBar);
    connect(btn4, &QPushButton::clicked, this, &StatusInfoInterface::createBottomRightInfoBar);
    connect(btn5, &QPushButton::clicked, this, &StatusInfoInterface::createBottomInfoBar);
    connect(btn6, &QPushButton::clicked, this, &StatusInfoInterface::createBottomLeftInfoBar);

    positionLayout->addWidget(btn1);
    positionLayout->addWidget(btn2);
    positionLayout->addWidget(btn3);
    positionLayout->addWidget(btn4);
    positionLayout->addWidget(btn5);
    positionLayout->addWidget(btn6);
    positionLayout->setContentsMargins(0, 0, 0, 0);
    positionLayout->setSpacing(15);
    addExampleCard(tr("InfoBar with different pop-up locations"), positionWidget,
                   "https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/examples/"
                   "status_info/info_bar/demo.py");

    // Indeterminate progress bar
    auto* indetBar = new IndeterminateProgressBar(this);
    indetBar->setFixedWidth(200);
    addExampleCard(tr("An indeterminate progress bar"), indetBar,
                   "https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/examples/"
                   "status_info/progress_bar/demo.py");

    // Determinate progress bar
    auto* detBar = new ProgressBar(this);
    detBar->setFixedWidth(200);
    addExampleCard(tr("An determinate progress bar"), new ProgressWidget(detBar, this),
                   "https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/examples/"
                   "status_info/progress_bar/demo.py");

    // Indeterminate progress ring
    auto* indetRing = new IndeterminateProgressRing(this);
    indetRing->setFixedSize(70, 70);
    addExampleCard(tr("An indeterminate progress ring"), indetRing,
                   "https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/examples/"
                   "status_info/progress_ring/demo.py");

    // Determinate progress ring
    auto* detRing = new ProgressRing(this);
    detRing->setFixedSize(80, 80);
    addExampleCard(tr("An determinate progress ring"), new ProgressWidget(detRing, this),
                   "https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/examples/"
                   "status_info/progress_ring/demo.py");
}

void StatusInfoInterface::onStateButtonClicked() {
    auto* button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    if (stateTooltip_) {
        stateTooltip_->setContent(tr("The model training is complete!") + " 😆");
        button->setText(tr("Show StateToolTip"));
        stateTooltip_->setState(true);
        stateTooltip_ = nullptr;
    } else {
        stateTooltip_ =
            new StateToolTip(tr("Training model"), tr("Please wait patiently"), window());
        button->setText(tr("Hide StateToolTip"));
        stateTooltip_->move(stateTooltip_->getSuitablePos());
        stateTooltip_->show();
    }
}

void StatusInfoInterface::createTopRightInfoBar() {
    InfoBar::info(tr("Lesson 3"), tr("Believe in the spin, just keep believing!"), Qt::Horizontal,
                  true, 2000, InfoBarPosition::TopRight, this);
}

void StatusInfoInterface::createTopInfoBar() {
    InfoBar::success(tr("Lesson 4"),
                     tr("With respect, let's advance towards a new stage of the spin."),
                     Qt::Horizontal, true, 2000, InfoBarPosition::Top, this);
}

void StatusInfoInterface::createTopLeftInfoBar() {
    InfoBar::warning(tr("Lesson 5"), tr("The shortest shortcut is to take a detour."),
                     Qt::Horizontal, false, 2000, InfoBarPosition::TopLeft, this);
}

void StatusInfoInterface::createBottomRightInfoBar() {
    InfoBar::error(tr("No Internet"), tr("An error message which won't disappear automatically."),
                   Qt::Horizontal, true, -1, InfoBarPosition::BottomRight, this);
}

void StatusInfoInterface::createBottomInfoBar() {
    InfoBar::success(tr("Lesson 1"), tr("Don't have any strange expectations of me."),
                     Qt::Horizontal, true, 2000, InfoBarPosition::Bottom, this);
}

void StatusInfoInterface::createBottomLeftInfoBar() {
    InfoBar::warning(tr("Lesson 2"), tr("Don't let your muscles notice."), Qt::Horizontal, true,
                     1500, InfoBarPosition::BottomLeft, this);
}

}  // namespace qfw
