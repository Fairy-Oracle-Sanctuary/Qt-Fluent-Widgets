#pragma once

#include "common/icon.h"
#include "components/widgets/list_view.h"

namespace qfw {

class RoundListItemDelegate : public ListItemDelegate {
    Q_OBJECT
    Q_PROPERTY(bool transparentSelection READ transparentSelection WRITE setTransparentSelection)
    Q_PROPERTY(bool keepNormalBackground READ keepNormalBackground WRITE setKeepNormalBackground)
public:
    explicit RoundListItemDelegate(QListView* parent = nullptr);
    bool transparentSelection() const;
    void setTransparentSelection(bool transparent);
    bool keepNormalBackground() const;
    void setKeepNormalBackground(bool keep);
    // Owns a clone; model items may outlive the original FluentIcon instance.
    static QIcon themedIcon(const FluentIconBase& icon);
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override;
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

private:
    void refresh();
    bool transparentSelection_ = false;
    bool keepNormalBackground_ = false;
};

class RoundListWidget : public ListWidget {
    Q_OBJECT
public:
    explicit RoundListWidget(QWidget* parent = nullptr);
    using QListWidget::addItem;
    using QListWidget::addItems;
    void setCurrentRow(int row);
    void setCurrentRow(int row, QItemSelectionModel::SelectionFlags command);
    void setCurrentItem(QListWidgetItem* item);
    void setCurrentItem(QListWidgetItem* item, QItemSelectionModel::SelectionFlags command);
    void addItem(const QString& text, const QIcon& icon);
    void addItem(const QString& text, const FluentIconBase& icon);
    void addItem(const QString& text, FluentIconEnum icon);
    RoundListItemDelegate* roundDelegate() const;
    void setCheckedColor(const QColor& light, const QColor& dark);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
};

class RoundListView : public ListView {
    Q_OBJECT
public:
    explicit RoundListView(QWidget* parent = nullptr);
    RoundListItemDelegate* roundDelegate() const;
    void setCheckedColor(const QColor& light, const QColor& dark);
    void clearSelection();
    void setCurrentIndex(const QModelIndex& index);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
};

class TransparentRoundListWidget : public RoundListWidget {
    Q_OBJECT
public:
    explicit TransparentRoundListWidget(QWidget* parent = nullptr);
};
class TransparentRoundListView : public RoundListView {
    Q_OBJECT
public:
    explicit TransparentRoundListView(QWidget* parent = nullptr);
};
class CategoryCardListWidget : public RoundListWidget {
    Q_OBJECT
public:
    explicit CategoryCardListWidget(QWidget* parent = nullptr);
};
class CategoryCardListView : public RoundListView {
    Q_OBJECT
public:
    explicit CategoryCardListView(QWidget* parent = nullptr);
};

}  // namespace qfw
