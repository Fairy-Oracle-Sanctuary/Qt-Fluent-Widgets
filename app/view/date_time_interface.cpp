#include "view/date_time_interface.h"

#include <QDate>
#include <Qt>
#include <QVBoxLayout>

#include "common/translator.h"
#include "components/date_time/calendar_picker.h"
#include "components/date_time/range_calendar_picker.h"
#include "components/widgets/check_box.h"
#include "components/widgets/label.h"
#include "components/date_time/date_picker.h"
#include "components/date_time/fast_calendar_picker.h"
#include "components/date_time/time_picker.h"
#include "view/pro_input_examples.h"

namespace qfw {

DateTimeInterface::DateTimeInterface(QWidget* parent)
    : GalleryInterface(Translator().dateTime(),
                       QStringLiteral("qtfluentwidgets.components.date_time"), parent) {
    setObjectName(QStringLiteral("dateTimeInterface"));
    ProInputExamples::addDateTime(this);
    auto* initialHost=new QWidget(this);
    auto* initialLayout=new QVBoxLayout(initialHost);
    initialLayout->setContentsMargins(0,0,0,0); initialLayout->setSpacing(12);
    auto* initialPicker=new PickerBase(initialHost);
    initialPicker->addColumn(tr("Priority"),{tr("Low"),tr("Normal"),tr("High")},140);
    initialPicker->setColumnInitialValue(0,tr("Normal")); initialPicker->setResetEnabled(true);
    initialLayout->addWidget(initialPicker,0,Qt::AlignLeft);
    initialLayout->addWidget(new CaptionLabel(tr("The draft starts at Normal; confirm to commit, reset to clear."),initialHost));
    addExampleCard(tr("Picker with a column initial value"),initialHost,
        QStringLiteral("https://github.com/Fairy-Oracle-Sanctuary/Qt-Fluent-Widgets"));

    for (bool fast : {false, true}) {
        auto* host = new QWidget(this);
        auto* layout = new QVBoxLayout(host);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(12);
        auto* picker = fast ? static_cast<RangeCalendarPicker*>(new FastRangeCalendarPicker(host))
                            : new RangeCalendarPicker(host);
        picker->setDateFormat("yyyy-MM-dd");
        picker->setResetEnabled(true);
        picker->setMinimumWidth(300);
        auto* weekends = new CheckBox(tr("Disable weekend endpoints"), host);
        auto* state = new BodyLabel(tr("Click two dates to select a range; hover to preview."), host);
        state->setWordWrap(true);
        layout->addWidget(picker, 0, Qt::AlignLeft);
        layout->addWidget(weekends);
        layout->addWidget(state);
        connect(weekends, &QCheckBox::toggled, picker, [picker](bool checked) {
            picker->setDisabledWeekDays(checked ? QSet<int>{6, 7} : QSet<int>{});
        });
        connect(picker, &RangeCalendarPicker::rangeChanged, state, [state, this](QDate a, QDate b) {
            state->setText(a.isValid() ? tr("Selected range: %1 - %2").arg(a.toString(Qt::ISODate), b.toString(Qt::ISODate))
                                      : tr("Click two dates to select a range; hover to preview."));
        });
        addExampleCard(fast ? tr("Fast date range picker") : tr("Date range picker"), host,
                       QStringLiteral("https://github.com/Fairy-Oracle-Sanctuary/PySide6-Fluent-Widgets-Pro"));
    }

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
