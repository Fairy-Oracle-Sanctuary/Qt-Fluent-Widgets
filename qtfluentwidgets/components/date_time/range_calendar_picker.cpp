#include "components/date_time/range_calendar_picker.h"

#include <QApplication>
#include <QKeyEvent>
#include <QPainter>
#include <QStackedWidget>
#include <QTimer>
#include <functional>
#include <stdexcept>
#include <type_traits>
#include "common/style_sheet.h"
#include "components/date_time/calendar_view.h"
#include "components/date_time/fast_calendar_view.h"

namespace qfw {
namespace {
QPair<QDate, QDate> ordered(QDate a, QDate b) {
    return a <= b ? qMakePair(a, b) : qMakePair(b, a);
}

class RangeDayDelegate : public DayScrollItemDelegate {
public:
    explicit RangeDayDelegate(ScrollViewBase* scroll)
        : DayScrollItemDelegate(QDate(), QDate(), scroll), scroll_(scroll) {}
    QDate start, end;
    QSet<int> disabled;
protected:
    void drawBackground(QPainter* p, const QStyleOptionViewItem& o,
                        const QModelIndex& i) const override {
        const QDate d = i.data(Qt::UserRole).toDate();
        if (!d.isValid()) return;
        p->save();
        if (start.isValid() && start <= d && d <= end) {
            QRectF band = QRectF(o.rect).adjusted(0, 3, 0, -3);
            const bool rtl = o.direction == Qt::RightToLeft;
            if (d == start) {
                if (rtl) band.setRight(QRectF(o.rect).center().x());
                else band.setLeft(QRectF(o.rect).center().x());
            }
            if (d == end) {
                if (rtl) band.setLeft(QRectF(o.rect).center().x());
                else band.setRight(QRectF(o.rect).center().x());
            }
            p->fillRect(band, isDarkTheme() ? QColor(255, 255, 255, 28) : QColor(0, 0, 0, 12));
        }
        const bool endpoint = d == start || d == end;
        p->setPen(endpoint && d != QDate::currentDate() ? QPen(themeColor(), 1.4) : QPen(Qt::NoPen));
        if (d == QDate::currentDate()) p->setBrush(themeColor());
        else if (endpoint) p->setBrush(isDarkTheme() ? QColor(61, 61, 61) : QColor(243, 243, 243));
        else if (o.state & QStyle::State_MouseOver)
            p->setBrush(isDarkTheme() ? QColor(255, 255, 255, 9) : QColor(0, 0, 0, 9));
        else p->setBrush(Qt::transparent);
        p->drawEllipse(QRectF(o.rect).adjusted(3, 3, -3, -3));
        p->restore();
    }
    void drawText(QPainter* p, const QStyleOptionViewItem& o,
                  const QModelIndex& i) const override {
        const QDate d = i.data(Qt::UserRole).toDate();
        if (!d.isValid()) return;
        const auto range = scroll_->currentPageRange();
        const bool inRange = range.first <= d && d <= range.second;
        const bool blocked = disabled.contains(d.dayOfWeek());
        p->save();
        QFont f = font_;
        f.setStrikeOut(blocked && inRange);
        p->setFont(f);
        if (blocked) {
            p->setPen(isDarkTheme() ? Qt::white : Qt::black);
            p->setOpacity(inRange ? .4 : .6);
        } else if (d == QDate::currentDate()) p->setPen(isDarkTheme() ? Qt::black : Qt::white);
        else if (d == start || d == end) p->setPen(themeColor());
        else {
            p->setPen(isDarkTheme() ? Qt::white : Qt::black);
            if (!inRange) p->setOpacity(.6);
        }
        p->drawText(o.rect, Qt::AlignCenter, i.data(Qt::DisplayRole).toString());
        p->restore();
    }
private:
    ScrollViewBase* scroll_;
};

template<class Base>
class RangeCalendarView : public Base {
public:
    RangeCalendarView(QDate start, QDate end, QSet<int> days, QWidget* parent)
        : Base(parent), initial_(start) {
        this->setObjectName("rangeCalendarView");
        // Popup windows do not automatically inherit their owner's direction.
        this->setLayoutDirection(parent ? parent->layoutDirection() : qApp->layoutDirection());
        scroll_ = this->dayView_->scrollView();
        scroll_->setObjectName("rangeCalendarDays");
        delegate_ = new RangeDayDelegate(scroll_);
        delegate_->start = start;
        delegate_->end = end;
        delegate_->disabled = days;
        // Keep the calendar's own delegate alive for its navigation bookkeeping.
        scroll_->setItemDelegate(delegate_);
        scroll_->setMouseTracking(true);
        scroll_->viewport()->setMouseTracking(true);
        scroll_->viewport()->installEventFilter(this);
    }
    ~RangeCalendarView() override { qApp->removeEventFilter(this); }
    std::function<void(QDate, QDate)> committed;
protected:
    void onDayItemClicked(const QDate& date) override {
        if (!selectable(date)) return;
        if (!anchor_.isValid()) {
            anchor_ = date;
            highlight(date, date);
        } else {
            const auto range = ordered(anchor_, date);
            anchor_ = QDate();
            highlight(range.first, range.second);
            const auto callback = committed;
            if (callback) callback(range.first, range.second);
        }
    }
    void showEvent(QShowEvent* e) override {
        Base::showEvent(e);
        qApp->installEventFilter(this);
        QTimer::singleShot(0, this, [this] {
            this->setDate(initial_.isValid() ? initial_ : QDate::currentDate());
        });
    }
    void hideEvent(QHideEvent* e) override {
        qApp->removeEventFilter(this);
        Base::hideEvent(e);
    }
    bool eventFilter(QObject* object, QEvent* e) override {
        if (this->isVisible()) {
            if (e->type() == QEvent::KeyPress && static_cast<QKeyEvent*>(e)->key() == Qt::Key_Escape) {
                this->window()->close();
                return true;
            }
            if (e->type() == QEvent::MouseButtonPress) {
                auto* mouse = static_cast<QMouseEvent*>(e);
                if (!this->stackedWidget_->rect().contains(this->stackedWidget_->mapFromGlobal(
                        mouse->QMouseEvent_globalPosition_toPoint()))) {
                    this->window()->close();
                    return true;
                }
            }
        }
        if (object == scroll_->viewport() && anchor_.isValid()) {
            if (e->type() == QEvent::MouseMove) {
                const auto index = scroll_->indexAt(static_cast<QMouseEvent*>(e)->pos());
                const QDate d = index.data(Qt::UserRole).toDate();
                if (selectable(d)) highlight(anchor_, d);
            } else if (e->type() == QEvent::Leave) highlight(anchor_, anchor_);
        }
        return Base::eventFilter(object, e);
    }
private:
    bool selectable(QDate d) const { return d.isValid() && !delegate_->disabled.contains(d.dayOfWeek()); }
    void highlight(QDate a, QDate b) {
        const auto range = ordered(a, b);
        delegate_->start = range.first;
        delegate_->end = range.second;
        scroll_->viewport()->update();
    }
    QDate initial_, anchor_;
    ScrollViewBase* scroll_;
    RangeDayDelegate* delegate_;
};
} // namespace

RangeCalendarPicker::RangeCalendarPicker(QWidget* parent) : RangeCalendarPicker(false, parent) {}
RangeCalendarPicker::RangeCalendarPicker(bool fast, QWidget* parent) : CalendarPicker(parent), fast_(fast) {
    updateText();
}
FastRangeCalendarPicker::FastRangeCalendarPicker(QWidget* parent) : RangeCalendarPicker(true, parent) {}
RangeCalendarPicker::~RangeCalendarPicker() { closePopup(); }
void RangeCalendarPicker::setDateRange(const QDate& start, const QDate& end) {
    if (!start.isValid() && !end.isValid()) { reset(); return; }
    if (!start.isValid() || !end.isValid()) throw std::invalid_argument("Both range endpoints must be valid");
    if (disabled_.contains(start.dayOfWeek()) || disabled_.contains(end.dayOfWeek()))
        throw std::invalid_argument("Range endpoints cannot fall on a disabled weekday");
    const auto range = ordered(start, end);
    if (dateRange() == range) return;
    start_ = range.first;
    end_ = range.second;
    updateText();
    emit rangeChanged(range.first, range.second);
}
void RangeCalendarPicker::reset() {
    const bool hadRange = start_.isValid();
    start_ = end_ = QDate();
    updateText();
    if (hadRange) emit rangeChanged(QDate(), QDate());
}
void RangeCalendarPicker::setDisabledWeekDays(const QSet<int>& days) {
    for (int day : days) if (day < 1 || day > 7) throw std::invalid_argument("Weekdays must be between 1 and 7");
    if (disabled_ == days) return;
    disabled_ = days;
    closePopup();
    if (days.contains(start_.dayOfWeek()) || days.contains(end_.dayOfWeek())) reset();
}
void RangeCalendarPicker::setDateFormat(Qt::DateFormat format) {
    CalendarPicker::setDateFormat(format);
    format_.clear();
    updateText();
}
void RangeCalendarPicker::setDateFormat(const QString& format) { format_ = format; updateText(); }
void RangeCalendarPicker::updateText() {
    const auto formatted = [this](QDate d) { return format_.isEmpty() ? d.toString(dateFormat()) : d.toString(format_); };
    setText(start_.isValid() ? formatted(start_) + " - " + formatted(end_) : tr("Pick a date range"));
    setProperty("hasDate", start_.isValid());
    setStyle(QApplication::style());
    updateGeometry();
    update();
}
void RangeCalendarPicker::closePopup() {
    if (popup_) { popup_->close(); popup_.clear(); }
}
void RangeCalendarPicker::showCalendarView() {
    if (!isEnabled() || !isVisible()) return;
    closePopup();
    const int year = QDate::currentDate().year();
    const QPointer<RangeCalendarPicker> guard(this);
    const auto setup = [this, year, guard](auto* view, QWidget* popup) {
        view->setResetEnabled(isResetEnabled());
        view->setYearRange(year - minYearRange_, year + maxYearRange_);
        if (start_.isValid()) view->setDate(start_);
        const QPointer<QWidget> popupGuard(popup);
        view->committed = [guard, popupGuard](QDate start, QDate end) {
            if (guard) guard->setDateRange(start, end);
            if (popupGuard) popupGuard->close();
        };
        using View = std::remove_pointer_t<decltype(view)>;
        connect(view, &View::resetted, this, [guard, popupGuard] {
            if (guard) guard->reset();
            if (popupGuard) popupGuard->close();
        });
        popup_ = popup;
    };
    if (fast_) {
        auto* view = new RangeCalendarView<FastCalendarView>(start_, end_, disabled_, window());
        // Initialize before Flyout::make, which shows its content immediately.
        view->setYearRange(year - minYearRange_, year + maxYearRange_);
        auto* popup = Flyout::make(view, QVariant::fromValue(static_cast<QWidget*>(this)), window(), animation_);
        setup(view, popup);
    } else {
        auto* view = new RangeCalendarView<CalendarView>(start_, end_, disabled_, window());
        setup(view, view);
        view->exec(mapToGlobal(QPoint(width() / 2 - view->sizeHint().width() / 2, height())));
    }
}
void RangeCalendarPicker::hideEvent(QHideEvent* e) { closePopup(); CalendarPicker::hideEvent(e); }
void RangeCalendarPicker::changeEvent(QEvent* e) {
    if (!isEnabled()) closePopup();
    if (e->type() == QEvent::LanguageChange) updateText();
    CalendarPicker::changeEvent(e);
}
} // namespace qfw
