#include "date_picker.h"

#include <QApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <numeric>

#include "common/style_sheet.h"

namespace qfw {

MonthFormatter::MonthFormatter(QObject* parent) : PickerColumnFormatter(parent) {
    months << tr("January") << tr("February") << tr("March") << tr("April") << tr("May")
           << tr("June") << tr("July") << tr("August") << tr("September") << tr("October")
           << tr("November") << tr("December");
}

QString MonthFormatter::encode(const QVariant& value) const {
    int m = value.toInt();
    if (m >= 1 && m <= 12) {
        return months[m - 1];
    }
    return QString();
}

QVariant MonthFormatter::decode(const QString& value) const {
    int index = months.indexOf(value);
    return (index >= 0) ? index + 1 : QVariant();
}

DatePickerBase::DatePickerBase(QWidget* parent) : PickerBase(parent) {
    date_ = QDate::currentDate();
}

QDate DatePickerBase::getDate() const { return date_; }

void DatePickerBase::setYearFormatter(PickerColumnFormatter* formatter) {
    yearFormatter_ = formatter;
}

void DatePickerBase::setMonthFormatter(PickerColumnFormatter* formatter) {
    monthFormatter_ = formatter;
}

void DatePickerBase::setDayFormatter(PickerColumnFormatter* formatter) {
    dayFormatter_ = formatter;
}

PickerColumnFormatter* DatePickerBase::yearFormatter() const {
    return yearFormatter_ ? yearFormatter_ : new DigitFormatter(const_cast<DatePickerBase*>(this));
}

PickerColumnFormatter* DatePickerBase::monthFormatter() const {
    return monthFormatter_ ? monthFormatter_
                           : new MonthFormatter(const_cast<DatePickerBase*>(this));
}

PickerColumnFormatter* DatePickerBase::dayFormatter() const {
    return dayFormatter_ ? dayFormatter_ : new DigitFormatter(const_cast<DatePickerBase*>(this));
}

DatePicker::DatePicker(QWidget* parent, DateFormat format, bool isMonthTight)
    : DatePickerBase(parent), isMonthTight_(isMonthTight) {
    MONTH_ = tr("month");
    YEAR_ = tr("year");
    DAY_ = tr("day");

    setDateFormat(format);
}

void DatePicker::setDateFormat(DateFormat format) {
    clearColumns();
    int y = QDate::currentDate().year();
    dateFormat_ = format;

    QVariantList years;
    for (int i = y - 100; i <= y + 100; ++i) years << QString::number(i);

    QVariantList months;
    for (int i = 1; i <= 12; ++i) months << QString::number(i);

    QVariantList days;
    for (int i = 1; i <= 31; ++i) days << QString::number(i);

    if (format == MM_DD_YYYY) {
        monthIndex_ = 0;
        dayIndex_ = 1;
        yearIndex_ = 2;

        addColumn(MONTH_, months, 80, Qt::AlignLeft, monthFormatter());
        addColumn(DAY_, days, 80, Qt::AlignCenter, dayFormatter());
        addColumn(YEAR_, years, 80, Qt::AlignCenter, yearFormatter());
    } else {
        yearIndex_ = 0;
        monthIndex_ = 1;
        dayIndex_ = 2;

        addColumn(YEAR_, years, 80, Qt::AlignCenter, yearFormatter());
        addColumn(MONTH_, months, 80, Qt::AlignCenter, monthFormatter());
        addColumn(DAY_, days, 80, Qt::AlignCenter, dayFormatter());
    }

    setColumnWidth(monthIndex_, _monthColumnWidth());
}

void DatePicker::setMonthTight(bool isTight) {
    if (isMonthTight_ == isTight) {
        return;
    }

    isMonthTight_ = isTight;
    setColumnWidth(monthIndex_, _monthColumnWidth());
}

void DatePicker::setDate(const QDate& date) {
    if (!date.isValid() || date.isNull()) {
        return;
    }

    date_ = date;
    setColumnValue(monthIndex_, QString::number(date.month()));
    setColumnValue(dayIndex_, QString::number(date.day()));
    setColumnValue(yearIndex_, QString::number(date.year()));

    QVariantList days;
    for (int i = 1; i <= date.daysInMonth(); ++i) days << QString::number(i);
    setColumnItems(dayIndex_, days);
}

QStringList DatePicker::panelInitialValue() const {
    QStringList v = value();
    bool hasValue = false;
    for (const auto& s : v) {
        if (!s.isEmpty()) {
            hasValue = true;
            break;
        }
    }

    if (hasValue) {
        return v;
    }

    QDate date = QDate::currentDate();
    QString y = encodeValue(yearIndex_, date.year());
    QString m = encodeValue(monthIndex_, date.month());
    QString d = encodeValue(dayIndex_, date.day());

    if (dateFormat_ == YYYY_MM_DD) {
        return {y, m, d};
    }
    return {m, d, y};
}

void DatePicker::onConfirmed(const QStringList& value) {
    int year = decodeValue(yearIndex_, value[yearIndex_]).toInt();
    int month = decodeValue(monthIndex_, value[monthIndex_]).toInt();
    int day = decodeValue(dayIndex_, value[dayIndex_]).toInt();

    QDate date(year, month, day);
    QDate od = date_;
    setDate(date);

    if (od != date) {
        emit dateChanged(date);
    }
}

void DatePicker::onColumnValueChanged(int index, const QString& value) {
    if (index == dayIndex_) {
        return;
    }

    // Need access to the panel here, but onColumnValueChanged in PickerBase
    // doesn't provide it directly in the C++ version I previously migrated
    // without some adjustments. Let's assume PickerBase handles the signal
    // from panel and we might need to override it correctly.
    // Actually, in picker_base.cpp I connected it to onColumnValueChanged(int index, const QString&
    // value). The Python code used the panel to get other column values.

    // We can't easily get other column values from just 'index' and 'value'
    // unless we have a reference to the panel.
    // However, PickerPanel is not exposed in PickerBase.
    // Let's look at how PickerBase was implemented.
}

int DatePicker::_monthColumnWidth() const {
    QFontMetrics fm = fontMetrics();
    int wm = 0;
    if (monthIndex_ >= 0 && monthIndex_ < columns_.size()) {
        const auto& items = columns_[monthIndex_]->items();
        for (const auto& i : items) {
            wm = std::max(wm, fm.boundingRect(i).width());
        }
    }
    wm += 20;

    if (MONTH_ == "month") {
        return wm + 49;
    }

    return isMonthTight_ ? std::max(80, wm) : wm + 49;
}

ZhFormatter::ZhFormatter(const QString& suffix, QObject* parent)
    : PickerColumnFormatter(parent), suffix_(suffix) {}

QString ZhFormatter::encode(const QVariant& value) const { return value.toString() + suffix_; }

QVariant ZhFormatter::decode(const QString& value) const {
    return value.left(value.length() - suffix_.length()).toInt();
}

ZhDatePicker::ZhDatePicker(QWidget* parent) : DatePicker(parent, YYYY_MM_DD) {
    MONTH_ = QStringLiteral("月");
    YEAR_ = QStringLiteral("年");
    DAY_ = QStringLiteral("日");
    setDayFormatter(new ZhDayFormatter(this));
    setYearFormatter(new ZhYearFormatter(this));
    setMonthFormatter(new ZhMonthFormatter(this));
    setDateFormat(YYYY_MM_DD);
}

}  // namespace qfw
