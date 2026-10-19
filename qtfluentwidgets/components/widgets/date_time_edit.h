#pragma once

#include <QDateEdit>
#include <QDateTimeEdit>
#include <QTimeEdit>

#include "components/widgets/spin_box.h"

namespace qfw {

/** Native date/time section editor with inline step buttons. */
class DateEdit : public QDateEdit, public InlineSpinBoxBase {
    Q_OBJECT
    Q_PROPERTY(bool error READ isError WRITE setError)
public:
    explicit DateEdit(QWidget* parent = nullptr);
    void setReadOnly(bool readOnly) override;
    void setSymbolVisible(bool visible) override;
    void setAccelerated(bool accelerated);
    void stepUp() override;
    void stepDown() override;

protected:
    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
};

/** Native date/time section editor with inline step buttons. */
class TimeEdit : public QTimeEdit, public InlineSpinBoxBase {
    Q_OBJECT
    Q_PROPERTY(bool error READ isError WRITE setError)
public:
    explicit TimeEdit(QWidget* parent = nullptr);
    void setReadOnly(bool readOnly) override;
    void setSymbolVisible(bool visible) override;
    void setAccelerated(bool accelerated);
    void stepUp() override;
    void stepDown() override;

protected:
    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
};

/** Native date/time section editor with inline step buttons. */
class DateTimeEdit : public QDateTimeEdit, public InlineSpinBoxBase {
    Q_OBJECT
    Q_PROPERTY(bool error READ isError WRITE setError)
public:
    explicit DateTimeEdit(QWidget* parent = nullptr);
    void setReadOnly(bool readOnly) override;
    void setSymbolVisible(bool visible) override;
    void setAccelerated(bool accelerated);
    void stepUp() override;
    void stepDown() override;

protected:
    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
};

/** Native date/time section editor with a compact stepping flyout. */
class CompactDateEdit : public QDateEdit, public CompactSpinBoxBase {
    Q_OBJECT
    Q_PROPERTY(bool error READ isError WRITE setError)
public:
    explicit CompactDateEdit(QWidget* parent = nullptr);
    void setReadOnly(bool readOnly) override;
    void setSymbolVisible(bool visible) override;
    void setAccelerated(bool accelerated);
    void stepUp() override;
    void stepDown() override;

protected:
    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
};

/** Native date/time section editor with a compact stepping flyout. */
class CompactTimeEdit : public QTimeEdit, public CompactSpinBoxBase {
    Q_OBJECT
    Q_PROPERTY(bool error READ isError WRITE setError)
public:
    explicit CompactTimeEdit(QWidget* parent = nullptr);
    void setReadOnly(bool readOnly) override;
    void setSymbolVisible(bool visible) override;
    void setAccelerated(bool accelerated);
    void stepUp() override;
    void stepDown() override;

protected:
    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
};

/** Native date/time section editor with a compact stepping flyout. */
class CompactDateTimeEdit : public QDateTimeEdit, public CompactSpinBoxBase {
    Q_OBJECT
    Q_PROPERTY(bool error READ isError WRITE setError)
public:
    explicit CompactDateTimeEdit(QWidget* parent = nullptr);
    void setReadOnly(bool readOnly) override;
    void setSymbolVisible(bool visible) override;
    void setAccelerated(bool accelerated);
    void stepUp() override;
    void stepDown() override;

protected:
    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
};

}  // namespace qfw
