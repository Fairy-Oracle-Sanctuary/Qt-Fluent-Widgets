#include "subtitle_button.h"

#include <QFontMetrics>
#include <QPainter>
#include <QStyle>
#include <algorithm>
#include <stdexcept>

#include "common/style_sheet.h"

namespace qfw {
namespace {
constexpr int textOffset = 29;
constexpr int trailingMargin = 10;
QFont subtitleFont(const QWidget* widget) {
    QFont font = widget->font();
    font.setPixelSize(12);
    return font;
}
int titleHeight(const QWidget* widget) { return widget->fontMetrics().height() + 4; }
bool isCheckbox(const QWidget* widget) { return qobject_cast<const QCheckBox*>(widget); }
int defaultMargin(const QWidget* widget) { return isCheckbox(widget) ? 2 : 6; }
int subtitleY(const QWidget* widget, int margin) {
    return titleHeight(widget) + (isCheckbox(widget) ? 3 : -5) + margin - defaultMargin(widget);
}
int contentHeight(const QWidget* widget, const QString& subtitle, int margin, int width) {
    if (subtitle.isEmpty()) return titleHeight(widget);
    const int available = std::max(1, width - textOffset - trailingMargin);
    const QFontMetrics metrics(subtitleFont(widget));
    const int extraLines = std::max(
        0,
        metrics.boundingRect(QRect(0, 0, available, 100000), Qt::TextWordWrap, subtitle).height() -
            metrics.height());
    return titleHeight(widget) * 2 + margin - defaultMargin(widget) + extraLines;
}
QSize contentSize(const QAbstractButton* button, const QString& subtitle, int margin) {
    const int width =
        std::clamp(textOffset + trailingMargin +
                       std::max(button->fontMetrics().horizontalAdvance(button->text()),
                                QFontMetrics(subtitleFont(button)).horizontalAdvance(subtitle)),
                   70, 480);
    return QSize(width, contentHeight(button, subtitle, margin, width));
}
void initSubtitleButton(QAbstractButton* button) {
    QSizePolicy policy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    policy.setHeightForWidth(true);
    button->setSizePolicy(policy);
    button->setAttribute(Qt::WA_MacShowFocusRect, false);
}
void drawSubtitleText(QPainter* painter, QAbstractButton* button, const QString& subtitle,
                      int margin, QColor color) {
    if (!button->isEnabled()) painter->setOpacity(0.36);
    const bool rtl = button->layoutDirection() == Qt::RightToLeft;
    const int available = std::max(0, button->width() - textOffset - trailingMargin);
    const int x = rtl ? trailingMargin : textOffset;
    const int flags = Qt::AlignVCenter | Qt::AlignAbsolute |
                      (rtl ? Qt::AlignRight : Qt::AlignLeft) |
                      (button->style()->styleHint(QStyle::SH_UnderlineShortcut, nullptr, button)
                           ? Qt::TextShowMnemonic
                           : Qt::TextHideMnemonic);
    painter->setFont(button->font());
    painter->setPen(color);
    const int rowHeight = titleHeight(button);
    // Python centers the checkbox title/indicator in its two-line base height;
    // radio buttons use the first half. Wrapping must not shift the title.
    const int titleRectHeight =
        isCheckbox(button) && !subtitle.isEmpty() ? rowHeight * 2 : rowHeight;
    painter->drawText(QRect(x, 0, available, titleRectHeight), flags, button->text());
    if (subtitle.isEmpty()) return;
    painter->setFont(subtitleFont(button));
    color.setAlpha(128);
    painter->setPen(color);
    const int y = subtitleY(button, margin);
    const QFontMetrics metrics(subtitleFont(button));
    const int wrappedHeight =
        metrics
            .boundingRect(QRect(0, 0, std::max(1, available), 100000), Qt::TextWordWrap, subtitle)
            .height();
    painter->drawText(
        QRect(x, y, available, rowHeight + std::max(0, wrappedHeight - metrics.height())),
        Qt::AlignVCenter | Qt::AlignAbsolute | Qt::TextWordWrap |
            (rtl ? Qt::AlignRight : Qt::AlignLeft),
        subtitle);
}
void validateMargin(int margin) {
    if (margin < 0 || margin > 64)
        throw std::invalid_argument("Subtitle margin must be between 0 and 64");
}
}  // namespace

SubtitleCheckBox::SubtitleCheckBox(QWidget* parent) : CheckBox(parent) { initSubtitleButton(this); }
SubtitleCheckBox::SubtitleCheckBox(const QString& text, const QString& subText, QWidget* parent)
    : SubtitleCheckBox(parent) {
    setText(text);
    setSubText(subText);
}
QString SubtitleCheckBox::subText() const { return subText_; }
void SubtitleCheckBox::setSubText(const QString& text) {
    if (subText_ == text) return;
    subText_ = text;
    setAccessibleDescription(text);
    updateGeometry();
    update();
    emit subTextChanged(text);
}
void SubtitleCheckBox::setText(const QString& text) {
    QCheckBox::setText(text);
    updateGeometry();
    update();
}
int SubtitleCheckBox::margin() const { return margin_; }
void SubtitleCheckBox::setMargin(int margin) {
    validateMargin(margin);
    margin_ = margin;
    updateGeometry();
    update();
}
QSize SubtitleCheckBox::sizeHint() const { return contentSize(this, subText_, margin_); }
QSize SubtitleCheckBox::minimumSizeHint() const { return QSize(70, titleHeight(this)); }
int SubtitleCheckBox::heightForWidth(int width) const {
    return contentHeight(this, subText_, margin_, width);
}
bool SubtitleCheckBox::hitButton(const QPoint& position) const {
    return contentsRect().contains(position);
}
void SubtitleCheckBox::changeEvent(QEvent* event) {
    CheckBox::changeEvent(event);
    if (event->type() == QEvent::FontChange || event->type() == QEvent::StyleChange)
        updateGeometry();
}
void SubtitleCheckBox::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    const int x = layoutDirection() == Qt::RightToLeft ? width() - 21 : 1;
    const int titleRectHeight = titleHeight(this) * (subText_.isEmpty() ? 1 : 2);
    drawIndicator(&painter, QRect(x, (titleRectHeight - 20) / 2, 20, 20));
    drawSubtitleText(&painter, this, subText_, margin_, textColor());
}

SubtitleRadioButton::SubtitleRadioButton(QWidget* parent) : RadioButton(parent) {
    // Do not inherit the plain radio button's fixed 24px QSS height.
    setProperty("qssClass", "SubtitleRadioButton");
    initSubtitleButton(this);
}
SubtitleRadioButton::SubtitleRadioButton(const QString& text, const QString& subText,
                                         QWidget* parent)
    : SubtitleRadioButton(parent) {
    setText(text);
    setSubText(subText);
}
QString SubtitleRadioButton::subText() const { return subText_; }
void SubtitleRadioButton::setSubText(const QString& text) {
    if (subText_ == text) return;
    subText_ = text;
    setAccessibleDescription(text);
    updateGeometry();
    update();
    emit subTextChanged(text);
}
void SubtitleRadioButton::setText(const QString& text) {
    QRadioButton::setText(text);
    updateGeometry();
    update();
}
int SubtitleRadioButton::margin() const { return margin_; }
void SubtitleRadioButton::setMargin(int margin) {
    validateMargin(margin);
    margin_ = margin;
    updateGeometry();
    update();
}
QSize SubtitleRadioButton::sizeHint() const { return contentSize(this, subText_, margin_); }
QSize SubtitleRadioButton::minimumSizeHint() const { return QSize(70, titleHeight(this)); }
int SubtitleRadioButton::heightForWidth(int width) const {
    return contentHeight(this, subText_, margin_, width);
}
bool SubtitleRadioButton::hitButton(const QPoint& position) const {
    return contentsRect().contains(position);
}
void SubtitleRadioButton::changeEvent(QEvent* event) {
    RadioButton::changeEvent(event);
    if (event->type() == QEvent::FontChange || event->type() == QEvent::StyleChange)
        updateGeometry();
}
void SubtitleRadioButton::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter.save();
    const int x = layoutDirection() == Qt::RightToLeft ? width() - 11 : 11;
    painter.translate(x - 11, titleHeight(this) / 2 - 12);
    drawIndicator(&painter);
    painter.restore();
    drawSubtitleText(&painter, this, subText_, margin_,
                     isDarkTheme() ? darkTextColor() : lightTextColor());
}
}  // namespace qfw
