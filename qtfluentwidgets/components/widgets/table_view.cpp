#include "components/widgets/table_view.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QHelpEvent>
#include <QKeyEvent>
#include <QMargins>
#include <QPainter>
#include <QPalette>
#include <QScrollBar>
#include <QStyleFactory>

#include "common/color.h"
#include "common/font.h"
#include "common/style_sheet.h"
#include "components/widgets/check_box.h"
#include "components/widgets/line_edit.h"
#include "components/widgets/scroll_bar.h"
#include "components/widgets/tool_tip.h"

namespace qfw {

// ============================================================================
// TableItemDelegate
// ============================================================================

TableItemDelegate::TableItemDelegate(QTableView* parent) : QStyledItemDelegate(parent) {
    if (parent) {
        tooltipDelegate_ = new ItemViewToolTipDelegate(parent, 100, ItemViewToolTipType::Table);
    }
}

TableItemDelegate::TableItemDelegate(QAbstractItemView* parent) : QStyledItemDelegate(parent) {
    if (parent) {
        tooltipDelegate_ = new ItemViewToolTipDelegate(parent, 100, ItemViewToolTipType::Table);
    }
}

void TableItemDelegate::setHoverRow(int row) { hoverRow_ = row; }

void TableItemDelegate::setPressedRow(int row) { pressedRow_ = row; }

void TableItemDelegate::setSelectedRows(const QList<QModelIndex>& indexes) {
    selectedRows_.clear();
    for (const auto& index : indexes) {
        selectedRows_.insert(index.row());
        if (index.row() == pressedRow_) {
            pressedRow_ = -1;
        }
    }
}

void TableItemDelegate::setCheckedColor(const QColor& light, const QColor& dark) {
    lightCheckedColor_ = light;
    darkCheckedColor_ = dark;
    if (auto* view = qobject_cast<QTableView*>(parent())) {
        view->viewport()->update();
    }
}

void TableItemDelegate::refresh() {
    if (auto* view = qobject_cast<QAbstractItemView*>(parent())) {
        view->doItemsLayout();
        view->viewport()->update();
    }
}
bool TableItemDelegate::showIndicator() const { return showIndicator_; }
void TableItemDelegate::setShowIndicator(bool show) {
    showIndicator_ = show;
    refresh();
}
int TableItemDelegate::borderRadius() const { return borderRadius_; }
void TableItemDelegate::setBorderRadius(int radius) {
    borderRadius_ = qMax(0, radius);
    refresh();
}
bool TableItemDelegate::useStackedWidgetStyle() const { return useStackedWidgetStyle_; }
void TableItemDelegate::setUseStackedWidgetStyle(bool use) {
    useStackedWidgetStyle_ = use;
    refresh();
}
bool TableItemDelegate::useTransparentRows() const { return useTransparentRows_; }
void TableItemDelegate::setUseTransparentRows(bool use) {
    useTransparentRows_ = use;
    refresh();
}
bool TableItemDelegate::useThemeColorSelection() const { return useThemeColorSelection_; }
void TableItemDelegate::setUseThemeColorSelection(bool use) {
    useThemeColorSelection_ = use;
    refresh();
}
bool TableItemDelegate::showRowDivider() const { return showRowDivider_; }
void TableItemDelegate::setShowRowDivider(bool show) {
    showRowDivider_ = show;
    refresh();
}
int TableItemDelegate::rowMargin() const { return margin_; }
void TableItemDelegate::setRowMargin(int margin) {
    margin_ = qMax(0, margin);
    refresh();
}

QSize TableItemDelegate::sizeHint(const QStyleOptionViewItem& option,
                                  const QModelIndex& index) const {
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    size = size.grownBy(QMargins(0, margin_, 0, margin_));
    return size;
}

QWidget* TableItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                                         const QModelIndex& index) const {
    Q_UNUSED(index);

    auto* lineEdit = new LineEdit(parent);
    lineEdit->setProperty("transparent", false);
    lineEdit->setStyle(QApplication::style());
    lineEdit->setText(option.text);
    lineEdit->setClearButtonEnabled(true);
    return lineEdit;
}

void TableItemDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
                                             const QModelIndex& index) const {
    QRect rect = option.rect;
    int y = rect.y() + (rect.height() - editor->height()) / 2;
    int x = qMax(8, rect.x());
    int w = rect.width();

    if (index.column() == 0) {
        w -= 8;
    }

    editor->setGeometry(x, y, w, rect.height());
}

void TableItemDelegate::initStyleOption(QStyleOptionViewItem* option,
                                        const QModelIndex& index) const {
    QStyledItemDelegate::initStyleOption(option, index);

    // font
    QVariant fontData = index.data(Qt::FontRole);
    if (fontData.isValid()) {
        option->font = fontData.value<QFont>();
    } else {
        option->font = qfw::getFont(13);
    }

    // text color
    QColor textColor = isDarkTheme() ? Qt::white : Qt::black;
    QVariant textBrush = index.data(Qt::ForegroundRole);
    if (textBrush.isValid()) {
        textColor = textBrush.value<QBrush>().color();
    }

    option->palette.setColor(QPalette::Text, textColor);
    option->palette.setColor(QPalette::HighlightedText, textColor);
    if (useStackedWidgetStyle_ || useTransparentRows_ || useThemeColorSelection_ || showRowDivider_)
        option->fontMetrics = QFontMetrics(option->font);
}

void TableItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                              const QModelIndex& index) const {
    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setRenderHint(QPainter::Antialiasing);

    // set clipping rect
    painter->setClipping(true);
    painter->setClipRect(option.rect);

    // adjust rect for border
    QStyleOptionViewItem opt = option;
    opt.rect.adjust(0, margin_, 0, -margin_);

    // draw highlight background
    bool isHover = hoverRow_ == index.row();
    auto* tableView = qobject_cast<QTableView*>(parent());
    const bool extended =
        useStackedWidgetStyle_ || useTransparentRows_ || useThemeColorSelection_ || showRowDivider_;
    bool isPressed = pressedRow_ == index.row();
    bool isAlternate = index.row() % 2 == 0 && tableView && tableView->alternatingRowColors();
    const bool isSelected =
        extended && tableView
            ? tableView->selectionModel() &&
                  tableView->selectionModel()->rowIntersectsSelection(index.row(), index.parent())
            : selectedRows_.contains(index.row());
    bool isDark = isDarkTheme();

    int c = isDark ? 255 : 0;
    int alpha = 0;

    if (!isSelected) {
        if (isPressed) {
            alpha = isDark ? 9 : 6;
        } else if (isHover) {
            alpha = 12;
        } else if (isAlternate) {
            alpha = 5;
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
    } else if (useThemeColorSelection_ && isSelected) {
        QColor color = themeColor();
        color.setAlpha(isHover ? 40 : isPressed ? 60 : 30);
        painter->setBrush(color);
    } else if ((useTransparentRows_ ? isSelected
                                    : useStackedWidgetStyle_ && (isAlternate || isSelected)) &&
               !isHover && !isPressed) {
        painter->setBrush(QColor(255, 255, 255, isDark ? 8 : 127));
    } else {
        painter->setBrush(QColor(c, c, c, alpha));
    }

    drawBackground(painter, opt, index);

    if ((useTransparentRows_ ? isSelected
                             : useStackedWidgetStyle_ && (isAlternate || isSelected)) &&
        !isHover && !isPressed && !bgData.isValid()) {
        painter->setPen(QColor(0, 0, 0, isDark ? 46 : 17));
        painter->setBrush(Qt::NoBrush);
        drawBackground(painter, opt, index);
        painter->setPen(Qt::NoPen);
    }

    // draw indicator
    if (showIndicator_ && isSelected && index.column() == 0 && tableView &&
        tableView->horizontalScrollBar()->value() == 0) {
        drawIndicator(painter, opt, index);
    }

    // draw checkbox
    QVariant checkData = index.data(Qt::CheckStateRole);
    if (checkData.isValid()) {
        drawCheckBox(painter, opt, index);
    }

    if (showRowDivider_) {
        // Paint within each cell's clip, not an unclipped line from logical column
        // zero. Dividers stay continuous after hiding/moving columns and scrolling.
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(c, c, c, isDark ? 47 : 17));
        painter->drawRect(QRectF(option.rect.left(), option.rect.bottom(), option.rect.width(), 1));
    }

    painter->restore();
    QStyledItemDelegate::paint(painter, opt, index);
}

void TableItemDelegate::drawBackground(QPainter* painter, const QStyleOptionViewItem& option,
                                       const QModelIndex& index) const {
    int r = borderRadius_;
    if (r == 0) {
        painter->drawRect(option.rect.adjusted(-1, 0, 1, 0));
        return;
    }
    if (useStackedWidgetStyle_ || useTransparentRows_) {
        // One shared outline per row, clipped into the individual cells. Visible
        // physical edges, rather than logical columns 0/N, also handle RTL and
        // single-column tables without seams or squared-off outer corners.
        if (auto* view = qobject_cast<QTableView*>(parent())) {
            auto* header = view->horizontalHeader();
            int first = -1, last = -1;
            for (int visual = 0; visual < header->count(); ++visual) {
                const int logical = header->logicalIndex(visual);
                if (header->isSectionHidden(logical)) continue;
                if (first < 0) first = logical;
                last = logical;
            }
            if (first >= 0) {
                const int start = qMin(header->sectionViewportPosition(first),
                                       header->sectionViewportPosition(last));
                const int end =
                    qMax(header->sectionViewportPosition(first) + header->sectionSize(first),
                         header->sectionViewportPosition(last) + header->sectionSize(last));
                painter->drawRoundedRect(QRectF(start + 4, option.rect.top(),
                                                qMax(0, end - start - 8), option.rect.height()),
                                         r, r);
                return;
            }
        }
    }
    auto* model = index.model();
    int columnCount = model->columnCount(index.parent());

    if (index.column() == 0) {
        QRect rect = option.rect.adjusted(4, 0, r + 1, 0);
        painter->drawRoundedRect(rect, r, r);
    } else if (index.column() == columnCount - 1) {
        QRect rect = option.rect.adjusted(-r - 1, 0, -4, 0);
        painter->drawRoundedRect(rect, r, r);
    } else {
        QRect rect = option.rect.adjusted(-1, 0, 1, 0);
        painter->drawRect(rect);
    }
}

void TableItemDelegate::drawIndicator(QPainter* painter, const QStyleOptionViewItem& option,
                                      const QModelIndex& index) const {
    Q_UNUSED(index);

    int y = option.rect.y();
    int h = option.rect.height();
    int ph = qRound(pressedRow_ == index.row() ? 0.35 * h : 0.257 * h);

    const QColor color = autoFallbackThemeColor(lightCheckedColor_, darkCheckedColor_);
    painter->setBrush(color);
    painter->drawRoundedRect(4, ph + y, 3, h - 2 * ph, 1.5, 1.5);
}

void TableItemDelegate::drawCheckBox(QPainter* painter, const QStyleOptionViewItem& option,
                                     const QModelIndex& index) const {
    painter->save();

    Qt::CheckState checkState = static_cast<Qt::CheckState>(index.data(Qt::CheckStateRole).toInt());
    bool isDark = isDarkTheme();

    qreal r = 4.5;
    qreal x = option.direction == Qt::RightToLeft ? option.rect.right() - 33 : option.rect.x() + 15;
    qreal y = option.rect.center().y() - 9.5;
    QRectF rect(x, y, 19, 19);

    if (checkState == Qt::Unchecked) {
        painter->setBrush(isDark ? QColor(0, 0, 0, 26) : QColor(0, 0, 0, 6));
        painter->setPen(isDark ? QColor(255, 255, 255, 142) : QColor(0, 0, 0, 122));
        painter->drawRoundedRect(rect, r, r);
    } else {
        const QColor color = autoFallbackThemeColor(lightCheckedColor_, darkCheckedColor_);
        painter->setPen(color);
        painter->setBrush(color);
        painter->drawRoundedRect(rect, r, r);

        CheckBoxIcon icon = (checkState == Qt::Checked) ? CheckBoxIcon(CheckBoxIcon::Accept)
                                                        : CheckBoxIcon(CheckBoxIcon::PartialAccept);
        icon.render(painter, rect.toRect());
    }

    painter->restore();
}

bool TableItemDelegate::helpEvent(QHelpEvent* event, QAbstractItemView* view,
                                  const QStyleOptionViewItem& option, const QModelIndex& index) {
    if (tooltipDelegate_) {
        return tooltipDelegate_->helpEvent(event, view, option, index);
    }
    return QStyledItemDelegate::helpEvent(event, view, option, index);
}

// ============================================================================
// TableBase
// ============================================================================

TableBase::TableBase() {}

void TableBase::initTableBase(QTableView* view) {
    tableView_ = view;
    delegate_ = new TableItemDelegate(view);
    scrollDelegate_ = new SmoothScrollDelegate(view);
    scrollDelegate_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollDelegate_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    qfw::setStyleSheet(view, FluentStyleSheet::TableView);

    view->setShowGrid(false);
    view->setMouseTracking(true);
    view->setAlternatingRowColors(true);
    view->setItemDelegate(delegate_);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->horizontalHeader()->setHighlightSections(false);
    view->verticalHeader()->setHighlightSections(false);
    view->verticalHeader()->setDefaultSectionSize(38);

    QObject::connect(view, &QTableView::entered, view,
                     [this](const QModelIndex& index) { setHoverRow(index.row()); });

    QObject::connect(view, &QTableView::pressed, view,
                     [this](const QModelIndex& index) { setPressedRow(index.row()); });

    QObject::connect(view->verticalHeader(), &QHeaderView::sectionClicked, view, [this](int row) {
        if (auto* widget = qobject_cast<TableWidget*>(tableView_)) {
            widget->selectRow(row);
        } else if (auto* widget = qobject_cast<TableView*>(tableView_)) {
            widget->selectRow(row);
        }
    });
}

void TableBase::setBorderVisible(bool visible) {
    tableView_->setProperty("isBorderVisible", visible);
    qfw::updateDynamicStyle(tableView_);
}

void TableBase::setBorderRadius(int radius) {
    QString qss = QStringLiteral("QTableView{border-radius: %1px}").arg(radius);
    qfw::setCustomStyleSheet(tableView_, qss, qss);
}

void TableBase::setCheckedColor(const QColor& light, const QColor& dark) {
    delegate_->setCheckedColor(light, dark);
}

void TableBase::setHoverRow(int row) {
    delegate_->setHoverRow(row);
    tableView_->viewport()->update();
}

void TableBase::setPressedRow(int row) {
    if (tableView_->selectionMode() == QAbstractItemView::NoSelection) {
        return;
    }
    delegate_->setPressedRow(row);
    tableView_->viewport()->update();
}

void TableBase::setSelectedRows(const QList<QModelIndex>& indexes) {
    if (tableView_->selectionMode() == QAbstractItemView::NoSelection) {
        return;
    }
    delegate_->setSelectedRows(indexes);
    tableView_->viewport()->update();
}

void TableBase::updateSelectedRows() {
    if (!tableView_->selectionModel()) return;
    setSelectedRows(tableView_->selectionModel()->selectedIndexes());
}

void TableBase::leaveEvent(QEvent* e) {
    setHoverRow(-1);
    Q_UNUSED(e);
}

void TableBase::resizeEvent(QResizeEvent* e) {
    tableView_->viewport()->update();
    Q_UNUSED(e);
}

void TableBase::keyPressEvent(QKeyEvent* e) {
    Q_UNUSED(e);
    updateSelectedRows();
}

void TableBase::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton || isSelectRightClickedRow_) {
        return;
    }

    QModelIndex index = tableView_->indexAt(e->pos());
    if (index.isValid()) {
        setPressedRow(index.row());
    }
}

void TableBase::mouseReleaseEvent(QMouseEvent* e) {
    updateSelectedRows();

    if (delegate_->useStackedWidgetStyle() || delegate_->useTransparentRows() ||
        delegate_->useThemeColorSelection() || delegate_->showRowDivider() ||
        tableView_->indexAt(e->pos()).row() < 0 || e->button() == Qt::RightButton) {
        setPressedRow(-1);
    }
}

// ============================================================================
// TableWidget
// ============================================================================

TableWidget::TableWidget(QWidget* parent) : QTableWidget(parent), TableBase() {
    initTableBase(this);
    qfw::updateDynamicStyle(this);
    verticalHeader()->hide();
}

void TableWidget::setBorderVisible(bool visible) { TableBase::setBorderVisible(visible); }

void TableWidget::setBorderRadius(int radius) { TableBase::setBorderRadius(radius); }

void TableWidget::setCheckedColor(const QColor& light, const QColor& dark) {
    TableBase::setCheckedColor(light, dark);
}

void TableWidget::setCurrentCell(int row, int column, QItemSelectionModel::SelectionFlags command) {
    setCurrentItem(item(row, column), command);
}

void TableWidget::setCurrentItem(QTableWidgetItem* item,
                                 QItemSelectionModel::SelectionFlags command) {
    if (command == QItemSelectionModel::NoUpdate) {
        QTableWidget::setCurrentItem(item);
    } else {
        QTableWidget::setCurrentItem(item, command);
    }
    updateSelectedRows();
}

bool TableWidget::isSelectRightClickedRow() const { return isSelectRightClickedRow_; }

void TableWidget::setSelectRightClickedRow(bool isSelect) { isSelectRightClickedRow_ = isSelect; }

void TableWidget::selectAll() {
    QTableWidget::selectAll();
    updateSelectedRows();
}

void TableWidget::selectRow(int row) {
    if (delegate_->useStackedWidgetStyle() || delegate_->useTransparentRows() ||
        delegate_->useThemeColorSelection() || delegate_->showRowDivider()) {
        const auto index = model()->index(row, 0, rootIndex());
        if (index.isValid() && selectionMode() != QAbstractItemView::NoSelection) {
            const auto command = selectionCommand(index) | QItemSelectionModel::Rows;
            selectionModel()->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
            selectionModel()->select(index, command);
        }
    } else {
        QTableWidget::selectRow(row);
    }
    updateSelectedRows();
}

void TableWidget::clearSelection() {
    QTableWidget::clearSelection();
    updateSelectedRows();
}

void TableWidget::leaveEvent(QEvent* e) { TableBase::leaveEvent(e); }

void TableWidget::resizeEvent(QResizeEvent* e) {
    QTableWidget::resizeEvent(e);
    TableBase::resizeEvent(e);
}

void TableWidget::keyPressEvent(QKeyEvent* e) {
    QTableWidget::keyPressEvent(e);
    TableBase::keyPressEvent(e);
}

void TableWidget::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton || isSelectRightClickedRow_) {
        QTableWidget::mousePressEvent(e);
        return;
    }
    TableBase::mousePressEvent(e);
}

void TableWidget::mouseReleaseEvent(QMouseEvent* e) {
    QTableWidget::mouseReleaseEvent(e);
    TableBase::mouseReleaseEvent(e);
}

// ============================================================================
// TableView
// ============================================================================

TableView::TableView(QWidget* parent) : QTableView(parent), TableBase() {
    initTableBase(this);
    qfw::updateDynamicStyle(this);
}

void TableView::setBorderVisible(bool visible) { TableBase::setBorderVisible(visible); }

void TableView::setBorderRadius(int radius) { TableBase::setBorderRadius(radius); }

void TableView::setCheckedColor(const QColor& light, const QColor& dark) {
    TableBase::setCheckedColor(light, dark);
}

bool TableView::isSelectRightClickedRow() const { return isSelectRightClickedRow_; }

void TableView::setSelectRightClickedRow(bool isSelect) { isSelectRightClickedRow_ = isSelect; }

void TableView::selectAll() {
    QTableView::selectAll();
    updateSelectedRows();
}

void TableView::selectRow(int row) {
    if (delegate_->useStackedWidgetStyle() || delegate_->useTransparentRows() ||
        delegate_->useThemeColorSelection() || delegate_->showRowDivider()) {
        // Native QTableView::selectRow uses columnAt(0); RTL tables whose columns
        // do not fill the viewport can have empty space there. Select the model
        // row instead, including before show and with hidden/reordered columns.
        const auto index = model() ? model()->index(row, 0, rootIndex()) : QModelIndex();
        if (index.isValid() && selectionModel() &&
            selectionMode() != QAbstractItemView::NoSelection) {
            const auto command = selectionCommand(index) | QItemSelectionModel::Rows;
            selectionModel()->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
            selectionModel()->select(index, command);
        }
    } else {
        QTableView::selectRow(row);
    }
    updateSelectedRows();
}

void TableView::clearSelection() {
    QTableView::clearSelection();
    updateSelectedRows();
}

void TableView::setCurrentIndex(const QModelIndex& index) {
    QTableView::setCurrentIndex(index);
    updateSelectedRows();
}

void TableView::leaveEvent(QEvent* e) { TableBase::leaveEvent(e); }

void TableView::resizeEvent(QResizeEvent* e) {
    QTableView::resizeEvent(e);
    TableBase::resizeEvent(e);
}

void TableView::keyPressEvent(QKeyEvent* e) {
    QTableView::keyPressEvent(e);
    TableBase::keyPressEvent(e);
}

void TableView::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton || isSelectRightClickedRow_) {
        QTableView::mousePressEvent(e);
        return;
    }
    TableBase::mousePressEvent(e);
}

void TableView::mouseReleaseEvent(QMouseEvent* e) {
    QTableView::mouseReleaseEvent(e);
    TableBase::mouseReleaseEvent(e);
}

}  // namespace qfw
