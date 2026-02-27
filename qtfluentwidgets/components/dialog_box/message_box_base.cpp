#include "message_box_base.h"
#include "../../common/style_sheet.h"

namespace qfw {

MessageBoxBase::MessageBoxBase(QWidget* parent) : MaskDialogBase(parent) {
    buttonGroup = new QFrame(widget);
    yesButton = new PrimaryPushButton(tr("OK"), buttonGroup);
    cancelButton = new QPushButton(tr("Cancel"), buttonGroup);

    vBoxLayout = new QVBoxLayout(widget);
    viewLayout = new QVBoxLayout();
    buttonLayout = new QHBoxLayout(buttonGroup);

    initWidget();
}

void MessageBoxBase::initWidget() {
    setQss();
    initLayout();

    setShadowEffect(60, QPoint(0, 10), QColor(0, 0, 0, 50));
    setMaskColor(QColor(0, 0, 0, 76));

    // fixes https://github.com/zhiyiYo/PyQt-Fluent-Widgets/issues/19
    yesButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    cancelButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    
    yesButton->setAttribute(Qt::WA_MacShowFocusRect, false);

    yesButton->setFocus();
    buttonGroup->setFixedHeight(81);
    buttonGroup->setProperty("isMessageBox", true);

    connect(yesButton, &QPushButton::clicked, this, &MessageBoxBase::onYesButtonClicked);
    connect(cancelButton, &QPushButton::clicked, this, &MessageBoxBase::onCancelButtonClicked);
}

void MessageBoxBase::initLayout() {
    hBoxLayout->removeWidget(widget);
    hBoxLayout->addWidget(widget, 1, Qt::AlignCenter);

    vBoxLayout->setSpacing(0);
    vBoxLayout->setContentsMargins(0, 0, 0, 0);
    vBoxLayout->addLayout(viewLayout, 1);
    vBoxLayout->addWidget(buttonGroup, 0, Qt::AlignBottom);

    viewLayout->setSpacing(12);
    viewLayout->setContentsMargins(24, 24, 24, 24);

    buttonLayout->setSpacing(12);
    buttonLayout->setContentsMargins(24, 24, 24, 24);
    buttonLayout->addWidget(yesButton, 1, Qt::AlignVCenter);
    buttonLayout->addWidget(cancelButton, 1, Qt::AlignVCenter);
}

bool MessageBoxBase::validate() {
    return true;
}

void MessageBoxBase::onCancelButtonClicked() {
    reject();
}

void MessageBoxBase::onYesButtonClicked() {
    if (validate()) {
        accept();
    }
}

void MessageBoxBase::setQss() {
    buttonGroup->setObjectName(QStringLiteral("buttonGroup"));
    cancelButton->setObjectName(QStringLiteral("cancelButton"));
    qfw::setStyleSheet(this, FluentStyleSheet::Dialog);
}

void MessageBoxBase::hideYesButton() {
    yesButton->hide();
    buttonLayout->insertStretch(0, 1);
}

void MessageBoxBase::hideCancelButton() {
    cancelButton->hide();
    buttonLayout->insertStretch(0, 1);
}

} // namespace qfw
