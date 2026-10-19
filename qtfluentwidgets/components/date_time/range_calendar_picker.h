#pragma once

#include <QPair>
#include <QPointer>
#include <QSet>
#include "components/date_time/calendar_picker.h"
#include "components/widgets/flyout.h"

namespace qfw {

class RangeCalendarPicker : public CalendarPicker {
    Q_OBJECT
    Q_PROPERTY(QDate startDate READ startDate)
    Q_PROPERTY(QDate endDate READ endDate)
public:
    explicit RangeCalendarPicker(QWidget* parent = nullptr);
    ~RangeCalendarPicker() override;
    QDate startDate() const { return start_; }
    QDate endDate() const { return end_; }
    QPair<QDate, QDate> dateRange() const { return {start_, end_}; }
    QDate date() const override { return start_; }
    void setDateRange(const QDate& start, const QDate& end);
    void setDate(const QDate& date) override { setDateRange(date, date); }
    void reset() override;
    void setDateFormat(Qt::DateFormat format) override;
    void setDateFormat(const QString& format) override;
    QSet<int> disabledWeekDays() const { return disabled_; }
    void setDisabledWeekDays(const QSet<int>& days);
    FlyoutAnimationType flyoutAnimationType() const { return animation_; }
    void setFlyoutAnimationType(FlyoutAnimationType type) { animation_ = type; }
signals:
    void rangeChanged(const QDate& start, const QDate& end);
protected:
    RangeCalendarPicker(bool fast, QWidget* parent);
    void showCalendarView() override;
    void hideEvent(QHideEvent* event) override;
    void changeEvent(QEvent* event) override;
private:
    void updateText();
    void closePopup();
    QDate start_, end_;
    QString format_ = QStringLiteral("ddd MMM d yyyy");
    QSet<int> disabled_;
    bool fast_;
    QPointer<QWidget> popup_;
    FlyoutAnimationType animation_ = FlyoutAnimationType::DropDown;
};

class FastRangeCalendarPicker : public RangeCalendarPicker {
    Q_OBJECT
public:
    explicit FastRangeCalendarPicker(QWidget* parent = nullptr);
};
} // namespace qfw
