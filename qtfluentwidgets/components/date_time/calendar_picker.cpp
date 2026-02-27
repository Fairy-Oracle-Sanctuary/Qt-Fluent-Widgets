#include "components/date_time/calendar_picker.h"

#include <QApplication>
#include <QLocale>
#include <QPainter>
#include <QRectF>
#include <QStyle>

#include "common/style_sheet.h"
#include "components/date_time/calendar_view.h"

namespace qfw {

CalendarPicker::CalendarPicker(QWidget* parent) : QPushButton(parent) {
    setText(tr("Pick a date"));
    setProperty("qssClass", QStringLiteral("CalendarPicker"));
    qfw::setStyleSheet(this, FluentStyleSheet::CalendarPicker);

    customFormat_ = QStringLiteral("ddd MMM d yyyy");

    connect(this, &QPushButton::clicked, this, &CalendarPicker::showCalendarView);
}

QDate CalendarPicker::date() const { return date_; }

void CalendarPicker::setDate(const QDate& date) { onDateChanged(date); }

Qt::DateFormat CalendarPicker::dateFormat() const { return dateFormat_; }

void CalendarPicker::setDateFormat(Qt::DateFormat format) {
    dateFormat_ = format;
    customFormat_.clear();
    if (date_.isValid()) {
        setText(date_.toString(format));
    }
}

void CalendarPicker::setDateFormat(const QString& format) {
    customFormat_ = format;
    if (date_.isValid()) {
        setText(QLocale::system().toString(date_, format));
    }
}

bool CalendarPicker::isResetEnabled() const { return isResetEnabled_; }

void CalendarPicker::setResetEnabled(bool enabled) { isResetEnabled_ = enabled; }

void CalendarPicker::setYearRange(int minYear, int maxYear) {
    minYearRange_ = minYear;
    maxYearRange_ = maxYear;
}

void CalendarPicker::reset() {
    date_ = QDate();
    setText(tr("Pick a date"));
    setProperty("hasDate", false);
    setStyle(QApplication::style());
    update();
}

void CalendarPicker::onDateChanged(const QDate& date) {
    date_ = date;
    if (!customFormat_.isEmpty()) {
        setText(QLocale::system().toString(date, customFormat_));
    } else {
        setText(date.toString(dateFormat_));
    }
    setProperty("hasDate", true);
    setStyle(QApplication::style());
    update();
    emit dateChanged(date);
}

void CalendarPicker::showCalendarView() {
    auto* view = new CalendarView(window());
    view->setResetEnabled(isResetEnabled_);

    // Calculate year range based on current date
    const int currentYear = QDate::currentDate().year();
    view->setYearRange(currentYear - minYearRange_, currentYear + maxYearRange_);

    connect(view, &CalendarView::resetted, this, &CalendarPicker::reset);
    connect(view, &CalendarView::dateChanged, this, &CalendarPicker::onDateChanged);

    if (date_.isValid()) {
        view->setDate(date_);
    }

    const int x = width() / 2 - view->sizeHint().width() / 2;
    const int y = height();
    view->exec(mapToGlobal(QPoint(x, y)));
}

void CalendarPicker::paintEvent(QPaintEvent* e) {
    QPushButton::paintEvent(e);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    if (!property("hasDate").toBool()) {
        painter.setOpacity(0.6);
    }

    const int w = 12;
    const QRect rect(static_cast<int>(width() - 23), static_cast<int>(height() / 2 - w / 2), w, w);
    FluentIcon(FluentIconEnum::Calendar).render(&painter, rect);
}

}  // namespace qfw
