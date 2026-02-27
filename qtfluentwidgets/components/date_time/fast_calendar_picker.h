#pragma once

#include "components/date_time/calendar_picker.h"

#include "components/widgets/flyout.h"

namespace qfw {

class FastCalendarPicker : public CalendarPicker {
    Q_OBJECT

public:
    explicit FastCalendarPicker(QWidget* parent = nullptr);

    FlyoutAnimationType flyoutAnimationType() const;
    void setFlyoutAnimationType(FlyoutAnimationType type);

protected slots:
    void showCalendarView() override;

private:
    FlyoutAnimationType flyoutAnimationType_ = FlyoutAnimationType::DropDown;
};

}  // namespace qfw
