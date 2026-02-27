#pragma once

#include <QFrame>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

#include "../widgets/button.h"
#include "mask_dialog_base.h"

namespace qfw {

/**
 * @brief Message box base class
 */
class MessageBoxBase : public MaskDialogBase {
    Q_OBJECT

public:
    explicit MessageBoxBase(QWidget* parent = nullptr);
    ~MessageBoxBase() = default;

    /**
     * @brief Validate the data of form before closing dialog
     * @return Whether the data of form is legal
     */
    virtual bool validate();

    void hideYesButton();
    void hideCancelButton();

    QFrame* buttonGroup;
    PrimaryPushButton* yesButton;
    QPushButton* cancelButton;

protected:
    void setQss();

    QVBoxLayout* vBoxLayout;
    QVBoxLayout* viewLayout;
    QHBoxLayout* buttonLayout;

private slots:
    void onYesButtonClicked();
    void onCancelButtonClicked();

private:
    void initWidget();
    void initLayout();
};

}  // namespace qfw
