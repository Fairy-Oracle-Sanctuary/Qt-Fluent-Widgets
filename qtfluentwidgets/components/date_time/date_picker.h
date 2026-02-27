#ifndef DATE_PICKER_H
#define DATE_PICKER_H

#include <QCalendar>
#include <QDate>

#include "picker_base.h"

namespace qfw {

/**
 * @brief Month formatter
 */
class MonthFormatter : public PickerColumnFormatter {
public:
    explicit MonthFormatter(QObject* parent = nullptr);

    QString encode(const QVariant& value) const override;
    QVariant decode(const QString& value) const override;

private:
    QStringList months;
};

/**
 * @brief Date picker base class
 */
class DatePickerBase : public PickerBase {
    Q_OBJECT
    Q_PROPERTY(QDate date READ getDate WRITE setDate NOTIFY dateChanged)

public:
    explicit DatePickerBase(QWidget* parent = nullptr);

    QDate getDate() const;
    virtual void setDate(const QDate& date) = 0;

    void setYearFormatter(PickerColumnFormatter* formatter);
    void setMonthFormatter(PickerColumnFormatter* formatter);
    void setDayFormatter(PickerColumnFormatter* formatter);

    PickerColumnFormatter* yearFormatter() const;
    PickerColumnFormatter* monthFormatter() const;
    PickerColumnFormatter* dayFormatter() const;

signals:
    void dateChanged(const QDate& date);

protected:
    QDate date_;
    QCalendar calendar_;
    PickerColumnFormatter* yearFormatter_ = nullptr;
    PickerColumnFormatter* monthFormatter_ = nullptr;
    PickerColumnFormatter* dayFormatter_ = nullptr;
};

/**
 * @brief Date picker
 */
class DatePicker : public DatePickerBase {
    Q_OBJECT

public:
    enum DateFormat { MM_DD_YYYY = 0, YYYY_MM_DD = 1 };

    explicit DatePicker(QWidget* parent = nullptr, DateFormat format = MM_DD_YYYY,
                        bool isMonthTight = true);

    void setDateFormat(DateFormat format);
    void setMonthTight(bool isTight);
    void setDate(const QDate& date) override;

protected:
    QStringList panelInitialValue() const override;
    void onConfirmed(const QStringList& value) override;
    void onColumnValueChanged(int index, const QString& value) override;

protected:
    int _monthColumnWidth() const;

    DateFormat dateFormat_ = MM_DD_YYYY;
    bool isMonthTight_ = true;
    int monthIndex_ = 0;
    int dayIndex_ = 1;
    int yearIndex_ = 2;

    QString MONTH_;
    QString YEAR_;
    QString DAY_;
};

/**
 * @brief Chinese date formatter
 */
class ZhFormatter : public PickerColumnFormatter {
public:
    explicit ZhFormatter(const QString& suffix, QObject* parent = nullptr);
    QString encode(const QVariant& value) const override;
    QVariant decode(const QString& value) const override;

private:
    QString suffix_;
};

/**
 * @brief Chinese year formatter
 */
class ZhYearFormatter : public ZhFormatter {
public:
    explicit ZhYearFormatter(QObject* parent = nullptr)
        : ZhFormatter(QStringLiteral("年"), parent) {}
};

/**
 * @brief Chinese month formatter
 */
class ZhMonthFormatter : public ZhFormatter {
public:
    explicit ZhMonthFormatter(QObject* parent = nullptr)
        : ZhFormatter(QStringLiteral("月"), parent) {}
};

/**
 * @brief Chinese day formatter
 */
class ZhDayFormatter : public ZhFormatter {
public:
    explicit ZhDayFormatter(QObject* parent = nullptr)
        : ZhFormatter(QStringLiteral("日"), parent) {}
};

/**
 * @brief Chinese date picker
 */
class ZhDatePicker : public DatePicker {
    Q_OBJECT

public:
    explicit ZhDatePicker(QWidget* parent = nullptr);
};

}  // namespace qfw

#endif  // DATE_PICKER_H
