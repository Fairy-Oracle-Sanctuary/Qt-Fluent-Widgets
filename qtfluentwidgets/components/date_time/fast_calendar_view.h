#pragma once

#include <QDate>
#include <QFont>
#include <QListWidget>
#include <QModelIndex>
#include <QPair>
#include <QStyledItemDelegate>
#include <QWidget>

#include "components/date_time/calendar_view.h"
#include "components/widgets/flyout.h"

class QHBoxLayout;
class QStackedWidget;
class QMouseEvent;
class QWheelEvent;

namespace qfw {

class FastScrollItemDelegate : public ScrollItemDelegate {
    Q_OBJECT

public:
    explicit FastScrollItemDelegate(const QDate& minDate = QDate(), const QDate& maxDate = QDate(),
                                    QObject* parent = nullptr);

    void setSelectedDate(const QDate& date);
    void setCurrentDate(const QDate& date);

protected:
    void drawBackground(QPainter* painter, const QStyleOptionViewItem& option,
                        const QModelIndex& index) const override;
    void drawText(QPainter* painter, const QStyleOptionViewItem& option,
                  const QModelIndex& index) const override;

    QDate selectedDate_;
    QDate currentDate_;
};

class FastYearScrollItemDelegate : public FastScrollItemDelegate {
    Q_OBJECT

public:
    using FastScrollItemDelegate::FastScrollItemDelegate;

protected:
    int itemMargin() const override;
};

class FastDayScrollItemDelegate : public FastScrollItemDelegate {
    Q_OBJECT

public:
    using FastScrollItemDelegate::FastScrollItemDelegate;

protected:
    int itemMargin() const override;
};

class FastScrollViewBase : public ScrollViewBase {
    Q_OBJECT

public:
    explicit FastScrollViewBase(FastScrollItemDelegate* delegate, QWidget* parent = nullptr);

    void scrollToPage(int page) override;

protected:
    void wheelEvent(QWheelEvent* e) override;

    int pageSize() const;
    virtual int pageCount() const = 0;
    virtual void updateItems() = 0;
};

class FastYearScrollView : public FastScrollViewBase {
    Q_OBJECT

public:
    explicit FastYearScrollView(QWidget* parent = nullptr);

    void scrollToDate(const QDate& date) override;
    QPair<QDate, QDate> currentPageRange() const override;

protected:
    void initItems() override;
    void updateItems() override;
    int pageCount() const override;

private:
    int startYear() const;
};

class FastMonthScrollView : public FastScrollViewBase {
    Q_OBJECT

public:
    explicit FastMonthScrollView(QWidget* parent = nullptr);

    void scrollToDate(const QDate& date) override;
    QPair<QDate, QDate> currentPageRange() const override;

protected:
    void initItems() override;
    void updateItems() override;
    int pageCount() const override;

private:
    QStringList months_;
};

class FastDayScrollView : public FastScrollViewBase {
    Q_OBJECT

public:
    explicit FastDayScrollView(QWidget* parent = nullptr);

    QSize gridSize() const override;

    void setDate(const QDate& date) override;
    void scrollToDate(const QDate& date) override;
    QPair<QDate, QDate> currentPageRange() const override;

protected:
    void initItems() override;
    void updateItems() override;
    int pageCount() const override;

    void mouseReleaseEvent(QMouseEvent* e) override;

private:
    QDate pageToDate() const;
};

class FastYearCalendarView : public CalendarViewBase {
    Q_OBJECT

public:
    explicit FastYearCalendarView(QWidget* parent = nullptr);

protected:
    QDate currentPageDate() const override;
    void updateTitle() override;
};

class FastMonthCalendarView : public CalendarViewBase {
    Q_OBJECT

public:
    explicit FastMonthCalendarView(QWidget* parent = nullptr);

    QDate currentPageDate() const override;

protected:
    void updateTitle() override;
};

class FastDayCalendarView : public CalendarViewBase {
    Q_OBJECT

public:
    explicit FastDayCalendarView(QWidget* parent = nullptr);

    QDate currentPageDate() const override;

    void scrollToDate(const QDate& date);

protected:
    void updateTitle() override;
};

class FastCalendarView : public FlyoutViewBase {
    Q_OBJECT

public:
    explicit FastCalendarView(QWidget* parent = nullptr);

    void addWidget(QWidget* widget, int stretch = 0, Qt::Alignment align = Qt::AlignLeft) override;

    bool isResetEnabled() const;
    void setResetEnabled(bool enabled);

    void setYearRange(int minYear, int maxYear);

    void setDate(const QDate& date);

signals:
    void resetted();
    void dateChanged(const QDate& date);

private slots:
    void onResetted();
    void onDayViewTitleClicked();
    void onMonthTitleClicked();
    void onMonthItemClicked(const QDate& date);
    void onYearItemClicked(const QDate& date);
    void onDayItemClicked(const QDate& date);

private:
    void initWidget();

    QHBoxLayout* hBoxLayout_ = nullptr;
    QStackedWidget* stackedWidget_ = nullptr;

    FastYearCalendarView* yearView_ = nullptr;
    FastMonthCalendarView* monthView_ = nullptr;
    FastDayCalendarView* dayView_ = nullptr;

    QDate date_;
    bool isResetEnabled_ = false;
};

}  // namespace qfw
