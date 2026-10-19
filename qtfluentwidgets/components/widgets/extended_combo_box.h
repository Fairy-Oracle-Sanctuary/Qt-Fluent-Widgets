#pragma once

#include <QFontDatabase>
#include <QSet>

#include "components/widgets/combo_box.h"

class QFrame;
class QHBoxLayout;
class QListWidget;
class QParallelAnimationGroup;
class QScrollArea;

namespace qfw {

class FontComboBox : public ComboBox {
    Q_OBJECT
    Q_PROPERTY(int fontFilters READ fontFilters WRITE setFontFilters)
    Q_PROPERTY(QFont currentFont READ currentFont WRITE setCurrentFont NOTIFY currentFontChanged)
public:
    enum FontFilter {
        AllFonts = 0,
        ScalableFonts = 1,
        NonScalableFonts = 2,
        MonospacedFonts = 4,
        ProportionalFonts = 8
    };
    Q_ENUM(FontFilter)
    explicit FontComboBox(QWidget* parent = nullptr);
    ~FontComboBox() override;
    int fontFilters() const;
    void setFontFilters(int filters);
    QFontDatabase::WritingSystem writingSystem() const;
    void setWritingSystem(QFontDatabase::WritingSystem system);
    QFont currentFont() const;
    void setCurrentFont(const QFont& font);
    void setCurrentIndex(int index) override;
    void showPopup();
    void hidePopup();
    bool isPopupVisible() const;
    QListWidget* popupView() const;
    void clear();

signals:
    void currentFontChanged(const QFont& font);

protected:
    ComboBoxMenu* createComboMenu() override;
    void keyPressEvent(QKeyEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void moveEvent(QMoveEvent* event) override;
    void changeEvent(QEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    bool eventFilter(QObject* object, QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void onItemClicked(int index) override;

private:
    void populateFonts();
    int filters_ = AllFonts;
    QFontDatabase::WritingSystem writingSystem_ = QFontDatabase::Any;
    QString lastFamily_;
    quint64 revision_ = 0;
};

// Selection is identified by index, including when two options have identical
// text. Text/data results are always returned in insertion order.
class MultiSelectionComboBox : public QPushButton {
    Q_OBJECT
    Q_PROPERTY(QString placeholderText READ placeholderText WRITE setPlaceholderText)
    Q_PROPERTY(bool chipsMode READ isChipsMode WRITE setChipsMode)
public:
    explicit MultiSelectionComboBox(QWidget* parent = nullptr);
    ~MultiSelectionComboBox() override;
    void addItem(const QString& text, const QVariant& icon = QVariant(),
                 const QVariant& userData = QVariant());
    void addItems(const QStringList& texts);
    void insertItem(int index, const QString& text, const QVariant& icon = QVariant(),
                    const QVariant& userData = QVariant());
    void removeItem(int index);
    void clear();
    int count() const;
    QString itemText(int index) const;
    QVariant itemData(int index) const;
    QIcon itemIcon(int index) const;
    bool isItemEnabled(int index) const;
    void setItemText(int index, const QString& text);
    void setItemData(int index, const QVariant& value);
    void setItemIcon(int index, const QVariant& icon);
    void setItemEnabled(int index, bool enabled);
    QSet<int> selectedIndices() const;
    QList<ComboItem> selectedItems() const;
    QStringList selectedTexts() const;
    QList<QVariant> selectedData() const;
    void setSelectedIndices(const QSet<int>& indexes);
    void clearSelection();
    void setPlaceholderText(const QString& text);
    QString placeholderText() const;
    void setChipsMode(bool enabled = true);
    bool isChipsMode() const;
    void setMaxVisibleItems(int count);
    int maxVisibleItems() const;
    void showPopup();
    void hidePopup();
    bool isPopupVisible() const;
    QListWidget* popupView() const;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    void selectionChanged(const QSet<int>& indexes);
    void selectedTextChanged(const QStringList& texts);

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    bool eventFilter(QObject* object, QEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void moveEvent(QMoveEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    bool validIndex(int index) const;
    void publish(const QSet<int>& previous);
    void refreshDisplay();
    void rebuildPopup();
    void refreshTheme();
    void positionChips();
    void togglePopup();
    void toggleIndex(int index);
    QList<ComboItem> items_;
    QList<quint64> identities_;
    quint64 nextIdentity_ = 0;
    QSet<int> selected_;
    QStringList lastTexts_;
    quint64 revision_ = 0;
    QString placeholder_;
    bool chipsMode_ = false;
    bool chipsPressed_ = false;
    int maxVisible_ = -1;
    QFrame* popup_ = nullptr;
    QFrame* panel_ = nullptr;
    QListWidget* view_ = nullptr;
    QParallelAnimationGroup* animation_ = nullptr;
    QScrollArea* chips_ = nullptr;
    QWidget* chipsContainer_ = nullptr;
    QHBoxLayout* chipsLayout_ = nullptr;
};
}  // namespace qfw
