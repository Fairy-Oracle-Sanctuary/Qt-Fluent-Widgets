#pragma once

#include <QAction>
#include <QIcon>
#include <QList>
#include <QObject>
#include <QPoint>
#include <QPointer>
#include <QPushButton>
#include <QString>
#include <QVariant>

#include "common/config.h"
#include "common/font.h"
#include "common/icon.h"
#include "components/widgets/line_edit.h"
#include "components/widgets/menu.h"

namespace qfw {

class ComboItem {
public:
    ComboItem(const QString& text, const QVariant& icon = QVariant(),
              const QVariant& userData = QVariant(), bool isEnabled = true);

    QString text;
    QVariant userData;
    bool isEnabled;

    QIcon icon() const;
    void setIcon(const QVariant& icon);

private:
    QVariant icon_;
};

class ComboBoxMenu;

class ComboBoxBase {
public:
    virtual ~ComboBoxBase() = default;

    void addItem(const QString& text, const QVariant& icon = QVariant(),
                 const QVariant& userData = QVariant());
    void addItems(const QStringList& texts);
    void removeItem(int index);

    int currentIndex() const;
    virtual void setCurrentIndex(int index);

    virtual void setText(const QString& text) = 0;
    QString currentText() const;
    QVariant currentData() const;
    void setCurrentText(const QString& text);

    void setItemText(int index, const QString& text);
    QVariant itemData(int index) const;
    QString itemText(int index) const;
    QIcon itemIcon(int index) const;

    void setItemData(int index, const QVariant& value);
    void setItemIcon(int index, const QVariant& icon);
    void setItemEnabled(int index, bool isEnabled);

    int findData(const QVariant& data) const;
    int findText(const QString& text) const;

    void clear();
    int count() const;

    void insertItem(int index, const QString& text, const QVariant& icon = QVariant(),
                    const QVariant& userData = QVariant());
    void insertItems(int index, const QStringList& texts);

    void setMaxVisibleItems(int num);
    int maxVisibleItems() const;

protected:
    void setUpUi(QWidget* widget);
    void closeComboMenu();
    void onDropMenuClosed();
    virtual ComboBoxMenu* createComboMenu();
    void showComboMenu(QWidget* widget);
    void toggleComboMenu(QWidget* widget);
    virtual void onItemClicked(int index);

    bool isHover = false;
    bool isPressed = false;
    QList<ComboItem> items;
    int currentIndex_ = -1;
    int maxVisibleItems_ = -1;
    QPointer<ComboBoxMenu> dropMenu;
    QString placeholderText_;

    QWidget* ownerWidget_ = nullptr;
};

class ComboBoxMenu : public RoundMenu {
    Q_OBJECT
public:
    explicit ComboBoxMenu(QWidget* parent = nullptr);
    void execAt(const QPoint& pos, bool ani = true,
                MenuAnimationType aniType = MenuAnimationType::DropDown) override;
};

class ComboBox : public QPushButton, public ComboBoxBase {
    Q_OBJECT

public:
    explicit ComboBox(QWidget* parent = nullptr);

    void setPlaceholderText(const QString& text);
    void setCurrentIndex(int index) override;
    void setText(const QString& text) override;

signals:
    void currentIndexChanged(int index);
    void currentTextChanged(const QString& text);
    void activated(int index);
    void textActivated(const QString& text);

protected:
    bool eventFilter(QObject* obj, QEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

private:
    void updateTextState(bool isPlaceholder);
    void onItemClicked(int index) override;
};

class EditableComboBox : public LineEdit, public ComboBoxBase {
    Q_OBJECT

public:
    explicit EditableComboBox(QWidget* parent = nullptr);

    void setPlaceholderText(const QString& text);
    QString currentText() const;
    void setCurrentIndex(int index) override;
    void clear();

    void setText(const QString& text) override;

signals:
    void currentIndexChanged(int index);
    void currentTextChanged(const QString& text);
    void activated(int index);
    void textActivated(const QString& text);

protected:
    bool eventFilter(QObject* obj, QEvent* e) override;

private:
    void onComboTextChanged(const QString& text);
    void onReturnPressed();
    void onDropMenuClosed();
    void onClearButtonClicked();
    void onItemClicked(int index) override;

    LineEditButton* dropButton_ = nullptr;
};

}  // namespace qfw
