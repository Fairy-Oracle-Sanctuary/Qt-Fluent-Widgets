#include "date_time_edit.h"

#include <QLineEdit>
#include <QPainter>

#include "common/font.h"
#include "common/style_sheet.h"

namespace qfw {

DateEdit::DateEdit(QWidget* parent) : QDateEdit(parent), InlineSpinBoxBase(this) {
    setProperty("qssClass", "DateEdit");
    setProperty("transparent", true);
    qfw::setStyleSheet(this, FluentStyleSheet::SpinBox);
    qfw::setFont(this);
    setFixedHeight(33);
    setMinimumWidth(210);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setButtonSymbols(QAbstractSpinBox::NoButtons);
    setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    // Keep the native editor's standard editing/context-menu and section semantics.
    updateButtonPosition();
}
void DateEdit::setReadOnly(bool readOnly) {
    QDateEdit::setReadOnly(readOnly);
    InlineSpinBoxBase::setReadOnly(readOnly);
}
void DateEdit::setSymbolVisible(bool visible) { InlineSpinBoxBase::setSymbolVisible(visible); }
void DateEdit::setAccelerated(bool accelerated) {
    QDateEdit::setAccelerated(accelerated);
    InlineSpinBoxBase::setAccelerated(accelerated);
}
void DateEdit::stepUp() {
    if (isEnabled() && !isReadOnly()) QDateEdit::stepUp();
}
void DateEdit::stepDown() {
    if (isEnabled() && !isReadOnly()) QDateEdit::stepDown();
}
void DateEdit::resizeEvent(QResizeEvent* event) {
    QDateEdit::resizeEvent(event);
    updateButtonPosition();
}
void DateEdit::paintEvent(QPaintEvent* event) {
    QDateEdit::paintEvent(event);
    QPainter painter(this);
    drawBorderBottom(this, &painter);
}

TimeEdit::TimeEdit(QWidget* parent) : QTimeEdit(parent), InlineSpinBoxBase(this) {
    setProperty("qssClass", "TimeEdit");
    setProperty("transparent", true);
    qfw::setStyleSheet(this, FluentStyleSheet::SpinBox);
    qfw::setFont(this);
    setFixedHeight(33);
    setMinimumWidth(210);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setButtonSymbols(QAbstractSpinBox::NoButtons);
    setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    // Keep the native editor's standard editing/context-menu and section semantics.
    updateButtonPosition();
}
void TimeEdit::setReadOnly(bool readOnly) {
    QTimeEdit::setReadOnly(readOnly);
    InlineSpinBoxBase::setReadOnly(readOnly);
}
void TimeEdit::setSymbolVisible(bool visible) { InlineSpinBoxBase::setSymbolVisible(visible); }
void TimeEdit::setAccelerated(bool accelerated) {
    QTimeEdit::setAccelerated(accelerated);
    InlineSpinBoxBase::setAccelerated(accelerated);
}
void TimeEdit::stepUp() {
    if (isEnabled() && !isReadOnly()) QTimeEdit::stepUp();
}
void TimeEdit::stepDown() {
    if (isEnabled() && !isReadOnly()) QTimeEdit::stepDown();
}
void TimeEdit::resizeEvent(QResizeEvent* event) {
    QTimeEdit::resizeEvent(event);
    updateButtonPosition();
}
void TimeEdit::paintEvent(QPaintEvent* event) {
    QTimeEdit::paintEvent(event);
    QPainter painter(this);
    drawBorderBottom(this, &painter);
}

DateTimeEdit::DateTimeEdit(QWidget* parent) : QDateTimeEdit(parent), InlineSpinBoxBase(this) {
    setProperty("qssClass", "DateTimeEdit");
    setProperty("transparent", true);
    qfw::setStyleSheet(this, FluentStyleSheet::SpinBox);
    qfw::setFont(this);
    setFixedHeight(33);
    setMinimumWidth(280);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setButtonSymbols(QAbstractSpinBox::NoButtons);
    setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    // Keep the native editor's standard editing/context-menu and section semantics.
    updateButtonPosition();
}
void DateTimeEdit::setReadOnly(bool readOnly) {
    QDateTimeEdit::setReadOnly(readOnly);
    InlineSpinBoxBase::setReadOnly(readOnly);
}
void DateTimeEdit::setSymbolVisible(bool visible) { InlineSpinBoxBase::setSymbolVisible(visible); }
void DateTimeEdit::setAccelerated(bool accelerated) {
    QDateTimeEdit::setAccelerated(accelerated);
    InlineSpinBoxBase::setAccelerated(accelerated);
}
void DateTimeEdit::stepUp() {
    if (isEnabled() && !isReadOnly()) QDateTimeEdit::stepUp();
}
void DateTimeEdit::stepDown() {
    if (isEnabled() && !isReadOnly()) QDateTimeEdit::stepDown();
}
void DateTimeEdit::resizeEvent(QResizeEvent* event) {
    QDateTimeEdit::resizeEvent(event);
    updateButtonPosition();
}
void DateTimeEdit::paintEvent(QPaintEvent* event) {
    QDateTimeEdit::paintEvent(event);
    QPainter painter(this);
    drawBorderBottom(this, &painter);
}

CompactDateEdit::CompactDateEdit(QWidget* parent) : QDateEdit(parent), CompactSpinBoxBase(this) {
    setProperty("qssClass", "CompactDateEdit");
    setProperty("transparent", true);
    qfw::setStyleSheet(this, FluentStyleSheet::SpinBox);
    qfw::setFont(this);
    setFixedHeight(33);
    setMinimumWidth(160);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setButtonSymbols(QAbstractSpinBox::NoButtons);
    setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    // Keep the native editor's standard editing/context-menu and section semantics.
    updateButtonPosition();
}
void CompactDateEdit::setReadOnly(bool readOnly) {
    QDateEdit::setReadOnly(readOnly);
    CompactSpinBoxBase::setReadOnly(readOnly);
}
void CompactDateEdit::setSymbolVisible(bool visible) {
    CompactSpinBoxBase::setSymbolVisible(visible);
}
void CompactDateEdit::setAccelerated(bool accelerated) {
    QDateEdit::setAccelerated(accelerated);
    CompactSpinBoxBase::setAccelerated(accelerated);
}
void CompactDateEdit::stepUp() {
    if (isEnabled() && !isReadOnly()) QDateEdit::stepUp();
}
void CompactDateEdit::stepDown() {
    if (isEnabled() && !isReadOnly()) QDateEdit::stepDown();
}
void CompactDateEdit::resizeEvent(QResizeEvent* event) {
    QDateEdit::resizeEvent(event);
    updateButtonPosition();
}
void CompactDateEdit::paintEvent(QPaintEvent* event) {
    QDateEdit::paintEvent(event);
    QPainter painter(this);
    drawBorderBottom(this, &painter);
}
void CompactDateEdit::focusInEvent(QFocusEvent* event) {
    QDateEdit::focusInEvent(event);
    CompactSpinBoxBase::focusInEvent(this, event);
}

CompactTimeEdit::CompactTimeEdit(QWidget* parent) : QTimeEdit(parent), CompactSpinBoxBase(this) {
    setProperty("qssClass", "CompactTimeEdit");
    setProperty("transparent", true);
    qfw::setStyleSheet(this, FluentStyleSheet::SpinBox);
    qfw::setFont(this);
    setFixedHeight(33);
    setMinimumWidth(160);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setButtonSymbols(QAbstractSpinBox::NoButtons);
    setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    // Keep the native editor's standard editing/context-menu and section semantics.
    updateButtonPosition();
}
void CompactTimeEdit::setReadOnly(bool readOnly) {
    QTimeEdit::setReadOnly(readOnly);
    CompactSpinBoxBase::setReadOnly(readOnly);
}
void CompactTimeEdit::setSymbolVisible(bool visible) {
    CompactSpinBoxBase::setSymbolVisible(visible);
}
void CompactTimeEdit::setAccelerated(bool accelerated) {
    QTimeEdit::setAccelerated(accelerated);
    CompactSpinBoxBase::setAccelerated(accelerated);
}
void CompactTimeEdit::stepUp() {
    if (isEnabled() && !isReadOnly()) QTimeEdit::stepUp();
}
void CompactTimeEdit::stepDown() {
    if (isEnabled() && !isReadOnly()) QTimeEdit::stepDown();
}
void CompactTimeEdit::resizeEvent(QResizeEvent* event) {
    QTimeEdit::resizeEvent(event);
    updateButtonPosition();
}
void CompactTimeEdit::paintEvent(QPaintEvent* event) {
    QTimeEdit::paintEvent(event);
    QPainter painter(this);
    drawBorderBottom(this, &painter);
}
void CompactTimeEdit::focusInEvent(QFocusEvent* event) {
    QTimeEdit::focusInEvent(event);
    CompactSpinBoxBase::focusInEvent(this, event);
}

CompactDateTimeEdit::CompactDateTimeEdit(QWidget* parent)
    : QDateTimeEdit(parent), CompactSpinBoxBase(this) {
    setProperty("qssClass", "CompactDateTimeEdit");
    setProperty("transparent", true);
    qfw::setStyleSheet(this, FluentStyleSheet::SpinBox);
    qfw::setFont(this);
    setFixedHeight(33);
    setMinimumWidth(220);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setButtonSymbols(QAbstractSpinBox::NoButtons);
    setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    // Keep the native editor's standard editing/context-menu and section semantics.
    updateButtonPosition();
}
void CompactDateTimeEdit::setReadOnly(bool readOnly) {
    QDateTimeEdit::setReadOnly(readOnly);
    CompactSpinBoxBase::setReadOnly(readOnly);
}
void CompactDateTimeEdit::setSymbolVisible(bool visible) {
    CompactSpinBoxBase::setSymbolVisible(visible);
}
void CompactDateTimeEdit::setAccelerated(bool accelerated) {
    QDateTimeEdit::setAccelerated(accelerated);
    CompactSpinBoxBase::setAccelerated(accelerated);
}
void CompactDateTimeEdit::stepUp() {
    if (isEnabled() && !isReadOnly()) QDateTimeEdit::stepUp();
}
void CompactDateTimeEdit::stepDown() {
    if (isEnabled() && !isReadOnly()) QDateTimeEdit::stepDown();
}
void CompactDateTimeEdit::resizeEvent(QResizeEvent* event) {
    QDateTimeEdit::resizeEvent(event);
    updateButtonPosition();
}
void CompactDateTimeEdit::paintEvent(QPaintEvent* event) {
    QDateTimeEdit::paintEvent(event);
    QPainter painter(this);
    drawBorderBottom(this, &painter);
}
void CompactDateTimeEdit::focusInEvent(QFocusEvent* event) {
    QDateTimeEdit::focusInEvent(event);
    CompactSpinBoxBase::focusInEvent(this, event);
}

}  // namespace qfw
