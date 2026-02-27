#pragma once

#include <QDialog>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#ifdef MessageBox
#undef MessageBox
#endif

#include "components/dialog_box/mask_dialog_base.h"
#include "components/widgets/button.h"
#include "components/widgets/label.h"
#include "components/window/frameless_window.h"

namespace qfw {

/**
 * @brief Base UI components for message boxes and dialogs
 */
class MessageBoxUI : public QObject {
    Q_OBJECT

public:
    explicit MessageBoxUI(QObject* parent = nullptr);

    void setUpUi(const QString& title, const QString& content, QWidget* parentWidget);
    void adjustText(QWidget* widget);
    void setContentCopyable(bool isCopyable);
    void hideYesButton();
    void hideCancelButton();

signals:
    void yesSignal();
    void cancelSignal();

public:
    QString content;
    QLabel* titleLabel = nullptr;
    BodyLabel* contentLabel = nullptr;
    QFrame* buttonGroup = nullptr;
    PrimaryPushButton* yesButton = nullptr;
    QPushButton* cancelButton = nullptr;
    QVBoxLayout* vBoxLayout = nullptr;
    QVBoxLayout* textLayout = nullptr;
    QHBoxLayout* buttonLayout = nullptr;

private slots:
    void onYesButtonClicked();
    void onCancelButtonClicked();

private:
    void initWidget(QWidget* parentWidget);
    void initLayout();
    void setQss(QWidget* widget);
};

/**
 * @brief Frameless dialog box
 */
class Dialog : public FramelessDialog {
    Q_OBJECT

public:
    explicit Dialog(const QString& title, const QString& content, QWidget* parent = nullptr);

    void setTitleBarVisible(bool isVisible);
    void setContentCopyable(bool isCopyable);

    MessageBoxUI* ui() const { return ui_; }

signals:
    void yesSignal();
    void cancelSignal();

private:
    MessageBoxUI* ui_ = nullptr;
    QLabel* windowTitleLabel = nullptr;
};

/**
 * @brief Message box with a mask
 */
class MessageBox : public MaskDialogBase {
    Q_OBJECT

public:
    explicit MessageBox(const QString& title, const QString& content, QWidget* parent = nullptr);

    void setContentCopyable(bool isCopyable);

    MessageBoxUI* ui() const { return ui_; }

signals:
    void yesSignal();
    void cancelSignal();

protected:
    bool eventFilter(QObject* obj, QEvent* e) override;

private:
    MessageBoxUI* ui_ = nullptr;
};

}  // namespace qfw
