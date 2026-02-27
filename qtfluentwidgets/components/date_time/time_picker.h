#pragma once

#include <QTime>

#include "picker_base.h"

namespace qfw {

class MinuteFormatter : public DigitFormatter {
    Q_OBJECT

public:
    using DigitFormatter::DigitFormatter;

    QString encode(const QVariant& value) const override;
};

class AMHourFormatter : public DigitFormatter {
    Q_OBJECT

public:
    using DigitFormatter::DigitFormatter;

    QString encode(const QVariant& value) const override;
};

class AMPMFormatter : public PickerColumnFormatter {
    Q_OBJECT

public:
    explicit AMPMFormatter(QObject* parent = nullptr);

    QString encode(const QVariant& value) const override;

private:
    QString AM_;
    QString PM_;
};

class TimePickerBase : public PickerBase {
    Q_OBJECT
    Q_PROPERTY(QTime time READ time WRITE setTime)
    Q_PROPERTY(bool secondVisible READ isSecondVisible WRITE setSecondVisible)

public:
    explicit TimePickerBase(QWidget* parent = nullptr, bool showSeconds = false);

    QTime time() const;
    virtual void setTime(const QTime& time);

    bool isSecondVisible() const;
    virtual void setSecondVisible(bool isVisible);

signals:
    void timeChanged(const QTime& time);

protected:
    bool secondVisible_ = false;
    QTime time_;
};

class TimePicker : public TimePickerBase {
    Q_OBJECT

public:
    explicit TimePicker(QWidget* parent = nullptr, bool showSeconds = false);

    void setTime(const QTime& time) override;
    void setSecondVisible(bool isVisible) override;

protected:
    QStringList panelInitialValue() const override;
    void onConfirmed(const QStringList& value) override;
};

class AMTimePicker : public TimePickerBase {
    Q_OBJECT

public:
    explicit AMTimePicker(QWidget* parent = nullptr, bool showSeconds = false);

    void setTime(const QTime& time) override;
    void setSecondVisible(bool isVisible) override;

protected:
    QStringList panelInitialValue() const override;
    void onConfirmed(const QStringList& value) override;

private:
    QString AM_;
    QString PM_;
};

}  // namespace qfw
