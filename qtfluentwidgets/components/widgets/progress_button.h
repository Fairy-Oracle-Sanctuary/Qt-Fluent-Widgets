#pragma once

#include "components/widgets/button.h"

class QHideEvent;
class QShowEvent;
class QParallelAnimationGroup;
class QPropertyAnimation;

namespace qfw {

// Click-to-start is opt-in. While active, clicks request cancellation without
// changing the task state/value; the application decides when to stop.
class ProgressPushButton : public PrimaryPushButton {
    Q_OBJECT
    Q_PROPERTY(bool progressing READ isProgressing WRITE setProgressing NOTIFY progressChanged)
    Q_PROPERTY(bool autoProgressEnabled READ isAutoProgressEnabled WRITE setAutoProgressEnabled)
    Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(qreal displayValue READ displayValue WRITE setDisplayValue)
public:
    explicit ProgressPushButton(QWidget* parent = nullptr);
    explicit ProgressPushButton(const QString& text, QWidget* parent = nullptr);
    explicit ProgressPushButton(const QIcon& icon, const QString& text, QWidget* parent = nullptr);
    explicit ProgressPushButton(const FluentIconBase& icon, const QString& text,
                                QWidget* parent = nullptr);

    bool isProgressing() const;
    void setProgressing(bool progressing);
    bool isAutoProgressEnabled() const;
    void setAutoProgressEnabled(bool enabled);
    int value() const;
    void setValue(int value);
    qreal displayValue() const;
    void setDisplayValue(qreal value);

signals:
    void progressChanged(bool progressing);
    void valueChanged(int value);
    void stopRequested();

protected:
    void paintEvent(QPaintEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private:
    bool progressing_ = false;
    bool autoProgress_ = false;
    int value_ = 0;
    qreal displayValue_ = 0;
    QPropertyAnimation* progressAnimation_;
};

class IndeterminateProgressPushButton : public PrimaryPushButton {
    Q_OBJECT
    Q_PROPERTY(int startAngle READ startAngle WRITE setStartAngle)
    Q_PROPERTY(int spanAngle READ spanAngle WRITE setSpanAngle)
public:
    explicit IndeterminateProgressPushButton(QWidget* parent = nullptr);
    explicit IndeterminateProgressPushButton(const QString& text, QWidget* parent = nullptr);
    explicit IndeterminateProgressPushButton(const QIcon& icon, const QString& text,
                                             QWidget* parent = nullptr);
    explicit IndeterminateProgressPushButton(const FluentIconBase& icon, const QString& text,
                                             QWidget* parent = nullptr);
    void start();
    void stop();
    bool isSpinning() const;
    int startAngle() const;
    void setStartAngle(int angle);
    int spanAngle() const;
    void setSpanAngle(int angle);
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    void syncAnimation();
    bool spinning_ = true;
    int startAngle_ = 0;
    int spanAngle_ = 0;
    QParallelAnimationGroup* animationGroup_ = nullptr;
};

}  // namespace qfw
