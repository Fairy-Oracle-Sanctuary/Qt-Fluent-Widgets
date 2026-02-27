#pragma once

#include "components/dialog_box/message_box_base.h"
#include "view/gallery_interface.h"

class QUrl;

namespace qfw {

class PushButton;
class LineEdit;
class SubtitleLabel;

class DialogInterface : public GalleryInterface {
    Q_OBJECT

public:
    explicit DialogInterface(QWidget* parent = nullptr);

private slots:
    void showDialog();
    void showMessageDialog();
    void showCustomDialog();
    void showColorDialog();
    void showSimpleFlyout();
    void showComplexFlyout();
    void showBottomTeachingTip();
    void showLeftBottomTeachingTip();

private:
    PushButton* simpleFlyoutButton_ = nullptr;
    PushButton* complexFlyoutButton_ = nullptr;
    PushButton* teachingButton_ = nullptr;
    PushButton* teachingRightButton_ = nullptr;
};

class CustomMessageBox : public MessageBoxBase {
    Q_OBJECT

public:
    explicit CustomMessageBox(QWidget* parent = nullptr);

    LineEdit* urlLineEdit() const;

private slots:
    void onUrlTextChanged(const QString& text);

private:
    SubtitleLabel* titleLabel_ = nullptr;
    LineEdit* urlLineEdit_ = nullptr;
};

}  // namespace qfw
