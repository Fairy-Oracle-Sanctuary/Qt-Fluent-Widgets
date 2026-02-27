#pragma once

#include "view/gallery_interface.h"

namespace qfw {

class AMTimePicker;
class CalendarPicker;
class DatePicker;
class FastCalendarPicker;
class TimePicker;
class ZhDatePicker;

class DateTimeInterface : public GalleryInterface {
    Q_OBJECT

public:
    explicit DateTimeInterface(QWidget* parent = nullptr);
};

}  // namespace qfw
