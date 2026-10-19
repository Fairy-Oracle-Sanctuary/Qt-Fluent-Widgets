#include "components/widgets/tree_combo_box.h"

#include <QApplication>
#include <QGraphicsDropShadowEffect>
#include <QHeaderView>
#include <QHideEvent>
#include <QIdentityProxyModel>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QScreen>
#include <QScrollArea>
#include <QScrollBar>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QTextLayout>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <algorithm>
#include <functional>
#include <stdexcept>

#include "common/config.h"
#include "common/font.h"
#include "common/icon.h"
#include "common/style_sheet.h"
#include "components/widgets/chip.h"
#include "components/widgets/line_edit.h"
#include "components/widgets/tree_view.h"

namespace qfw {
namespace {
constexpr int treeRowHeight = 32;
QString pathText(QModelIndex index) {
    QStringList parts;
    while (index.isValid()) {
        parts.prepend(index.data().toString());
        index = index.parent();
    }
    return parts.join('/');
}
QRect checkRect(const QRect& row, Qt::LayoutDirection direction) {
    return QStyle::visualRect(direction, row, QRect(row.left() + 4, row.center().y() - 9, 18, 18));
}
QRect branchRect(const QRect& row, Qt::LayoutDirection direction, int indentation) {
    return direction == Qt::RightToLeft
               ? QRect(row.right() + 1, row.top(), indentation, row.height())
               : QRect(row.left() - indentation, row.top(), indentation, row.height());
}
class ComboTreeView : public TreeView {
public:
    using TreeView::TreeView;

protected:
    bool viewportEvent(QEvent* event) override {
        if (event->type() == QEvent::MouseMove) {
            hoverPosition_ = static_cast<QMouseEvent*>(event)->pos();
            hovering_ = true;
            viewport()->update();
        } else if (event->type() == QEvent::Leave || event->type() == QEvent::Hide) {
            hovering_ = false;
            viewport()->update();
        }
        return TreeView::viewportEvent(event);
    }
    void drawRow(QPainter* painter, const QStyleOptionViewItem& option,
                 const QModelIndex& index) const override {
        // Delegates only receive the cell after its branch indentation. Paint
        // the highlight here so nested items and their branches share one row.
        const bool selected = selectionModel() && selectionModel()->isSelected(index);
        if (selected || (hovering_ && indexAt(hoverPosition_) == index &&
                         (index.flags() & Qt::ItemIsEnabled))) {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            painter->setPen(Qt::NoPen);
            const int shade = isDarkTheme() ? 255 : 0;
            painter->setBrush(QColor(shade, shade, shade, 15));
            painter->drawRoundedRect(
                QRectF(0, option.rect.y() + 2, viewport()->width() - 2, option.rect.height() - 4),
                4, 4);
            painter->restore();
        }
        // Single selection uses the same leading accent marker as Python.
        // Multi-selection already communicates its state through checkboxes.
        if (selected && !index.data(Qt::CheckStateRole).isValid() &&
            horizontalScrollBar()->value() == 0) {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            painter->setPen(Qt::NoPen);
            painter->setBrush(themeColor());
            const int x = layoutDirection() == Qt::RightToLeft ? viewport()->width() - 7 : 4;
            painter->drawRoundedRect(QRectF(x, option.rect.y() + 9, 3, option.rect.height() - 17),
                                     1.5, 1.5);
            painter->restore();
        }
        auto content = option;
        content.state &= ~(QStyle::State_Selected | QStyle::State_MouseOver);
        QTreeView::drawRow(painter, content, index);
    }
    void drawBranches(QPainter* painter, const QRect&, const QModelIndex& index) const override {
        if (!model()->hasChildren(index)) return;
        const QRect branch = branchRect(visualRect(index), layoutDirection(), indentation());
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        QColor color = isDarkTheme() ? Qt::white : QColor("#606060");
        if (!(model()->flags(index) & Qt::ItemIsEnabled)) color.setAlpha(90);
        painter->setPen(QPen(color, 1.5));
        const QPointF c = branch.center();
        QPolygonF arrow;
        if (isExpanded(index))
            arrow << c + QPointF(-4, -2) << c + QPointF(0, 2) << c + QPointF(4, -2);
        else {
            const int sign = layoutDirection() == Qt::RightToLeft ? -1 : 1;
            arrow << c + QPointF(-2 * sign, -4) << c + QPointF(2 * sign, 0)
                  << c + QPointF(-2 * sign, 4);
        }
        painter->drawPolyline(arrow);
        painter->restore();
    }

private:
    QPoint hoverPosition_;
    bool hovering_ = false;
};
// The standard TreeItemDelegate uses a different checkbox hit area. Keep the
// popup's drawing and mouse geometry together without changing ordinary trees.
class ComboTreeDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        QSize size = QStyledItemDelegate::sizeHint(option, index);
        size.setHeight(treeRowHeight);
        return size;
    }
    bool editorEvent(QEvent*, QAbstractItemModel*, const QStyleOptionViewItem&,
                     const QModelIndex&) override {
        return false;
    }
    void paint(QPainter* p, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override {
        p->save();
        p->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
        const bool dark = isDarkTheme();
        const int shade = dark ? 255 : 0;
        if (!(index.flags() & Qt::ItemIsEnabled)) p->setOpacity(0.36);
        int start = 4;
        const QVariant checked = index.data(Qt::CheckStateRole);
        if (checked.isValid()) {
            const QRect box = checkRect(option.rect, option.direction);
            const int state = checked.toInt();
            p->setBrush(state == Qt::Unchecked ? QColor(shade, shade, shade, 8) : themeColor());
            p->setPen(state == Qt::Unchecked ? QPen(QColor(shade, shade, shade, 130))
                                             : QPen(themeColor()));
            p->drawRoundedRect(QRectF(box), 4, 4);
            if (state != Qt::Unchecked) {
                p->setPen(QPen(dark ? Qt::black : Qt::white, 1.7));
                if (state == Qt::PartiallyChecked) {
                    // QRect::center() rounds an even-sized box toward the top
                    // left. Use its geometric center, including half pixels.
                    const QPointF center = QRectF(box).center();
                    p->drawLine(center + QPointF(-4, 0), center + QPointF(4, 0));
                } else {
                    QPolygonF tick;
                    tick << QPointF(box.x() + 4, box.y() + 9) << QPointF(box.x() + 8, box.y() + 13)
                         << QPointF(box.x() + 14, box.y() + 5);
                    p->drawPolyline(tick);
                }
            }
            start += 26;
        }
        const QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
        if (!icon.isNull()) {
            icon.paint(p, QStyle::visualRect(option.direction, option.rect,
                                             QRect(option.rect.left() + start,
                                                   option.rect.center().y() - 8, 16, 16)));
            start += 24;
        }
        const QRect text = QStyle::visualRect(option.direction, option.rect,
                                              option.rect.adjusted(start, 0, -8, 0));
        p->setFont(option.font);
        p->setPen(dark ? Qt::white : Qt::black);
        const QString displayed =
            QFontMetrics(option.font)
                .elidedText(index.data().toString(), Qt::ElideRight, text.width());
        const QString query = parent()->property("treeSearchText").toString();
        QTextLayout layout(displayed, option.font);
        QTextOption textOption;
        textOption.setTextDirection(option.direction);
        textOption.setAlignment(option.direction == Qt::RightToLeft ? Qt::AlignRight
                                                                    : Qt::AlignLeft);
        layout.setTextOption(textOption);
        QVector<QTextLayout::FormatRange> matches;
        if (!query.isEmpty()) {
            int start = 0;
            while ((start = displayed.indexOf(query, start, Qt::CaseInsensitive)) >= 0) {
                QTextLayout::FormatRange match;
                match.start = start;
                match.length = query.size();
                match.format.setBackground(QColor(255, 207, 64, 100));
                matches.append(match);
                start += query.size();
            }
        }
        layout.setFormats(matches);
        layout.beginLayout();
        auto line = layout.createLine();
        if (line.isValid()) line.setLineWidth(text.width());
        layout.endLayout();
        if (line.isValid())
            layout.draw(p, QPointF(text.x(), text.y() + (text.height() - line.height()) / 2));
        p->restore();
    }
};
}  // namespace

class TreeComboProxy : public QIdentityProxyModel {
public:
    explicit TreeComboProxy(TreeComboBox* owner) : QIdentityProxyModel(owner), owner_(owner) {}
    Qt::ItemFlags flags(const QModelIndex& index) const override {
        auto result = QIdentityProxyModel::flags(index);
        if (hasChildren(index)) result &= ~Qt::ItemIsSelectable;
        result &= ~Qt::ItemIsEditable;
        if (owner_->isMultiSelection())
            result |= Qt::ItemIsUserCheckable;
        else
            result &= ~Qt::ItemIsUserCheckable;
        return result;
    }
    QVariant data(const QModelIndex& index, int role) const override {
        if (role == Qt::CheckStateRole) {
            return owner_->isMultiSelection()
                       ? QVariant(int(owner_->checkState(mapToSource(index))))
                       : QVariant();
        }
        return QIdentityProxyModel::data(index, role);
    }
    bool setData(const QModelIndex& index, const QVariant& value, int role) override {
        if (role == Qt::CheckStateRole) return false;
        return QIdentityProxyModel::setData(index, value, role);
    }
    void refresh() {
        // Repaint all visible rows, including aggregate parent states.
        if (owner_->treeView()) owner_->treeView()->viewport()->update();
    }

private:
    TreeComboBox* owner_;
};

TreeComboBox::TreeComboBox(QWidget* parent) : QPushButton(parent) {
    setMinimumSize(220, 36);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::StrongFocus);
    qfw::setFont(this);
    setAccessibleName(tr("Tree selection"));
    proxy_ = new TreeComboProxy(this);
    setModel(new QStandardItemModel(this));
    connect(this, &QAbstractButton::clicked, this, &TreeComboBox::togglePopup);
    connect(&QConfig::instance(), &QConfig::themeChanged, this, [this] {
        refreshPopupTheme();
        update();
    });
    connect(&QConfig::instance(), &QConfig::themeColorChanged, this, [this] {
        refreshChecks();
        update();
    });
}
TreeComboBox::~TreeComboBox() {
    for (const auto& connection : modelConnections_) disconnect(connection);
    // QWidget teardown can dispatch native-window events after its tree child
    // has already gone. Never let our filter dereference that child during teardown.
    if (tree_) {
        tree_->removeEventFilter(this);
        tree_->viewport()->removeEventFilter(this);
    }
    if (popup_) popup_->removeEventFilter(this);
    auto* popup = popup_;
    popup_ = nullptr;
    tree_ = nullptr;
    panel_ = nullptr;
    animation_ = nullptr;
    delete popup;
}
QAbstractItemModel* TreeComboBox::model() const { return model_.data(); }
void TreeComboBox::setModel(QAbstractItemModel* model) {
    if (!model) throw std::invalid_argument("model must not be null");
    if (model == model_) return;
    hidePopup();
    filtering_ = false;
    expandedBeforeSearch_.clear();
    currentBeforeSearch_ = QPersistentModelIndex();
    for (const auto& connection : modelConnections_) disconnect(connection);
    modelConnections_.clear();
    model_ = model;
    current_ = QPersistentModelIndex();
    proxy_->setSourceModel(model);
    const auto refresh = [this] { synchronize(); };
    modelConnections_ << connect(model, &QAbstractItemModel::dataChanged, this, refresh)
                      << connect(model, &QAbstractItemModel::rowsRemoved, this, refresh)
                      << connect(model, &QAbstractItemModel::rowsInserted, this, refresh)
                      << connect(model, &QAbstractItemModel::rowsMoved, this, refresh)
                      << connect(model, &QAbstractItemModel::columnsRemoved, this,
                                 [this] {
                                     hidePopup();
                                     synchronize();
                                 })
                      << connect(model, &QAbstractItemModel::columnsInserted, this,
                                 [this] {
                                     hidePopup();
                                     synchronize();
                                 })
                      << connect(model, &QAbstractItemModel::columnsMoved, this,
                                 [this] {
                                     hidePopup();
                                     synchronize();
                                 })
                      << connect(model, &QAbstractItemModel::layoutChanged, this, refresh)
                      << connect(model, &QAbstractItemModel::modelReset, this,
                                 [this] {
                                     hidePopup();
                                     synchronize();
                                 })
                      << connect(model, &QObject::destroyed, this, [this] {
                             hidePopup();
                             model_ = nullptr;
                             current_ = QPersistentModelIndex();
                             synchronize();
                         });
    synchronize();
}
void TreeComboBox::validateIndex(const QModelIndex& index, bool leaf) const {
    if (!index.isValid()) return;
    if (!model_ || index.model() != model_ || index.column() != 0)
        throw std::invalid_argument("index must belong to column zero of this model");
    if (leaf && model_->hasChildren(index))
        throw std::invalid_argument("only leaf nodes may be selected");
}
QModelIndex TreeComboBox::addItem(const QString& text, const QModelIndex& parent,
                                  const QVariant& data, const QIcon& icon) {
    validateIndex(parent, false);
    auto* standard = qobject_cast<QStandardItemModel*>(model());
    if (!standard) throw std::invalid_argument("addItem requires a QStandardItemModel");
    auto* item = new QStandardItem(icon, text);
    item->setEditable(false);
    item->setData(data, Qt::UserRole);
    auto* root = parent.isValid() ? standard->itemFromIndex(parent) : standard->invisibleRootItem();
    root->appendRow(item);
    return item->index();
}
QList<QModelIndex> TreeComboBox::addItems(const QStringList& texts, const QModelIndex& parent) {
    validateIndex(parent, false);
    QList<QModelIndex> result;
    for (const auto& text : texts) result.append(addItem(text, parent));
    return result;
}
void TreeComboBox::clear() {
    auto* standard = qobject_cast<QStandardItemModel*>(model());
    if (!standard) throw std::invalid_argument("clear requires a QStandardItemModel");
    standard->clear();
}
void TreeComboBox::setPlaceholderText(const QString& text) {
    placeholder_ = text;
    refreshDisplay();
}
QString TreeComboBox::placeholderText() const { return placeholder_; }
void TreeComboBox::setMaxVisibleItems(int count) {
    maxVisible_ = count < 0 ? -1 : std::max(1, count);
    hidePopup();
}
int TreeComboBox::maxVisibleItems() const { return maxVisible_; }
void TreeComboBox::setSearchEnabled(bool enabled) {
    if (searchEnabled_ == enabled) return;
    hidePopup();
    searchEnabled_ = enabled;
    if (!enabled) setSearchText({});
    if (search_) search_->setVisible(enabled);
    applySearch();
}
bool TreeComboBox::isSearchEnabled() const { return searchEnabled_; }
void TreeComboBox::setSearchText(const QString& text) {
    if (searchText_ == text) return;
    searchText_ = text;
    if (search_ && search_->text() != text) search_->setText(text);
    applySearch();
    emit searchTextChanged(text);
}
QString TreeComboBox::searchText() const { return searchText_; }
SearchLineEdit* TreeComboBox::searchLineEdit() const { return search_; }
void TreeComboBox::applySearch() {
    if (!tree_ || !model_) return;
    const QString query = searchEnabled_ ? searchText_.trimmed() : QString();
    const bool wasFiltering = filtering_;
    if (!wasFiltering && !query.isEmpty()) {
        currentBeforeSearch_ = tree_->currentIndex();
        expandedBeforeSearch_.clear();
        std::function<void(const QModelIndex&)> save = [&](const QModelIndex& parent) {
            for (int r = 0; r < proxy_->rowCount(parent); ++r) {
                const auto index = proxy_->index(r, 0, parent);
                if (tree_->isExpanded(index)) expandedBeforeSearch_.append(index);
                save(index);
            }
        };
        save({});
    }
    filtering_ = !query.isEmpty();
    tree_->setProperty("treeSearchText", query);
    // Hide rows in the view, not the model: source indexes, leaf identity and
    // parent bulk-selection semantics must not change while searching.
    std::function<bool(const QModelIndex&, bool)> filter = [&](const QModelIndex& parent,
                                                               bool ancestorMatches) {
        bool any = false;
        for (int r = 0; r < proxy_->rowCount(parent); ++r) {
            const auto index = proxy_->index(r, 0, parent);
            const bool matches = ancestorMatches || query.isEmpty() ||
                                 index.data().toString().contains(query, Qt::CaseInsensitive);
            const bool childMatches = filter(index, matches);
            const bool visible = matches || childMatches;
            tree_->setRowHidden(r, parent, !visible);
            if (filtering_ && visible && proxy_->hasChildren(index)) tree_->expand(index);
            any |= visible;
        }
        return any;
    };
    const bool any = filter({}, false);
    if (wasFiltering && !filtering_) {
        if (current_.isValid())
            tree_->setCurrentIndex(proxy_->mapFromSource(current_));
        else if (currentBeforeSearch_.isValid())
            tree_->setCurrentIndex(currentBeforeSearch_);
        currentBeforeSearch_ = QPersistentModelIndex();
        tree_->collapseAll();
        for (const auto& index : expandedBeforeSearch_)
            if (index.isValid()) tree_->expand(index);
        expandedBeforeSearch_.clear();
    }
    emptySearch_->setVisible(!any);
    tree_->setVisible(any);
    auto current = tree_->currentIndex();
    bool visible = current.isValid();
    for (auto index = current; index.isValid(); index = index.parent())
        visible &= !tree_->isRowHidden(index.row(), index.parent());
    if (!visible) {
        QModelIndex first;
        for (int r = 0; r < proxy_->rowCount(); ++r)
            if (!tree_->isRowHidden(r, {})) {
                first = proxy_->index(r, 0);
                break;
            }
        tree_->setCurrentIndex(first);
    }
    tree_->viewport()->update();
}
QModelIndex TreeComboBox::currentModelIndex() const { return current_; }
QString TreeComboBox::currentText() const { return pathText(current_); }
QVariant TreeComboBox::currentData(int role) const { return current_.data(role); }
void TreeComboBox::setCurrentModelIndex(const QModelIndex& index) {
    validateIndex(index);
    if (current_ == index) return;
    current_ = QPersistentModelIndex(index);
    hadCurrent_ = index.isValid();
    const auto oldText = lastText_;
    lastText_ = currentText();
    const auto text = lastText_;
    const quint64 revision = ++revision_;
    refreshDisplay();
    QPointer<TreeComboBox> guard(this);
    emit currentModelIndexChanged(index);
    if (!guard || revision_ != revision) return;
    if (oldText != text) emit currentTextChanged(text);
}
void TreeComboBox::synchronize() {
    const bool had = hadCurrent_;
    if (current_.isValid() && model_ && (current_.column() != 0 || model_->hasChildren(current_)))
        current_ = QPersistentModelIndex();
    hadCurrent_ = current_.isValid();
    const auto oldText = lastText_;
    lastText_ = currentText();
    const auto text = lastText_;
    const quint64 revision = ++revision_;
    refreshDisplay();
    refreshChecks();
    QPointer<TreeComboBox> guard(this);
    if (had != hadCurrent_) emit currentModelIndexChanged(current_);
    if (!guard || revision_ != revision) return;
    if (oldText != text) emit currentTextChanged(text);
}
void TreeComboBox::refreshDisplay() {
    const auto text = currentText();
    QPushButton::setText(current_.isValid() ? text : placeholder_);
    setProperty("isPlaceholderText", !current_.isValid());
    setAccessibleDescription(text);
    update();
}
bool TreeComboBox::isMultiSelection() const { return false; }
Qt::CheckState TreeComboBox::checkState(const QModelIndex&) const { return Qt::Unchecked; }
QList<QModelIndex> TreeComboBox::leafDescendants(const QModelIndex& parent) const {
    QList<QModelIndex> leaves;
    if (!model_) return leaves;
    QList<QModelIndex> pending;
    for (int row = model_->rowCount(parent) - 1; row >= 0; --row)
        pending.append(model_->index(row, 0, parent));
    while (!pending.isEmpty()) {
        const QModelIndex index = pending.takeLast();
        if (!(model_->flags(index) & Qt::ItemIsEnabled)) continue;
        if (model_->hasChildren(index)) {
            for (int row = model_->rowCount(index) - 1; row >= 0; --row)
                pending.append(model_->index(row, 0, index));
        } else
            leaves.append(index);
    }
    return leaves;
}
void TreeComboBox::refreshChecks() {
    static_cast<TreeComboProxy*>(proxy_)->refresh();
    applySearch();
}
void TreeComboBox::activateIndex(const QModelIndex& source, bool) {
    if (!source.isValid() || !model_ || model_->hasChildren(source) ||
        !(model_->flags(source) & Qt::ItemIsEnabled) ||
        !(model_->flags(source) & Qt::ItemIsSelectable))
        return;
    QPointer<TreeComboBox> guard(this);
    QPersistentModelIndex selected(source);
    setCurrentModelIndex(source);
    if (!guard || !selected.isValid() || current_ != selected) return;
    hidePopup();
    emit activated(selected);
}
void TreeComboBox::createPopup() {
    popup_ = new QFrame(this, Qt::Popup | Qt::FramelessWindowHint);
    popup_->setObjectName("treeComboPopup");
    popup_->setAttribute(Qt::WA_TranslucentBackground);
    popup_->installEventFilter(this);
    auto* outer = new QVBoxLayout(popup_);
    outer->setContentsMargins(12, 12, 12, 12);
    panel_ = new QFrame(popup_);
    panel_->setObjectName("treeComboPanel");
    auto* shadow = new QGraphicsDropShadowEffect(panel_);
    shadow->setBlurRadius(20);
    shadow->setOffset(0, 3);
    shadow->setColor(QColor(0, 0, 0, 65));
    panel_->setGraphicsEffect(shadow);
    outer->addWidget(panel_);
    auto* layout = new QVBoxLayout(panel_);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);
    search_ = new SearchLineEdit(panel_);
    search_->setObjectName("treeComboSearch");
    search_->setPlaceholderText(tr("Search items"));
    search_->setAccessibleName(tr("Search items"));
    search_->setFixedHeight(30);
    qfw::setFont(search_, 12);
    search_->setText(searchText_);
    search_->setVisible(searchEnabled_);
    search_->installEventFilter(this);
    connect(search_, &QLineEdit::textChanged, this, &TreeComboBox::setSearchText);
    layout->addWidget(search_);
    tree_ = new ComboTreeView(panel_);
    qfw::setFont(tree_, 13);
    tree_->setObjectName("treeComboView");
    tree_->setModel(proxy_);
    tree_->setItemDelegate(new ComboTreeDelegate(tree_));
    tree_->setHeaderHidden(true);
    tree_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tree_->setSelectionBehavior(QAbstractItemView::SelectRows);
    tree_->setSelectionMode(isMultiSelection() ? QAbstractItemView::NoSelection
                                               : QAbstractItemView::SingleSelection);
    tree_->setExpandsOnDoubleClick(false);
    tree_->setIndentation(22);
    tree_->viewport()->installEventFilter(this);
    tree_->installEventFilter(this);
    layout->addWidget(tree_);
    auto* empty = new QLabel(tr("No matching items"), panel_);
    empty->setObjectName("treeComboEmptySearch");
    empty->setAlignment(Qt::AlignCenter);
    empty->setMinimumHeight(treeRowHeight);
    empty->hide();
    emptySearch_ = empty;
    layout->addWidget(empty);
    animation_ = new QParallelAnimationGroup(popup_);
    for (const auto* property : {"pos", "windowOpacity"}) {
        auto* ani = new QPropertyAnimation(popup_, property, animation_);
        ani->setDuration(160);
        ani->setEasingCurve(QEasingCurve::OutQuad);
        animation_->addAnimation(ani);
    }
    refreshPopupTheme();
}
void TreeComboBox::refreshPopupTheme() {
    if (!panel_) return;
    panel_->setStyleSheet(
        QStringLiteral(
            "QFrame#treeComboPanel {background:%1; border:1px solid %2; border-radius:8px;}")
            .arg(isDarkTheme() ? "#2b2b2b" : "#fafafa", isDarkTheme() ? "#454545" : "#dddddd"));
    tree_->viewport()->update();
    emptySearch_->setStyleSheet(isDarkTheme() ? "color: #bbbbbb; border: none;"
                                              : "color: #606060; border: none;");
}
void TreeComboBox::showPopup() {
    if (!isEnabled() || !isVisible() || !model_ || model_->rowCount() == 0 || isPopupVisible())
        return;
    if (!popup_) createPopup();
    tree_->setColumnHidden(0, false);
    for (int column = 1; column < proxy_->columnCount(); ++column) tree_->hideColumn(column);
    tree_->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    tree_->expandAll();
    tree_->setLayoutDirection(layoutDirection());
    search_->setLayoutDirection(layoutDirection());
    if (current_.isValid())
        tree_->setCurrentIndex(proxy_->mapFromSource(current_));
    else if (!tree_->currentIndex().isValid())
        tree_->setCurrentIndex(proxy_->index(0, 0));
    applySearch();
    int rows = 0;
    QModelIndex next;
    for (int r = 0; r < proxy_->rowCount(); ++r)
        if (!tree_->isRowHidden(r, {})) {
            next = proxy_->index(r, 0);
            break;
        }
    const int limit = maxVisible_ < 0 ? 12 : std::min(200, maxVisible_);
    while (next.isValid() && rows < limit) {
        ++rows;
        next = tree_->indexBelow(next);
    }
    const QPoint below = mapToGlobal(QPoint(0, height()));
    const QPoint above = mapToGlobal(QPoint(0, 0));
    QScreen* screen = QApplication::screenAt(above);
    if (!screen) screen = QApplication::primaryScreen();
    if (!screen) return;
    const QRect available = screen->availableGeometry().adjusted(4, 4, -4, -4);
    const int wanted = std::max(1, rows) * treeRowHeight + 38 +
                       (searchEnabled_ ? search_->height() + panel_->layout()->spacing() : 0);
    const int belowSpace = std::max(0, available.bottom() - below.y() + 1);
    const int aboveSpace = std::max(0, above.y() - available.top());
    const bool opensAbove = wanted > belowSpace && aboveSpace > belowSpace;
    const int h = std::min(wanted, std::max(1, opensAbove ? aboveSpace : belowSpace));
    const int w = std::min(std::max(width() + 24, 264), available.width());
    const int anchorX =
        layoutDirection() == Qt::RightToLeft ? below.x() + width() + 12 - w : below.x() - 12;
    const int x = std::clamp(anchorX, available.left(), available.right() - w + 1);
    const int y = std::clamp(opensAbove ? above.y() - h : below.y(), available.top(),
                             available.bottom() - h + 1);
    const QPoint end(x, y), start = end + QPoint(0, opensAbove ? 8 : -8);
    animation_->stop();
    popup_->setGeometry(QRect(start, QSize(w, h)));
    auto* slide = static_cast<QPropertyAnimation*>(animation_->animationAt(0));
    auto* opacity = static_cast<QPropertyAnimation*>(animation_->animationAt(1));
    slide->setStartValue(start);
    slide->setEndValue(end);
    opacity->setStartValue(0.0);
    opacity->setEndValue(1.0);
    popup_->setWindowOpacity(0);
    popup_->show();
    popup_->activateWindow();
    animation_->start();
    if (searchEnabled_)
        search_->setFocus(Qt::PopupFocusReason);
    else
        tree_->setFocus(Qt::PopupFocusReason);
    if (tree_->currentIndex().isValid()) tree_->scrollTo(tree_->currentIndex());
}
void TreeComboBox::hidePopup() {
    if (popup_) popup_->hide();
}
bool TreeComboBox::isPopupVisible() const { return popup_ && popup_->isVisible(); }
TreeView* TreeComboBox::treeView() const { return tree_; }
void TreeComboBox::togglePopup() { isPopupVisible() ? hidePopup() : showPopup(); }
bool TreeComboBox::eventFilter(QObject* object, QEvent* event) {
    if (object == search_ && event->type() == QEvent::KeyPress) {
        auto* key = static_cast<QKeyEvent*>(event);
        if (key->key() == Qt::Key_Escape) {
            hidePopup();
            setFocus(Qt::PopupFocusReason);
            return true;
        }
        if (key->key() == Qt::Key_Down || key->key() == Qt::Key_Up) {
            if (tree_->isVisible()) tree_->setFocus(Qt::OtherFocusReason);
            return true;
        }
        if (key->key() == Qt::Key_Return || key->key() == Qt::Key_Enter) {
            if (tree_->isVisible()) tree_->setFocus(Qt::OtherFocusReason);
            return true;
        }
    }
    if (object == popup_ && event->type() == QEvent::Hide) {
        animation_->stop();
        popup_->setWindowOpacity(1);
        pressedIndex_ = QPersistentModelIndex();
        pressedArea_ = 0;
    }
    if (tree_ && object == tree_ && event->type() == QEvent::KeyPress) {
        auto* key = static_cast<QKeyEvent*>(event);
        if (key->key() == Qt::Key_Escape) {
            hidePopup();
            setFocus(Qt::PopupFocusReason);
            return true;
        }
        if (key->key() == Qt::Key_Space || key->key() == Qt::Key_Return ||
            key->key() == Qt::Key_Enter) {
            if (!key->isAutoRepeat())
                activateIndex(proxy_->mapToSource(tree_->currentIndex()), true);
            return true;
        }
    }
    if (tree_ && object == tree_->viewport()) {
        if (event->type() == QEvent::MouseButtonDblClick) return true;
        if (event->type() == QEvent::MouseButtonPress ||
            event->type() == QEvent::MouseButtonRelease) {
            auto* mouse = static_cast<QMouseEvent*>(event);
            if (mouse->button() != Qt::LeftButton) return true;
            const QModelIndex index = tree_->indexAt(mouse->pos());
            const QRect row = tree_->visualRect(index);
            const bool branch = index.isValid() && proxy_->hasChildren(index) &&
                                branchRect(row, tree_->layoutDirection(), tree_->indentation())
                                    .contains(mouse->pos());
            const bool checkbox = checkRect(row, tree_->layoutDirection()).contains(mouse->pos());
            const int area = branch ? 1 : checkbox ? 2 : 3;
            if (event->type() == QEvent::MouseButtonPress) {
                pressedIndex_ = index;
                pressedArea_ = index.isValid() ? area : 0;
                if (index.isValid()) tree_->setCurrentIndex(index);
            } else {
                const bool valid =
                    pressedIndex_.isValid() && pressedIndex_ == index && pressedArea_ == area;
                pressedIndex_ = QPersistentModelIndex();
                pressedArea_ = 0;
                if (valid && (index.flags() & Qt::ItemIsEnabled)) {
                    if (branch)
                        tree_->setExpanded(index, !tree_->isExpanded(index));
                    else
                        activateIndex(proxy_->mapToSource(index), checkbox);
                }
            }
            return true;
        }
    }
    return QPushButton::eventFilter(object, event);
}
QSize TreeComboBox::sizeHint() const {
    return QSize(
        std::max(minimumWidth(), std::min(420, fontMetrics().horizontalAdvance(text()) + 44)), 36);
}
QSize TreeComboBox::minimumSizeHint() const { return QSize(minimumWidth(), 36); }
void TreeComboBox::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const bool dark = isDarkTheme();
    const int shade = dark ? 255 : 0;
    p.setPen(QPen(QColor(shade, shade, shade, dark ? 20 : 18), 1));
    p.setBrush(dark ? QColor(255, 255, 255,
                             isDown()       ? 8
                             : underMouse() ? 21
                                            : 15)
                    : QColor(255, 255, 255,
                             isDown()       ? 90
                             : underMouse() ? 160
                                            : 190));
    p.drawRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), 5, 5);
    QColor foreground = dark ? Qt::white : Qt::black;
    if (!isEnabled())
        foreground.setAlpha(90);
    else if (property("isPlaceholderText").toBool())
        foreground.setAlpha(155);
    p.setPen(foreground);
    p.setFont(font());
    const QRect textArea = QStyle::visualRect(layoutDirection(), rect(),
                                              QRect(11, 0, std::max(0, width() - 42), height()));
    p.drawText(textArea, Qt::AlignLeading | Qt::AlignVCenter,
               fontMetrics().elidedText(text(), Qt::ElideRight, textArea.width()));
    const QRect arrow = QStyle::visualRect(layoutDirection(), rect(),
                                           QRect(width() - 22, height() / 2 - 5, 10, 10));
    FluentIcon(FluentIconEnum::ArrowDown).icon(Theme::Auto, foreground).paint(&p, arrow);
    if (hasFocus() && testAttribute(Qt::WA_KeyboardFocusChange)) {
        p.setPen(QPen(foreground, 1, Qt::DashLine));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(QRectF(rect()).adjusted(3, 3, -3, -3), 3, 3);
    }
}
void TreeComboBox::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Down || event->key() == Qt::Key_Return ||
        event->key() == Qt::Key_Enter) {
        if (!event->isAutoRepeat()) showPopup();
        event->accept();
        return;
    }
    QPushButton::keyPressEvent(event);
}
void TreeComboBox::hideEvent(QHideEvent* event) {
    hidePopup();
    QPushButton::hideEvent(event);
}
void TreeComboBox::resizeEvent(QResizeEvent* event) {
    hidePopup();
    QPushButton::resizeEvent(event);
}
void TreeComboBox::moveEvent(QMoveEvent* event) {
    hidePopup();
    QPushButton::moveEvent(event);
}
void TreeComboBox::changeEvent(QEvent* event) {
    QPushButton::changeEvent(event);
    if (event->type() == QEvent::EnabledChange && !isEnabled()) hidePopup();
    if (event->type() == QEvent::LayoutDirectionChange) hidePopup();
    if (event->type() == QEvent::LanguageChange) {
        setAccessibleName(tr("Tree selection"));
        if (search_) {
            search_->setPlaceholderText(tr("Search items"));
            search_->setAccessibleName(tr("Search items"));
            static_cast<QLabel*>(emptySearch_)->setText(tr("No matching items"));
        }
    }
    update();
}

MultiSelectionTreeComboBox::MultiSelectionTreeComboBox(QWidget* parent) : TreeComboBox(parent) {
    setMinimumWidth(260);
    chips_ = new QScrollArea(this);
    chips_->setObjectName("treeComboChips");
    chips_->setFrameShape(QFrame::NoFrame);
    chips_->setWidgetResizable(true);
    chips_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    chips_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    chips_->setStyleSheet(
        "QScrollArea, QScrollArea > QWidget > QWidget {background:transparent; border:none;}");
    chips_->setFocusPolicy(Qt::NoFocus);
    chipsContainer_ = new QWidget;
    chipsLayout_ = new QHBoxLayout(chipsContainer_);
    chipsLayout_->setContentsMargins(2, 0, 2, 0);
    chipsLayout_->setSpacing(4);
    chips_->setWidget(chipsContainer_);
    chips_->viewport()->installEventFilter(this);
    chipsContainer_->installEventFilter(this);
    refreshDisplay();
}
void MultiSelectionTreeComboBox::setModel(QAbstractItemModel* model) {
    TreeComboBox::setModel(model);
}
bool MultiSelectionTreeComboBox::isMultiSelection() const { return true; }
QList<QModelIndex> MultiSelectionTreeComboBox::selectedIndexes() const {
    QList<QModelIndex> result;
    for (const auto& index : selected_)
        if (index.isValid() && model() && !model()->hasChildren(index)) result.append(index);
    return result;
}
QStringList MultiSelectionTreeComboBox::selectedTexts() const {
    QStringList result;
    for (const auto& index : selectedIndexes()) result.append(index.data().toString());
    return result;
}
QList<QVariant> MultiSelectionTreeComboBox::selectedData(int role) const {
    QList<QVariant> result;
    for (const auto& index : selectedIndexes()) result.append(index.data(role));
    return result;
}
void MultiSelectionTreeComboBox::setSelectedIndexes(const QList<QModelIndex>& indexes) {
    QList<QPersistentModelIndex> selection;
    for (const auto& index : indexes) {
        validateIndex(index);
        if (index.isValid() && !selection.contains(index))
            selection.append(QPersistentModelIndex(index));
    }
    if (selection == selected_) return;
    selected_ = selection;
    publish(true);
}
void MultiSelectionTreeComboBox::synchronize() {
    const auto before = selected_;
    QList<QPersistentModelIndex> valid;
    for (const auto& index : selected_) {
        if (index.isValid() && index.model() == model() && index.column() == 0 &&
            !model()->hasChildren(index))
            valid.append(index);
    }
    selected_ = valid;
    publish(before != valid);
}
void MultiSelectionTreeComboBox::publish(bool indexesChanged) {
    const QStringList oldTexts = lastTexts_;
    lastTexts_ = selectedTexts();
    const auto indexes = selectedIndexes();
    const auto texts = lastTexts_;
    const quint64 revision = ++revision_;
    refreshDisplay();
    refreshChecks();
    QPointer<MultiSelectionTreeComboBox> guard(this);
    if (indexesChanged) emit selectionChanged(indexes);
    if (!guard || revision_ != revision) return;
    if (oldTexts != texts) emit selectedTextChanged(texts);
}
void MultiSelectionTreeComboBox::clearSelection() { setSelectedIndexes({}); }
void MultiSelectionTreeComboBox::setChipsMode(bool enabled) {
    if (chipsMode_ != enabled) {
        chipsMode_ = enabled;
        refreshDisplay();
    }
}
bool MultiSelectionTreeComboBox::isChipsMode() const { return chipsMode_; }
Qt::CheckState MultiSelectionTreeComboBox::checkState(const QModelIndex& source) const {
    if (!source.isValid() || !model()) return Qt::Unchecked;
    if (!model()->hasChildren(source))
        return selected_.contains(source) ? Qt::Checked : Qt::Unchecked;
    const auto leaves = leafDescendants(source);
    int count = 0;
    for (const auto& leaf : leaves)
        if (selected_.contains(leaf)) ++count;
    return count == 0 ? Qt::Unchecked : count == leaves.size() ? Qt::Checked : Qt::PartiallyChecked;
}
void MultiSelectionTreeComboBox::setDescendantsSelected(const QModelIndex& parent, bool checked) {
    validateIndex(parent, false);
    const auto leaves = leafDescendants(parent);
    auto selection = selectedIndexes();
    for (const auto& leaf : leaves) {
        if (checked && !selection.contains(leaf))
            selection.append(leaf);
        else if (!checked)
            selection.removeAll(leaf);
    }
    setSelectedIndexes(selection);
}
void MultiSelectionTreeComboBox::activateIndex(const QModelIndex& source, bool checkbox) {
    if (!source.isValid() || !model() || !(source.flags() & Qt::ItemIsEnabled)) return;
    if (model()->hasChildren(source)) {
        if (checkbox) setDescendantsSelected(source, checkState(source) != Qt::Checked);
        return;
    }
    auto selection = selectedIndexes();
    if (selection.contains(source))
        selection.removeAll(source);
    else
        selection.append(source);
    setSelectedIndexes(selection);
}
void MultiSelectionTreeComboBox::refreshDisplay() {
    const auto texts = selectedTexts();
    QPushButton::setText(texts.isEmpty() ? placeholder_
                         : chipsMode_    ? QString()
                                         : texts.join(", "));
    setProperty("isPlaceholderText", texts.isEmpty());
    setAccessibleDescription(texts.join(", "));
    if (!chips_) {
        update();
        return;
    }
    const auto indexes = selectedIndexes();
    bool reusable = chipsMode_ && !texts.isEmpty() && chipsLayout_->count() == indexes.size() + 1;
    for (int i = 0; reusable && i < indexes.size(); ++i) {
        auto* chip = qobject_cast<Chip*>(chipsLayout_->itemAt(i)->widget());
        reusable = chip && chip->text() == texts[i] &&
                   chip->property("sourceIndex").value<QPersistentModelIndex>() == indexes[i];
    }
    if (reusable) {
        positionChips();
        chips_->show();
        update();
        return;
    }
    while (chipsLayout_->count()) {
        auto* item = chipsLayout_->takeAt(0);
        if (auto* widget = item->widget()) {
            widget->hide();
            widget->deleteLater();
        }
        delete item;
    }
    if (chipsMode_ && !texts.isEmpty()) {
        for (const auto& index : selectedIndexes()) {
            auto* chip = new Chip(index.data().toString(), chipsContainer_);
            chip->setObjectName("treeSelectionChip");
            chip->setCheckable(false);
            chip->setFixedHeight(26);
            chip->setFixedWidth(std::min(240, chip->sizeHint().width()));
            const QPersistentModelIndex key(index);
            chip->setProperty("sourceIndex", QVariant::fromValue(key));
            connect(chip, &Chip::closed, this, [this, key] {
                auto selection = selectedIndexes();
                selection.removeAll(key);
                setSelectedIndexes(selection);
            });
            connect(chip, &QAbstractButton::clicked, this, &TreeComboBox::showPopup);
            chipsLayout_->addWidget(chip);
        }
        chipsLayout_->addStretch();
        chips_->show();
        positionChips();
    } else
        chips_->hide();
    update();
}
void MultiSelectionTreeComboBox::positionChips() {
    if (chips_)
        chips_->setGeometry(QStyle::visualRect(layoutDirection(), rect(),
                                               QRect(8, 0, std::max(0, width() - 37), height())));
}
void MultiSelectionTreeComboBox::resizeEvent(QResizeEvent* event) {
    TreeComboBox::resizeEvent(event);
    positionChips();
}
void MultiSelectionTreeComboBox::changeEvent(QEvent* event) {
    TreeComboBox::changeEvent(event);
    if (event->type() == QEvent::LayoutDirectionChange) positionChips();
}
bool MultiSelectionTreeComboBox::eventFilter(QObject* object, QEvent* event) {
    if (chips_ && (object == chips_->viewport() || object == chipsContainer_)) {
        if (event->type() == QEvent::Wheel) {
            auto* wheel = static_cast<QWheelEvent*>(event);
            const QPoint pixels = wheel->pixelDelta();
            const int delta = !pixels.isNull()
                                  ? (pixels.x() ? pixels.x() : pixels.y())
                                  : (wheel->angleDelta().x() ? wheel->angleDelta().x()
                                                             : wheel->angleDelta().y()) /
                                        2;
            chips_->horizontalScrollBar()->setValue(chips_->horizontalScrollBar()->value() - delta);
            return true;
        }
        if (event->type() == QEvent::Hide) chipsPressed_ = false;
        if (event->type() == QEvent::MouseButtonPress) {
            chipsPressed_ = static_cast<QMouseEvent*>(event)->button() == Qt::LeftButton;
            return true;
        }
        if (event->type() == QEvent::MouseButtonRelease) {
            auto* mouse = static_cast<QMouseEvent*>(event);
            const bool activate = chipsPressed_ && mouse->button() == Qt::LeftButton &&
                                  static_cast<QWidget*>(object)->rect().contains(mouse->pos());
            chipsPressed_ = false;
            if (activate) togglePopup();
            return true;
        }
    }
    return TreeComboBox::eventFilter(object, event);
}
}  // namespace qfw
