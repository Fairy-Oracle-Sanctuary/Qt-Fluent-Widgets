#include "components/dialog_box/dialog.h"

#include <QApplication>

#include "common/auto_wrap.h"
#include "common/style_sheet.h"

namespace qfw {

MessageBoxUI::MessageBoxUI(QObject* parent) : QObject(parent) {}

void MessageBoxUI::setUpUi(const QString& title, const QString& content, QWidget* parentWidget) {
    this->content = content;
    titleLabel = new QLabel(title, parentWidget);
    contentLabel = new BodyLabel(content, parentWidget);

    buttonGroup = new QFrame(parentWidget);
    buttonGroup->setObjectName(QStringLiteral("buttonGroup"));
    if (qobject_cast<MaskDialogBase*>(parentWidget->parentWidget()) ||
        qobject_cast<MaskDialogBase*>(parentWidget)) {
        buttonGroup->setProperty("isMessageBox", true);
    }

    yesButton = new PrimaryPushButton(tr("OK"), buttonGroup);
    cancelButton = new QPushButton(tr("Cancel"), buttonGroup);

    vBoxLayout = new QVBoxLayout(parentWidget);
    textLayout = new QVBoxLayout();
    buttonLayout = new QHBoxLayout(buttonGroup);

    initWidget(parentWidget);
}

void MessageBoxUI::initWidget(QWidget* parentWidget) {
    setQss(parentWidget);
    initLayout();

    yesButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    cancelButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);

    yesButton->setFocus();
    buttonGroup->setFixedHeight(81);

    contentLabel->setContextMenuPolicy(Qt::CustomContextMenu);
    adjustText(parentWidget);

    connect(yesButton, &QPushButton::clicked, this, &MessageBoxUI::onYesButtonClicked);
    connect(cancelButton, &QPushButton::clicked, this, &MessageBoxUI::onCancelButtonClicked);
}

void MessageBoxUI::adjustText(QWidget* widget) {
    int w = 0;
    int chars = 0;

    if (widget->isWindow()) {
        if (widget->parentWidget()) {
            w = qMax(titleLabel->width(), widget->parentWidget()->width());
            chars = qMax(qMin(w / 9, 140), 30);
        } else {
            chars = 100;
        }
    } else {
        w = qMax(titleLabel->width(), widget->window()->width());
        chars = qMax(qMin(w / 9, 100), 30);
    }

    contentLabel->setText(TextWrap::wrap(content, chars, false).first);
}

void MessageBoxUI::initLayout() {
    vBoxLayout->setSpacing(0);
    vBoxLayout->setContentsMargins(0, 0, 0, 0);
    vBoxLayout->addLayout(textLayout, 1);
    vBoxLayout->addWidget(buttonGroup, 0, Qt::AlignBottom);
    vBoxLayout->setSizeConstraint(QLayout::SetMinimumSize);

    textLayout->setSpacing(12);
    textLayout->setContentsMargins(24, 24, 24, 24);
    textLayout->addWidget(titleLabel, 0, Qt::AlignTop);
    textLayout->addWidget(contentLabel, 0, Qt::AlignTop);

    buttonLayout->setSpacing(12);
    buttonLayout->setContentsMargins(24, 24, 24, 24);
    buttonLayout->addWidget(yesButton, 1, Qt::AlignVCenter);
    buttonLayout->addWidget(cancelButton, 1, Qt::AlignVCenter);
}

void MessageBoxUI::onCancelButtonClicked() {
    auto* dialog = qobject_cast<QDialog*>(parent());
    if (dialog) {
        dialog->reject();
    }
    emit cancelSignal();
}

void MessageBoxUI::onYesButtonClicked() {
    auto* dialog = qobject_cast<QDialog*>(parent());
    if (dialog) {
        dialog->accept();
    }
    emit yesSignal();
}

void MessageBoxUI::setQss(QWidget* widget) {
    titleLabel->setObjectName(QStringLiteral("titleLabel"));
    contentLabel->setObjectName(QStringLiteral("contentLabel"));
    cancelButton->setObjectName(QStringLiteral("cancelButton"));

    qfw::setStyleSheet(widget, FluentStyleSheet::Dialog);
    qfw::setStyleSheet(contentLabel, FluentStyleSheet::Dialog);

    yesButton->adjustSize();
    cancelButton->adjustSize();
}

void MessageBoxUI::setContentCopyable(bool isCopyable) {
    if (isCopyable) {
        contentLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    } else {
        contentLabel->setTextInteractionFlags(Qt::NoTextInteraction);
    }
}

void MessageBoxUI::hideYesButton() {
    yesButton->hide();
    buttonLayout->insertStretch(0, 1);
}

void MessageBoxUI::hideCancelButton() {
    cancelButton->hide();
    buttonLayout->insertStretch(0, 1);
}

// ============================================================================
// Dialog
// ============================================================================

Dialog::Dialog(const QString& title, const QString& content, QWidget* parent)
    : FramelessDialog(parent) {
    ui_ = new MessageBoxUI(this);
    ui_->setUpUi(title, content, this);

    connect(ui_, &MessageBoxUI::yesSignal, this, &Dialog::yesSignal);
    connect(ui_, &MessageBoxUI::cancelSignal, this, &Dialog::cancelSignal);

    windowTitleLabel = new QLabel(title, this);
    windowTitleLabel->setObjectName(QStringLiteral("windowTitleLabel"));

#ifdef Q_OS_WIN
    setResizeEnabled(false);
#endif
    resize(240, 192);

    ui_->vBoxLayout->insertWidget(0, windowTitleLabel, 0, Qt::AlignTop);

    qfw::setStyleSheet(this, FluentStyleSheet::Dialog);
    setFixedSize(size());
}

void Dialog::setTitleBarVisible(bool isVisible) { windowTitleLabel->setVisible(isVisible); }

void Dialog::setContentCopyable(bool isCopyable) { ui_->setContentCopyable(isCopyable); }

// ============================================================================
// MessageBox
// ============================================================================

MessageBox::MessageBox(const QString& title, const QString& content, QWidget* parent)
    : MaskDialogBase(parent) {
    ui_ = new MessageBoxUI(this);
    ui_->setUpUi(title, content, widget);

    connect(ui_, &MessageBoxUI::yesSignal, this, &MessageBox::yesSignal);
    connect(ui_, &MessageBoxUI::cancelSignal, this, &MessageBox::cancelSignal);

    setShadowEffect(60, QPoint(0, 10), QColor(0, 0, 0, 50));
    setMaskColor(QColor(0, 0, 0, 76));

    hBoxLayout->removeWidget(widget);
    hBoxLayout->addWidget(widget, 1, Qt::AlignCenter);

    ui_->buttonGroup->setMinimumWidth(280);
    ui_->buttonGroup->setProperty("isMessageBox", true);
    widget->setFixedSize(qMax(ui_->contentLabel->width(), ui_->titleLabel->width()) + 48,
                         ui_->contentLabel->y() + ui_->contentLabel->height() + 105);
}

void MessageBox::setContentCopyable(bool isCopyable) { ui_->setContentCopyable(isCopyable); }

bool MessageBox::eventFilter(QObject* obj, QEvent* e) {
    if (obj == window()) {
        if (e->type() == QEvent::Resize) {
            ui_->adjustText(this);
        }
    }
    return MaskDialogBase::eventFilter(obj, e);
}

}  // namespace qfw
