#include "view/date_time_interface.h"

#include <QDate>
#include <Qt>

#include "common/translator.h"
#include "components/date_time/calendar_picker.h"
#include "components/date_time/date_picker.h"
#include "components/date_time/fast_calendar_picker.h"
#include "components/date_time/time_picker.h"

namespace qfw {

DateTimeInterface::DateTimeInterface(QWidget* parent)
    : GalleryInterface(Translator().dateTime(),
                       QStringLiteral("qtfluentwidgets.components.date_time"), parent) {
    setObjectName(QStringLiteral("dateTimeInterface"));

    // calendar picker
    addExampleCard(tr("A simple CalendarPicker"), new CalendarPicker(this),
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/date_time/calendar_picker/demo.py"));

    addExampleCard(tr("A fast CalendarPicker"), new FastCalendarPicker(this),
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/date_time/calendar_picker/demo.py"));

    auto* calendarPicker = new CalendarPicker(this);
    calendarPicker->setDateFormat(Qt::TextDate);
    addExampleCard(tr("A CalendarPicker in another format"), calendarPicker,
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/date_time/calendar_picker/demo.py"));

    // date picker
    addExampleCard(tr("A simple DatePicker"), new DatePicker(this),
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/date_time/time_picker/demo.py"));

    addExampleCard(tr("A DatePicker in another format"), new ZhDatePicker(this),
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/date_time/time_picker/demo.py"));

    // AM/PM time picker
    addExampleCard(tr("A simple TimePicker"), new AMTimePicker(this),
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/date_time/time_picker/demo.py"));

    // 24 hours time picker
    addExampleCard(tr("A TimePicker using a 24-hour clock"), new TimePicker(this),
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/date_time/time_picker/demo.py"));

    // 24 hours time picker with seconds
    auto* timePickerWithSeconds = new TimePicker(this);
    timePickerWithSeconds->setSecondVisible(true);
    addExampleCard(tr("A TimePicker with seconds column"), timePickerWithSeconds,
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/date_time/time_picker/demo.py"));
}

}  // namespace qfw
