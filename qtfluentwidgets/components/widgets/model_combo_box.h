#pragma once

#include <QAbstractItemModel>
#include <QEvent>
#include <QIcon>
#include <QLineEdit>
#include <QModelIndex>
#include <QPushButton>
#include <QStandardItemModel>
#include <QString>
#include <QVariant>
#include <QWidget>

#include "components/widgets/line_edit.h"

namespace qfw {

// Forward declarations
class ComboBoxMenu;

/**
 * @brief Abstract combo box built in data model
 *
 * This is a mixin class that provides combo box functionality with a data model.
 * Inherit from this and a QWidget-derived class to create a functional combo box.
 */
class ModelComboBoxBase {
public:
    virtual ~ModelComboBoxBase() = default;

    void setUpUi();

    void setModel(QAbstractItemModel* model);
    QAbstractItemModel* model() const { return model_; }

    // Item management
    QModelIndex insertItem(int index, const QString& text, const QVariant& userData = QVariant(),
                           const QIcon& icon = QIcon());
    void insertItems(int index, const QStringList& texts);
    void addItem(const QString& text, const QVariant& userData = QVariant(),
                 const QIcon& icon = QIcon());
    void addItems(const QStringList& texts);
    void removeItem(int index);

    // Current item
    int currentIndex() const { return currentIndex_; }
    void setCurrentIndex(int index);
    QString currentText() const;
    QVariant currentData() const;
    void setCurrentText(const QString& text);

    // Item data accessors
    QVariant itemData(int index) const;
    QString itemText(int index) const;
    QIcon itemIcon(int index) const;
    void setItemData(int index, const QVariant& value, int role = Qt::UserRole);
    void setItemText(int index, const QString& text);
    void setItemIcon(int index, const QIcon& icon);

    // Find items
    int findData(const QVariant& data, int role = Qt::UserRole,
                 Qt::MatchFlags flags = Qt::MatchExactly) const;
    int findText(const QString& text, Qt::MatchFlags flags = Qt::MatchExactly) const;

    // Count
    int count() const;

    // Max visible items
    void setMaxVisibleItems(int num);
    int maxVisibleItems() const { return maxVisibleItems_; }

    // Clear
    void clear();

    // Placeholder text
    void setPlaceholderText(const QString& text);
    QString placeholderText() const { return placeholderText_; }

    // State
    bool isHover() const { return isHover_; }
    bool isPressed() const { return isPressed_; }

protected:
    ModelComboBoxBase();

    // Virtual methods to be implemented by derived class
    virtual void setText(const QString& text) = 0;
    virtual QString text() const = 0;
    virtual void adjustSize() = 0;
    virtual QWidget* asWidget() = 0;
    virtual const QWidget* asWidget() const = 0;
    virtual QRect rect() const = 0;
    virtual QPoint mapToGlobal(const QPoint& pos) const = 0;
    virtual QPoint mapFromGlobal(const QPoint& pos) const = 0;
    virtual int height() const = 0;
    virtual int width() const = 0;

    // Model signal handlers
    void onModelRowInserted(const QModelIndex& parentIndex, int first, int last);
    void onRowsRemoved(const QModelIndex& parentIndex, int first, int last);
    void onModelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight,
                            const QList<int>& roles);

    // Event filter
    bool eventFilter(QObject* obj, QEvent* e);

    // Drop menu management
    void closeComboMenu();
    virtual void onDropMenuClosed();
    virtual ComboBoxMenu* createComboMenu();
    void showComboMenu();
    void toggleComboMenu();
    void onItemClicked(int index);

    bool isValidIndex(int index) const;
    QModelIndex insertItemFromValues(int row, const QMap<int, QVariant>& values);

    // Member variables
    QAbstractItemModel* model_ = nullptr;
    int currentIndex_ = -1;
    int maxVisibleItems_ = -1;
    QString placeholderText_;
    ComboBoxMenu* dropMenu_ = nullptr;
    bool isHover_ = false;
    bool isPressed_ = false;
};

/**
 * @brief Combo box built in data model
 */
class ModelComboBox : public QPushButton, public ModelComboBoxBase {
    Q_OBJECT

public:
    explicit ModelComboBox(QWidget* parent = nullptr);

    void setIconVisible(bool visible);
    bool isIconVisible() const { return isIconVisible_; }

    void setPlaceholderText(const QString& text);
    void setCurrentIndex(int index);
    void clear();
    void setItemIcon(int index, const QIcon& icon);

signals:
    void currentIndexChanged(int index);
    void currentTextChanged(const QString& text);
    void activated(int index);
    void textActivated(const QString& text);

protected:
    void setText(const QString& text) override;
    QString text() const override;
    void adjustSize() override;
    QWidget* asWidget() override;
    const QWidget* asWidget() const override;
    QRect rect() const override;
    QPoint mapToGlobal(const QPoint& pos) const override;
    QPoint mapFromGlobal(const QPoint& pos) const override;
    int height() const override;
    int width() const override;

    bool eventFilter(QObject* obj, QEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

    void onItemClicked(int index);

private:
    void updateIcon();
    void updateTextState(bool isPlaceholder);

    bool isIconVisible_ = true;
};

class LineEditButton;

/**
 * @brief Editable combo box built in data model
 */
class EditableModelComboBox : public LineEdit, public ModelComboBoxBase {
    Q_OBJECT

public:
    explicit EditableModelComboBox(QWidget* parent = nullptr);

    QString currentText() const;
    void setCurrentIndex(int index);
    void clear();
    void setPlaceholderText(const QString& text);

signals:
    void currentIndexChanged(int index);
    void currentTextChanged(const QString& text);
    void activated(int index);
    void textActivated(const QString& text);

protected:
    void setText(const QString& text) override;
    QString text() const override;
    void adjustSize() override;
    QWidget* asWidget() override;
    const QWidget* asWidget() const override;
    QRect rect() const override;
    QPoint mapToGlobal(const QPoint& pos) const override;
    QPoint mapFromGlobal(const QPoint& pos) const override;
    int height() const override;
    int width() const override;

    void onDropMenuClosed() override;

private slots:
    void onReturnPressed();
    void onComboTextChanged(const QString& text);
    void onClearButtonClicked();

private:
    class LineEditButton* dropButton_ = nullptr;
};

}  // namespace qfw
