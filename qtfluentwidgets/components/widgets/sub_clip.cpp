#include "components/widgets/sub_clip.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QStyle>
#include "common/config.h"
#include "common/style_sheet.h"

namespace qfw {
SubClipCloseButton::SubClipCloseButton(QWidget* parent) : QPushButton(parent) {
    setObjectName("clipCloseButton");
    setFixedSize(24, 24);
    setCursor(Qt::PointingHandCursor);
    setAccessibleName(SubClip::tr("Remove"));
    setToolTip(SubClip::tr("Remove"));
    connect(&QConfig::instance(), &QConfig::themeChanged, this, [this] { update(); });
}
void SubClipCloseButton::enterEvent(enterEvent_QEnterEvent* e) {
    hovered_ = true;
    update();
    QPushButton::enterEvent(e);
}
void SubClipCloseButton::leaveEvent(QEvent* e) {
    hovered_ = false;
    update();
    QPushButton::leaveEvent(e);
}
void SubClipCloseButton::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    const int value = isDarkTheme() ? 255 : 0;
    if (isDown()) p.setBrush(QColor(value, value, value, 11));
    else if (hovered_) p.setBrush(QColor(value, value, value, 20));
    else p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(QRectF(rect()), 5, 5);
    if (isDown()) p.setOpacity(.6);
    p.setPen(QPen(isDarkTheme() ? QColor(255, 255, 255, 200) : QColor(0, 0, 0, 180), 1.5));
    const QRect r(8, 8, 8, 8);
    p.drawLine(r.topLeft(), r.bottomRight());
    p.drawLine(r.topRight(), r.bottomLeft());
}
SubClip::SubClip(const QString& text, QWidget* parent) : QWidget(parent) {
    setObjectName("subClip");
    setProperty("qssClass", "SubClip");
    setProperty("isPressed", false);
    setAttribute(Qt::WA_StyledBackground);
    setFixedHeight(24);
    layout_ = new QHBoxLayout(this);
    layout_->setContentsMargins(8, 0, 0, 0);
    layout_->setSpacing(4);
    textLabel_ = new QLabel(text, this);
    textLabel_->setObjectName("clipTextLabel");
    closeButton_ = new SubClipCloseButton(this);
    layout_->addWidget(textLabel_);
    layout_->addWidget(closeButton_);
    connect(closeButton_, &QPushButton::clicked, this, [this] {
        const QString value = this->text();
        emit closed(value);
    });
    qfw::setStyleSheet(this, FluentStyleSheet::ComboBox);
    adjustSize();
}
QString SubClip::text() const { return textLabel_->text(); }
void SubClip::setText(const QString& text) {
    if (textLabel_->text() == text) return;
    textLabel_->setText(text);
    layout_->invalidate();
    updateGeometry();
}
QSize SubClip::sizeHint() const { return QSize(layout_->totalMinimumSize().width() + 16, 24); }
QSize SubClip::minimumSizeHint() const { return QSize(layout_->totalMinimumSize().width(), 24); }
void SubClip::setPressed(bool pressed) {
    if (property("isPressed").toBool() == pressed) return;
    setProperty("isPressed", pressed);
    style()->unpolish(this);
    style()->polish(this);
    update();
}
void SubClip::mousePressEvent(QMouseEvent* e) {
    setPressed(true);
    QWidget::mousePressEvent(e);
}
void SubClip::mouseReleaseEvent(QMouseEvent* e) {
    setPressed(false);
    QWidget::mouseReleaseEvent(e);
}
void SubClip::hideEvent(QHideEvent* e) { setPressed(false); QWidget::hideEvent(e); }
void SubClip::changeEvent(QEvent* e) {
    if (!isEnabled()) setPressed(false);
    if (e->type() == QEvent::LanguageChange) {
        closeButton_->setAccessibleName(tr("Remove"));
        closeButton_->setToolTip(tr("Remove"));
    }
    QWidget::changeEvent(e);
}
} // namespace qfw
