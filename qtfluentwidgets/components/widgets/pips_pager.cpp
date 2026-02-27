#include "pips_pager.h"

#include <QListWidgetItem>
#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>

#include "common/style_sheet.h"
#include "components/widgets/scroll_bar.h"
#include "components/widgets/tool_tip.h"

namespace qfw {

// PipsScrollButton implementation

PipsScrollButton::PipsScrollButton(FluentIconEnum iconEnum, QWidget* parent)
    : QWidget(parent), iconEnum_(iconEnum) {
    setFixedSize(12, 12);
}

void PipsScrollButton::setHover(bool hover) {
    isHover_ = hover;
    update();
}

void PipsScrollButton::setPressed(bool pressed) {
    isPressed_ = pressed;
    update();
}

bool PipsScrollButton::isHover() const { return isHover_; }

bool PipsScrollButton::isPressed() const { return isPressed_; }

void PipsScrollButton::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e)

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    QColor color;
    if (isDarkTheme()) {
        color = QColor(255, 255, 255);
        painter.setOpacity(isHover_ || isPressed_ ? 0.773 : 0.541);
    } else {
        color = QColor(0, 0, 0);
        painter.setOpacity(isHover_ || isPressed_ ? 0.616 : 0.45);
    }

    QRectF rect;
    if (isPressed_) {
        rect = QRectF(3, 3, 6, 6);
    } else {
        rect = QRectF(2, 2, 8, 8);
    }

    FluentIcon(iconEnum_).render(&painter, rect.toRect(), Theme::Auto, {{"fill", color.name()}});
}

void PipsScrollButton::enterEvent(QEnterEvent* e) {
    isHover_ = true;
    update();
    QWidget::enterEvent(e);
}

void PipsScrollButton::leaveEvent(QEvent* e) {
    isHover_ = false;
    update();
    QWidget::leaveEvent(e);
}

void PipsScrollButton::mousePressEvent(QMouseEvent* e) {
    isPressed_ = true;
    update();
    QWidget::mousePressEvent(e);
}

void PipsScrollButton::mouseReleaseEvent(QMouseEvent* e) {
    isPressed_ = false;
    update();
    if (rect().contains(e->pos())) {
        emit clicked();
    }
    QWidget::mouseReleaseEvent(e);
}

// PipsDelegate implementation

PipsDelegate::PipsDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

void PipsDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                         const QModelIndex& index) const {
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);

    bool isHover = index.row() == hoveredRow_;
    bool isPressed = index.row() == pressedRow_;

    QColor color;
    if (isDarkTheme()) {
        if (isHover || isPressed) {
            color = QColor(255, 255, 255, 197);
        } else {
            color = QColor(255, 255, 255, 138);
        }
    } else {
        if (isHover || isPressed) {
            color = QColor(0, 0, 0, 157);
        } else {
            color = QColor(0, 0, 0, 114);
        }
    }

    painter->setBrush(color);

    int r = 2;
    if (option.state & QStyle::State_Selected || (isHover && !isPressed)) {
        r = 3;
    }

    int x = option.rect.x() + 6 - r;
    int y = option.rect.y() + 6 - r;
    painter->drawEllipse(QRectF(x, y, 2 * r, 2 * r));

    painter->restore();
}

void PipsDelegate::setHoveredRow(int row) {
    hoveredRow_ = row;
    if (parent()) {
        qobject_cast<QListWidget*>(parent())->viewport()->update();
    }
}

void PipsDelegate::setPressedRow(int row) {
    pressedRow_ = row;
    if (parent()) {
        qobject_cast<QListWidget*>(parent())->viewport()->update();
    }
}

int PipsDelegate::hoveredRow() const { return hoveredRow_; }

int PipsDelegate::pressedRow() const { return pressedRow_; }

// PipsPager implementation

PipsPager::PipsPager(QWidget* parent) : QListWidget(parent) { init(Qt::Horizontal); }

PipsPager::PipsPager(Qt::Orientation orientation, QWidget* parent) : QListWidget(parent) {
    init(orientation);
}

void PipsPager::init(Qt::Orientation orientation) {
    orientation_ = orientation;
    visibleNumber_ = 5;
    isHover_ = false;

    delegate_ = new PipsDelegate(this);
    scrollBar_ = new SmoothScrollBar(orientation_, this);

    scrollBar_->setScrollAnimation(500);
    scrollBar_->setForceHidden(true);

    setMouseTracking(true);
    setUniformItemSizes(true);
    setGridSize(QSize(12, 12));
    setItemDelegate(delegate_);
    setMovement(QListWidget::Static);
    setVerticalScrollMode(QListWidget::ScrollPerPixel);
    setHorizontalScrollMode(QListWidget::ScrollPerPixel);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setStyleSheet(QStringLiteral("QListWidget{background:transparent;border:none;}"));

    if (isHorizontal()) {
        setFlow(QListWidget::LeftToRight);
        setViewportMargins(15, 0, 15, 0);
        preButton_ = new PipsScrollButton(FluentIconEnum::CareLeftSolid, this);
        nextButton_ = new PipsScrollButton(FluentIconEnum::CareRightSolid, this);
        setFixedHeight(12);

        preButton_->installEventFilter(new ToolTipFilter(preButton_, 1000, ToolTipPosition::Left));
        nextButton_->installEventFilter(
            new ToolTipFilter(nextButton_, 1000, ToolTipPosition::Right));
    } else {
        setViewportMargins(0, 15, 0, 15);
        preButton_ = new PipsScrollButton(FluentIconEnum::CareUpSolid, this);
        nextButton_ = new PipsScrollButton(FluentIconEnum::CareDownSolid, this);
        setFixedWidth(12);

        preButton_->installEventFilter(new ToolTipFilter(preButton_, 1000, ToolTipPosition::Top));
        nextButton_->installEventFilter(
            new ToolTipFilter(nextButton_, 1000, ToolTipPosition::Bottom));
    }

    setPreviousButtonDisplayMode(PipsScrollButtonDisplayMode::Never);
    setNextButtonDisplayMode(PipsScrollButtonDisplayMode::Never);
    preButton_->setToolTip(tr("Previous Page"));
    nextButton_->setToolTip(tr("Next Page"));

    connect(preButton_, &PipsScrollButton::clicked, this, &PipsPager::scrollPrevious);
    connect(nextButton_, &PipsScrollButton::clicked, this, &PipsPager::scrollNext);
    connect(this, &QListWidget::itemPressed, this, &PipsPager::setPressedItem);
    connect(this, &QListWidget::itemEntered, this, &PipsPager::setHoveredItem);
}

void PipsPager::setPressedItem(QListWidgetItem* item) {
    delegate_->setPressedRow(row(item));
    setCurrentIndex(row(item));
}

void PipsPager::setHoveredItem(QListWidgetItem* item) { delegate_->setHoveredRow(row(item)); }

void PipsPager::setPageNumber(int n) {
    clear();
    for (int i = 0; i < n; ++i) {
        auto* item = new QListWidgetItem();
        item->setData(Qt::UserRole, i + 1);
        item->setSizeHint(gridSize());
        addItem(item);
    }
    setCurrentIndex(0);
    adjustSize();
}

int PipsPager::pageNumber() const { return count(); }

void PipsPager::setVisibleNumber(int n) {
    visibleNumber_ = n;
    adjustSize();
}

int PipsPager::visibleNumber() const { return visibleNumber_; }

void PipsPager::setCurrentIndex(int index) {
    if (index < 0 || index >= count()) {
        return;
    }

    auto* item = this->item(index);
    scrollToItem(item);
    QListWidget::setCurrentItem(item);

    updateScrollButtonVisibility();
}

int PipsPager::currentIndex() const { return QListWidget::currentRow(); }

void PipsPager::setPreviousButtonDisplayMode(PipsScrollButtonDisplayMode mode) {
    previousButtonDisplayMode_ = mode;
    preButton_->setVisible(isPreviousButtonVisible());
}

void PipsPager::setNextButtonDisplayMode(PipsScrollButtonDisplayMode mode) {
    nextButtonDisplayMode_ = mode;
    nextButton_->setVisible(isNextButtonVisible());
}

PipsScrollButtonDisplayMode PipsPager::previousButtonDisplayMode() const {
    return previousButtonDisplayMode_;
}

PipsScrollButtonDisplayMode PipsPager::nextButtonDisplayMode() const {
    return nextButtonDisplayMode_;
}

void PipsPager::scrollNext() { setCurrentIndex(currentIndex() + 1); }

void PipsPager::scrollPrevious() { setCurrentIndex(currentIndex() - 1); }

bool PipsPager::isHorizontal() const { return orientation_ == Qt::Horizontal; }

Qt::Orientation PipsPager::orientation() const { return orientation_; }

void PipsPager::scrollToItem(QListWidgetItem* item) {
    int index = row(item);
    QSize size = item->sizeHint();
    int s = isHorizontal() ? size.width() : size.height();
    scrollBar_->scrollTo(s * (index - visibleNumber_ / 2));

    clearSelection();
    item->setSelected(false);

    emit currentIndexChanged(index);
}

void PipsPager::adjustSize() {
    QMargins m = viewportMargins();

    if (isHorizontal()) {
        int w = visibleNumber_ * gridSize().width() + m.left() + m.right();
        setFixedWidth(w);
    } else {
        int h = visibleNumber_ * gridSize().height() + m.top() + m.bottom();
        setFixedHeight(h);
    }
}

bool PipsPager::isPreviousButtonVisible() const {
    if (currentIndex() <= 0 || previousButtonDisplayMode_ == PipsScrollButtonDisplayMode::Never) {
        return false;
    }

    if (previousButtonDisplayMode_ == PipsScrollButtonDisplayMode::OnHover) {
        return isHover_;
    }

    return true;
}

bool PipsPager::isNextButtonVisible() const {
    if (currentIndex() >= count() - 1 ||
        nextButtonDisplayMode_ == PipsScrollButtonDisplayMode::Never) {
        return false;
    }

    if (nextButtonDisplayMode_ == PipsScrollButtonDisplayMode::OnHover) {
        return isHover_;
    }

    return true;
}

void PipsPager::updateScrollButtonVisibility() {
    preButton_->setVisible(isPreviousButtonVisible());
    nextButton_->setVisible(isNextButtonVisible());
}

void PipsPager::mouseReleaseEvent(QMouseEvent* e) {
    QListWidget::mouseReleaseEvent(e);
    delegate_->setPressedRow(-1);
}

void PipsPager::enterEvent(QEnterEvent* e) {
    QListWidget::enterEvent(e);
    isHover_ = true;
    updateScrollButtonVisibility();
}

void PipsPager::leaveEvent(QEvent* e) {
    QListWidget::leaveEvent(e);
    isHover_ = false;
    delegate_->setHoveredRow(-1);
    updateScrollButtonVisibility();
}

void PipsPager::wheelEvent(QWheelEvent* e) {
    Q_UNUSED(e)
    // Ignore wheel events
}

void PipsPager::resizeEvent(QResizeEvent* e) {
    QListWidget::resizeEvent(e);

    int w = width();
    int h = height();
    int bw = preButton_->width();
    int bh = preButton_->height();

    if (isHorizontal()) {
        preButton_->move(0, h / 2 - bh / 2);
        nextButton_->move(w - bw, h / 2 - bh / 2);
    } else {
        preButton_->move(w / 2 - bw / 2, 0);
        nextButton_->move(w / 2 - bw / 2, h - bh);
    }
}

// HorizontalPipsPager implementation

HorizontalPipsPager::HorizontalPipsPager(QWidget* parent) : PipsPager(Qt::Horizontal, parent) {}

// VerticalPipsPager implementation

VerticalPipsPager::VerticalPipsPager(QWidget* parent) : PipsPager(Qt::Vertical, parent) {}

}  // namespace qfw
