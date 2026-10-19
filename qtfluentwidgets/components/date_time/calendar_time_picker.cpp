#include "components/date_time/calendar_time_picker.h"

#include <QApplication>
#include <QKeyEvent>
#include <QPainter>
#include <QSet>
#include <QStackedWidget>
#include <QTimeZone>
#include <functional>

#include "common/color.h"
#include "common/style_sheet.h"
#include "components/date_time/calendar_view.h"
#include "components/date_time/fast_calendar_view.h"
#include "components/date_time/picker_base.h"
#include "components/widgets/cycle_list_widget.h"

namespace qfw {
namespace {
template <class Base>
class EmbeddedCalendar : public Base {
public:
    explicit EmbeddedCalendar(QWidget* parent) : Base(parent) {
        this->setObjectName("calendarTimeCalendar");
        this->setWindowFlags(Qt::Widget);
        this->setAttribute(Qt::WA_DeleteOnClose, false);
        this->layout()->setContentsMargins(0, 0, 0, 0);
        this->layout()->setSpacing(0);
        this->stackedWidget_->setGraphicsEffect(nullptr);
        const QString style =
            "CalendarViewBase {border: none; border-radius: 0; background: transparent;}";
        for (auto* view :
             {static_cast<QWidget*>(this->dayView_), static_cast<QWidget*>(this->monthView_),
              static_cast<QWidget*>(this->yearView_)})
            setCustomStyleSheet(view, style, style);
    }
    QDate selectedDate() const { return this->date_; }

protected:
    void showEvent(QShowEvent* event) override {
        Base::showEvent(event);
        // The day list has no scroll range until its embedded viewport is laid out.
        // Reapply once visible rather than keeping the constructor's clamped position.
        QTimer::singleShot(0, this, [this] { this->setDate(this->date_); });
    }
    void onDayItemClicked(const QDate& date) override {
        if (date.isValid() && date != this->date_) {
            this->setDate(date);
            emit this->dateChanged(date);
        }
    }
    void onResetted() override { emit this->resetted(); }
    void paintEvent(QPaintEvent*) override {}
};

class EmbeddedTimeMask : public ItemMaskWidget {
public:
    EmbeddedTimeMask(QList<CycleListWidget*>* columns, QWidget* parent)
        : ItemMaskWidget(columns, parent) {
        setObjectName("calendarTimeMask");
        setAttribute(Qt::WA_TransparentForMouseEvents);
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
        painter.setPen(Qt::NoPen);
        painter.setBrush(autoFallbackThemeColor(lightBackgroundColor_, darkBackgroundColor_));
        painter.drawRoundedRect(rect().adjusted(4, 0, -3, 0), 5, 5);
        painter.setPen(isDarkTheme() ? Qt::black : Qt::white);
        painter.setFont(font());
        for (auto* column : *listWidgets_) {
            auto* viewport = column->viewport();
            const int top = viewport->mapFromGlobal(mapToGlobal(QPoint())).y();
            QSet<int> seen;
            for (int y : {top, top + height() - 1}) {
                auto* item = column->itemAt(QPoint(viewport->width() / 2, y));
                if (!item || seen.contains(column->row(item))) continue;
                seen.insert(column->row(item));
                const QPoint pos =
                    mapFromGlobal(viewport->mapToGlobal(column->visualItemRect(item).topLeft()));
                painter.save();
                painter.translate(pos);
                drawText(item, &painter, 0);
                painter.restore();
            }
        }
    }
};

class EmbeddedTimePanel : public PickerPanel {
public:
    EmbeddedTimePanel(QWidget* parent, bool seconds) : PickerPanel(parent) {
        setObjectName("calendarTimeWheelsPanel");
        setWindowFlags(Qt::Widget);
        view_->setGraphicsEffect(nullptr);
        view_->setObjectName("calendarTimeWheels");
        delete itemMaskWidget_;
        itemMaskWidget_ = new EmbeddedTimeMask(&listWidgets_, this);
        hBoxLayout_->setContentsMargins(9, 11, 9, 11);
        hBoxLayout_->setSpacing(0);
        listLayout_->setSpacing(0);
        buttonLayout_->setContentsMargins(0, 0, 0, 0);
        buttonLayout_->setSpacing(0);
        yesButton_->hide();
        cancelButton_->hide();
        resetButton_->hide();
        hSeparatorWidget_->hide();
        QStringList hours, minutes;
        for (int i = 0; i < 24; ++i) hours << QString::number(i);
        for (int i = 0; i < 60; ++i) minutes << QString::number(i);
        addColumn(hours, seconds ? 80 : 120);
        addColumn(minutes, seconds ? 80 : 120);
        if (seconds) addColumn(minutes, 80);
    }
};
}  // namespace

class CalendarTimeView : public FlyoutViewBase {
    Q_OBJECT
public:
    CalendarTimeView(const QDateTime& initial, bool fast, bool seconds, bool resetEnabled,
                     int minYear, int maxYear, QWidget* parent)
        : FlyoutViewBase(parent), initial_(initial) {
        setObjectName("calendarTimeView");
        QWidget* calendar = nullptr;
        if (fast) {
            auto* view = new EmbeddedCalendar<FastCalendarView>(this);
            view->setResetEnabled(resetEnabled);
            view->setYearRange(minYear, maxYear);
            view->setDate(initial.date());
            date_ = [view] { return view->selectedDate(); };
            connect(view, &FastCalendarView::resetted, this, &CalendarTimeView::resetted);
            calendar = view;
        } else {
            auto* view = new EmbeddedCalendar<CalendarView>(this);
            view->setResetEnabled(resetEnabled);
            view->setYearRange(minYear, maxYear);
            view->setDate(initial.date());
            date_ = [view] { return view->selectedDate(); };
            connect(view, &CalendarView::resetted, this, &CalendarTimeView::resetted);
            calendar = view;
        }
        panel_ = new EmbeddedTimePanel(this, seconds);
        QStringList values{QString::number(initial.time().hour()),
                           QString::number(initial.time().minute())};
        if (seconds) values << QString::number(initial.time().second());
        panel_->setValue(values);
        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(1, 1, 1, 1);
        layout->setSpacing(0);
        auto* body = new QHBoxLayout;
        body->setContentsMargins(0, 0, 0, 0);
        body->setSpacing(0);
        body->addWidget(calendar);
        body->addWidget(new SeparatorWidget(Qt::Vertical, this));
        body->addWidget(panel_);
        layout->addLayout(body);
        layout->addWidget(new SeparatorWidget(Qt::Horizontal, this));
        auto* buttons = new QWidget(this);
        buttons->setFixedHeight(36);
        auto* row = new QHBoxLayout(buttons);
        row->setContentsMargins(3, 3, 3, 3);
        row->setSpacing(0);
        auto* confirm = new TransparentToolButton(FluentIconEnum::Accept, buttons);
        confirm->setObjectName("calendarTimeConfirm");
        auto* cancel = new TransparentToolButton(FluentIconEnum::Close, buttons);
        cancel->setObjectName("calendarTimeCancel");
        confirm->setAccessibleName(CalendarTimePicker::tr("Confirm"));
        cancel->setAccessibleName(CalendarTimePicker::tr("Cancel"));
        for (auto* button : {confirm, cancel}) {
            button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            row->addWidget(button, 1);
        }
        layout->addWidget(buttons);
        connect(confirm, &QAbstractButton::clicked, this, [this] { emit confirmed(dateTime()); });
        connect(cancel, &QAbstractButton::clicked, this, &CalendarTimeView::cancelled);
    }
    void addWidget(QWidget* widget, int stretch, Qt::Alignment alignment) override {
        layout()->addWidget(widget);
        Q_UNUSED(stretch);
        Q_UNUSED(alignment);
    }
    QDateTime dateTime() const {
        const QStringList values = panel_->value();
        QDateTime result = initial_;
        result.setDate(date_());
        result.setTime(QTime(values.value(0).toInt(), values.value(1).toInt(),
                             values.size() == 3 ? values[2].toInt() : 0));
        return result;
    }
signals:
    void confirmed(const QDateTime& value);
    void cancelled();
    void resetted();

protected:
    void showEvent(QShowEvent* event) override {
        qApp->installEventFilter(this);
        FlyoutViewBase::showEvent(event);
    }
    void hideEvent(QHideEvent* event) override {
        qApp->removeEventFilter(this);
        FlyoutViewBase::hideEvent(event);
    }
    bool eventFilter(QObject* object, QEvent* event) override {
        if (isVisible()) {
            if (event->type() == QEvent::MouseButtonPress) {
                auto* mouse = static_cast<QMouseEvent*>(event);
                if (!rect().contains(mapFromGlobal(mouse->QMouseEvent_globalPosition_toPoint()))) {
                    window()->close();
                    return true;
                }
            } else if (event->type() == QEvent::KeyPress) {
                const int key = static_cast<QKeyEvent*>(event)->key();
                if (key == Qt::Key_Escape) {
                    emit cancelled();
                    return true;
                }
                if (key == Qt::Key_Return || key == Qt::Key_Enter) {
                    emit confirmed(dateTime());
                    return true;
                }
            }
        }
        return FlyoutViewBase::eventFilter(object, event);
    }

private:
    QDateTime initial_;
    std::function<QDate()> date_;
    EmbeddedTimePanel* panel_;
};

CalendarTimePicker::CalendarTimePicker(QWidget* parent, bool seconds)
    : CalendarTimePicker(false, parent, seconds) {}
CalendarTimePicker::CalendarTimePicker(bool fast, QWidget* parent, bool seconds)
    : CalendarPicker(parent), fast_(fast), showSeconds_(seconds) {
    format_ = seconds ? "yyyy-MM-dd HH:mm:ss" : "yyyy-MM-dd HH:mm";
    updateText();
}
FastCalendarTimePicker::FastCalendarTimePicker(QWidget* parent, bool seconds)
    : CalendarTimePicker(true, parent, seconds) {}
CalendarTimePicker::~CalendarTimePicker() { closePopup(); }
void CalendarTimePicker::setDateTime(const QDateTime& value) {
    if (value == value_ && value.timeZone() == value_.timeZone()) return;
    const QDateTime previous = value_;
    value_ = value;
    updateText();
    QPointer<CalendarTimePicker> guard(this);
    emit dateTimeChanged(value);
    if (!guard || value_ != value || value_.timeZone() != value.timeZone()) return;
    if (previous.date() != value.date()) emit dateChanged(value.date());
    if (!guard || value_ != value || value_.timeZone() != value.timeZone()) return;
    if (previous.time() != value.time()) emit timeChanged(value.time());
}
void CalendarTimePicker::setDate(const QDate& date) {
    if (!date.isValid()) {
        reset();
        return;
    }
    QDateTime next = value_.isValid() ? value_ : QDateTime(date, QTime(0, 0));
    next.setDate(date);
    setDateTime(next);
}
void CalendarTimePicker::setTime(const QTime& time) {
    if (!time.isValid()) {
        reset();
        return;
    }
    QDateTime next = value_.isValid() ? value_ : QDateTime(QDate::currentDate(), time);
    next.setTime(time);
    setDateTime(next);
}
void CalendarTimePicker::setDateTimeFormat(const QString& format) {
    format_ = format;
    updateText();
}
void CalendarTimePicker::setDateFormat(Qt::DateFormat format) {
    CalendarPicker::setDateFormat(format);
    format_ = format == Qt::ISODateWithMs ? "yyyy-MM-dd HH:mm:ss.zzz"
              : format == Qt::TextDate    ? "ddd MMM d yyyy HH:mm:ss"
                                          : "yyyy-MM-dd HH:mm:ss";
    updateText();
}
void CalendarTimePicker::setSecondVisible(bool visible) {
    if (showSeconds_ == visible) return;
    showSeconds_ = visible;
    if (format_ == "yyyy-MM-dd HH:mm:ss" || format_ == "yyyy-MM-dd HH:mm")
        setDateTimeFormat(visible ? "yyyy-MM-dd HH:mm:ss" : "yyyy-MM-dd HH:mm");
    closePopup();  // The next popup uses the new wheel structure.
}
void CalendarTimePicker::reset() { setDateTime(QDateTime()); }
void CalendarTimePicker::updateText() {
    setText(value_.isValid() ? value_.toString(format_) : tr("Pick a date and time"));
    setProperty("hasDate", value_.isValid());
    setStyle(QApplication::style());
    updateGeometry();
    update();
}
void CalendarTimePicker::closePopup() {
    if (flyout_) {
        flyout_->close();
        flyout_.clear();
    }
}
void CalendarTimePicker::showCalendarView() {
    if (!isEnabled() || !isVisible()) return;
    closePopup();
    const QDateTime initial = value_.isValid() ? value_ : QDateTime::currentDateTime();
    const int year = QDate::currentDate().year();
    auto* view = new CalendarTimeView(initial, fast_, showSeconds_, isResetEnabled(),
                                      year - minYearRange_, year + maxYearRange_, window());
    auto* popup =
        Flyout::make(view, QVariant::fromValue(static_cast<QWidget*>(this)), window(), animation_);
    flyout_ = popup;
    QPointer<Flyout> popupGuard(popup);
    connect(view, &CalendarTimeView::confirmed, this, [this, popupGuard](const QDateTime& value) {
        setDateTime(value);
        if (popupGuard) popupGuard->close();
    });
    connect(view, &CalendarTimeView::cancelled, popup, &Flyout::close);
    connect(view, &CalendarTimeView::resetted, this, [this, popupGuard] {
        reset();
        if (popupGuard) popupGuard->close();
    });
}
void CalendarTimePicker::paintEvent(QPaintEvent* event) {
    QPushButton::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    if (!value_.isValid()) painter.setOpacity(.6);
    if (!isEnabled()) painter.setOpacity(.36);
    FluentIcon(FluentIconEnum::DateTime)
        .render(&painter, QRect(width() - 23, height() / 2 - 6, 12, 12));
}
void CalendarTimePicker::hideEvent(QHideEvent* event) {
    closePopup();
    CalendarPicker::hideEvent(event);
}
void CalendarTimePicker::changeEvent(QEvent* event) {
    if (!isEnabled()) closePopup();
    if (event->type() == QEvent::LanguageChange) updateText();
    CalendarPicker::changeEvent(event);
}
}  // namespace qfw
#include "calendar_time_picker.moc"
