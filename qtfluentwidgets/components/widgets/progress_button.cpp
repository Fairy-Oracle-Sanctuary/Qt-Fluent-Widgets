#include "components/widgets/progress_button.h"

#include <QHideEvent>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QShowEvent>
#include <QStyle>
#include <QStyleOptionButton>
#include <algorithm>
#include <cmath>

#include "common/config.h"

namespace qfw {

ProgressPushButton::ProgressPushButton(QWidget* parent) : PrimaryPushButton(parent) {
    progressAnimation_ = new QPropertyAnimation(this, "displayValue", this);
    progressAnimation_->setDuration(150);
    connect(this, &QAbstractButton::clicked, this, [this] {
        if (progressing_) {
            emit stopRequested();
        } else if (autoProgress_) {
            setProgressing(true);
        }
    });
}
ProgressPushButton::ProgressPushButton(const QString& text, QWidget* parent)
    : ProgressPushButton(parent) {
    setText(text);
}
ProgressPushButton::ProgressPushButton(const QIcon& icon, const QString& text, QWidget* parent)
    : ProgressPushButton(text, parent) {
    setIcon(icon);
}
ProgressPushButton::ProgressPushButton(const FluentIconBase& icon, const QString& text,
                                       QWidget* parent)
    : ProgressPushButton(text, parent) {
    setIcon(icon);
}
bool ProgressPushButton::isProgressing() const { return progressing_; }
void ProgressPushButton::setProgressing(bool progressing) {
    if (progressing_ == progressing) {
        return;
    }
    progressing_ = progressing;
    update();
    emit progressChanged(progressing_);
}
bool ProgressPushButton::isAutoProgressEnabled() const { return autoProgress_; }
void ProgressPushButton::setAutoProgressEnabled(bool enabled) { autoProgress_ = enabled; }
int ProgressPushButton::value() const { return value_; }
void ProgressPushButton::setValue(int value) {
    value = std::clamp(value, 0, 100);
    if (value_ == value) {
        return;
    }
    value_ = value;
    progressAnimation_->stop();
    if (isVisible()) {
        progressAnimation_->setStartValue(displayValue_);
        progressAnimation_->setEndValue(qreal(value));
        progressAnimation_->start();
    } else {
        setDisplayValue(value);
    }
    emit valueChanged(value_);
}
qreal ProgressPushButton::displayValue() const { return displayValue_; }
void ProgressPushButton::setDisplayValue(qreal value) {
    displayValue_ = std::clamp(value, qreal(0), qreal(100));
    update();
}
void ProgressPushButton::hideEvent(QHideEvent* event) {
    progressAnimation_->stop();
    setDisplayValue(value_);
    PrimaryPushButton::hideEvent(event);
}
void ProgressPushButton::paintEvent(QPaintEvent* event) {
    if (!progressing_) {
        PrimaryPushButton::paintEvent(event);
        return;
    }
    QStyleOptionButton option;
    initStyleOption(&option);
    option.text.clear();
    option.icon = QIcon();
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    style()->drawControl(QStyle::CE_PushButtonBevel, &option, &painter, this);
    const QColor foreground = isDarkTheme() ? QColor(Qt::black) : QColor(Qt::white);
    if (!isEnabled()) {
        painter.setOpacity(0.43);
    } else if (isDown()) {
        painter.setOpacity(0.63);
    }
    const int diameter = std::min({21, std::max(0, height() - 8), std::max(0, width() - 8)});
    if (diameter > 3) {
        const QRectF ring((width() - diameter) / 2.0 + 1.5, (height() - diameter) / 2.0 + 1.5,
                          diameter - 3, diameter - 3);
        QColor track = foreground;
        track.setAlphaF(0.35);
        painter.setPen(QPen(track, 2, Qt::SolidLine, Qt::RoundCap));
        painter.drawEllipse(ring);
        painter.setPen(QPen(foreground, 2, Qt::SolidLine, Qt::RoundCap));
        painter.drawArc(ring, 90 * 16, -qRound(displayValue_ * 3.6 * 16));
    }
    const int stopSize = std::min({8, std::max(0, width() - 4), std::max(0, height() - 4)});
    painter.setPen(Qt::NoPen);
    painter.setBrush(foreground);
    painter.drawRoundedRect(
        QRectF((width() - stopSize) / 2.0, (height() - stopSize) / 2.0, stopSize, stopSize), 2, 2);
}

IndeterminateProgressPushButton::IndeterminateProgressPushButton(QWidget* parent)
    : PrimaryPushButton(parent) {
    auto* startGroup = new QSequentialAnimationGroup;
    auto* spanGroup = new QSequentialAnimationGroup;
    const auto addPhase = [this](QSequentialAnimationGroup* group, const char* property, int from,
                                 int to) {
        auto* animation = new QPropertyAnimation(this, property);
        animation->setDuration(1000);
        animation->setStartValue(from);
        animation->setEndValue(to);
        group->addAnimation(animation);
    };
    addPhase(startGroup, "startAngle", 0, 450);
    addPhase(startGroup, "startAngle", 450, 1080);
    addPhase(spanGroup, "spanAngle", 0, 180);
    addPhase(spanGroup, "spanAngle", 180, 0);
    animationGroup_ = new QParallelAnimationGroup(this);
    animationGroup_->addAnimation(startGroup);
    animationGroup_->addAnimation(spanGroup);
    animationGroup_->setLoopCount(-1);
    setMinimumWidth(150);
}
IndeterminateProgressPushButton::IndeterminateProgressPushButton(const QString& text,
                                                                 QWidget* parent)
    : IndeterminateProgressPushButton(parent) {
    setText(text);
}
IndeterminateProgressPushButton::IndeterminateProgressPushButton(const QIcon& icon,
                                                                 const QString& text,
                                                                 QWidget* parent)
    : IndeterminateProgressPushButton(text, parent) {
    setIcon(icon);
}
IndeterminateProgressPushButton::IndeterminateProgressPushButton(const FluentIconBase& icon,
                                                                 const QString& text,
                                                                 QWidget* parent)
    : IndeterminateProgressPushButton(text, parent) {
    setIcon(icon);
}
void IndeterminateProgressPushButton::start() {
    spinning_ = true;
    syncAnimation();
    update();
}
void IndeterminateProgressPushButton::stop() {
    spinning_ = false;
    animationGroup_->stop();
    startAngle_ = spanAngle_ = 0;
    update();
}
bool IndeterminateProgressPushButton::isSpinning() const { return spinning_; }
int IndeterminateProgressPushButton::startAngle() const { return startAngle_; }
void IndeterminateProgressPushButton::setStartAngle(int angle) {
    startAngle_ = angle;
    update();
}
int IndeterminateProgressPushButton::spanAngle() const { return spanAngle_; }
void IndeterminateProgressPushButton::setSpanAngle(int angle) {
    spanAngle_ = angle;
    update();
}
QSize IndeterminateProgressPushButton::sizeHint() const {
    QSize hint = PrimaryPushButton::sizeHint();
    // Reserve the loading arc even when idle, so start/stop never changes width.
    hint.setWidth(std::max({hint.width(), 150, fontMetrics().horizontalAdvance(text()) + 52}));
    return hint;
}
QSize IndeterminateProgressPushButton::minimumSizeHint() const { return sizeHint(); }
void IndeterminateProgressPushButton::syncAnimation() {
    if (!animationGroup_) {
        return;
    }
    if (spinning_ && isVisible() && isEnabled()) {
        if (animationGroup_->state() != QAbstractAnimation::Running) {
            animationGroup_->start();
        }
    } else {
        animationGroup_->stop();
    }
}
void IndeterminateProgressPushButton::showEvent(QShowEvent* event) {
    PrimaryPushButton::showEvent(event);
    syncAnimation();
}
void IndeterminateProgressPushButton::hideEvent(QHideEvent* event) {
    animationGroup_->stop();
    PrimaryPushButton::hideEvent(event);
}
void IndeterminateProgressPushButton::changeEvent(QEvent* event) {
    PrimaryPushButton::changeEvent(event);
    if (event->type() == QEvent::EnabledChange) {
        syncAnimation();
    }
}
void IndeterminateProgressPushButton::paintEvent(QPaintEvent* event) {
    if (!spinning_) {
        PrimaryPushButton::paintEvent(event);
        return;
    }
    QStyleOptionButton option;
    initStyleOption(&option);
    option.text.clear();
    option.icon = QIcon();
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    style()->drawControl(QStyle::CE_PushButtonBevel, &option, &painter, this);
    QColor color = isDarkTheme() ? QColor(Qt::black) : QColor(Qt::white);
    if (!isEnabled()) {
        color.setAlphaF(0.43);
    } else if (isDown()) {
        color.setAlphaF(0.63);
    }
    painter.setPen(QPen(color, 2, Qt::SolidLine, Qt::RoundCap));
    const QString label =
        fontMetrics().elidedText(text(), Qt::ElideRight, std::max(0, width() - 52));
    const int textWidth = fontMetrics().horizontalAdvance(label);
    const int x = std::max(12, (width() - 28 - textWidth) / 2);
    const QRect arcLogical(x + 2, (height() - 14) / 2, 14, 14);
    painter.drawArc(QStyle::visualRect(layoutDirection(), rect(), arcLogical),
                    ((-startAngle_ + 180) % 360) * 16, -spanAngle_ * 16);
    painter.setPen(color);
    painter.setFont(font());
    const QRect labelLogical(x + 28, 0, textWidth + 1, height());
    painter.drawText(QStyle::visualRect(layoutDirection(), rect(), labelLogical), Qt::AlignCenter,
                     label);
}

}  // namespace qfw
