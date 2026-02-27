#include "components/date_time/fast_calendar_view.h"

#include <QCalendar>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QMouseEvent>
#include <QPainter>
#include <QStackedWidget>
#include <QStyle>
#include <QVBoxLayout>
#include <QWheelEvent>

#include "common/color.h"
#include "common/font.h"
#include "common/style_sheet.h"

namespace qfw {

FastScrollItemDelegate::FastScrollItemDelegate(const QDate& minDate, const QDate& maxDate,
                                               QObject* parent)
    : ScrollItemDelegate(minDate, maxDate, parent) {
    currentDate_ = QDate::currentDate();
}

void FastScrollItemDelegate::setSelectedDate(const QDate& date) { selectedDate_ = date; }

void FastScrollItemDelegate::setCurrentDate(const QDate& date) { currentDate_ = date; }

void FastScrollItemDelegate::drawBackground(QPainter* painter, const QStyleOptionViewItem& option,
                                            const QModelIndex& index) const {
    const QDate d = index.data(Qt::UserRole).toDate();
    if (!d.isValid()) {
        return;
    }

    painter->save();

    if (d != selectedDate_) {
        painter->setPen(Qt::NoPen);
    } else {
        painter->setPen(themeColor());
    }

    if (d == currentDate_) {
        if (index == pressedIndex_) {
            painter->setBrush(
                themedColor(themeColor(), isDarkTheme(), QStringLiteral("ThemeColorLight2")));
        } else if (option.state & QStyle::State_MouseOver) {
            painter->setBrush(
                themedColor(themeColor(), isDarkTheme(), QStringLiteral("ThemeColorLight1")));
        } else {
            // Use ThemeColorLight1 in dark theme as default
            painter->setBrush(
                isDarkTheme() ? themedColor(themeColor(), true, QStringLiteral("ThemeColorLight1"))
                              : themeColor());
        }
    } else {
        const int c = isDarkTheme() ? 255 : 0;
        if (index == pressedIndex_) {
            painter->setBrush(QColor(c, c, c, 7));
        } else if (option.state & QStyle::State_MouseOver) {
            painter->setBrush(QColor(c, c, c, 9));
        } else {
            painter->setBrush(Qt::transparent);
        }
    }

    const int m = itemMargin();
    painter->drawEllipse(option.rect.adjusted(m, m, -m, -m));

    painter->restore();
}

void FastScrollItemDelegate::drawText(QPainter* painter, const QStyleOptionViewItem& option,
                                      const QModelIndex& index) const {
    const QDate d = index.data(Qt::UserRole).toDate();
    if (!d.isValid()) {
        return;
    }

    painter->save();
    painter->setFont(font_);

    if (d == currentDate_) {
        const int c = isDarkTheme() ? 0 : 255;
        painter->setPen(QColor(c, c, c));
    } else {
        painter->setPen(isDarkTheme() ? Qt::white : Qt::black);

        const bool inRange = d.isValid() && (minDate_.isValid() && maxDate_.isValid()) &&
                             (minDate_ <= d && d <= maxDate_);
        if ((!inRange && (option.state & QStyle::State_MouseOver) == 0) || index == pressedIndex_) {
            painter->setOpacity(0.6);
        }
    }

    painter->drawText(option.rect, Qt::AlignCenter, index.data(Qt::DisplayRole).toString());
    painter->restore();
}

int FastYearScrollItemDelegate::itemMargin() const { return 8; }

int FastDayScrollItemDelegate::itemMargin() const { return 3; }

FastScrollViewBase::FastScrollViewBase(FastScrollItemDelegate* delegate, QWidget* parent)
    : ScrollViewBase(delegate, parent) {
    if (delegate) {
        delegate->setRange(currentPageRange().first, currentPageRange().second);
        delegate->setCurrentDate(currentDate_);
    }
}

int FastScrollViewBase::pageSize() const { return pageRows_ * cols_; }

void FastScrollViewBase::scrollToPage(int page) {
    if (page < 0 || page >= pageCount()) {
        return;
    }

    currentPage_ = page;
    updateItems();

    if (delegate_) {
        const auto r = currentPageRange();
        delegate_->setRange(r.first, r.second);
    }

    emit pageChanged(page);
}

void FastScrollViewBase::wheelEvent(QWheelEvent* e) {
    if (e) {
        e->ignore();
    }
}

FastYearScrollView::FastYearScrollView(QWidget* parent)
    : FastScrollViewBase(new FastYearScrollItemDelegate(QDate(), QDate(), nullptr), parent) {
    initialize();

    if (auto* d = dynamic_cast<FastScrollItemDelegate*>(delegate_)) {
        d->setCurrentDate(QDate(currentDate_.year(), 1, 1));
    }
}

int FastYearScrollView::startYear() const {
    if (minYear_ % 10 <= 2) {
        return minYear_ - minYear_ % 10;
    }
    return minYear_ - minYear_ % 10 + 10;
}

void FastYearScrollView::initItems() {
    clear();

    for (int i = 0; i < 16; ++i) {
        addItem(QString());
        auto* it = item(i);
        if (it) {
            it->setSizeHint(gridSize());
        }
    }

    updateItems();
}

int FastYearScrollView::pageCount() const {
    const int span = maxYear_ - minYear_ + 1;
    if (span <= 0) {
        return 0;
    }
    return (span + 10 - 1) / 10;
}

void FastYearScrollView::scrollToDate(const QDate& date) {
    if (!date.isValid()) {
        return;
    }

    const int page = (date.year() - startYear()) / 10;
    scrollToPage(page);
}

QPair<QDate, QDate> FastYearScrollView::currentPageRange() const {
    const int year = currentPage_ * 10 + startYear();
    return {QDate(year, 1, 1), QDate(year + 9, 1, 1)};
}

void FastYearScrollView::updateItems() {
    const auto r = currentPageRange();
    const int s = r.first.year();

    const int index = (s - minYear_) % 4;
    const int left = s - index;
    const int right = left + 16;

    for (int i = 0; i < 16; ++i) {
        const int year = left + i;
        auto* it = item(i);
        if (!it) {
            continue;
        }
        it->setText(QString::number(year));
        it->setData(Qt::UserRole, QDate(year, 1, 1));
    }

    if (delegate_) {
        delegate_->setCurrentIndex(QModelIndex());
    }

    viewport()->update();
}

FastMonthScrollView::FastMonthScrollView(QWidget* parent)
    : FastScrollViewBase(new FastYearScrollItemDelegate(QDate(), QDate(), nullptr), parent) {
    initialize();

    if (auto* d = dynamic_cast<FastScrollItemDelegate*>(delegate_)) {
        d->setCurrentDate(QDate(currentDate_.year(), currentDate_.month(), 1));
    }
}

void FastMonthScrollView::initItems() {
    clear();

    const auto loc = QLocale::system();
    months_.clear();
    for (int m = 1; m <= 12; ++m) {
        months_.append(loc.standaloneMonthName(m, QLocale::ShortFormat));
    }
    for (int m = 1; m <= 4; ++m) {
        months_.append(loc.standaloneMonthName(m, QLocale::ShortFormat));
    }

    for (int i = 0; i < months_.size(); ++i) {
        addItem(months_.at(i));
        auto* it = item(i);
        if (it) {
            it->setSizeHint(gridSize());
        }
    }

    updateItems();
}

int FastMonthScrollView::pageCount() const {
    const int span = maxYear_ - minYear_ + 1;
    return span > 0 ? span : 0;
}

void FastMonthScrollView::scrollToDate(const QDate& date) {
    if (!date.isValid()) {
        return;
    }
    scrollToPage(date.year() - minYear_);
}

QPair<QDate, QDate> FastMonthScrollView::currentPageRange() const {
    const int year = minYear_ + currentPage_;
    return {QDate(year, 1, 1), QDate(year, 12, 31)};
}

void FastMonthScrollView::updateItems() {
    const int year = minYear_ + currentPage_;

    for (int i = 0; i < 16; ++i) {
        const int m = i % 12 + 1;
        const int y = year + (i > 11 ? 1 : 0);
        auto* it = item(i);
        if (it) {
            it->setData(Qt::UserRole, QDate(y, m, 1));
        }
    }

    viewport()->update();
}

FastDayScrollView::FastDayScrollView(QWidget* parent)
    : FastScrollViewBase(new FastDayScrollItemDelegate(QDate(), QDate(), nullptr), parent) {
    cols_ = 7;
    pageRows_ = 6;

    setGridSize(gridSize());

    // 创建星期标题组
    auto* group = new QWidget(this);
    group->setObjectName(QStringLiteral("weekDayGroup"));

    auto* layout = new QHBoxLayout(group);
    layout->setSpacing(0);
    layout->setContentsMargins(3, 12, 3, 12);

    const QStringList weekDays = {tr("Mo"), tr("Tu"), tr("We"), tr("Th"),
                                  tr("Fr"), tr("Sa"), tr("Su")};
    for (const auto& day : weekDays) {
        auto* label = new QLabel(day, group);
        label->setObjectName(QStringLiteral("weekDayLabel"));
        layout->addWidget(label, 1, Qt::AlignHCenter);
    }

    // 创建主布局并添加星期标题组（这是缺失的关键代码）
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignTop);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(group);

    setViewportMargins(0, 38, 0, 0);

    initialize();

    if (auto* d = dynamic_cast<FastScrollItemDelegate*>(delegate_)) {
        d->setCurrentDate(currentDate_);
    }
}

QSize FastDayScrollView::gridSize() const { return QSize(44, 44); }

int FastDayScrollView::pageCount() const {
    const int span = maxYear_ - minYear_ + 1;
    if (span <= 0) {
        return 0;
    }
    return span * 12;
}

void FastDayScrollView::initItems() {
    clear();

    for (int i = 0; i < 42; ++i) {
        addItem(QString());
        auto* it = item(i);
        if (it) {
            it->setSizeHint(gridSize());
        }
    }

    updateItems();
}

void FastDayScrollView::setDate(const QDate& date) {
    if (!date.isValid()) {
        return;
    }
    scrollToDate(date);
    if (auto* d = dynamic_cast<FastScrollItemDelegate*>(delegate_)) {
        d->setSelectedDate(date);
    }
    viewport()->update();
}

void FastDayScrollView::scrollToDate(const QDate& date) {
    if (!date.isValid()) {
        return;
    }

    const int page = (date.year() - minYear_) * 12 + date.month() - 1;
    scrollToPage(page);

    if (auto* d = dynamic_cast<FastScrollItemDelegate*>(delegate_)) {
        d->setSelectedDate(date);
    }
}

QPair<QDate, QDate> FastDayScrollView::currentPageRange() const {
    const QDate d = pageToDate();
    return {d, d.addMonths(1).addDays(-1)};
}

QDate FastDayScrollView::pageToDate() const {
    const int year = currentPage_ / 12 + minYear_;
    const int month = currentPage_ % 12 + 1;
    return QDate(year, month, 1);
}

void FastDayScrollView::updateItems() {
    const QDate first = pageToDate();
    if (!first.isValid()) {
        return;
    }

    QDate left = first.addDays(-first.dayOfWeek() + 1);

    for (int i = 0; i < 42; ++i) {
        QDate d = left.addDays(i);
        auto* it = item(i);
        if (!it) {
            continue;
        }
        it->setText(QString::number(d.day()));
        it->setData(Qt::UserRole, d);
        it->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    }

    viewport()->update();
}

void FastDayScrollView::mouseReleaseEvent(QMouseEvent* e) {
    ScrollViewBase::mouseReleaseEvent(e);

    auto* it = currentItem();
    if (!it) {
        return;
    }

    const QDate d = it->data(Qt::UserRole).toDate();
    if (!d.isValid()) {
        return;
    }

    if (auto* del = dynamic_cast<FastScrollItemDelegate*>(delegate_)) {
        del->setSelectedDate(d);
    }

    viewport()->update();
}

FastYearCalendarView::FastYearCalendarView(QWidget* parent) : CalendarViewBase(parent) {
    setScrollView(new FastYearScrollView(this));
    if (titleButton_) {
        titleButton_->setEnabled(false);
    }
}

QDate FastYearCalendarView::currentPageDate() const {
    if (!scrollView_) {
        return {};
    }

    const auto r = scrollView_->currentPageRange();
    return r.first;
}

void FastYearCalendarView::updateTitle() {
    if (!scrollView_) {
        return;
    }

    const auto r = scrollView_->currentPageRange();
    setTitle(QStringLiteral("%1 - %2").arg(r.first.year()).arg(r.second.year()));
}

FastMonthCalendarView::FastMonthCalendarView(QWidget* parent) : CalendarViewBase(parent) {
    setScrollView(new FastMonthScrollView(this));
}

QDate FastMonthCalendarView::currentPageDate() const {
    if (!scrollView_) {
        return {};
    }

    const auto r = scrollView_->currentPageRange();

    auto* item = scrollView_->currentItem();
    int month = 1;
    if (item) {
        const QDate d = item->data(Qt::UserRole).toDate();
        if (d.isValid()) {
            month = d.month();
        }
    }

    return QDate(r.first.year(), month, 1);
}

void FastMonthCalendarView::updateTitle() {
    if (!scrollView_) {
        return;
    }

    const auto r = scrollView_->currentPageRange();
    setTitle(QString::number(r.first.year()));
}

FastDayCalendarView::FastDayCalendarView(QWidget* parent) : CalendarViewBase(parent) {
    setScrollView(new FastDayScrollView(this));
}

QDate FastDayCalendarView::currentPageDate() const {
    if (!scrollView_) {
        return {};
    }

    const auto r = scrollView_->currentPageRange();
    return r.first;
}

void FastDayCalendarView::scrollToDate(const QDate& date) {
    if (scrollView_) {
        scrollView_->scrollToDate(date);
    }
    updateTitle();
}

void FastDayCalendarView::updateTitle() {
    const QDate date = currentPageDate();
    if (!date.isValid()) {
        return;
    }

    const auto cal = QCalendar();
    const QString name = cal.monthName(locale(), date.month(), date.year());
    setTitle(QStringLiteral("%1 %2").arg(name).arg(date.year()));
}

FastCalendarView::FastCalendarView(QWidget* parent) : FlyoutViewBase(parent) { initWidget(); }

void FastCalendarView::addWidget(QWidget* widget, int stretch, Qt::Alignment align) {
    if (hBoxLayout_ && widget) {
        hBoxLayout_->addWidget(widget, stretch, align);
    }
}

void FastCalendarView::initWidget() {
    hBoxLayout_ = new QHBoxLayout(this);
    hBoxLayout_->setContentsMargins(0, 0, 0, 0);

    stackedWidget_ = new QStackedWidget(this);

    yearView_ = new FastYearCalendarView(this);
    monthView_ = new FastMonthCalendarView(this);
    dayView_ = new FastDayCalendarView(this);

    stackedWidget_->addWidget(dayView_);
    stackedWidget_->addWidget(monthView_);
    stackedWidget_->addWidget(yearView_);

    hBoxLayout_->addWidget(stackedWidget_);

    if (dayView_) {
        dayView_->setDate(QDate::currentDate());
    }

    QObject::connect(dayView_, &CalendarViewBase::titleClicked, this,
                     &FastCalendarView::onDayViewTitleClicked);
    QObject::connect(monthView_, &CalendarViewBase::titleClicked, this,
                     &FastCalendarView::onMonthTitleClicked);

    QObject::connect(monthView_, &CalendarViewBase::itemClicked, this,
                     &FastCalendarView::onMonthItemClicked);
    QObject::connect(yearView_, &CalendarViewBase::itemClicked, this,
                     &FastCalendarView::onYearItemClicked);
    QObject::connect(dayView_, &CalendarViewBase::itemClicked, this,
                     &FastCalendarView::onDayItemClicked);

    QObject::connect(monthView_, &CalendarViewBase::resetted, this, &FastCalendarView::onResetted);
    QObject::connect(yearView_, &CalendarViewBase::resetted, this, &FastCalendarView::onResetted);
    QObject::connect(dayView_, &CalendarViewBase::resetted, this, &FastCalendarView::onResetted);
}

bool FastCalendarView::isResetEnabled() const { return isResetEnabled_; }

void FastCalendarView::setResetEnabled(bool enabled) {
    isResetEnabled_ = enabled;
    if (yearView_) {
        yearView_->setResetEnabled(enabled);
    }
    if (monthView_) {
        monthView_->setResetEnabled(enabled);
    }
    if (dayView_) {
        dayView_->setResetEnabled(enabled);
    }
}

void FastCalendarView::setYearRange(int minYear, int maxYear) {
    if (dayView_ && dayView_->scrollView()) {
        dayView_->scrollView()->setYearRange(minYear, maxYear);
    }
    if (monthView_ && monthView_->scrollView()) {
        monthView_->scrollView()->setYearRange(minYear, maxYear);
    }
    if (yearView_ && yearView_->scrollView()) {
        yearView_->scrollView()->setYearRange(minYear, maxYear);
    }
}

void FastCalendarView::setDate(const QDate& date) {
    if (dayView_) {
        dayView_->setDate(date);
    }
    date_ = date;
}

void FastCalendarView::onResetted() {
    emit resetted();
    close();
}

void FastCalendarView::onDayViewTitleClicked() {
    if (!stackedWidget_ || !monthView_ || !dayView_) {
        return;
    }
    stackedWidget_->setCurrentWidget(monthView_);
    monthView_->setDate(dayView_->currentPageDate());
}

void FastCalendarView::onMonthTitleClicked() {
    if (!stackedWidget_ || !yearView_ || !monthView_) {
        return;
    }
    stackedWidget_->setCurrentWidget(yearView_);
    yearView_->setDate(monthView_->currentPageDate());
}

void FastCalendarView::onMonthItemClicked(const QDate& date) {
    if (!stackedWidget_ || !dayView_) {
        return;
    }
    stackedWidget_->setCurrentWidget(dayView_);
    dayView_->scrollToDate(date);
}

void FastCalendarView::onYearItemClicked(const QDate& date) {
    if (!stackedWidget_ || !monthView_) {
        return;
    }
    stackedWidget_->setCurrentWidget(monthView_);
    monthView_->setDate(date);
}

void FastCalendarView::onDayItemClicked(const QDate& date) {
    close();
    if (date != date_) {
        date_ = date;
        emit dateChanged(date);
    }
}

}  // namespace qfw
