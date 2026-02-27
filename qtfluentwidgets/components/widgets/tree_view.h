#pragma once

#include <QColor>
#include <QStyledItemDelegate>
#include <QTreeView>
#include <QTreeWidget>

namespace qfw {

class SmoothScrollDelegate;

class TreeItemDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit TreeItemDelegate(QTreeView* parent = nullptr);

    void setCheckedColor(const QColor& light, const QColor& dark);

protected:
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
    void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override;

private:
    void drawBackground(QPainter* painter, const QStyleOptionViewItem& option,
                        const QModelIndex& index) const;
    void drawIndicator(QPainter* painter, const QStyleOptionViewItem& option,
                       const QModelIndex& index) const;
    void drawCheckBox(QPainter* painter, const QStyleOptionViewItem& option,
                      const QModelIndex& index) const;

    QColor lightCheckedColor_;
    QColor darkCheckedColor_;
};

class TreeViewBase {
protected:
    TreeViewBase();
    ~TreeViewBase() = default;

    void initTreeBase(QTreeView* view);

    void setCheckedColor(const QColor& light, const QColor& dark);
    void setBorderVisible(bool visible);
    void setBorderRadius(int radius);

    TreeItemDelegate* delegate_ = nullptr;
    SmoothScrollDelegate* scrollDelegate_ = nullptr;

private:
    QTreeView* view_ = nullptr;
};

class TreeWidget : public QTreeWidget, protected TreeViewBase {
    Q_OBJECT

public:
    explicit TreeWidget(QWidget* parent = nullptr);

    void setCheckedColor(const QColor& light, const QColor& dark);
    void setBorderVisible(bool visible);
    void setBorderRadius(int radius);

protected:
    void drawBranches(QPainter* painter, const QRect& rect,
                      const QModelIndex& index) const override;
    bool viewportEvent(QEvent* event) override;
};

class TreeView : public QTreeView, protected TreeViewBase {
    Q_OBJECT

public:
    explicit TreeView(QWidget* parent = nullptr);

    void setCheckedColor(const QColor& light, const QColor& dark);
    void setBorderVisible(bool visible);
    void setBorderRadius(int radius);

protected:
    void drawBranches(QPainter* painter, const QRect& rect,
                      const QModelIndex& index) const override;
    bool viewportEvent(QEvent* event) override;
};

}  // namespace qfw
