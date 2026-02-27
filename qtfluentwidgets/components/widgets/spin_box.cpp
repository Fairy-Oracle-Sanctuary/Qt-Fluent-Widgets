#include "spin_box.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyle>

#include "common/color.h"
#include "common/font.h"
#include "common/icon.h"
#include "common/style_sheet.h"

namespace qfw {

SpinIcon::SpinIcon(SpinIconType type) : type_(type) {}

QString SpinIcon::path(Theme theme) const {
    QString typeStr = (type_ == SpinIconType::UP) ? "Up" : "Down";
    return QString(":/qfluentwidgets/images/spin_box/%1_%2.svg")
        .arg(typeStr)
        .arg(getIconColor(theme));
}

FluentIconBase* SpinIcon::clone() const { return new SpinIcon(*this); }

void SpinIcon::render(SpinIconType type, QPainter* painter, const QRectF& rect, Theme theme) {
    SpinIcon icon(type);
    icon.FluentIconBase::render(painter, rect.toRect(), theme);
}

SpinButton::SpinButton(SpinIconType icon, QWidget* parent) : QToolButton(parent), iconType_(icon) {
    setFixedSize(31, 23);
    setIconSize(QSize(10, 10));
    setProperty("qssClass", "SpinButton");
    qfw::setStyleSheet(this, FluentStyleSheet::SpinBox);
}

void SpinButton::mousePressEvent(QMouseEvent* e) {
    isPressed_ = true;
    QToolButton::mousePressEvent(e);
}

void SpinButton::mouseReleaseEvent(QMouseEvent* e) {
    isPressed_ = false;
    QToolButton::mouseReleaseEvent(e);
}

void SpinButton::paintEvent(QPaintEvent* e) {
    QToolButton::paintEvent(e);
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    if (!isEnabled()) {
        painter.setOpacity(0.36);
    } else if (isPressed_) {
        painter.setOpacity(0.7);
    }

    SpinIcon::render(iconType_, &painter, QRectF(10, 6.5, 11, 11));
}

CompactSpinButton::CompactSpinButton(QWidget* parent) : QToolButton(parent) {
    setFixedSize(26, 33);
    setCursor(Qt::IBeamCursor);
}

void CompactSpinButton::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    double x = (width() - 10) / 2.0;
    double s = 9.0;

    SpinIcon::render(SpinIconType::UP, &painter, QRectF(x, height() / 2.0 - s + 1, s, s));
    SpinIcon::render(SpinIconType::DOWN, &painter, QRectF(x, height() / 2.0, s, s));
}

SpinFlyoutView::SpinFlyoutView(QWidget* parent) : FlyoutViewBase(parent) {
    upIcon_ = std::make_unique<SpinIcon>(SpinIconType::UP);
    downIcon_ = std::make_unique<SpinIcon>(SpinIconType::DOWN);

    upButton = new TransparentToolButton(*upIcon_, this);
    downButton = new TransparentToolButton(*downIcon_, this);
    vBoxLayout_ = new QVBoxLayout(this);

    upButton->setFixedSize(36, 36);
    downButton->setFixedSize(36, 36);
    upButton->setIconSize(QSize(13, 13));
    downButton->setIconSize(QSize(13, 13));

    vBoxLayout_->setContentsMargins(6, 6, 6, 6);
    vBoxLayout_->setSpacing(0);
    vBoxLayout_->addWidget(upButton);
    vBoxLayout_->addWidget(downButton);
}

void SpinFlyoutView::addWidget(QWidget* widget, int stretch, Qt::Alignment align) {
    vBoxLayout_->addWidget(widget, stretch, align);
}

void SpinFlyoutView::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    painter.setBrush(isDarkTheme() ? QColor(46, 46, 46) : QColor(249, 249, 249));
    painter.setPen(isDarkTheme() ? QColor(0, 0, 0, 51) : QColor(0, 0, 0, 15));

    QRect rect = this->rect().adjusted(1, 1, -1, -1);
    painter.drawRoundedRect(rect, 8, 8);
}

InlineSpinBoxBase::InlineSpinBoxBase(QWidget* widget) : widget_(widget) {
    upButton = new SpinButton(SpinIconType::UP, widget_);
    downButton = new SpinButton(SpinIconType::DOWN, widget_);

    upButton->setFocusProxy(widget_);
    downButton->setFocusProxy(widget_);

    QObject::connect(upButton, &QToolButton::clicked, [this]() { stepUp(); });
    QObject::connect(downButton, &QToolButton::clicked, [this]() { stepDown(); });
}

void InlineSpinBoxBase::updateButtonPosition() {
    const int w = widget_->width();
    const int h = widget_->height();

    const int btnW = 31;
    const int btnH = 23;
    const int rightMargin = 4;
    const int spacing = 5;
    const int y = 4;

    downButton->setGeometry(w - btnW - rightMargin, y, btnW, btnH);
    upButton->setGeometry(w - btnW * 2 - rightMargin - spacing, y, btnW, btnH);
}

void InlineSpinBoxBase::setSymbolVisible(bool isVisible) {
    widget_->setProperty("symbolVisible", isVisible);
    widget_->style()->unpolish(widget_);
    widget_->style()->polish(widget_);
    upButton->setVisible(isVisible);
    downButton->setVisible(isVisible);
}

void InlineSpinBoxBase::setAccelerated(bool on) {
    upButton->setAutoRepeat(on);
    downButton->setAutoRepeat(on);
}

bool InlineSpinBoxBase::isError() const { return isError_; }

void InlineSpinBoxBase::setError(bool isError) {
    if (isError == isError_) return;
    isError_ = isError;
    widget_->update();
}

void InlineSpinBoxBase::setReadOnly(bool isReadOnly) { setSymbolVisible(!isReadOnly); }

void InlineSpinBoxBase::setCustomFocusedBorderColor(const QColor& light, const QColor& dark) {
    lightFocusedBorderColor_ = light;
    darkFocusedBorderColor_ = dark;
    widget_->update();
}

QColor InlineSpinBoxBase::focusedBorderColor() const {
    if (isError_) {
        return getFluentSystemColor(FluentSystemColor::CriticalForeground);
    }
    return autoFallbackThemeColor(lightFocusedBorderColor_, darkFocusedBorderColor_);
}

void InlineSpinBoxBase::drawBorderBottom(QWidget* widget, QPainter* painter) {
    if (!widget->hasFocus()) return;

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);

    QPainterPath path;
    int w = widget->width();
    int h = widget->height();
    path.addRoundedRect(QRectF(0, h - 10, w, 10), 5, 5);

    QPainterPath rectPath;
    rectPath.addRect(0, h - 10, w, 8);
    path = path.subtracted(rectPath);

    painter->fillPath(path, focusedBorderColor());
    painter->restore();
}

CompactSpinBoxBase::CompactSpinBoxBase(QWidget* widget) : widget_(widget) {
    compactSpinButton = new CompactSpinButton(widget_);
    spinFlyoutView = new SpinFlyoutView(widget_);
    spinFlyout = new Flyout(spinFlyoutView, widget_, false);

    compactSpinButton->setFocusProxy(widget_);

    QObject::connect(compactSpinButton, &CompactSpinButton::clicked, [this]() { showFlyout(); });
    QObject::connect(spinFlyoutView->upButton, &QToolButton::clicked, [this]() { stepUp(); });
    QObject::connect(spinFlyoutView->downButton, &QToolButton::clicked, [this]() { stepDown(); });

    spinFlyout->hide();
}

void CompactSpinBoxBase::updateButtonPosition() {
    const int w = widget_->width();
    const int h = widget_->height();

    const int btnW = 26;
    const int btnH = 33;

    compactSpinButton->setGeometry(w - btnW, (h - btnH) / 2, btnW, btnH);
}

void CompactSpinBoxBase::setSymbolVisible(bool isVisible) {
    widget_->setProperty("symbolVisible", isVisible);
    widget_->style()->unpolish(widget_);
    widget_->style()->polish(widget_);
    compactSpinButton->setVisible(isVisible);
}

void CompactSpinBoxBase::setAccelerated(bool on) {
    spinFlyoutView->upButton->setAutoRepeat(on);
    spinFlyoutView->downButton->setAutoRepeat(on);
}

bool CompactSpinBoxBase::isError() const { return isError_; }

void CompactSpinBoxBase::setError(bool isError) {
    if (isError == isError_) return;
    isError_ = isError;
    widget_->update();
}

void CompactSpinBoxBase::setReadOnly(bool isReadOnly) { setSymbolVisible(!isReadOnly); }

void CompactSpinBoxBase::setCustomFocusedBorderColor(const QColor& light, const QColor& dark) {
    lightFocusedBorderColor_ = light;
    darkFocusedBorderColor_ = dark;
    widget_->update();
}

QColor CompactSpinBoxBase::focusedBorderColor() const {
    if (isError_) {
        return getFluentSystemColor(FluentSystemColor::CriticalForeground);
    }
    return autoFallbackThemeColor(lightFocusedBorderColor_, darkFocusedBorderColor_);
}

void CompactSpinBoxBase::drawBorderBottom(QWidget* widget, QPainter* painter) {
    if (!widget->hasFocus()) return;

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);

    QPainterPath path;
    int w = widget->width();
    int h = widget->height();
    path.addRoundedRect(QRectF(0, h - 10, w, 10), 5, 5);

    QPainterPath rectPath;
    rectPath.addRect(0, h - 10, w, 8);
    path = path.subtracted(rectPath);

    painter->fillPath(path, focusedBorderColor());
    painter->restore();
}

void CompactSpinBoxBase::focusInEvent(QWidget* widget, QFocusEvent* e) {
    Q_UNUSED(e);
    showFlyout();
}

void CompactSpinBoxBase::showFlyout() {
    if (spinFlyout->isVisible()) return;

    int y = compactSpinButton->height() / 2 - 46;
    QPoint pos = compactSpinButton->mapToGlobal(QPoint(-12, y));

    spinFlyout->exec(pos, FlyoutAnimationType::FadeIn);
}

// SpinBox implementation
SpinBox::SpinBox(QWidget* parent) : QSpinBox(parent), InlineSpinBoxBase(this) {
    setProperty("qssClass", "SpinBox");
    this->QSpinBox::setProperty("transparent", true);
    qfw::setStyleSheet(this, FluentStyleSheet::SpinBox);
    setFixedHeight(33);
    setMinimumWidth(130);
    qfw::setFont(this);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setButtonSymbols(QSpinBox::NoButtons);
    setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    updateButtonPosition();
}

void SpinBox::setReadOnly(bool isReadOnly) {
    QSpinBox::setReadOnly(isReadOnly);
    InlineSpinBoxBase::setReadOnly(isReadOnly);
}

void SpinBox::setSymbolVisible(bool isVisible) { InlineSpinBoxBase::setSymbolVisible(isVisible); }

void SpinBox::stepUp() { QSpinBox::stepUp(); }
void SpinBox::stepDown() { QSpinBox::stepDown(); }

void SpinBox::resizeEvent(QResizeEvent* e) {
    QSpinBox::resizeEvent(e);
    updateButtonPosition();
}

void SpinBox::paintEvent(QPaintEvent* e) {
    QSpinBox::paintEvent(e);
    QPainter painter(this);
    drawBorderBottom(this, &painter);
}

// CompactSpinBox implementation
CompactSpinBox::CompactSpinBox(QWidget* parent) : QSpinBox(parent), CompactSpinBoxBase(this) {
    setProperty("qssClass", "CompactSpinBox");
    this->QSpinBox::setProperty("transparent", true);
    qfw::setStyleSheet(this, FluentStyleSheet::SpinBox);
    setFixedHeight(33);
    setMinimumWidth(130);
    qfw::setFont(this);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setButtonSymbols(QSpinBox::NoButtons);
    setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    updateButtonPosition();
}

void CompactSpinBox::setReadOnly(bool isReadOnly) {
    QSpinBox::setReadOnly(isReadOnly);
    CompactSpinBoxBase::setReadOnly(isReadOnly);
}

void CompactSpinBox::setSymbolVisible(bool isVisible) {
    CompactSpinBoxBase::setSymbolVisible(isVisible);
}

void CompactSpinBox::focusInEvent(QFocusEvent* e) {
    QSpinBox::focusInEvent(e);
    CompactSpinBoxBase::focusInEvent(this, e);
}

void CompactSpinBox::stepUp() { QSpinBox::stepUp(); }
void CompactSpinBox::stepDown() { QSpinBox::stepDown(); }

void CompactSpinBox::resizeEvent(QResizeEvent* e) {
    QSpinBox::resizeEvent(e);
    updateButtonPosition();
}

void CompactSpinBox::paintEvent(QPaintEvent* e) {
    QSpinBox::paintEvent(e);
    QPainter painter(this);
    drawBorderBottom(this, &painter);
}

// DoubleSpinBox implementation
DoubleSpinBox::DoubleSpinBox(QWidget* parent) : QDoubleSpinBox(parent), InlineSpinBoxBase(this) {
    setProperty("qssClass", "DoubleSpinBox");
    this->QDoubleSpinBox::setProperty("transparent", true);
    qfw::setStyleSheet(this, FluentStyleSheet::SpinBox);
    setFixedHeight(33);
    setMinimumWidth(130);
    qfw::setFont(this);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setButtonSymbols(QSpinBox::NoButtons);
    setDecimals(2);
    setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    updateButtonPosition();
}

void DoubleSpinBox::setReadOnly(bool isReadOnly) {
    QDoubleSpinBox::setReadOnly(isReadOnly);
    InlineSpinBoxBase::setReadOnly(isReadOnly);
}

void DoubleSpinBox::setSymbolVisible(bool isVisible) {
    InlineSpinBoxBase::setSymbolVisible(isVisible);
}

void DoubleSpinBox::stepUp() { QDoubleSpinBox::stepUp(); }
void DoubleSpinBox::stepDown() { QDoubleSpinBox::stepDown(); }

void DoubleSpinBox::resizeEvent(QResizeEvent* e) {
    QDoubleSpinBox::resizeEvent(e);
    updateButtonPosition();
}

void DoubleSpinBox::paintEvent(QPaintEvent* e) {
    QDoubleSpinBox::paintEvent(e);
    QPainter painter(this);
    drawBorderBottom(this, &painter);
}

// CompactDoubleSpinBox implementation
CompactDoubleSpinBox::CompactDoubleSpinBox(QWidget* parent)
    : QDoubleSpinBox(parent), CompactSpinBoxBase(this) {
    setProperty("qssClass", "CompactDoubleSpinBox");
    this->QDoubleSpinBox::setProperty("transparent", true);
    qfw::setStyleSheet(this, FluentStyleSheet::SpinBox);
    setFixedHeight(33);
    setMinimumWidth(130);
    qfw::setFont(this);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setButtonSymbols(QSpinBox::NoButtons);
    setDecimals(2);
    setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    updateButtonPosition();
}

void CompactDoubleSpinBox::setReadOnly(bool isReadOnly) {
    QDoubleSpinBox::setReadOnly(isReadOnly);
    CompactSpinBoxBase::setReadOnly(isReadOnly);
}

void CompactDoubleSpinBox::setSymbolVisible(bool isVisible) {
    CompactSpinBoxBase::setSymbolVisible(isVisible);
}

void CompactDoubleSpinBox::focusInEvent(QFocusEvent* e) {
    QDoubleSpinBox::focusInEvent(e);
    CompactSpinBoxBase::focusInEvent(this, e);
}

void CompactDoubleSpinBox::stepUp() { QDoubleSpinBox::stepUp(); }
void CompactDoubleSpinBox::stepDown() { QDoubleSpinBox::stepDown(); }

void CompactDoubleSpinBox::resizeEvent(QResizeEvent* e) {
    QDoubleSpinBox::resizeEvent(e);
    updateButtonPosition();
}

void CompactDoubleSpinBox::paintEvent(QPaintEvent* e) {
    QDoubleSpinBox::paintEvent(e);
    QPainter painter(this);
    drawBorderBottom(this, &painter);
}

}  // namespace qfw
