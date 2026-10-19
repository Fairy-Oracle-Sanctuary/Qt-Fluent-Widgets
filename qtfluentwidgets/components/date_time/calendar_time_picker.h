#pragma once

#include <QDateTime>
#include <QPointer>

#include "components/date_time/calendar_picker.h"
#include "components/widgets/flyout.h"

namespace qfw {
class CalendarTimePicker : public CalendarPicker {
    Q_OBJECT
    Q_PROPERTY(QDateTime dateTime READ dateTime WRITE setDateTime NOTIFY dateTimeChanged)
    Q_PROPERTY(QTime time READ time WRITE setTime NOTIFY timeChanged)
    Q_PROPERTY(QString dateTimeFormat READ dateTimeFormat WRITE setDateTimeFormat)
    Q_PROPERTY(bool secondVisible READ isSecondVisible WRITE setSecondVisible)
public:
    explicit CalendarTimePicker(QWidget* parent = nullptr, bool showSeconds = true);
    ~CalendarTimePicker() override;
    QDateTime dateTime() const { return value_; }
    void setDateTime(const QDateTime& value);
    QDate date() const override { return value_.date(); }
    void setDate(const QDate& date) override;
    QTime time() const { return value_.time(); }
    void setTime(const QTime& time);
    QString dateTimeFormat() const { return format_; }
    void setDateTimeFormat(const QString& format);
    void setDateFormat(const QString& format) override { setDateTimeFormat(format); }
    void setDateFormat(Qt::DateFormat format) override;
    bool isSecondVisible() const { return showSeconds_; }
    void setSecondVisible(bool visible);
    FlyoutAnimationType flyoutAnimationType() const { return animation_; }
    void setFlyoutAnimationType(FlyoutAnimationType type) { animation_ = type; }
    void reset() override;

signals:
    void dateTimeChanged(const QDateTime& value);
    void timeChanged(const QTime& time);

protected:
    CalendarTimePicker(bool fast, QWidget* parent, bool showSeconds);
    void showCalendarView() override;
    void paintEvent(QPaintEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    void updateText();
    void closePopup();
    QDateTime value_;
    QString format_;
    bool fast_ = false;
    bool showSeconds_ = true;
    FlyoutAnimationType animation_ = FlyoutAnimationType::DropDown;
    QPointer<Flyout> flyout_;
};

class FastCalendarTimePicker : public CalendarTimePicker {
    Q_OBJECT
public:
    explicit FastCalendarTimePicker(QWidget* parent = nullptr, bool showSeconds = true);
};
}  // namespace qfw
