#pragma once

#include <QAbstractItemModel>
#include <QIcon>
#include <QPersistentModelIndex>
#include <QPointer>
#include <QPushButton>

class QFrame;
class QParallelAnimationGroup;
class QScrollArea;
class QHBoxLayout;
class QIdentityProxyModel;

namespace qfw {
class TreeView;
class SearchLineEdit;
class TreeComboProxy;

// External models are borrowed. Selection is kept in a proxy, never written
// into the source model's CheckStateRole. Only column-zero leaves can be selected.
class TreeComboBox : public QPushButton {
    Q_OBJECT
    Q_PROPERTY(QString placeholderText READ placeholderText WRITE setPlaceholderText)
    Q_PROPERTY(bool searchEnabled READ isSearchEnabled WRITE setSearchEnabled)
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)
public:
    explicit TreeComboBox(QWidget* parent = nullptr);
    ~TreeComboBox() override;
    virtual void setModel(QAbstractItemModel* model);
    QAbstractItemModel* model() const;
    QModelIndex addItem(const QString& text, const QModelIndex& parent = QModelIndex(),
                        const QVariant& userData = QVariant(), const QIcon& icon = QIcon());
    QList<QModelIndex> addItems(const QStringList& texts,
                                const QModelIndex& parent = QModelIndex());
    void clear();
    void setPlaceholderText(const QString& text);
    QString placeholderText() const;
    void setMaxVisibleItems(int count);
    int maxVisibleItems() const;
    void setSearchEnabled(bool enabled);
    bool isSearchEnabled() const;
    void setSearchText(const QString& text);
    QString searchText() const;
    SearchLineEdit* searchLineEdit() const;
    QModelIndex currentModelIndex() const;
    void setCurrentModelIndex(const QModelIndex& index);
    QString currentText() const;
    QVariant currentData(int role = Qt::UserRole) const;
    void showPopup();
    void hidePopup();
    bool isPopupVisible() const;
    TreeView* treeView() const;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    void currentModelIndexChanged(const QModelIndex& index);
    void currentTextChanged(const QString& text);
    void activated(const QModelIndex& index);
    void searchTextChanged(const QString& text);

protected:
    bool eventFilter(QObject* object, QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void moveEvent(QMoveEvent* event) override;
    void changeEvent(QEvent* event) override;
    virtual bool isMultiSelection() const;
    virtual Qt::CheckState checkState(const QModelIndex& source) const;
    virtual void activateIndex(const QModelIndex& source, bool checkbox);
    virtual void synchronize();
    virtual void refreshDisplay();
    void validateIndex(const QModelIndex& index, bool leaf = true) const;
    QList<QModelIndex> leafDescendants(const QModelIndex& parent) const;
    void refreshChecks();
    void togglePopup();
    QString placeholder_;
    quint64 revision_ = 0;

private:
    friend class TreeComboProxy;
    void createPopup();
    void refreshPopupTheme();
    void applySearch();
    QPointer<QAbstractItemModel> model_;
    QList<QMetaObject::Connection> modelConnections_;
    QIdentityProxyModel* proxy_ = nullptr;
    QPersistentModelIndex current_;
    bool hadCurrent_ = false;
    QString lastText_;
    int maxVisible_ = -1;
    QFrame* popup_ = nullptr;
    QFrame* panel_ = nullptr;
    TreeView* tree_ = nullptr;
    SearchLineEdit* search_ = nullptr;
    QWidget* emptySearch_ = nullptr;
    QString searchText_;
    bool searchEnabled_ = true;
    bool filtering_ = false;
    QList<QPersistentModelIndex> expandedBeforeSearch_;
    QPersistentModelIndex currentBeforeSearch_;
    QParallelAnimationGroup* animation_ = nullptr;
    QPersistentModelIndex pressedIndex_;
    int pressedArea_ = 0;
};

class MultiSelectionTreeComboBox : public TreeComboBox {
    Q_OBJECT
    Q_PROPERTY(bool chipsMode READ isChipsMode WRITE setChipsMode)
public:
    explicit MultiSelectionTreeComboBox(QWidget* parent = nullptr);
    void setModel(QAbstractItemModel* model) override;
    QList<QModelIndex> selectedIndexes() const;
    QStringList selectedTexts() const;
    QList<QVariant> selectedData(int role = Qt::UserRole) const;
    void setSelectedIndexes(const QList<QModelIndex>& indexes);
    void clearSelection();
    void setChipsMode(bool enabled = true);
    bool isChipsMode() const;
    void setDescendantsSelected(const QModelIndex& parent, bool selected);

signals:
    void selectionChanged(const QList<QModelIndex>& indexes);
    void selectedTextChanged(const QStringList& texts);

protected:
    bool isMultiSelection() const override;
    Qt::CheckState checkState(const QModelIndex& source) const override;
    void activateIndex(const QModelIndex& source, bool checkbox) override;
    void synchronize() override;
    void refreshDisplay() override;
    void resizeEvent(QResizeEvent* event) override;
    bool eventFilter(QObject* object, QEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    void publish(bool indexesChanged);
    void positionChips();
    QList<QPersistentModelIndex> selected_;
    QStringList lastTexts_;
    bool chipsMode_ = true;
    bool chipsPressed_ = false;
    QScrollArea* chips_ = nullptr;
    QWidget* chipsContainer_ = nullptr;
    QHBoxLayout* chipsLayout_ = nullptr;
};
}  // namespace qfw
