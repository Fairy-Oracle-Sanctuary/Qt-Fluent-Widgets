#pragma once

#include <QPointer>

#include "components/widgets/flyout.h"

namespace qfw {
class BodyLabel;
class SubtitleLabel;
class TransparentToolButton;

class FlyoutDialog : public FlyoutViewBase {
    Q_OBJECT
public:
    explicit FlyoutDialog(const QString& title = {}, const QString& content = {},
                          QWidget* parent = nullptr);
    void addWidget(QWidget* widget, int stretch = 0, Qt::Alignment align = Qt::AlignLeft) override;
    QSize sizeHint() const override;
    // The returned popup owns this view and deletes it after closing.
    // Create a fresh heap-allocated view for each interaction.
    Flyout* showAt(const QVariant& target = {}, QWidget* parent = nullptr,
                   FlyoutAnimationType aniType = FlyoutAnimationType::PullUp);
    Flyout* showAt(QWidget* target, QWidget* parent = nullptr,
                   FlyoutAnimationType aniType = FlyoutAnimationType::PullUp);
    Flyout* flyout() const;
    QVBoxLayout* vBoxLayout;
    QWidget* contentWidget;
    QVBoxLayout* viewLayout;
    SubtitleLabel* titleLabel;
    BodyLabel* contentLabel;
    QWidget* buttonBar;
    QHBoxLayout* buttonLayout;
    TransparentToolButton* yesButton;
    TransparentToolButton* cancelButton;
public slots:
    void accept();
    void reject();
signals:
    void accepted();
    void rejected();
    void closed();

protected:
    QColor borderColor() const override;
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    void bindFlyout(Flyout* popup);
    void finish(bool accepted);
    bool resolved_ = false;
    QPointer<Flyout> flyout_;
};
}  // namespace qfw
