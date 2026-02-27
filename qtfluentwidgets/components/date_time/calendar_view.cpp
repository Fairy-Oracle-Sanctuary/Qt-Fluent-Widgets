#include "components/date_time/calendar_view.h"

#include <QCalendar>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QRect>
#include <QStackedWidget>
#include <QStyle>
#include <QVBoxLayout>
#include <QWheelEvent>

#include "common/color.h"
#include "common/font.h"
#include "common/screen.h"
#include "common/style_sheet.h"

namespace qfw {

// =========================================================================
// CalendarScrollButton
// =========================================================================

CalendarScrollButton::CalendarScrollButton(FluentIconEnum icon, QWidget* parent)
    : TransparentToolButton(icon, parent), icon_(icon) {
    setProperty("qssClass", QStringLiteral("ScrollButton"));
    setIcon(QIcon());  // Clear the icon of QToolButton to avoid double drawing
}

void CalendarScrollButton::paintEvent(QPaintEvent* e) {
    TransparentToolButton::paintEvent(e);  // Use base class to draw background

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    const int w = isPressed ? 9 : 10;
    const int h = isPressed ? 9 : 10;

    const qreal x = (width() - w) / 2.0;
    const qreal y = (height() - h) / 2.0;

    const FluentIcon icon(icon_);

    if (!isDarkTheme()) {
        icon.render(&painter, QRect(qRound(x), qRound(y), w, h), Theme::Auto,
                    QVariantMap{{QStringLiteral("fill"), QStringLiteral("#5e5e5e")}});
    } else {
        icon.render(&painter, QRect(qRound(x), qRound(y), w, h), Theme::Auto,
                    QVariantMap{{QStringLiteral("fill"), QStringLiteral("#9c9c9c")}});
    }
}

// =========================================================================
// ScrollItemDelegate
// =========================================================================

ScrollItemDelegate::ScrollItemDelegate(const QDate& minDate, const QDate& maxDate, QObject* parent)
    : QStyledItemDelegate(parent), minDate_(minDate), maxDate_(maxDate) {
    font_ = getFont();
}

void ScrollItemDelegate::setRange(const QDate& minDate, const QDate& maxDate) {
    minDate_ = minDate;
    maxDate_ = maxDate;
}

void ScrollItemDelegate::setPressedIndex(const QModelIndex& index) { pressedIndex_ = index; }

void ScrollItemDelegate::setCurrentIndex(const QModelIndex& index) { currentIndex_ = index; }

void ScrollItemDelegate::setSelectedIndex(const QModelIndex& index) { selectedIndex_ = index; }

void ScrollItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                               const QModelIndex& index) const {
    if (!painter) {
        return;
    }

    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    drawBackground(painter, option, index);
    drawText(painter, option, index);
}

void ScrollItemDelegate::drawBackground(QPainter* painter, const QStyleOptionViewItem& option,
                                        const QModelIndex& index) const {
    painter->save();

    // outer ring
    if (index != selectedIndex_) {
        painter->setPen(Qt::NoPen);
    } else {
        painter->setPen(themeColor());
    }

    if (index == currentIndex_) {
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

void ScrollItemDelegate::drawText(QPainter* painter, const QStyleOptionViewItem& option,
                                  const QModelIndex& index) const {
    painter->save();
    painter->setFont(font_);

    if (index == currentIndex_) {
        const int c = isDarkTheme() ? 0 : 255;
        painter->setPen(QColor(c, c, c));
    } else {
        painter->setPen(isDarkTheme() ? Qt::white : Qt::black);

        const QDate d = index.data(Qt::UserRole).toDate();
        const bool inRange = d.isValid() && (minDate_.isValid() && maxDate_.isValid()) &&
                             (minDate_ <= d && d <= maxDate_);

        if ((!inRange && !(option.state & QStyle::State_MouseOver)) || index == pressedIndex_) {
            painter->setOpacity(0.6);
        }
    }

    const QString text = index.data(Qt::DisplayRole).toString();
    painter->drawText(option.rect, Qt::AlignCenter, text);

    painter->restore();
}

int ScrollItemDelegate::itemMargin() const { return 0; }

// =========================================================================
// YearScrollItemDelegate
// =========================================================================

int YearScrollItemDelegate::itemMargin() const { return 8; }

// =========================================================================
// DayScrollItemDelegate
// =========================================================================

int DayScrollItemDelegate::itemMargin() const { return 3; }

// =========================================================================
// ScrollViewBase
// =========================================================================

ScrollViewBase::ScrollViewBase(ScrollItemDelegate* delegate, QWidget* parent)
    : QListWidget(parent), delegate_(delegate) {
    setProperty("qssClass", QStringLiteral("ScrollViewBase"));
    vScrollBar_ = new SmoothScrollBar(Qt::Vertical, this);

    currentDate_ = QDate::currentDate();
    date_ = QDate::currentDate();

    setYearRange(currentDate_.year() - 5, currentDate_.year() + 5);

    setUniformItemSizes(true);
    initWidget();
}

void ScrollViewBase::initialize() {
    initItems();
    applyDateAndRange();
    initialized_ = true;
}

void ScrollViewBase::initWidget() {
    setSpacing(0);
    setMovement(QListWidget::Static);
    setGridSize(gridSize());
    setViewportMargins(0, 0, 0, 0);

    if (delegate_) {
        delegate_->setParent(this);
        setItemDelegate(delegate_);
    }

    setViewMode(QListWidget::IconMode);
    setResizeMode(QListWidget::Adjust);

    if (firstScroll_) {
        vScrollBar_->setScrollAnimation(1);
        QObject::connect(vScrollBar_, &SmoothScrollBar::valueChanged, this, [this](int) {
            if (!firstScroll_) {
                return;
            }
            firstScroll_ = false;
            vScrollBar_->setScrollAnimation(300, QEasingCurve::OutQuad);
        });
    }

    vScrollBar_->setForceHidden(true);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void ScrollViewBase::applyDateAndRange() {
    setDate(date_);
    if (delegate_) {
        const auto r = currentPageRange();
        delegate_->setRange(r.first, r.second);
    }
}

void ScrollViewBase::scrollUp() { scrollToPage(currentPage_ - 1); }

void ScrollViewBase::scrollDown() { scrollToPage(currentPage_ + 1); }

void ScrollViewBase::setYearRange(int minYear, int maxYear) {
    minYear_ = minYear;
    maxYear_ = maxYear;

    if (initialized_) {
        clear();
        initItems();
        applyDateAndRange();
    }
}

void ScrollViewBase::scrollToPage(int page) {
    const int pageSize = pageRows_ * cols_;
    if (pageSize <= 0) {
        return;
    }

    const int totalPages = (model() ? (model()->rowCount() + pageSize - 1) / pageSize : 0);
    if (page < 0 || page >= totalPages) {
        return;
    }

    currentPage_ = page;
    const int y = gridSize().height() * pageRows_ * page;
    vScrollBar_->setValue(y);

    if (delegate_) {
        const auto r = currentPageRange();
        delegate_->setRange(r.first, r.second);
    }

    emit pageChanged(page);
}

QPair<QDate, QDate> ScrollViewBase::currentPageRange() const { return {QDate(), QDate()}; }

void ScrollViewBase::setDate(const QDate& date) { scrollToDate(date); }

void ScrollViewBase::scrollToDate(const QDate& date) { Q_UNUSED(date); }

void ScrollViewBase::setPressedIndex(const QModelIndex& index) {
    if (delegate_) {
        delegate_->setPressedIndex(index);
    }
    viewport()->update();
}

void ScrollViewBase::setSelectedIndex(const QModelIndex& index) {
    if (delegate_) {
        delegate_->setSelectedIndex(index);
    }
    viewport()->update();
}

void ScrollViewBase::wheelEvent(QWheelEvent* e) {
    if (!e) {
        return;
    }

    if (vScrollBar_ && vScrollBar_->isAnimationRunning()) {
        return;
    }

    if (e->angleDelta().y() < 0) {
        scrollDown();
    } else {
        scrollUp();
    }
}

void ScrollViewBase::mousePressEvent(QMouseEvent* e) {
    QListWidget::mousePressEvent(e);
    if (e && e->button() == Qt::LeftButton && indexAt(e->pos()).row() >= 0) {
        setPressedIndex(currentIndex());
    }
}

void ScrollViewBase::mouseReleaseEvent(QMouseEvent* e) {
    QListWidget::mouseReleaseEvent(e);
    setPressedIndex(QModelIndex());
}

QSize ScrollViewBase::gridSize() const { return QSize(76, 76); }

// =========================================================================
// CalendarViewBase
// =========================================================================

CalendarViewBase::CalendarViewBase(QWidget* parent) : QFrame(parent) {
    setProperty("qssClass", QStringLiteral("CalendarViewBase"));
    titleButton_ = new QPushButton(this);
    resetButton_ = new CalendarScrollButton(FluentIconEnum::Cancel, this);
    upButton_ = new CalendarScrollButton(FluentIconEnum::CareUpSolid, this);
    downButton_ = new CalendarScrollButton(FluentIconEnum::CareDownSolid, this);

    hBoxLayout_ = new QHBoxLayout();
    vBoxLayout_ = new QVBoxLayout(this);

    setFixedSize(314, 355);
    upButton_->setFixedSize(32, 34);
    downButton_->setFixedSize(32, 34);
    resetButton_->setFixedSize(32, 34);
    titleButton_->setFixedHeight(34);

    hBoxLayout_->setContentsMargins(9, 8, 9, 8);
    hBoxLayout_->setSpacing(7);
    hBoxLayout_->addWidget(titleButton_, 1, Qt::AlignVCenter);
    hBoxLayout_->addWidget(resetButton_, 0, Qt::AlignVCenter);
    hBoxLayout_->addWidget(upButton_, 0, Qt::AlignVCenter);
    hBoxLayout_->addWidget(downButton_, 0, Qt::AlignVCenter);

    vBoxLayout_->setContentsMargins(0, 0, 0, 0);
    vBoxLayout_->setSpacing(0);
    vBoxLayout_->addLayout(hBoxLayout_);
    vBoxLayout_->setAlignment(Qt::AlignTop);

    setResetEnabled(false);

    titleButton_->setObjectName(QStringLiteral("titleButton"));
    qfw::setStyleSheet(this, FluentStyleSheet::CalendarPicker);

    QObject::connect(titleButton_, &QPushButton::clicked, this, &CalendarViewBase::titleClicked);
    QObject::connect(resetButton_, &QToolButton::clicked, this, &CalendarViewBase::resetted);
    QObject::connect(upButton_, &QToolButton::clicked, this, &CalendarViewBase::onScrollUp);
    QObject::connect(downButton_, &QToolButton::clicked, this, &CalendarViewBase::onScrollDown);
}

void CalendarViewBase::setScrollView(ScrollViewBase* view) {
    if (!view) {
        return;
    }

    scrollView_ = view;
    vBoxLayout_->addWidget(view);

    QObject::connect(view, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        if (!item) {
            return;
        }

        const QDate d = item->data(Qt::UserRole).toDate();
        if (d.isValid()) {
            emit itemClicked(d);
        }
    });

    QObject::connect(view, &ScrollViewBase::pageChanged, this, [this](int) { updateTitle(); });

    updateTitle();
}

ScrollViewBase* CalendarViewBase::scrollView() const { return scrollView_; }

void CalendarViewBase::setResetEnabled(bool enabled) { resetButton_->setVisible(enabled); }

bool CalendarViewBase::isResetEnabled() const { return resetButton_->isVisible(); }

void CalendarViewBase::setDate(const QDate& date) {
    if (scrollView_) {
        scrollView_->setDate(date);
    }
    updateTitle();
}

void CalendarViewBase::setTitle(const QString& title) { titleButton_->setText(title); }

void CalendarViewBase::onScrollUp() {
    if (!scrollView_) {
        return;
    }
    scrollView_->scrollUp();
    updateTitle();
}

void CalendarViewBase::onScrollDown() {
    if (!scrollView_) {
        return;
    }
    scrollView_->scrollDown();
    updateTitle();
}

// =========================================================================
// YearScrollView
// =========================================================================

YearScrollView::YearScrollView(QWidget* parent)
    : ScrollViewBase(new YearScrollItemDelegate(QDate(), QDate(), nullptr), parent) {
    initialize();
}

void YearScrollView::initItems() {
    const int count = maxYear_ - minYear_ + 1;
    for (int i = 0; i < count; ++i) {
        const int year = minYear_ + i;
        auto* it = new QListWidgetItem(QString::number(year), this);
        it->setData(Qt::UserRole, QDate(year, 1, 1));
        it->setSizeHint(gridSize());
        addItem(it);

        if (year == currentDate_.year() && delegate_) {
            delegate_->setCurrentIndex(indexFromItem(it));
        }
    }
}

void YearScrollView::scrollToDate(const QDate& date) {
    if (!date.isValid()) {
        return;
    }
    const int page = (date.year() - minYear_) / 12;
    scrollToPage(page);
}

QPair<QDate, QDate> YearScrollView::currentPageRange() const {
    const int pageSize = pageRows_ * cols_;
    const int left = currentPage_ * pageSize + minYear_;

    QHash<int, int> decades;
    for (int y = left; y < left + 16; ++y) {
        const int d = (y / 10) * 10;
        decades[d] += 1;
    }

    int bestDecade = left / 10 * 10;
    int bestCount = -1;
    for (auto it = decades.constBegin(); it != decades.constEnd(); ++it) {
        if (it.value() > bestCount) {
            bestCount = it.value();
            bestDecade = it.key();
        }
    }

    return {QDate(bestDecade, 1, 1), QDate(bestDecade + 10, 1, 1)};
}

// =========================================================================
// YearCalendarView
// =========================================================================

YearCalendarView::YearCalendarView(QWidget* parent) : CalendarViewBase(parent) {
    auto* view = new YearScrollView(this);
    setScrollView(view);
    titleButton_->setEnabled(false);
}

void YearCalendarView::updateTitle() {
    if (!scrollView_) {
        return;
    }

    const auto r = scrollView_->currentPageRange();
    setTitle(QString::number(r.first.year()) + QStringLiteral(" - ") +
             QString::number(r.second.year()));
}

QDate YearCalendarView::currentPageDate() const {
    if (!scrollView_) {
        return {};
    }

    const auto r = scrollView_->currentPageRange();
    return r.first;
}

// =========================================================================
// MonthScrollView
// =========================================================================

MonthScrollView::MonthScrollView(QWidget* parent)
    : ScrollViewBase(new YearScrollItemDelegate(QDate(), QDate(), nullptr), parent) {
    initialize();
}

void MonthScrollView::initItems() {
    months_ = {
        tr("Jan"), tr("Feb"), tr("Mar"), tr("Apr"), tr("May"), tr("Jun"),
        tr("Jul"), tr("Aug"), tr("Sep"), tr("Oct"), tr("Nov"), tr("Dec"),
    };

    const int yearsCount = maxYear_ - minYear_ + 1;
    const int total = yearsCount * 12;

    for (int i = 0; i < total; ++i) {
        const int year = i / 12 + minYear_;
        const int m = i % 12 + 1;

        auto* it = new QListWidgetItem(months_.value(m - 1), this);
        it->setData(Qt::UserRole, QDate(year, m, 1));
        it->setSizeHint(gridSize());
        addItem(it);

        if (year == currentDate_.year() && m == currentDate_.month() && delegate_) {
            delegate_->setCurrentIndex(indexFromItem(it));
        }
    }
}

void MonthScrollView::scrollToDate(const QDate& date) {
    if (!date.isValid()) {
        return;
    }

    const int page = date.year() - minYear_;
    scrollToPage(page);
}

QPair<QDate, QDate> MonthScrollView::currentPageRange() const {
    const int year = minYear_ + currentPage_;
    return {QDate(year, 1, 1), QDate(year, 12, 31)};
}

// =========================================================================
// MonthCalendarView
// =========================================================================

MonthCalendarView::MonthCalendarView(QWidget* parent) : CalendarViewBase(parent) {
    setScrollView(new MonthScrollView(this));
}

void MonthCalendarView::updateTitle() {
    if (!scrollView_) {
        return;
    }
    const auto r = scrollView_->currentPageRange();
    setTitle(QString::number(r.first.year()));
}

QDate MonthCalendarView::currentPageDate() const {
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

// =========================================================================
// DayScrollView
// =========================================================================

DayScrollView::DayScrollView(QWidget* parent)
    : ScrollViewBase(new DayScrollItemDelegate(QDate(), QDate(), nullptr), parent) {
    cols_ = 7;
    pageRows_ = 4;

    // Must reset grid size here since base class constructor called base gridSize()
    setGridSize(gridSize());

    weekDays_ = {tr("Mo"), tr("Tu"), tr("We"), tr("Th"), tr("Fr"), tr("Sa"), tr("Su")};

    weekDayGroup_ = new QWidget(this);
    weekDayGroup_->setObjectName(QStringLiteral("weekDayGroup"));
    weekDayLayout_ = new QHBoxLayout(weekDayGroup_);

    for (const auto& day : weekDays_) {
        auto* label = new QLabel(day, weekDayGroup_);
        label->setObjectName(QStringLiteral("weekDayLabel"));
        weekDayLayout_->addWidget(label, 1, Qt::AlignHCenter);
    }

    setViewportMargins(0, 38, 0, 0);
    weekDayLayout_->setSpacing(0);
    weekDayLayout_->setContentsMargins(3, 12, 3, 12);

    initialize();
}

QSize DayScrollView::gridSize() const { return QSize(44, 44); }

void DayScrollView::initItems() {
    const QDate startDate(minYear_, 1, 1);
    const QDate endDate(maxYear_, 12, 31);
    QDate current = startDate;

    // placeholder items
    const int bias = current.dayOfWeek() - 1;
    for (int i = 0; i < bias; ++i) {
        auto* it = new QListWidgetItem(this);
        it->setFlags(Qt::NoItemFlags);
        addItem(it);
    }

    QList<QDate> dates;
    while (current <= endDate) {
        dates.append(current);
        addItem(QString::number(current.day()));
        current = current.addDays(1);
    }

    for (int i = bias; i < count(); ++i) {
        auto* it = item(i);
        const QDate d = dates.value(i - bias);
        it->setData(Qt::UserRole, d);
        it->setSizeHint(gridSize());
    }

    if (delegate_) {
        delegate_->setCurrentIndex(model()->index(dateToRow(currentDate_), 0));
    }
}

void DayScrollView::setDate(const QDate& date) {
    scrollToDate(date);
    setCurrentIndex(model()->index(dateToRow(date), 0));
    setSelectedIndex(currentIndex());
}

void DayScrollView::scrollToDate(const QDate& date) {
    if (!date.isValid()) {
        return;
    }
    const int page = (date.year() - minYear_) * 12 + date.month() - 1;
    scrollToPage(page);
}

void DayScrollView::scrollToPage(int page) {
    if (page < 0 || page > 201 * 12 - 1) {
        return;
    }

    currentPage_ = page;

    const int index = dateToRow(pageToDate());
    const int y = (index / cols_) * gridSize().height();
    if (vScrollBar_) {
        vScrollBar_->scrollTo(y);
    }

    if (delegate_) {
        const auto r = currentPageRange();
        delegate_->setRange(r.first, r.second);
    }

    emit pageChanged(page);
}

QPair<QDate, QDate> DayScrollView::currentPageRange() const {
    const QDate d = pageToDate();
    return {d, d.addMonths(1).addDays(-1)};
}

QDate DayScrollView::pageToDate() const {
    const int year = currentPage_ / 12 + minYear_;
    const int month = currentPage_ % 12 + 1;
    return QDate(year, month, 1);
}

int DayScrollView::dateToRow(const QDate& date) const {
    const QDate startDate(minYear_, 1, 1);
    const int days = startDate.daysTo(date);
    return days + startDate.dayOfWeek() - 1;
}

void DayScrollView::mouseReleaseEvent(QMouseEvent* e) {
    ScrollViewBase::mouseReleaseEvent(e);
    setSelectedIndex(currentIndex());
}

void DayScrollView::resizeEvent(QResizeEvent* e) {
    ScrollViewBase::resizeEvent(e);
    if (weekDayGroup_) {
        weekDayGroup_->setGeometry(0, 0, width(), 38);
    }
}

// =========================================================================
// DayCalendarView
// =========================================================================

DayCalendarView::DayCalendarView(QWidget* parent) : CalendarViewBase(parent) {
    setScrollView(new DayScrollView(this));
}

void DayCalendarView::updateTitle() {
    if (!scrollView_) {
        return;
    }

    const QDate date = currentPageDate();
    const QString name = QCalendar().monthName(locale(), date.month(), date.year());
    setTitle(name + QStringLiteral(" ") + QString::number(date.year()));
}

QDate DayCalendarView::currentPageDate() const {
    if (!scrollView_) {
        return {};
    }
    return scrollView_->currentPageRange().first;
}

void DayCalendarView::scrollToDate(const QDate& date) {
    if (scrollView_) {
        scrollView_->scrollToDate(date);
    }
    updateTitle();
}

// =========================================================================
// CalendarView
// =========================================================================

CalendarView::CalendarView(QWidget* parent) : QWidget(parent) { initWidget(); }

void CalendarView::initWidget() {
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose, true);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 8, 12, 20);

    stackedWidget_ = new QStackedWidget(this);
    yearView_ = new YearCalendarView(this);
    monthView_ = new MonthCalendarView(this);
    dayView_ = new DayCalendarView(this);

    stackedWidget_->addWidget(dayView_);
    stackedWidget_->addWidget(monthView_);
    stackedWidget_->addWidget(yearView_);

    layout->addWidget(stackedWidget_);

    setShadowEffect();

    dayView_->setDate(QDate::currentDate());

    opacityAni_ = new QPropertyAnimation(this, "windowOpacity", this);
    slideAni_ = new QPropertyAnimation(this, "geometry", this);
    aniGroup_ = new QParallelAnimationGroup(this);
    aniGroup_->addAnimation(opacityAni_);
    aniGroup_->addAnimation(slideAni_);

    QObject::connect(dayView_, &CalendarViewBase::titleClicked, this,
                     &CalendarView::onDayViewTitleClicked);
    QObject::connect(monthView_, &CalendarViewBase::titleClicked, this,
                     &CalendarView::onMonthTitleClicked);

    QObject::connect(monthView_, &CalendarViewBase::itemClicked, this,
                     &CalendarView::onMonthItemClicked);
    QObject::connect(yearView_, &CalendarViewBase::itemClicked, this,
                     &CalendarView::onYearItemClicked);
    QObject::connect(dayView_, &CalendarViewBase::itemClicked, this,
                     &CalendarView::onDayItemClicked);

    QObject::connect(monthView_, &CalendarViewBase::resetted, this, &CalendarView::onResetted);
    QObject::connect(yearView_, &CalendarViewBase::resetted, this, &CalendarView::onResetted);
    QObject::connect(dayView_, &CalendarViewBase::resetted, this, &CalendarView::onResetted);
}

void CalendarView::setShadowEffect(int blurRadius, const QPoint& offset, const QColor& color) {
    if (!shadowEffect_) {
        shadowEffect_ = new QGraphicsDropShadowEffect(stackedWidget_);
    }

    shadowEffect_->setBlurRadius(blurRadius);
    shadowEffect_->setOffset(offset);
    shadowEffect_->setColor(color);

    stackedWidget_->setGraphicsEffect(nullptr);
    stackedWidget_->setGraphicsEffect(shadowEffect_);
}

bool CalendarView::isResetEnabled() const { return isResetEnabled_; }

void CalendarView::setResetEnabled(bool enabled) {
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

void CalendarView::setYearRange(int minYear, int maxYear) {
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

void CalendarView::setDate(const QDate& date) {
    if (dayView_) {
        dayView_->setDate(date);
    }
    date_ = date;
}

void CalendarView::exec(QPoint pos, bool ani) {
    if (isVisible()) {
        return;
    }

    const QRect rect = qfw::getCurrentScreenGeometry();
    const int w = sizeHint().width() + 5;
    const int h = sizeHint().height();

    pos.setX(qMax(rect.left(), qMin(pos.x(), rect.right() - w)));
    pos.setY(qMax(rect.top(), qMin(pos.y() - 4, rect.bottom() - h + 5)));

    move(pos);

    if (!ani) {
        show();
        return;
    }

    opacityAni_->stop();
    slideAni_->stop();

    opacityAni_->setStartValue(0);
    opacityAni_->setEndValue(1);
    opacityAni_->setDuration(150);
    opacityAni_->setEasingCurve(QEasingCurve::OutQuad);

    const QSize s = sizeHint();
    slideAni_->setStartValue(QRect(pos - QPoint(0, 8), s));
    slideAni_->setEndValue(QRect(pos, s));
    slideAni_->setDuration(150);
    slideAni_->setEasingCurve(QEasingCurve::OutQuad);

    aniGroup_->start();
    show();
}

void CalendarView::onResetted() {
    emit resetted();
    close();
}

void CalendarView::onDayViewTitleClicked() {
    stackedWidget_->setCurrentWidget(monthView_);
    monthView_->setDate(dayView_->currentPageDate());
}

void CalendarView::onMonthTitleClicked() {
    stackedWidget_->setCurrentWidget(yearView_);
    yearView_->setDate(monthView_->currentPageDate());
}

void CalendarView::onMonthItemClicked(const QDate& date) {
    stackedWidget_->setCurrentWidget(dayView_);
    dayView_->scrollToDate(date);
}

void CalendarView::onYearItemClicked(const QDate& date) {
    stackedWidget_->setCurrentWidget(monthView_);
    monthView_->setDate(date);
}

void CalendarView::onDayItemClicked(const QDate& date) {
    close();
    if (date != date_) {
        date_ = date;
        emit dateChanged(date);
    }
}

}  // namespace qfw
