#pragma once

#include <QHeaderView>
#include <QSet>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QTableWidget>

#include "components/widgets/scroll_bar.h"
#include "components/widgets/tool_tip.h"

namespace qfw {

class TableItemDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit TableItemDelegate(QTableView* parent = nullptr);
    explicit TableItemDelegate(QAbstractItemView* parent);

    void setHoverRow(int row);
    void setPressedRow(int row);
    void setSelectedRows(const QList<QModelIndex>& indexes);

    void setCheckedColor(const QColor& light, const QColor& dark);

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override;

    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
                              const QModelIndex& index) const override;

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override;

    bool helpEvent(QHelpEvent* event, QAbstractItemView* view, const QStyleOptionViewItem& option,
                   const QModelIndex& index) override;

protected:
    void drawBackground(QPainter* painter, const QStyleOptionViewItem& option,
                        const QModelIndex& index) const;
    void drawIndicator(QPainter* painter, const QStyleOptionViewItem& option,
                       const QModelIndex& index) const;
    void drawCheckBox(QPainter* painter, const QStyleOptionViewItem& option,
                      const QModelIndex& index) const;

    int margin_ = 2;
    int hoverRow_ = -1;
    int pressedRow_ = -1;
    QSet<int> selectedRows_;
    QColor lightCheckedColor_;
    QColor darkCheckedColor_;
    ItemViewToolTipDelegate* tooltipDelegate_ = nullptr;

    friend class TableBase;
    friend class ListBase;
};

class TableBase {
protected:
    TableBase();
    ~TableBase() = default;

    void initTableBase(QTableView* view);

    void setBorderVisible(bool visible);
    void setBorderRadius(int radius);
    void setCheckedColor(const QColor& light, const QColor& dark);

    void setHoverRow(int row);
    void setPressedRow(int row);
    void setSelectedRows(const QList<QModelIndex>& indexes);
    void updateSelectedRows();

    void leaveEvent(QEvent* e);
    void resizeEvent(QResizeEvent* e);
    void keyPressEvent(QKeyEvent* e);
    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);

    TableItemDelegate* delegate_ = nullptr;
    SmoothScrollDelegate* scrollDelegate_ = nullptr;
    bool isSelectRightClickedRow_ = false;

private:
    QTableView* tableView_ = nullptr;
};

class TableWidget : public QTableWidget, protected TableBase {
    Q_OBJECT
    Q_PROPERTY(
        bool selectRightClickedRow READ isSelectRightClickedRow WRITE setSelectRightClickedRow)

public:
    explicit TableWidget(QWidget* parent = nullptr);

    void setBorderVisible(bool visible);
    void setBorderRadius(int radius);
    void setCheckedColor(const QColor& light, const QColor& dark);

    void setCurrentCell(int row, int column,
                        QItemSelectionModel::SelectionFlag command = QItemSelectionModel::NoUpdate);
    void setCurrentItem(QTableWidgetItem* item,
                        QItemSelectionModel::SelectionFlag command = QItemSelectionModel::NoUpdate);

    bool isSelectRightClickedRow() const;
    void setSelectRightClickedRow(bool isSelect);

    void selectAll() override;
    void selectRow(int row);
    void clearSelection();

protected:
    void leaveEvent(QEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;
    void keyPressEvent(QKeyEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
};

class TableView : public QTableView, protected TableBase {
    Q_OBJECT
    Q_PROPERTY(
        bool selectRightClickedRow READ isSelectRightClickedRow WRITE setSelectRightClickedRow)

public:
    explicit TableView(QWidget* parent = nullptr);

    void setBorderVisible(bool visible);
    void setBorderRadius(int radius);
    void setCheckedColor(const QColor& light, const QColor& dark);

    bool isSelectRightClickedRow() const;
    void setSelectRightClickedRow(bool isSelect);

    void selectAll() override;
    void selectRow(int row);
    void clearSelection();
    void setCurrentIndex(const QModelIndex& index);

protected:
    void leaveEvent(QEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;
    void keyPressEvent(QKeyEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
};

}  // namespace qfw
