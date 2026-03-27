#pragma once

#include <QColor>
#include <QCompleter>
#include <QFocusEvent>
#include <QIcon>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPlainTextEdit>
#include <QTextBrowser>
#include <QToolButton>
#include <QVariant>

#include "components/widgets/menu.h"

class QAction;
class QHBoxLayout;
class QContextMenuEvent;
class QAbstractItemModel;

namespace qfw {

class SmoothScrollDelegate;
class LineEdit;

class LineEditButton : public QToolButton {
    Q_OBJECT

public:
    explicit LineEditButton(const QVariant& icon = QVariant(), QWidget* parent = nullptr);

    void setAction(QAction* action);
    QAction* action() const;

    void setIcon(const QVariant& icon);
    QVariant icon() const;

protected:
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

private slots:
    void onActionChanged();

private:
    QVariant icon_;
    QAction* action_ = nullptr;
    bool isPressed_ = false;
};

class CompleterMenu : public RoundMenu {
    Q_OBJECT

public:
    explicit CompleterMenu(LineEdit* lineEdit);

    bool setCompletion(QAbstractItemModel* model, int column = 0);
    void setItems(const QStringList& items);
    void setMaxVisibleItems(int num);

    void popup();

signals:
    void activated(const QString& text);
    void indexActivated(const QModelIndex& index);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void onItemClicked(QListWidgetItem* item);

private:
    void onCompletionItemSelected(const QString& text, int row);

    LineEdit* lineEdit_ = nullptr;
    QStringList items_;
    QList<QModelIndex> indexes_;
};

class LineEdit : public QLineEdit {
    Q_OBJECT

public:
    explicit LineEdit(QWidget* parent = nullptr);

    bool isError() const;
    void setError(bool isError);

    void setCustomFocusedBorderColor(const QColor& light, const QColor& dark);
    QColor focusedBorderColor() const;

    void setClearButtonEnabled(bool enable);
    bool isClearButtonEnabled() const;

    void setCompleter(QCompleter* completer);
    QCompleter* completer() const;

    void addAction(QAction* action,
                   QLineEdit::ActionPosition position = QLineEdit::TrailingPosition);
    void addActions(const QList<QAction*>& actions,
                    QLineEdit::ActionPosition position = QLineEdit::TrailingPosition);

protected:
    void focusOutEvent(QFocusEvent* e) override;
    void focusInEvent(QFocusEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

    QHBoxLayout* buttonLayout() const;
    LineEditButton* clearButton() const;

private slots:
    void onTextChanged(const QString& text);
    void onTextEdited(const QString& text);
    void showCompleterMenu();

private:
    void adjustTextMargins();
    void setCompleterMenu(CompleterMenu* menu);

    QColor lightFocusedBorderColor_;
    QColor darkFocusedBorderColor_;

    QList<LineEditButton*> leftButtons_;
    QList<LineEditButton*> rightButtons_;

    QHBoxLayout* hBoxLayout_ = nullptr;
    LineEditButton* clearButton_ = nullptr;

    QCompleter* completer_ = nullptr;
    CompleterMenu* completerMenu_ = nullptr;

protected:
    bool isClearButtonEnabled_ = false;
    bool isError_ = false;
};

class PlainTextEdit : public QPlainTextEdit {
    Q_OBJECT

public:
    explicit PlainTextEdit(QWidget* parent = nullptr);

protected:
    void contextMenuEvent(QContextMenuEvent* e) override;

private:
    SmoothScrollDelegate* scrollDelegate_ = nullptr;
};

class TextBrowser : public QTextBrowser {
    Q_OBJECT

public:
    explicit TextBrowser(QWidget* parent = nullptr);

protected:
    void contextMenuEvent(QContextMenuEvent* e) override;

private:
    SmoothScrollDelegate* scrollDelegate_ = nullptr;
};

class PasswordLineEdit : public LineEdit {
    Q_OBJECT

public:
    explicit PasswordLineEdit(QWidget* parent = nullptr);

    void setPasswordVisible(bool visible);
    bool isPasswordVisible() const;

private:
    void updateViewButtonIcon();

    LineEditButton* viewButton_ = nullptr;
    bool passwordVisible_ = false;
};

class SearchLineEdit : public LineEdit {
    Q_OBJECT

public:
    explicit SearchLineEdit(QWidget* parent = nullptr);

    void search();
    void setClearButtonEnabled(bool enable);

signals:
    void searchSignal(const QString& text);
    void clearSignal();

private:
    LineEditButton* searchButton_ = nullptr;
};

}  // namespace qfw
