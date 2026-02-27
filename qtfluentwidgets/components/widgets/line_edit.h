#pragma once

#include <QColor>
#include <QFocusEvent>
#include <QIcon>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPlainTextEdit>
#include <QTextBrowser>
#include <QToolButton>
#include <QVariant>

class QAction;
class QHBoxLayout;
class QContextMenuEvent;

namespace qfw {

class SmoothScrollDelegate;

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

private:
    void adjustTextMargins();

    bool isClearButtonEnabled_ = false;
    bool isError_ = false;

    QColor lightFocusedBorderColor_;
    QColor darkFocusedBorderColor_;

    QList<LineEditButton*> leftButtons_;
    QList<LineEditButton*> rightButtons_;

    QHBoxLayout* hBoxLayout_ = nullptr;
    LineEditButton* clearButton_ = nullptr;
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

signals:
    void searchSignal(const QString& text);
    void clearSignal();

private:
    LineEditButton* searchButton_ = nullptr;
};

}  // namespace qfw
