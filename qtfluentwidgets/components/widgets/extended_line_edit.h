#pragma once
#include <QHBoxLayout>
#include <QList>
#include <QPointer>
#include <QValidator>

#include "components/widgets/line_edit.h"

namespace qfw {
class PushButton;
class LabelLineEdit : public LineEdit {
    Q_OBJECT
    Q_PROPERTY(QString prefix READ prefix WRITE setPrefix)
    Q_PROPERTY(QString suffix READ suffix WRITE setSuffix)
public:
    LabelLineEdit(const QString& prefix, const QString& suffix, QWidget* parent = nullptr);
    QString prefix() const;
    QString suffix() const;
    void setPrefix(const QString& prefix);
    void setSuffix(const QString& suffix);
    PushButton* prefixButton() const;
    PushButton* suffixButton() const;

protected:
    void adjustTextMargins() override;
    bool eventFilter(QObject* watched, QEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    void updateLabels();
    PushButton* prefix_ = nullptr;
    PushButton* suffix_ = nullptr;
};

/** A PIN cell with a full focus outline and Python-compatible 2px focus contraction. */
class PinBoxLineEdit : public LineEdit {
    Q_OBJECT
public:
    explicit PinBoxLineEdit(QWidget* parent = nullptr);
    void setCellSize(const QSize& size);
    QSize cellSize() const;
    void setFixedSize(int width, int height);
    void setFixedSize(const QSize& size);
    void setFixedWidth(int width);
    void setFixedHeight(int height);
    void setCustomFocusedBorderColor(const QColor& light, const QColor& dark) override;
    QColor focusedBorderColor() const override;

protected:
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    void updateCellSize();
    QSize cellSize_{40, 32};
    QColor lightBorderColor_, darkBorderColor_;
};

// A row of native one-character editors. Validators are borrowed, not reparented.
class PinBox : public QWidget {
    Q_OBJECT
public:
    explicit PinBox(QWidget* parent = nullptr);
    int count() const;
    void setPinBoxCount(int count);
    void setPinBoxFixedWidth(int width);
    void setPinBoxFixedHeight(int height);
    void setEchoMode(QLineEdit::EchoMode mode);
    QLineEdit::EchoMode echoMode() const;
    void setPinBoxValidator(const QValidator* validator);
    QStringList texts() const;
    QString text() const;
    void setText(const QString& text);
    void clear();
    PinBoxLineEdit* editor(int index) const;
    QHBoxLayout* hBoxLayout;
signals:
    void textChanged(const QStringList& characters);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void notifyTextChanged();
    void focusEditor(int index);
    bool pasteAt(int index, const QString& text);
    QList<PinBoxLineEdit*> editors_;
    QPointer<const QValidator> validator_;
    QStringList lastText_;
    int boxWidth_ = 40, boxHeight_ = 32;
    QLineEdit::EchoMode echoMode_ = QLineEdit::Normal;
    bool changing_ = false;
};
}  // namespace qfw
