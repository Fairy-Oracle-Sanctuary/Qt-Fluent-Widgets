#pragma once

#include <QLabel>
#include <QPushButton>

#include "mask_dialog_base.h"

namespace qfw {

/** A rounded message dialog with a dimmed parent window. */
class MessageDialog : public MaskDialogBase {
    Q_OBJECT

public:
    explicit MessageDialog(const QString& title, const QString& content, QWidget* parent = nullptr);
    ~MessageDialog() = default;

    QPushButton* yesButton;
    QPushButton* cancelButton;

signals:
    void yesSignal();
    void cancelSignal();

private slots:
    void onYesButtonClicked();
    void onCancelButtonClicked();

private:
    QLabel* titleLabel;
    QLabel* contentLabel;
};

}  // namespace qfw
