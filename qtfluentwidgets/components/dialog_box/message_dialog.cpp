#include "message_dialog.h"
#include "../../common/style_sheet.h"
#include "../widgets/button.h"

#include <QVBoxLayout>

namespace qfw {

MessageDialog::MessageDialog(const QString& title, const QString& content, QWidget* parent)
    : MaskDialogBase(parent) {
    auto* buttonGroup = new QFrame(widget);
    buttonGroup->setObjectName(QStringLiteral("buttonGroup"));
    buttonGroup->setProperty("isMessageBox", true);
    buttonGroup->setFixedHeight(81);

    titleLabel = new QLabel(title, widget);
    titleLabel->setObjectName(QStringLiteral("titleLabel"));
    contentLabel = new QLabel(content, widget);
    contentLabel->setObjectName(QStringLiteral("contentLabel"));
    contentLabel->setWordWrap(true);
    yesButton = new PushButton(tr("OK"), buttonGroup);
    cancelButton = new PushButton(tr("Cancel"), buttonGroup);
    cancelButton->setObjectName(QStringLiteral("cancelButton"));

    auto* layout = new QVBoxLayout(widget);
    auto* viewLayout = new QVBoxLayout;
    auto* buttonLayout = new QHBoxLayout(buttonGroup);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addLayout(viewLayout, 1);
    layout->addWidget(buttonGroup);
    viewLayout->setContentsMargins(24, 24, 24, 24);
    viewLayout->setSpacing(12);
    viewLayout->addWidget(titleLabel);
    viewLayout->addWidget(contentLabel);
    viewLayout->addStretch();
    buttonLayout->setSpacing(12);
    buttonLayout->setContentsMargins(24, 24, 24, 24);
    buttonLayout->addWidget(yesButton, 1, Qt::AlignVCenter);
    buttonLayout->addWidget(cancelButton, 1, Qt::AlignVCenter);
    yesButton->setMinimumWidth(135);
    cancelButton->setMinimumWidth(135);
    yesButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    cancelButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);

    qfw::setStyleSheet(this, FluentStyleSheet::Dialog);
    widget->setFixedWidth(qBound(330, contentLabel->fontMetrics().horizontalAdvance(content) + 48, 480));
    widget->setMinimumHeight(210);
    hBoxLayout->removeWidget(widget);
    hBoxLayout->addWidget(widget, 0, Qt::AlignCenter);
    setShadowEffect(60, QPoint(0, 10), QColor(0, 0, 0, 50));
    setMaskColor(QColor(0, 0, 0, 76));
    yesButton->setFocus();

    connect(yesButton, &QPushButton::clicked, this, &MessageDialog::onYesButtonClicked);
    connect(cancelButton, &QPushButton::clicked, this, &MessageDialog::onCancelButtonClicked);
}

void MessageDialog::onCancelButtonClicked() {
    emit cancelSignal();
    reject();
}

void MessageDialog::onYesButtonClicked() {
    setEnabled(false);
    emit yesSignal();
    accept();
}

} // namespace qfw
