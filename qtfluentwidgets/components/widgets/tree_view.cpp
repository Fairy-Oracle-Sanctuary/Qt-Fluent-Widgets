#include "components/widgets/tree_view.h"

#include <QApplication>
#include <QEvent>
#include <QHeaderView>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QScrollBar>

#include "common/color.h"
#include "common/config.h"
#include "common/font.h"
#include "common/style_sheet.h"
#include "components/widgets/check_box.h"
#include "components/widgets/scroll_bar.h"

namespace qfw {

// ============================================================================
// TreeItemDelegate
// ============================================================================

TreeItemDelegate::TreeItemDelegate(QTreeView* parent) : QStyledItemDelegate(parent) {
    // Initialize with default theme color
    QColor themeColor = QConfig::instance().themeColor();
    lightCheckedColor_ = themeColor;
    darkCheckedColor_ = themeColor;

    // Connect theme color changed signal
    connect(&QConfig::instance(), &QConfig::themeColorChanged, this, [this](const QColor& color) {
        lightCheckedColor_ = color;
        darkCheckedColor_ = color;
        if (auto* view = qobject_cast<QAbstractItemView*>(this->parent())) {
            if (view->viewport()) {
                view->viewport()->update();
            }
        }
    });
}

void TreeItemDelegate::setCheckedColor(const QColor& light, const QColor& dark) {
    lightCheckedColor_ = QColor(light);
    darkCheckedColor_ = QColor(dark);

    if (auto* view = qobject_cast<QAbstractItemView*>(parent())) {
        if (view->viewport()) {
            view->viewport()->update();
        }
    }
}

void TreeItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                             const QModelIndex& index) const {
    if (!painter) {
        return;
    }

    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    QStyledItemDelegate::paint(painter, option, index);

    if (index.data(Qt::CheckStateRole).isValid()) {
        drawCheckBox(painter, option, index);
    }

    if (!(option.state & (QStyle::State_Selected | QStyle::State_MouseOver))) {
        return;
    }

    painter->save();
    painter->setPen(Qt::NoPen);

    drawBackground(painter, option, index);
    drawIndicator(painter, option, index);

    painter->restore();
}

void TreeItemDelegate::drawBackground(QPainter* painter, const QStyleOptionViewItem& option,
                                      const QModelIndex& index) const {
    Q_UNUSED(index);

    const int c = isDarkTheme() ? 255 : 0;
    painter->setBrush(QColor(c, c, c, 9));

    auto* view = qobject_cast<QTreeView*>(parent());
    if (!view || !view->header()) {
        painter->drawRect(option.rect);
        return;
    }

    const int column = index.column();
    const int lastColumn = view->header()->count() - 1;

    const qreal radius = 4.0;
    QPainterPath path;

    QRectF rect(option.rect);
    rect.setTop(option.rect.y() + 2);
    rect.setHeight(option.rect.height() - 4);

    if (column == 0) {
        rect.setX(4);
    }

    if (column == 0 && column == lastColumn) {
        path.addRoundedRect(rect, radius, radius);
    } else if (column == 0) {
        path.moveTo(rect.right(), rect.top());
        path.lineTo(rect.right(), rect.bottom());
        path.lineTo(rect.x() + radius, rect.bottom());
        path.arcTo(rect.x(), rect.bottom() - 2 * radius, 2 * radius, 2 * radius, 270, -90);
        path.lineTo(rect.x(), rect.top() + radius);
        path.arcTo(rect.x(), rect.top(), 2 * radius, 2 * radius, 180, -90);
        path.closeSubpath();
    } else if (column == lastColumn) {
        path.moveTo(rect.x(), rect.top());
        path.lineTo(rect.right() - radius, rect.top());
        path.arcTo(rect.right() - 2 * radius, rect.top(), 2 * radius, 2 * radius, 90, -90);
        path.lineTo(rect.right(), rect.bottom() - radius);
        path.arcTo(rect.right() - 2 * radius, rect.bottom() - 2 * radius, 2 * radius, 2 * radius, 0,
                   -90);
        path.lineTo(rect.x(), rect.bottom());
        path.closeSubpath();
    } else {
        path.addRect(rect);
    }

    painter->drawPath(path);
}

void TreeItemDelegate::drawIndicator(QPainter* painter, const QStyleOptionViewItem& option,
                                     const QModelIndex& index) const {
    Q_UNUSED(index);

    auto* view = qobject_cast<QTreeView*>(parent());
    if (!view) {
        return;
    }

    const int h = option.rect.height() - 4;

    if ((option.state & QStyle::State_Selected) && view->horizontalScrollBar() &&
        view->horizontalScrollBar()->value() == 0) {
        QColor color;
        if (isDarkTheme()) {
            color = themedColor(themeColor(), true, QStringLiteral("ThemeColorLight1"));
        } else {
            color = autoFallbackThemeColor(lightCheckedColor_, darkCheckedColor_);
        }
        painter->setBrush(color);
        painter->drawRoundedRect(QRectF(4, 9 + option.rect.y(), 3, h - 13), 1.5, 1.5);
    }
}

void TreeItemDelegate::drawCheckBox(QPainter* painter, const QStyleOptionViewItem& option,
                                    const QModelIndex& index) const {
    painter->save();

    const Qt::CheckState checkState =
        static_cast<Qt::CheckState>(index.data(Qt::CheckStateRole).toInt());

    const bool dark = isDarkTheme();

    const qreal r = 4.5;
    const qreal x = option.rect.x() + 23;
    const qreal y = option.rect.center().y() - 9;
    const QRectF rect(x, y, 19, 19);

    if (checkState == Qt::Unchecked) {
        painter->setBrush(dark ? QColor(0, 0, 0, 26) : QColor(0, 0, 0, 6));
        painter->setPen(dark ? QColor(255, 255, 255, 142) : QColor(0, 0, 0, 122));
        painter->drawRoundedRect(rect, r, r);
    } else {
        QColor color;
        if (dark) {
            color = themedColor(themeColor(), true, QStringLiteral("ThemeColorLight1"));
        } else {
            color = autoFallbackThemeColor(lightCheckedColor_, darkCheckedColor_);
        }
        painter->setPen(color);
        painter->setBrush(color);
        painter->drawRoundedRect(rect, r, r);

        if (checkState == Qt::Checked) {
            CheckBoxIcon(CheckBoxIcon::Accept).render(painter, rect.toRect());
        } else {
            CheckBoxIcon(CheckBoxIcon::PartialAccept).render(painter, rect.toRect());
        }
    }

    painter->restore();
}

void TreeItemDelegate::initStyleOption(QStyleOptionViewItem* option,
                                       const QModelIndex& index) const {
    QStyledItemDelegate::initStyleOption(option, index);

    if (!option) {
        return;
    }

    option->font = index.data(Qt::FontRole).value<QFont>();
    if (option->font.family().isEmpty()) {
        option->font = qfw::getFont(13);
    }

    QColor textColor = isDarkTheme() ? QColor(Qt::white) : QColor(Qt::black);
    const QVariant textBrushVar = index.data(Qt::ForegroundRole);
    if (textBrushVar.canConvert<QBrush>()) {
        const QBrush b = textBrushVar.value<QBrush>();
        if (b.color().isValid()) {
            textColor = b.color();
        }
    }

    option->palette.setColor(QPalette::Text, textColor);
    option->palette.setColor(QPalette::HighlightedText, textColor);
}

// ============================================================================
// TreeViewBase
// ============================================================================

TreeViewBase::TreeViewBase() {}

void TreeViewBase::initTreeBase(QTreeView* view) {
    view_ = view;
    if (!view_) {
        return;
    }

    scrollDelegate_ = new SmoothScrollDelegate(view_);

    if (auto* h = view_->header()) {
        h->setHighlightSections(false);
        h->setDefaultAlignment(Qt::AlignCenter);
    }

    delegate_ = new TreeItemDelegate(view_);
    view_->setItemDelegate(delegate_);
    view_->setIconSize(QSize(16, 16));
    view_->setMouseTracking(true);

    qfw::setStyleSheet(view_, FluentStyleSheet::TreeView);
    qfw::updateDynamicStyle(view_);
}

void TreeViewBase::setCheckedColor(const QColor& light, const QColor& dark) {
    if (delegate_) {
        delegate_->setCheckedColor(light, dark);
    }
}

void TreeViewBase::setBorderVisible(bool visible) {
    if (!view_) {
        return;
    }

    view_->setProperty("isBorderVisible", visible);
    qfw::updateDynamicStyle(view_);
}

void TreeViewBase::setBorderRadius(int radius) {
    if (!view_) {
        return;
    }

    const QString qss = QStringLiteral("QTreeView{border-radius: %1px}").arg(radius);
    qfw::setCustomStyleSheet(view_, qss, qss);
}

// ============================================================================
// TreeWidget
// ============================================================================

TreeWidget::TreeWidget(QWidget* parent) : QTreeWidget(parent), TreeViewBase() {
    initTreeBase(this);
}

void TreeWidget::setCheckedColor(const QColor& light, const QColor& dark) {
    TreeViewBase::setCheckedColor(light, dark);
}

void TreeWidget::setBorderVisible(bool visible) { TreeViewBase::setBorderVisible(visible); }

void TreeWidget::setBorderRadius(int radius) { TreeViewBase::setBorderRadius(radius); }

void TreeWidget::drawBranches(QPainter* painter, const QRect& rect,
                              const QModelIndex& index) const {
    QRect r = rect;
    r.moveLeft(15);
    QTreeWidget::drawBranches(painter, r, index);
}

bool TreeWidget::viewportEvent(QEvent* event) {
    if (!event || event->type() != QEvent::MouseButtonPress) {
        return QTreeWidget::viewportEvent(event);
    }

    auto* me = static_cast<QMouseEvent*>(event);
    const QModelIndex index = indexAt(me->pos());
    QTreeWidgetItem* item = itemFromIndex(index);

    if (!item) {
        return QTreeWidget::viewportEvent(event);
    }

    int level = 0;
    QTreeWidgetItem* it = item;
    while (it->parent() != nullptr) {
        it = it->parent();
        ++level;
    }

    const int indent = level * indentation() + 20;
    if (me->pos().x() > indent && me->pos().x() < indent + 10) {
        if (isExpanded(index)) {
            collapse(index);
        } else {
            expand(index);
        }
    }

    return QTreeWidget::viewportEvent(event);
}

// ============================================================================
// TreeView
// ============================================================================

TreeView::TreeView(QWidget* parent) : QTreeView(parent), TreeViewBase() { initTreeBase(this); }

void TreeView::setCheckedColor(const QColor& light, const QColor& dark) {
    TreeViewBase::setCheckedColor(light, dark);
}

void TreeView::setBorderVisible(bool visible) { TreeViewBase::setBorderVisible(visible); }

void TreeView::setBorderRadius(int radius) { TreeViewBase::setBorderRadius(radius); }

void TreeView::drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const {
    QRect r = rect;
    r.moveLeft(15);
    QTreeView::drawBranches(painter, r, index);
}

bool TreeView::viewportEvent(QEvent* event) {
    if (!event || event->type() != QEvent::MouseButtonPress) {
        return QTreeView::viewportEvent(event);
    }

    auto* me = static_cast<QMouseEvent*>(event);
    const QModelIndex index = indexAt(me->pos());
    if (!index.isValid()) {
        return QTreeView::viewportEvent(event);
    }

    int level = 0;
    QModelIndex currentIndex = index;
    while (currentIndex.parent().isValid()) {
        currentIndex = currentIndex.parent();
        ++level;
    }

    const int indent = level * indentation() + 20;
    if (me->pos().x() > indent && me->pos().x() < indent + 10) {
        if (isExpanded(index)) {
            collapse(index);
        } else {
            expand(index);
        }
    }

    return QTreeView::viewportEvent(event);
}

}  // namespace qfw
