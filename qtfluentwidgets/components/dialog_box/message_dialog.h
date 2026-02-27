#pragma once

#include <QLabel>
#include <QPushButton>

#include "mask_dialog_base.h"

namespace qfw {

/**
 * @brief Win10 style message dialog box with a mask
 */
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
    void initWidget();
    void initLayout();
    void setQss();

    QString content;
    QLabel* titleLabel;
    QLabel* contentLabel;
};

}  // namespace qfw
