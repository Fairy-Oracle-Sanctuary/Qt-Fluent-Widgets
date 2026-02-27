#pragma once

#include <QDate>
#include <QFont>
#include <QFrame>
#include <QHash>
#include <QListWidget>
#include <QModelIndex>
#include <QMouseEvent>
#include <QPair>
#include <QResizeEvent>
#include <QSize>
#include <QString>
#include <QStyleOptionViewItem>
#include <QStyledItemDelegate>

#include "common/icon.h"
#include "components/widgets/button.h"
#include "components/widgets/scroll_bar.h"

class QHBoxLayout;
class QVBoxLayout;
class QPushButton;
class QWidget;
class QLabel;
class QStackedWidget;
class QGraphicsDropShadowEffect;
class QPropertyAnimation;
class QParallelAnimationGroup;

namespace qfw {

class CalendarScrollButton : public TransparentToolButton {
    Q_OBJECT

public:
    explicit CalendarScrollButton(FluentIconEnum icon, QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* e) override;

private:
    FluentIconEnum icon_;
};

class ScrollItemDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit ScrollItemDelegate(const QDate& minDate = QDate(), const QDate& maxDate = QDate(),
                                QObject* parent = nullptr);

    void setRange(const QDate& minDate, const QDate& maxDate);

    void setPressedIndex(const QModelIndex& index);
    void setCurrentIndex(const QModelIndex& index);
    void setSelectedIndex(const QModelIndex& index);

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

protected:
    virtual int itemMargin() const;

    virtual void drawBackground(QPainter* painter, const QStyleOptionViewItem& option,
                                const QModelIndex& index) const;
    virtual void drawText(QPainter* painter, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const;

protected:
    QDate minDate_;
    QDate maxDate_;

    QFont font_;
    QModelIndex pressedIndex_;
    QModelIndex currentIndex_;
    QModelIndex selectedIndex_;
};

class YearScrollItemDelegate : public ScrollItemDelegate {
    Q_OBJECT

public:
    using ScrollItemDelegate::ScrollItemDelegate;

protected:
    int itemMargin() const override;
};

class DayScrollItemDelegate : public ScrollItemDelegate {
    Q_OBJECT

public:
    using ScrollItemDelegate::ScrollItemDelegate;

protected:
    int itemMargin() const override;
};

class ScrollViewBase : public QListWidget {
    Q_OBJECT

public:
    explicit ScrollViewBase(ScrollItemDelegate* delegate, QWidget* parent = nullptr);

    void initialize();

    void scrollUp();
    void scrollDown();
    virtual void scrollToPage(int page);

    virtual QPair<QDate, QDate> currentPageRange() const;

    virtual void setDate(const QDate& date);
    virtual void scrollToDate(const QDate& date);

    virtual QSize gridSize() const;

    void setYearRange(int minYear, int maxYear);

signals:
    void pageChanged(int page);

protected:
    void wheelEvent(QWheelEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;

    virtual void initItems() = 0;

    void setPressedIndex(const QModelIndex& index);
    void setSelectedIndex(const QModelIndex& index);

protected:
    int cols_ = 4;
    int pageRows_ = 3;
    int currentPage_ = 0;

    SmoothScrollBar* vScrollBar_ = nullptr;
    ScrollItemDelegate* delegate_ = nullptr;

    QDate currentDate_;
    QDate date_;

    int minYear_;
    int maxYear_;

private:
    void initWidget();
    void applyDateAndRange();
    bool initialized_ = false;
    bool firstScroll_ = true;
};

class CalendarViewBase : public QFrame {
    Q_OBJECT

public:
    explicit CalendarViewBase(QWidget* parent = nullptr);

    void setScrollView(ScrollViewBase* view);

    ScrollViewBase* scrollView() const;

    void setResetEnabled(bool enabled);
    bool isResetEnabled() const;

    void setDate(const QDate& date);

signals:
    void resetted();
    void titleClicked();
    void itemClicked(const QDate& date);

protected:
    void setTitle(const QString& title);

    virtual QDate currentPageDate() const = 0;
    virtual void updateTitle() = 0;

protected slots:
    void onScrollUp();
    void onScrollDown();

protected:
    QPushButton* titleButton_ = nullptr;
    CalendarScrollButton* resetButton_ = nullptr;
    CalendarScrollButton* upButton_ = nullptr;
    CalendarScrollButton* downButton_ = nullptr;

    ScrollViewBase* scrollView_ = nullptr;

    QHBoxLayout* hBoxLayout_ = nullptr;
    QVBoxLayout* vBoxLayout_ = nullptr;
};

class YearScrollView : public ScrollViewBase {
    Q_OBJECT

public:
    explicit YearScrollView(QWidget* parent = nullptr);

    void scrollToDate(const QDate& date) override;
    QPair<QDate, QDate> currentPageRange() const override;

protected:
    void initItems() override;
};

class YearCalendarView : public CalendarViewBase {
    Q_OBJECT

public:
    explicit YearCalendarView(QWidget* parent = nullptr);

protected:
    void updateTitle() override;
    QDate currentPageDate() const override;
};

class MonthScrollView : public ScrollViewBase {
    Q_OBJECT

public:
    explicit MonthScrollView(QWidget* parent = nullptr);

    void scrollToDate(const QDate& date) override;
    QPair<QDate, QDate> currentPageRange() const override;

protected:
    void initItems() override;

private:
    QStringList months_;
};

class MonthCalendarView : public CalendarViewBase {
    Q_OBJECT

public:
    explicit MonthCalendarView(QWidget* parent = nullptr);
    QDate currentPageDate() const override;

protected:
    void updateTitle() override;
};

class DayScrollView : public ScrollViewBase {
    Q_OBJECT

public:
    explicit DayScrollView(QWidget* parent = nullptr);

    QSize gridSize() const override;
    void setDate(const QDate& date) override;
    void scrollToDate(const QDate& date) override;
    void scrollToPage(int page) override;
    QPair<QDate, QDate> currentPageRange() const override;

protected:
    void initItems() override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;

private:
    QDate pageToDate() const;
    int dateToRow(const QDate& date) const;

    QWidget* weekDayGroup_ = nullptr;
    QHBoxLayout* weekDayLayout_ = nullptr;
    QStringList weekDays_;
};

class DayCalendarView : public CalendarViewBase {
    Q_OBJECT

public:
    explicit DayCalendarView(QWidget* parent = nullptr);

    void scrollToDate(const QDate& date);
    QDate currentPageDate() const override;

protected:
    void updateTitle() override;
};

class CalendarView : public QWidget {
    Q_OBJECT

public:
    explicit CalendarView(QWidget* parent = nullptr);

    bool isResetEnabled() const;
    void setResetEnabled(bool enabled);

    void setYearRange(int minYear, int maxYear);

    void setDate(const QDate& date);

    void exec(QPoint pos, bool ani = true);

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
    void setShadowEffect(int blurRadius = 30, const QPoint& offset = QPoint(0, 8),
                         const QColor& color = QColor(0, 0, 0, 30));

    QStackedWidget* stackedWidget_ = nullptr;
    YearCalendarView* yearView_ = nullptr;
    MonthCalendarView* monthView_ = nullptr;
    DayCalendarView* dayView_ = nullptr;

    QPropertyAnimation* opacityAni_ = nullptr;
    QPropertyAnimation* slideAni_ = nullptr;
    QParallelAnimationGroup* aniGroup_ = nullptr;

    QGraphicsDropShadowEffect* shadowEffect_ = nullptr;

    QDate date_;
    bool isResetEnabled_ = false;
};

}  // namespace qfw
