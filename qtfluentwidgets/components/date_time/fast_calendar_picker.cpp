#include "components/date_time/fast_calendar_picker.h"

#include <QDate>
#include <QVariant>

#include "components/date_time/fast_calendar_view.h"

namespace qfw {

FastCalendarPicker::FastCalendarPicker(QWidget* parent) : CalendarPicker(parent) {
    setFlyoutAnimationType(FlyoutAnimationType::DropDown);
}

FlyoutAnimationType FastCalendarPicker::flyoutAnimationType() const { return flyoutAnimationType_; }

void FastCalendarPicker::setFlyoutAnimationType(FlyoutAnimationType type) {
    flyoutAnimationType_ = type;
}

void FastCalendarPicker::showCalendarView() {
    auto* view = new FastCalendarView(window());
    view->setResetEnabled(isResetEnabled());

    const int currentYear = QDate::currentDate().year();
    view->setYearRange(currentYear - minYearRange_, currentYear + maxYearRange_);

    connect(view, &FastCalendarView::resetted, this, &FastCalendarPicker::reset);
    connect(view, &FastCalendarView::dateChanged, this, &FastCalendarPicker::onDateChanged);

    if (date().isValid()) {
        view->setDate(date());
    }

    auto* flyout = Flyout::make(view, QVariant::fromValue(static_cast<QWidget*>(this)), window(),
                                flyoutAnimationType_);
    connect(view, &FastCalendarView::dateChanged, flyout, &Flyout::close);
}

}  // namespace qfw
