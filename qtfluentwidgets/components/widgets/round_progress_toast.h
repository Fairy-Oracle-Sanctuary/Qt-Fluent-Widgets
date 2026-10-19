#pragma once

#include "components/widgets/toast.h"

namespace qfw {
class IndeterminateProgressRing;
// Loading-only capsule. Closing the notification does not cancel a task.
class RoundProgressToast : public Toast {
    Q_OBJECT
public:
    explicit RoundProgressToast(QWidget* parent);
    explicit RoundProgressToast(const QString& content = QString(), int duration = -1,
                                ToastPosition position = ToastPosition::TopRight,
                                QWidget* parent = nullptr);
    static RoundProgressToast* newToast(const QString& content = QString(), int duration = -1,
                                        ToastPosition position = ToastPosition::TopRight,
                                        QWidget* parent = nullptr);
    static RoundProgressToast* info(const QString& content = QString(), int duration = -1,
                                    ToastPosition position = ToastPosition::TopRight,
                                    QWidget* parent = nullptr);
    IndeterminateProgressRing* progressRing = nullptr;

protected:
    QColor defaultBackgroundColor() const override;
    void closeEvent(QCloseEvent* event) override;
    void changeEvent(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
};
}  // namespace qfw
