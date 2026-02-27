#include "view/dialog_interface.h"

#include <QColor>
#include <QUrl>

#include "common/translator.h"
#include "components/dialog_box/color_dialog.h"
#include "components/dialog_box/dialog.h"
#include "components/dialog_box/message_box_base.h"
#include "components/widgets/button.h"
#include "components/widgets/flyout.h"
#include "components/widgets/label.h"
#include "components/widgets/line_edit.h"
#include "components/widgets/teaching_tip.h"

namespace qfw {

DialogInterface::DialogInterface(QWidget* parent)
    : GalleryInterface(Translator().dialogs(),
                       QStringLiteral("qtfluentwidgets.components.dialog_box"), parent) {
    setObjectName(QStringLiteral("dialogInterface"));

    // Frameless message box
    auto* button = new PushButton(tr("Show dialog"), this);
    connect(button, &PushButton::clicked, this, &DialogInterface::showDialog);
    addExampleCard(tr("A frameless message box"), button,
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/dialog_flyout/dialog/demo.py"));

    // Message box with mask
    button = new PushButton(tr("Show dialog"), this);
    connect(button, &PushButton::clicked, this, &DialogInterface::showMessageDialog);
    addExampleCard(tr("A message box with mask"), button,
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/dialog_flyout/message_dialog/demo.py"));

    // Custom message box
    button = new PushButton(tr("Show dialog"), this);
    connect(button, &PushButton::clicked, this, &DialogInterface::showCustomDialog);
    addExampleCard(tr("A custom message box"), button,
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/dialog_flyout/custom_message_box/demo.py"));

    // Color dialog
    button = new PushButton(tr("Show dialog"), this);
    connect(button, &PushButton::clicked, this, &DialogInterface::showColorDialog);
    addExampleCard(tr("A color dialog"), button,
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/dialog_flyout/color_dialog/demo.py"));

    // Simple flyout
    simpleFlyoutButton_ = new PushButton(tr("Show flyout"), this);
    connect(simpleFlyoutButton_, &PushButton::clicked, this, &DialogInterface::showSimpleFlyout);
    addExampleCard(tr("A simple flyout"), simpleFlyoutButton_,
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/dialog_flyout/flyout/demo.py"));

    // Complex flyout
    complexFlyoutButton_ = new PushButton(tr("Show flyout"), this);
    connect(complexFlyoutButton_, &PushButton::clicked, this, &DialogInterface::showComplexFlyout);
    addExampleCard(tr("A flyout with image and button"), complexFlyoutButton_,
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/dialog_flyout/flyout/demo.py"));

    // Teaching tip
    teachingButton_ = new PushButton(tr("Show teaching tip"), this);
    connect(teachingButton_, &PushButton::clicked, this, &DialogInterface::showBottomTeachingTip);
    addExampleCard(tr("A teaching tip"), teachingButton_,
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/dialog_flyout/teaching_tip/demo.py"));

    // Teaching tip with image and button
    teachingRightButton_ = new PushButton(tr("Show teaching tip"), this);
    connect(teachingRightButton_, &PushButton::clicked, this,
            &DialogInterface::showLeftBottomTeachingTip);
    addExampleCard(tr("A teaching tip with image and button"), teachingRightButton_,
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/dialog_flyout/teaching_tip/demo.py"));
}

void DialogInterface::showDialog() {
    QString title = tr("This is a frameless message dialog");
    QString content =
        tr("If the content of the message box is veeeeeeeeeeeeeeeeeeeeeeeeeery long, "
           "it will automatically wrap like this.");
    auto* w = new Dialog(title, content, window());
    w->ui()->setContentCopyable(true);
    if (w->exec()) {
        qDebug("Yes button is pressed");
    } else {
        qDebug("Cancel button is pressed");
    }
}

void DialogInterface::showMessageDialog() {
    QString title = tr("This is a message dialog with mask");
    QString content =
        tr("If the content of the message box is veeeeeeeeeeeeeeeeeeeeeeeeeery long, "
           "it will automatically wrap like this.");
    auto* w = new MessageBox(title, content, window());
    w->setContentCopyable(true);
    if (w->exec()) {
        qDebug("Yes button is pressed");
    } else {
        qDebug("Cancel button is pressed");
    }
}

void DialogInterface::showCustomDialog() {
    auto* w = new CustomMessageBox(window());
    if (w->exec()) {
        qDebug() << w->urlLineEdit()->text();
    }
}

void DialogInterface::showColorDialog() {
    auto* w = new ColorDialog(Qt::cyan, tr("Choose color"), window());
    connect(w, &ColorDialog::colorChanged, [](const QColor& c) { qDebug() << c.name(); });
    w->exec();
}

void DialogInterface::showSimpleFlyout() {
    Flyout::create(QStringLiteral("Lesson 3"), tr("Believe in the spin, just keep believing!"),
                   QVariant(),  // icon
                   QVariant(),  // image
                   false,       // closable
                   QVariant::fromValue<QWidget*>(simpleFlyoutButton_), window());
}

void DialogInterface::showComplexFlyout() {
    auto* view = new FlyoutView(tr("Julius·Zeppeli"),
                                tr("触网而起的网球会落到哪一侧，谁也无法知晓。\n如果那种时刻到来，"
                                   "我希望「女神」是存在的。\n"
                                   "这样的话，不管网球落到哪一边，我都会坦然接受的吧。"),
                                QStringLiteral(":/gallery/images/SBR.jpg"), this);
    // view->setMaxImageWidth(400);

    // add button to view
    auto* button = new PushButton(tr("Action"), view);
    button->setFixedWidth(120);
    view->addWidget(button, 0, Qt::AlignRight);

    // adjust layout (optional)
    view->widgetLayout()->insertSpacing(1, 5);
    view->widgetLayout()->insertSpacing(0, 5);
    view->widgetLayout()->addSpacing(5);

    // show view
    Flyout::make(view, QVariant::fromValue<QWidget*>(complexFlyoutButton_), window(),
                 FlyoutAnimationType::SlideRight);
}

void DialogInterface::showBottomTeachingTip() {
    TeachingTip::create(teachingButton_, QStringLiteral("Lesson 4"),
                        tr("With respect, let's advance towards a new stage of the spin."),
                        QVariant(),  // icon
                        QVariant(),  // image
                        true,        // closable
                        -1,          // duration (-1 means no auto-close)
                        TeachingTipTailPosition::Bottom, this);
}

void DialogInterface::showLeftBottomTeachingTip() {
    auto pos = TeachingTipTailPosition::LeftBottom;
    auto* view = new TeachingTipView(QStringLiteral("Lesson 5"),
                                     tr("The shortest shortcut is to take a detour."),
                                     QIcon(),  // icon
                                     QStringLiteral(":/gallery/images/Gyro.jpg"),
                                     true,  // closable
                                     pos, this);
    auto* button = new PushButton(tr("Action"), view);
    button->setFixedWidth(120);
    view->addWidget(button, 0, Qt::AlignRight);

    auto* t = TeachingTip::make(view, teachingRightButton_, 3000, pos, this);
    connect(view, &TeachingTipView::closed, t, &TeachingTip::close);
}

// CustomMessageBox implementation
CustomMessageBox::CustomMessageBox(QWidget* parent) : MessageBoxBase(parent) {
    titleLabel_ = new SubtitleLabel(tr("Open URL"), this);
    urlLineEdit_ = new LineEdit(this);

    urlLineEdit_->setPlaceholderText(tr("Enter the URL of a file, stream, or playlist"));
    urlLineEdit_->setClearButtonEnabled(true);

    // add widget to view layout
    viewLayout->addWidget(titleLabel_);
    viewLayout->addWidget(urlLineEdit_);

    // change the text of button
    yesButton->setText(tr("Open"));
    cancelButton->setText(tr("Cancel"));

    widget->setMinimumWidth(360);
    yesButton->setDisabled(true);
    connect(urlLineEdit_, &LineEdit::textChanged, this, &CustomMessageBox::onUrlTextChanged);
}

LineEdit* CustomMessageBox::urlLineEdit() const { return urlLineEdit_; }

void CustomMessageBox::onUrlTextChanged(const QString& text) {
    yesButton->setEnabled(QUrl(text).isValid());
}

}  // namespace qfw
