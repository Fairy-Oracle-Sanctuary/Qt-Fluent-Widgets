#include "components/widgets/list_view.h"

#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>

#include "common/color.h"
#include "common/style_sheet.h"

namespace qfw {

// ============================================================================
// ListItemDelegate
// ============================================================================

ListItemDelegate::ListItemDelegate(QListView* parent) : TableItemDelegate(parent) {}

void ListItemDelegate::drawBackground(QPainter* painter, const QStyleOptionViewItem& option,
                                      const QModelIndex& index) const {
    Q_UNUSED(index);
    painter->drawRoundedRect(option.rect, 5, 5);
}

void ListItemDelegate::drawIndicator(QPainter* painter, const QStyleOptionViewItem& option,
                                     const QModelIndex& index) const {
    int y = option.rect.y();
    int h = option.rect.height();
    int ph = qRound((pressedRow_ == index.row()) ? 0.35 * h : 0.257 * h);
    QColor color;
    if (isDarkTheme()) {
        color = themedColor(themeColor(), true, QStringLiteral("ThemeColorLight1"));
    } else {
        color = autoFallbackThemeColor(lightCheckedColor_, darkCheckedColor_);
    }
    painter->setBrush(color);
    painter->drawRoundedRect(0, ph + y, 3, h - 2 * ph, 1.5, 1.5);
}

void ListItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                             const QModelIndex& index) const {
    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setRenderHint(QPainter::Antialiasing);

    painter->setClipping(true);
    painter->setClipRect(option.rect);

    QStyleOptionViewItem opt = option;
    // ListItemDelegate doesn't use the same margin adjustment as TableItemDelegate in python
    // but the paint() method in python calls super().paint() which does adjustment.
    // In list_view.py, ListItemDelegate inherits TableItemDelegate.
    opt.rect.adjust(0, margin_, 0, -margin_);

    bool isHover = (hoverRow_ == index.row());
    bool isPressed = (pressedRow_ == index.row());
    bool isDark = isDarkTheme();

    int c = isDark ? 255 : 0;
    int alpha = 0;

    if (!selectedRows_.contains(index.row())) {
        if (isPressed) {
            alpha = isDark ? 9 : 6;
        } else if (isHover) {
            alpha = 12;
        }
    } else {
        if (isPressed) {
            alpha = isDark ? 15 : 9;
        } else if (isHover) {
            alpha = 25;
        } else {
            alpha = 17;
        }
    }

    QVariant bgData = index.data(Qt::BackgroundRole);
    if (bgData.isValid()) {
        painter->setBrush(bgData.value<QBrush>());
    } else {
        painter->setBrush(QColor(c, c, c, alpha));
    }

    drawBackground(painter, opt, index);

    if (selectedRows_.contains(index.row())) {
        drawIndicator(painter, opt, index);
    }

    painter->restore();
    QStyledItemDelegate::paint(painter, opt, index);
}

// ============================================================================
// ListBase
// ============================================================================

ListBase::ListBase() {}

void ListBase::initListBase(QListView* view) {
    listView_ = view;
    delegate_ = new ListItemDelegate(view);
    scrollDelegate_ = new SmoothScrollDelegate(view);
    scrollDelegate_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollDelegate_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    qfw::setStyleSheet(view, FluentStyleSheet::ListView);
    view->setItemDelegate(delegate_);
    view->setMouseTracking(true);

    QObject::connect(view, &QListView::entered,
                     [this](const QModelIndex& index) { setHoverRow(index.row()); });

    QObject::connect(view, &QListView::pressed,
                     [this](const QModelIndex& index) { setPressedRow(index.row()); });
}

void ListBase::setHoverRow(int row) {
    delegate_->setHoverRow(row);
    listView_->viewport()->update();
}

void ListBase::setPressedRow(int row) {
    if (listView_->selectionMode() == QAbstractItemView::NoSelection) {
        return;
    }
    delegate_->setPressedRow(row);
    listView_->viewport()->update();
}

void ListBase::setSelectedRows(const QList<QModelIndex>& indexes) {
    if (listView_->selectionMode() == QAbstractItemView::NoSelection) {
        return;
    }
    delegate_->setSelectedRows(indexes);
    listView_->viewport()->update();
}

void ListBase::updateSelectedRows() {
    setSelectedRows(listView_->selectionModel()->selectedIndexes());
}

void ListBase::leaveEvent(QEvent* e) {
    setHoverRow(-1);
    Q_UNUSED(e);
}

void ListBase::resizeEvent(QResizeEvent* e) {
    listView_->viewport()->update();
    Q_UNUSED(e);
}

void ListBase::keyPressEvent(QKeyEvent* e) {
    Q_UNUSED(e);
    updateSelectedRows();
}

void ListBase::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton || isSelectRightClickedRow_) {
        return;
    }

    QModelIndex index = listView_->indexAt(e->pos());
    if (index.isValid()) {
        setPressedRow(index.row());
    }
}

void ListBase::mouseReleaseEvent(QMouseEvent* e) {
    updateSelectedRows();

    if (listView_->indexAt(e->pos()).row() < 0 || e->button() == Qt::RightButton) {
        setPressedRow(-1);
    }
}

void ListBase::setCheckedColor(const QColor& light, const QColor& dark) {
    delegate_->setCheckedColor(light, dark);
}

void ListBase::setBorderVisible(bool visible) {
    if (!listView_) {
        return;
    }

    listView_->setProperty("isBorderVisible", visible);
    qfw::updateDynamicStyle(listView_);
}

// ============================================================================
// ListWidget
// ============================================================================

ListWidget::ListWidget(QWidget* parent) : QListWidget(parent), ListBase() { initListBase(this); }

void ListWidget::setCurrentItem(QListWidgetItem* item, QItemSelectionModel::SelectionFlag command) {
    setCurrentRow(row(item), command);
}

void ListWidget::setCurrentRow(int row, QItemSelectionModel::SelectionFlag command) {
    if (command == QItemSelectionModel::NoUpdate) {
        QListWidget::setCurrentRow(row);
    } else {
        selectionModel()->setCurrentIndex(model()->index(row, 0), command);
    }
    updateSelectedRows();
}

bool ListWidget::isSelectRightClickedRow() const { return isSelectRightClickedRow_; }

void ListWidget::setSelectRightClickedRow(bool isSelect) { isSelectRightClickedRow_ = isSelect; }

void ListWidget::setCheckedColor(const QColor& light, const QColor& dark) {
    ListBase::setCheckedColor(light, dark);
}

void ListWidget::setBorderVisible(bool visible) { ListBase::setBorderVisible(visible); }

void ListWidget::clearSelection() {
    QListWidget::clearSelection();
    updateSelectedRows();
}

void ListWidget::leaveEvent(QEvent* e) {
    QListWidget::leaveEvent(e);
    ListBase::leaveEvent(e);
}

void ListWidget::resizeEvent(QResizeEvent* e) {
    QListWidget::resizeEvent(e);
    ListBase::resizeEvent(e);
}

void ListWidget::keyPressEvent(QKeyEvent* e) {
    QListWidget::keyPressEvent(e);
    ListBase::keyPressEvent(e);
}

void ListWidget::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton || isSelectRightClickedRow_) {
        QListWidget::mousePressEvent(e);
        return;
    }
    ListBase::mousePressEvent(e);
    QListWidget::mousePressEvent(e);
}

void ListWidget::mouseReleaseEvent(QMouseEvent* e) {
    QListWidget::mouseReleaseEvent(e);
    ListBase::mouseReleaseEvent(e);
}

// ============================================================================
// ListView
// ============================================================================

ListView::ListView(QWidget* parent) : QListView(parent), ListBase() { initListBase(this); }

bool ListView::isSelectRightClickedRow() const { return isSelectRightClickedRow_; }

void ListView::setSelectRightClickedRow(bool isSelect) { isSelectRightClickedRow_ = isSelect; }

void ListView::setCheckedColor(const QColor& light, const QColor& dark) {
    ListBase::setCheckedColor(light, dark);
}

void ListView::setBorderVisible(bool visible) { ListBase::setBorderVisible(visible); }

void ListView::clearSelection() {
    QListView::clearSelection();
    updateSelectedRows();
}

void ListView::setCurrentIndex(const QModelIndex& index) {
    QListView::setCurrentIndex(index);
    updateSelectedRows();
}

void ListView::leaveEvent(QEvent* e) {
    QListView::leaveEvent(e);
    ListBase::leaveEvent(e);
}

void ListView::resizeEvent(QResizeEvent* e) {
    QListView::resizeEvent(e);
    ListBase::resizeEvent(e);
}

void ListView::keyPressEvent(QKeyEvent* e) {
    QListView::keyPressEvent(e);
    ListBase::keyPressEvent(e);
}

void ListView::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton || isSelectRightClickedRow_) {
        QListView::mousePressEvent(e);
        return;
    }
    ListBase::mousePressEvent(e);
    QListView::mousePressEvent(e);
}

void ListView::mouseReleaseEvent(QMouseEvent* e) {
    QListView::mouseReleaseEvent(e);
    ListBase::mouseReleaseEvent(e);
}

}  // namespace qfw
