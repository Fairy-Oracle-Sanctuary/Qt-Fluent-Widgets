#include "view/pro_input_examples.h"

#include <QButtonGroup>
#include <QDir>
#include <QRegularExpressionValidator>

#include "qtfluentwidgets.h"
#include "view/gallery_interface.h"

namespace qfw {
namespace {
QString sourcePath() {
    return QStringLiteral(
        "https://github.com/Fairy-Oracle-Sanctuary/Qt-Fluent-Widgets/blob/main/app/view/"
        "pro_input_examples.cpp");
}
QWidget* column(int maximumWidth) {
    auto* host = new QWidget;
    host->setMaximumWidth(maximumWidth);
    auto* layout = new QVBoxLayout(host);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);
    return host;
}
}  // namespace
void ProInputExamples::addBasicInput(GalleryInterface* gallery) {
    addDropWidgets(gallery);
    auto* numeric = column(360);
    auto* numericLayout = static_cast<QVBoxLayout*>(numeric->layout());
    auto* rating = new RatingWidget(8.6, numeric);
    auto* score = new DoubleSpinBox(numeric);
    score->setRange(0, 10);
    score->setDecimals(1);
    score->setSingleStep(0.1);
    score->setValue(8.6);
    score->setFixedWidth(140);
    connect(score, qOverload<double>(&QDoubleSpinBox::valueChanged), rating,
            &RatingWidget::setValue);
    numericLayout->addWidget(rating, 0, Qt::AlignLeft);
    numericLayout->addWidget(score, 0, Qt::AlignLeft);
    gallery->addExampleCard(tr("Numeric rating with a vector star"), numeric, sourcePath(), 1);

    auto* interactive = column(400);
    auto* interactiveLayout = static_cast<QVBoxLayout*>(interactive->layout());
    auto* stars = new InteractiveRatingWidget(3.5, interactive);
    auto* selected = new BodyLabel(tr("Rating: %1 / 5").arg(stars->text()), interactive);
    auto* readOnly = new CheckBox(tr("Read only"), interactive);
    connect(readOnly, &QCheckBox::toggled, stars, &InteractiveRatingWidget::setReadOnly);
    connect(stars, &RatingWidget::valueChanged, selected,
            [selected, stars] { selected->setText(tr("Rating: %1 / 5").arg(stars->text())); });
    interactiveLayout->addWidget(stars, 0, Qt::AlignLeft);
    interactiveLayout->addWidget(selected);
    interactiveLayout->addWidget(readOnly, 0, Qt::AlignLeft);
    gallery->addExampleCard(tr("Continuous interactive rating"), interactive, sourcePath(), 1);

    auto* sliderHost = column(400);
    auto* sliderLayout = static_cast<QVBoxLayout*>(sliderHost->layout());
    auto* slider = new ToolTipSlider(Qt::Horizontal, sliderHost);
    slider->setRange(0, 100);
    slider->setValue(35);
    slider->setMaximumWidth(320);
    auto* current = new BodyLabel(tr("Value: %1").arg(slider->value()), sliderHost);
    connect(slider, &QSlider::valueChanged, current,
            [current](int value) { current->setText(tr("Value: %1").arg(value)); });
    sliderLayout->addWidget(slider);
    sliderLayout->addWidget(current);
    gallery->addExampleCard(tr("Slider with a value tooltip"), sliderHost, sourcePath(), 1);

    auto* rangeHost = column(420);
    auto* rangeLayout = static_cast<QVBoxLayout*>(rangeHost->layout());
    auto* range = new RangeSlider(Qt::Horizontal, rangeHost);
    range->setRange(0, 100);
    range->setValues(20, 80);
    auto* interval = new BodyLabel(
        tr("Selected range: %1 – %2").arg(range->minValue()).arg(range->maxValue()), rangeHost);
    connect(range, &RangeSlider::rangeChanged, interval, [interval](int low, int high) {
        interval->setText(tr("Selected range: %1 – %2").arg(low).arg(high));
    });
    rangeLayout->addWidget(range);
    rangeLayout->addWidget(interval);
    gallery->addExampleCard(tr("Two-handle range slider"), rangeHost, sourcePath(), 1);

    auto* verticalHost = new QWidget;
    auto* verticalLayout = new QHBoxLayout(verticalHost);
    verticalLayout->setContentsMargins(0, 0, 0, 0);
    verticalLayout->setSpacing(30);
    auto* vertical = new RangeSlider(Qt::Vertical, verticalHost);
    vertical->setRange(-50, 50);
    vertical->setValues(-20, 30);
    vertical->setFixedHeight(180);
    auto* verticalTip = new ToolTipSlider(Qt::Vertical, verticalHost);
    verticalTip->setRange(0, 100);
    verticalTip->setValue(40);
    verticalTip->setFixedSize(22, 180);
    verticalLayout->addWidget(vertical);
    verticalLayout->addWidget(verticalTip);
    gallery->addExampleCard(tr("Vertical extended sliders"), verticalHost, sourcePath());

    auto* options = column(400);
    auto* optionsLayout = static_cast<QVBoxLayout*>(options->layout());
    auto* subtitleCheck = new SubtitleCheckBox(
        tr("Keep local history"),
        tr("Store recent conversations on this device. You can clear them at any time."), options);
    subtitleCheck->setChecked(true);
    optionsLayout->addWidget(subtitleCheck);
    auto* enabled = new CheckBox(tr("Enable option"), options);
    enabled->setChecked(true);
    connect(enabled, &QCheckBox::toggled, subtitleCheck, &QWidget::setEnabled);
    optionsLayout->addWidget(enabled, 0, Qt::AlignLeft);
    gallery->addExampleCard(tr("Checkbox with a wrapping subtitle"), options, sourcePath(), 1);

    auto* choices = column(400);
    auto* choicesLayout = static_cast<QVBoxLayout*>(choices->layout());
    auto* radios = new QButtonGroup(choices);
    auto* light = new SubtitleRadioButton(
        tr("Light appearance"), tr("Use a bright background for this workspace."), choices);
    auto* dark = new SubtitleRadioButton(tr("Dark appearance"),
                                         tr("Use a dark background for this workspace."), choices);
    radios->addButton(light);
    radios->addButton(dark);
    light->setChecked(true);
    choicesLayout->addWidget(light);
    choicesLayout->addWidget(dark);
    gallery->addExampleCard(tr("Exclusive radio buttons with subtitles"), choices, sourcePath(), 1);

    auto* compact = column(300);
    auto* compactLayout = static_cast<QVBoxLayout*>(compact->layout());
    auto* integer = new CompactSpinBox(compact);
    integer->setRange(0, 100);
    integer->setValue(42);
    integer->setFixedWidth(160);
    auto* decimal = new CompactDoubleSpinBox(compact);
    decimal->setRange(0, 10);
    decimal->setValue(3.5);
    decimal->setSingleStep(0.1);
    decimal->setFixedWidth(160);
    auto* lock = new CheckBox(tr("Read only"), compact);
    connect(lock, &QCheckBox::toggled, integer, &CompactSpinBox::setReadOnly);
    connect(lock, &QCheckBox::toggled, decimal, &CompactDoubleSpinBox::setReadOnly);
    compactLayout->addWidget(integer, 0, Qt::AlignLeft);
    compactLayout->addWidget(decimal, 0, Qt::AlignLeft);
    compactLayout->addWidget(lock, 0, Qt::AlignLeft);
    gallery->addExampleCard(tr("Compact numeric editors with stepping flyouts"), compact,
                            sourcePath(), 1);
}
void ProInputExamples::addDropWidgets(GalleryInterface* gallery) {
    const QList<QPair<QString, DropMultiFilesWidget*>> examples = {
        {tr("Drop multiple files"), new DropMultiFilesWidget},
        {tr("Drop a single file"), new DropSingleFileWidget},
        {tr("Drop a single folder"), new DropSingleFolderWidget},
        {tr("Drop multiple folders"), new DropMultiFoldersWidget},
        {tr("Drop files and folders together"), new DropAnyWidget}};
    for (const auto& example : examples) {
        auto* host = column(400);
        auto* layout = static_cast<QVBoxLayout*>(host->layout());
        auto* drop = example.second;
        drop->setFixedSize(360, 200);
        drop->setDefaultDir(QDir::homePath());
        auto* status = new BodyLabel(tr("No paths selected"), host);
        status->setTextFormat(Qt::PlainText);
        status->setWordWrap(true);
        status->setTextInteractionFlags(Qt::TextSelectableByMouse);
        auto* dashed = new CheckBox(tr("Dashed border"), host);
        dashed->setChecked(true);
        connect(dashed, &QCheckBox::toggled, drop, &DropMultiFilesWidget::enableDashLine);
        const auto report = [status](const QStringList& paths) {
            status->setText(tr("Selected %1 path(s):\n%2").arg(paths.size()).arg(paths.join('\n')));
        };
        connect(drop, &DropMultiFilesWidget::draggedChange, status, report);
        connect(drop, &DropMultiFilesWidget::selectionChange, status, report);
        layout->addWidget(drop, 0, Qt::AlignLeft);
        layout->addWidget(dashed, 0, Qt::AlignLeft);
        layout->addWidget(status);
        gallery->addExampleCard(example.first, host, sourcePath(), 1);
    }
}
void ProInputExamples::addText(GalleryInterface* gallery) {
    auto* labelEdit = new LabelLineEdit(QStringLiteral("https://"), QStringLiteral(".org"));
    labelEdit->setFixedWidth(360);
    labelEdit->setText(QStringLiteral("fairy"));
    labelEdit->setClearButtonEnabled(true);
    connect(labelEdit->suffixButton(), &QAbstractButton::clicked, labelEdit, [labelEdit] {
        labelEdit->setSuffix(labelEdit->suffix() == QStringLiteral(".org")
                                 ? QStringLiteral(".net")
                                 : QStringLiteral(".org"));
    });
    gallery->addExampleCard(tr("Line edit with prefix and suffix buttons"), labelEdit,
                            sourcePath());

    auto* pinHost = column(400);
    auto* pinLayout = static_cast<QVBoxLayout*>(pinHost->layout());
    auto* pin = new PinBox(pinHost);
    pin->setPinBoxCount(6);
    pin->setPinBoxValidator(
        new QRegularExpressionValidator(QRegularExpression(QStringLiteral("[0-9]")), pin));
    pin->setText(QStringLiteral("123456"));
    auto* status = new BodyLabel(
        tr("Entered %1 of %2 characters").arg(pin->text().size()).arg(pin->count()), pinHost);
    auto* password = new CheckBox(tr("Hide characters"), pinHost);
    auto* clear = new PushButton(tr("Clear PIN"), pinHost);
    connect(password, &QCheckBox::toggled, pin, [pin](bool checked) {
        pin->setEchoMode(checked ? QLineEdit::Password : QLineEdit::Normal);
    });
    connect(clear, &QAbstractButton::clicked, pin, &PinBox::clear);
    connect(pin, &PinBox::textChanged, status, [pin, status] {
        status->setText(
            tr("Entered %1 of %2 characters").arg(pin->text().size()).arg(pin->count()));
    });
    pinLayout->addWidget(pin, 0, Qt::AlignLeft);
    pinLayout->addWidget(status);
    pinLayout->addWidget(password, 0, Qt::AlignLeft);
    pinLayout->addWidget(clear, 0, Qt::AlignLeft);
    gallery->addExampleCard(tr("PIN input with automatic focus navigation"), pinHost, sourcePath(),
                            1);
}
void ProInputExamples::addDateTime(GalleryInterface* gallery) {
    const QDate initialDate = QDate::currentDate();
    const QTime initialTime(12, 30, 0);
    auto* combinedHost = column(430);
    auto* combinedLayout = static_cast<QVBoxLayout*>(combinedHost->layout());
    auto* combined = new CalendarTimePicker(combinedHost);
    auto* fastCombined = new FastCalendarTimePicker(combinedHost);
    combined->setFixedWidth(320);
    fastCombined->setFixedWidth(320);
    combined->setResetEnabled(true);
    fastCombined->setResetEnabled(true);
    combined->setDateTime(QDateTime(initialDate, initialTime));
    fastCombined->setDateTime(QDateTime(initialDate, initialTime));
    auto* combinedStatus = new BodyLabel(combinedHost);
    const auto updateCombined = [combined, fastCombined, combinedStatus] {
        combinedStatus->setText(tr("Confirmed values: %1 / %2")
            .arg(combined->dateTime().isValid() ? combined->dateTime().toString("yyyy-MM-dd HH:mm:ss") : tr("Not selected"))
            .arg(fastCombined->dateTime().isValid() ? fastCombined->dateTime().toString("yyyy-MM-dd HH:mm:ss") : tr("Not selected")));
    };
    connect(combined, &CalendarTimePicker::dateTimeChanged, combinedStatus, updateCombined);
    connect(fastCombined, &CalendarTimePicker::dateTimeChanged, combinedStatus, updateCombined);
    updateCombined();
    combinedStatus->setWordWrap(true);
    auto* seconds = new CheckBox(tr("Show seconds in the calendar time picker"), combinedHost);
    seconds->setChecked(true);
    connect(seconds, &QCheckBox::toggled, combined, &CalendarTimePicker::setSecondVisible);
    connect(seconds, &QCheckBox::toggled, fastCombined, &CalendarTimePicker::setSecondVisible);
    auto* combinedHint = new BodyLabel(tr("Choose a date and scroll the time wheels, then confirm "
        "once. Cancel or Esc keeps the original value. The second picker uses the fast calendar."), combinedHost);
    combinedHint->setWordWrap(true);
    combinedLayout->addWidget(combined, 0, Qt::AlignLeft);
    combinedLayout->addWidget(fastCombined, 0, Qt::AlignLeft);
    combinedLayout->addWidget(combinedStatus);
    combinedLayout->addWidget(seconds);
    combinedLayout->addWidget(combinedHint);
    gallery->addExampleCard(tr("Calendar and time pickers with atomic confirmation"), combinedHost, sourcePath(), 1);

    auto* inlineHost = column(330);
    auto* inlineLayout = static_cast<QVBoxLayout*>(inlineHost->layout());
    auto* date = new DateEdit(inlineHost);
    date->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));
    date->setDate(initialDate);
    date->setFixedWidth(240);
    auto* time = new TimeEdit(inlineHost);
    time->setDisplayFormat(QStringLiteral("HH:mm:ss"));
    time->setTime(initialTime);
    time->setFixedWidth(240);
    auto* dateTime = new DateTimeEdit(inlineHost);
    dateTime->setDisplayFormat(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    dateTime->setDateTime(QDateTime(initialDate, initialTime));
    dateTime->setFixedWidth(320);
    auto* lock = new CheckBox(tr("Read only"), inlineHost);
    connect(lock, &QCheckBox::toggled, date, &DateEdit::setReadOnly);
    connect(lock, &QCheckBox::toggled, time, &TimeEdit::setReadOnly);
    connect(lock, &QCheckBox::toggled, dateTime, &DateTimeEdit::setReadOnly);
    inlineLayout->addWidget(date, 0, Qt::AlignLeft);
    inlineLayout->addWidget(time, 0, Qt::AlignLeft);
    inlineLayout->addWidget(dateTime, 0, Qt::AlignLeft);
    inlineLayout->addWidget(lock, 0, Qt::AlignLeft);
    gallery->addExampleCard(tr("Date and time editors with inline step buttons"), inlineHost,
                            sourcePath(), 1);

    auto* compactHost = column(330);
    auto* compactLayout = static_cast<QVBoxLayout*>(compactHost->layout());
    auto* compactDate = new CompactDateEdit(compactHost);
    compactDate->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));
    compactDate->setDate(initialDate);
    compactDate->setFixedWidth(180);
    auto* compactTime = new CompactTimeEdit(compactHost);
    compactTime->setDisplayFormat(QStringLiteral("HH:mm:ss"));
    compactTime->setTime(initialTime);
    compactTime->setFixedWidth(180);
    auto* compactDateTime = new CompactDateTimeEdit(compactHost);
    compactDateTime->setDisplayFormat(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    compactDateTime->setDateTime(QDateTime(initialDate, initialTime));
    compactDateTime->setFixedWidth(260);
    auto* compactLock = new CheckBox(tr("Read only"), compactHost);
    connect(compactLock, &QCheckBox::toggled, compactDate, &CompactDateEdit::setReadOnly);
    connect(compactLock, &QCheckBox::toggled, compactTime, &CompactTimeEdit::setReadOnly);
    connect(compactLock, &QCheckBox::toggled, compactDateTime, &CompactDateTimeEdit::setReadOnly);
    auto* instructions = new BodyLabel(
        tr("Select a date or time field, then use the arrows to adjust it."), compactHost);
    instructions->setWordWrap(true);
    compactLayout->addWidget(compactDate, 0, Qt::AlignLeft);
    compactLayout->addWidget(compactTime, 0, Qt::AlignLeft);
    compactLayout->addWidget(compactDateTime, 0, Qt::AlignLeft);
    compactLayout->addWidget(instructions);
    compactLayout->addWidget(compactLock, 0, Qt::AlignLeft);
    gallery->addExampleCard(tr("Compact date and time editors with stepping flyouts"), compactHost,
                            sourcePath(), 1);
}
}  // namespace qfw
