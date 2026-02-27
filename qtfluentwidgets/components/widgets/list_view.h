#pragma once

#include <QListView>
#include <QListWidget>

#include "components/widgets/table_view.h"

namespace qfw {

class ListItemDelegate : public TableItemDelegate {
    Q_OBJECT

public:
    explicit ListItemDelegate(QListView* parent = nullptr);

protected:
    void drawBackground(QPainter* painter, const QStyleOptionViewItem& option,
                        const QModelIndex& index) const;
    void drawIndicator(QPainter* painter, const QStyleOptionViewItem& option,
                       const QModelIndex& index) const;

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
};

class ListBase {
protected:
    ListBase();
    ~ListBase() = default;

    void initListBase(QListView* view);

    void setHoverRow(int row);
    void setPressedRow(int row);
    void setSelectedRows(const QList<QModelIndex>& indexes);
    void updateSelectedRows();

    void leaveEvent(QEvent* e);
    void resizeEvent(QResizeEvent* e);
    void keyPressEvent(QKeyEvent* e);
    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);

    void setCheckedColor(const QColor& light, const QColor& dark);

    void setBorderVisible(bool visible);

    ListItemDelegate* delegate_ = nullptr;
    SmoothScrollDelegate* scrollDelegate_ = nullptr;
    bool isSelectRightClickedRow_ = false;

private:
    QListView* listView_ = nullptr;
};

class ListWidget : public QListWidget, protected ListBase {
    Q_OBJECT
    Q_PROPERTY(
        bool selectRightClickedRow READ isSelectRightClickedRow WRITE setSelectRightClickedRow)

public:
    explicit ListWidget(QWidget* parent = nullptr);

    void setCurrentItem(QListWidgetItem* item,
                        QItemSelectionModel::SelectionFlag command = QItemSelectionModel::NoUpdate);
    void setCurrentRow(int row,
                       QItemSelectionModel::SelectionFlag command = QItemSelectionModel::NoUpdate);

    bool isSelectRightClickedRow() const;
    void setSelectRightClickedRow(bool isSelect);

    void setCheckedColor(const QColor& light, const QColor& dark);

    void setBorderVisible(bool visible);

    void clearSelection();

protected:
    void leaveEvent(QEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;
    void keyPressEvent(QKeyEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
};

class ListView : public QListView, protected ListBase {
    Q_OBJECT
    Q_PROPERTY(
        bool selectRightClickedRow READ isSelectRightClickedRow WRITE setSelectRightClickedRow)

public:
    explicit ListView(QWidget* parent = nullptr);

    bool isSelectRightClickedRow() const;
    void setSelectRightClickedRow(bool isSelect);

    void setCheckedColor(const QColor& light, const QColor& dark);

    void setBorderVisible(bool visible);

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
