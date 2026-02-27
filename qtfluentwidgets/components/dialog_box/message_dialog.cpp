#include "message_dialog.h"
#include "../../common/auto_wrap.h"
#include "../../common/style_sheet.h"

namespace qfw {

MessageDialog::MessageDialog(const QString& title, const QString& content, QWidget* parent)
    : MaskDialogBase(parent), content(content) {
    titleLabel = new QLabel(title, widget);
    contentLabel = new QLabel(content, widget);
    yesButton = new QPushButton(tr("OK"), widget);
    cancelButton = new QPushButton(tr("Cancel"), widget);

    initWidget();
}

void MessageDialog::initWidget() {
    windowMask->resize(size());
    widget->setMaximumWidth(540);
    titleLabel->move(24, 24);
    contentLabel->move(24, 56);
    contentLabel->setText(TextWrap::wrap(content, 71).first);

    setQss();
    initLayout();

    connect(yesButton, &QPushButton::clicked, this, &MessageDialog::onYesButtonClicked);
    connect(cancelButton, &QPushButton::clicked, this, &MessageDialog::onCancelButtonClicked);
}

void MessageDialog::initLayout() {
    contentLabel->adjustSize();
    widget->setFixedSize(48 + contentLabel->width(),
                         contentLabel->y() + contentLabel->height() + 92);
    
    int btnWidth = (widget->width() - 54) / 2;
    yesButton->resize(btnWidth, 32);
    cancelButton->resize(btnWidth, 32);
    
    yesButton->move(24, widget->height() - 56);
    cancelButton->move(widget->width() - 24 - cancelButton->width(), widget->height() - 56);
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

void MessageDialog::setQss() {
    windowMask->setObjectName(QStringLiteral("windowMask"));
    titleLabel->setObjectName(QStringLiteral("titleLabel"));
    contentLabel->setObjectName(QStringLiteral("contentLabel"));
    qfw::setStyleSheet(this, FluentStyleSheet::MessageDialog);
}

} // namespace qfw
