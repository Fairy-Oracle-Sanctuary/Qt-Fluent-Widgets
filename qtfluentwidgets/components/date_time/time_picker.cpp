#include "time_picker.h"

#include <QTime>

namespace qfw {

// MinuteFormatter
QString MinuteFormatter::encode(const QVariant& value) const {
    bool ok = false;
    int minute = value.toInt(&ok);
    if (ok) {
        return QString::number(minute).rightJustified(2, '0');
    }
    return value.toString();
}

// AMHourFormatter
QString AMHourFormatter::encode(const QVariant& value) const {
    bool ok = false;
    int hour = value.toInt(&ok);
    if (!ok) {
        return value.toString();
    }

    if (hour == 0 || hour == 12) {
        return QStringLiteral("12");
    }

    return QString::number(hour % 12);
}

// AMPMFormatter
AMPMFormatter::AMPMFormatter(QObject* parent) : PickerColumnFormatter(parent) {
    AM_ = tr("AM");
    PM_ = tr("PM");
}

QString AMPMFormatter::encode(const QVariant& value) const {
    QString str = value.toString();
    bool ok = false;
    int hour = str.toInt(&ok);

    if (!ok) {
        return str;
    }

    return hour < 12 ? AM_ : PM_;
}

// TimePickerBase
TimePickerBase::TimePickerBase(QWidget* parent, bool showSeconds)
    : PickerBase(parent), secondVisible_(showSeconds) {}

QTime TimePickerBase::time() const { return time_; }

void TimePickerBase::setTime(const QTime& time) {
    Q_UNUSED(time);
    // Abstract - to be implemented by derived classes
}

bool TimePickerBase::isSecondVisible() const { return secondVisible_; }

void TimePickerBase::setSecondVisible(bool isVisible) {
    Q_UNUSED(isVisible);
    // Abstract - to be implemented by derived classes
}

// TimePicker
TimePicker::TimePicker(QWidget* parent, bool showSeconds) : TimePickerBase(parent, showSeconds) {
    int w = showSeconds ? 80 : 120;

    // add hour column
    QVariantList hourItems;
    for (int i = 0; i < 24; ++i) {
        hourItems.append(i);
    }
    addColumn(tr("hour"), hourItems, w, Qt::AlignCenter, new DigitFormatter(this));

    // add minute column
    QVariantList minuteItems;
    for (int i = 0; i < 60; ++i) {
        minuteItems.append(i);
    }
    addColumn(tr("minute"), minuteItems, w, Qt::AlignCenter, new MinuteFormatter(this));

    // add seconds column
    QVariantList secondItems;
    for (int i = 0; i < 60; ++i) {
        secondItems.append(i);
    }
    addColumn(tr("second"), secondItems, w, Qt::AlignCenter, new MinuteFormatter(this));
    setColumnVisible(2, showSeconds);
}

void TimePicker::setTime(const QTime& time) {
    if (!time.isValid() || time.isNull()) {
        return;
    }

    time_ = time;
    setColumnValue(0, time.hour());
    setColumnValue(1, time.minute());
    setColumnValue(2, time.second());
}

void TimePicker::setSecondVisible(bool isVisible) {
    secondVisible_ = isVisible;
    setColumnVisible(2, isVisible);

    int w = isVisible ? 80 : 120;
    setColumnWidth(0, w);
    setColumnWidth(1, w);
    setColumnWidth(2, w);
}

void TimePicker::onConfirmed(const QStringList& value) {
    int h = decodeValue(0, value[0]).toInt();
    int m = decodeValue(1, value[1]).toInt();
    int s = 0;

    if (value.size() == 3) {
        s = decodeValue(2, value[2]).toInt();
    }

    QTime newTime(h, m, s);
    QTime oldTime = time_;
    setTime(newTime);

    if (oldTime != newTime) {
        emit timeChanged(newTime);
    }
}

QStringList TimePicker::panelInitialValue() const {
    QStringList v = value();
    if (!v.isEmpty() && !v[0].isEmpty()) {
        return v;
    }

    QTime currentTime = QTime::currentTime();
    QString h = encodeValue(0, currentTime.hour());
    QString m = encodeValue(1, currentTime.minute());
    QString s = encodeValue(2, currentTime.second());

    if (isSecondVisible()) {
        return {h, m, s};
    }
    return {h, m};
}

// AMTimePicker
AMTimePicker::AMTimePicker(QWidget* parent, bool showSeconds)
    : TimePickerBase(parent, showSeconds) {
    AM_ = tr("AM");
    PM_ = tr("PM");

    // add hour column (1-12)
    QVariantList hourItems;
    for (int i = 1; i <= 12; ++i) {
        hourItems.append(i);
    }
    addColumn(tr("hour"), hourItems, 80, Qt::AlignCenter, new AMHourFormatter(this));

    // add minute column
    QVariantList minuteItems;
    for (int i = 0; i < 60; ++i) {
        minuteItems.append(i);
    }
    addColumn(tr("minute"), minuteItems, 80, Qt::AlignCenter, new MinuteFormatter(this));

    // add second column
    QVariantList secondItems;
    for (int i = 0; i < 60; ++i) {
        secondItems.append(i);
    }
    addColumn(tr("second"), secondItems, 80, Qt::AlignCenter, new MinuteFormatter(this));
    setColumnVisible(2, showSeconds);

    // add AM/PM column
    addColumn(AM_, {AM_, PM_}, 80, Qt::AlignCenter, new AMPMFormatter(this));
}

void AMTimePicker::setTime(const QTime& time) {
    if (!time.isValid() || time.isNull()) {
        return;
    }

    time_ = time;
    setColumnValue(0, time.hour());
    setColumnValue(1, time.minute());
    setColumnValue(2, time.second());
    setColumnValue(3, time.hour());
}

void AMTimePicker::setSecondVisible(bool isVisible) {
    secondVisible_ = isVisible;
    setColumnVisible(2, isVisible);
}

void AMTimePicker::onConfirmed(const QStringList& value) {
    int h, m, s;
    QString p;

    if (value.size() == 3) {
        h = decodeValue(0, value[0]).toInt();
        m = decodeValue(1, value[1]).toInt();
        p = value[2];
        s = 0;
    } else {
        h = decodeValue(0, value[0]).toInt();
        m = decodeValue(1, value[1]).toInt();
        s = decodeValue(2, value[2]).toInt();
        p = value[3];
    }

    // Convert 12-hour to 24-hour
    if (p == AM_) {
        h = (h == 12) ? 0 : h;
    } else if (p == PM_) {
        h = (h == 12) ? 12 : h + 12;
    }

    QTime newTime(h, m, s);
    QTime oldTime = time_;
    setTime(newTime);

    if (oldTime != newTime) {
        emit timeChanged(newTime);
    }
}

QStringList AMTimePicker::panelInitialValue() const {
    QStringList v = value();
    if (!v.isEmpty() && !v[0].isEmpty()) {
        return v;
    }

    QTime currentTime = QTime::currentTime();
    QString h = encodeValue(0, currentTime.hour());
    QString m = encodeValue(1, currentTime.minute());
    QString s = encodeValue(2, currentTime.second());
    QString p = encodeValue(3, currentTime.hour());

    if (isSecondVisible()) {
        return {h, m, s, p};
    }
    return {h, m, p};
}

}  // namespace qfw
