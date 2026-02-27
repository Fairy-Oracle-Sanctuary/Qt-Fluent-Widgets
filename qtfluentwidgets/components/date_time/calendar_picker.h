#pragma once

#include <QDate>
#include <QPoint>
#include <QPushButton>
#include <Qt>

namespace qfw {

class CalendarPicker : public QPushButton {
    Q_OBJECT
    Q_PROPERTY(QDate date READ date WRITE setDate)
    Q_PROPERTY(Qt::DateFormat dateFormat READ dateFormat WRITE setDateFormat)

public:
    explicit CalendarPicker(QWidget* parent = nullptr);

    QDate date() const;
    void setDate(const QDate& date);

    Qt::DateFormat dateFormat() const;
    void setDateFormat(Qt::DateFormat format);
    void setDateFormat(const QString& format);

    bool isResetEnabled() const;
    void setResetEnabled(bool enabled);

    void setYearRange(int minYear, int maxYear);

    void reset();

signals:
    void dateChanged(const QDate& date);

protected:
    void paintEvent(QPaintEvent* e) override;

protected slots:
    virtual void showCalendarView();

    void onDateChanged(const QDate& date);

private:
    QDate date_;
    Qt::DateFormat dateFormat_ = Qt::ISODate;
    QString customFormat_;
    bool isResetEnabled_ = false;

protected:
    int minYearRange_ = 5;
    int maxYearRange_ = 5;
};

}  // namespace qfw
