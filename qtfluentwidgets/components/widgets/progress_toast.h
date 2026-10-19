#pragma once

#include <QMap>

#include "components/widgets/info_bar.h"
#include "components/widgets/toast.h"

namespace qfw {
// Persistent task notification. Progress, status and lifetime are independent.
class ProgressToast : public Toast {
    Q_OBJECT
    Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(qreal progress READ progress WRITE setProgress)
    Q_PROPERTY(bool useAni READ isUseAni WRITE setUseAni)
public:
    explicit ProgressToast(const QString& content,
                           InfoBarIconEnum icon = InfoBarIconEnum::Information, int value = 0,
                           int duration = -1, bool isClosable = true,
                           ToastPosition position = ToastPosition::TopRight,
                           QWidget* parent = nullptr);
    InfoBarIconEnum icon() const;
    void setIcon(InfoBarIconEnum icon);
    int value() const;
    void setValue(int value);
    qreal progress() const;
    void setProgress(qreal value);
    bool isUseAni() const;
    void setUseAni(bool enabled);
    QColor barColor() const;
    QColor statusColor() const override;
    void setStateColor(InfoBarIconEnum icon, const QColor& color);
    void setStateColor(InfoBarIconEnum icon, const QColor& light, const QColor& dark);
    void setCustomBarColor(const QColor& color);
    void setCustomBarColor(const QColor& light, const QColor& dark);
    static ProgressToast* newToast(const QString& content,
                                   InfoBarIconEnum icon = InfoBarIconEnum::Information,
                                   int value = 0, int duration = -1, bool isClosable = true,
                                   ToastPosition position = ToastPosition::TopRight,
                                   QWidget* parent = nullptr);
    static ProgressToast* info(const QString& content, int value = 0, int duration = -1,
                               bool isClosable = true,
                               ToastPosition position = ToastPosition::TopRight,
                               QWidget* parent = nullptr);
    static ProgressToast* success(const QString& content, int value = 100, int duration = -1,
                                  bool isClosable = true,
                                  ToastPosition position = ToastPosition::TopRight,
                                  QWidget* parent = nullptr);
    static ProgressToast* warning(const QString& content, int value = 0, int duration = -1,
                                  bool isClosable = true,
                                  ToastPosition position = ToastPosition::TopRight,
                                  QWidget* parent = nullptr);
    static ProgressToast* error(const QString& content, int value = 0, int duration = -1,
                                bool isClosable = true,
                                ToastPosition position = ToastPosition::TopRight,
                                QWidget* parent = nullptr);
    static ProgressToast* custom(const QString& content, const QColor& color, int value = 0,
                                 InfoBarIconEnum icon = InfoBarIconEnum::Information,
                                 int duration = -1, bool isClosable = true,
                                 ToastPosition position = ToastPosition::TopRight,
                                 QWidget* parent = nullptr);
    static ProgressToast* custom(const QString& content, const QColor& light, const QColor& dark,
                                 int value = 0, InfoBarIconEnum icon = InfoBarIconEnum::Information,
                                 int duration = -1, bool isClosable = true,
                                 ToastPosition position = ToastPosition::TopRight,
                                 QWidget* parent = nullptr);
    QWidget* iconWidget = nullptr;
signals:
    void valueChanged(int value);

protected:
    QColor defaultBackgroundColor() const override;
    void paintEvent(QPaintEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    void updateAppearance();
    void settleProgress();
    InfoBarIconEnum icon_;
    int value_;
    qreal progress_;
    bool useAni_ = true;
    QPropertyAnimation* progressAnimation_ = nullptr;
    QMap<InfoBarIconEnum, QPair<QColor, QColor>> stateColors_;
    QPair<QColor, QColor> customColors_;
};
}  // namespace qfw
