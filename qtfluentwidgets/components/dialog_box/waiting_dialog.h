#pragma once

#include <QLabel>
#include <QVBoxLayout>

#include "components/dialog_box/mask_dialog_base.h"
#include "components/widgets/progress_ring.h"

namespace qfw {

// Window-modal task UI. Dismissal does not itself cancel a business operation.
class WaitingDialog : public MaskDialogBase {
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(QString content READ content WRITE setContent)

public:
    explicit WaitingDialog(QWidget* parent);
    explicit WaitingDialog(const QString& title, const QString& content = QString(),
                           QWidget* parent = nullptr);
    ~WaitingDialog() override;

    QString title() const;
    void setTitle(const QString& title);
    QString content() const;
    void setContent(const QString& content);
    QFrame* panel() const;

    IndeterminateProgressRing* progressRing = nullptr;
    QLabel* titleLabel = nullptr;
    QLabel* contentLabel = nullptr;

public slots:
    void done(int code) override;

protected:
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void changeEvent(QEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    void fitToOwner();
    void stopWaiting();
    void animateOpacity(qreal start, qreal end, int duration);
    void finishFade();

    QPointer<QWidget> owner_;
    QVBoxLayout* textLayout_ = nullptr;
    QHBoxLayout* contentLayout_ = nullptr;
    QPropertyAnimation* opacityAnimation_ = nullptr;
    bool closing_ = false;
    bool filterInstalled_ = false;
    int resultCode_ = QDialog::Rejected;
};

}  // namespace qfw
