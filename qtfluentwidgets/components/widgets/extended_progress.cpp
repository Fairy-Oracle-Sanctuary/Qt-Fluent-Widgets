#include "components/widgets/extended_progress.h"

#include <QEvent>
#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QSignalBlocker>
#include <QStyleOptionFocusRect>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "common/color.h"
#include "common/font.h"
#include "common/style_sheet.h"
#include "components/widgets/label.h"

namespace qfw {
namespace {
void watchTheme(QWidget* widget) {
    auto& config = QConfig::instance();
    QObject::connect(&config, &QConfig::themeChanged, widget, [widget] { widget->update(); });
    QObject::connect(&config, &QConfig::themeColorChanged, widget, [widget] { widget->update(); });
}
qreal fraction(const ProgressBar* bar) {
    const qreal span = qreal(bar->maximum()) - bar->minimum();
    return span > 0 ? std::clamp((qreal(bar->getVal()) - bar->minimum()) / span, 0.0, 1.0) : 0;
}
QRectF ringRect(const QWidget* widget, qreal stroke) {
    const qreal side = std::max(0.0, qreal(std::min(widget->width(), widget->height())) - stroke);
    return QRectF((widget->width() - side) / 2, (widget->height() - side) / 2, side, side);
}
qreal ringStroke(const ProgressRing* ring) {
    return std::clamp(qreal(ring->strokeWidth()), 1.0,
                      qreal(std::max(1, std::min(ring->width(), ring->height()))));
}
void checkCount(int count) {
    if (count < 1 || count > 128)
        throw std::invalid_argument("step count must be between 1 and 128");
}
}  // namespace

FilledProgressBar::FilledProgressBar(QWidget* parent, bool useAni) : ProgressBar(parent, useAni) {
    setFixedSize(34, 210);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setCustomBarColor(QColor(33, 148, 243), QColor(33, 148, 243));
    setCustomBackgroundColor(QColor(0, 0, 0, 21), QColor(255, 255, 255, 30));
    qfw::setFont(this, 12);
    watchTheme(this);
}
FilledProgressBar::FilledProgressBar(const QIcon& icon, QWidget* parent, bool useAni)
    : FilledProgressBar(parent, useAni) {
    setIcon(icon);
}
FilledProgressBar::FilledProgressBar(const FluentIconBase& icon, QWidget* parent, bool useAni)
    : FilledProgressBar(parent, useAni) {
    setIcon(icon);
}
void FilledProgressBar::setIcon(const QIcon& icon) {
    fluentIcon_.reset();
    icon_ = icon;
    update();
}
void FilledProgressBar::setIcon(const FluentIconBase& icon) {
    fluentIcon_.reset(icon.clone());
    icon_ = icon.qicon();
    update();
}
QIcon FilledProgressBar::icon() const { return icon_; }
void FilledProgressBar::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    if (!isEnabled()) painter.setOpacity(0.43);
    const qreal top = isTextVisible() ? 26 : 0;
    const QRectF bar(0, top, width(), std::max(0.0, height() - top));
    if (isTextVisible()) {
        painter.setPen(isDarkTheme() ? Qt::white : Qt::black);
        QFont textFont = font();
        const QString text = valText();
        const int textWidth = fontMetrics().horizontalAdvance(text);
        if (textWidth > width() - 2 && textWidth > 0) {
            textFont.setPixelSize(
                std::max(8, int(fontMetrics().height() * qreal(width() - 2) / textWidth)));
        }
        painter.setFont(textFont);
        painter.drawText(QRect(0, 2, width(), 20), Qt::AlignCenter,
                         QFontMetrics(textFont).elidedText(text, Qt::ElideRight, width()));
    }
    if (bar.isEmpty()) return;
    QPainterPath clip;
    clip.addRoundedRect(bar, width() / 2.0, width() / 2.0);
    painter.setPen(Qt::NoPen);
    painter.fillPath(clip, isDarkTheme() ? darkBackgroundColor : lightBackgroundColor);
    painter.save();
    painter.setClipPath(clip);
    const qreal filled = fraction(this) * bar.height();
    painter.setBrush(barColor());
    if (filled > 0) {
        painter.drawRoundedRect(QRectF(0, bar.bottom() - filled, width(), filled), width() / 2.0,
                                width() / 2.0);
    }
    painter.restore();
    const int size = std::min({16, width(), int(bar.height()) - 8});
    if (size <= 0) return;
    const QRect iconRect((width() - size) / 2, int(bar.bottom()) - size - 8, size, size);
    if (fluentIcon_) {
        fluentIcon_->render(&painter, iconRect, Theme::Dark,
                            {{QStringLiteral("fill"), QStringLiteral("#ffffff")}});
    } else {
        icon_.paint(&painter, iconRect, Qt::AlignCenter,
                    isEnabled() ? QIcon::Normal : QIcon::Disabled);
    }
}

RadialGauge::RadialGauge(QWidget* parent, bool useAni) : ProgressRing(parent, useAni) {
    setFixedSize(100, 100);
    setTextVisible(true);
    qfw::setFont(this);
    watchTheme(this);
}
void RadialGauge::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    if (!isEnabled()) painter.setOpacity(0.43);
    const qreal stroke = ringStroke(this);
    const QRectF bounds = ringRect(this, stroke);
    QPen pen(isDarkTheme() ? darkBackgroundColor : lightBackgroundColor, stroke, Qt::SolidLine,
             Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);
    painter.drawArc(bounds, 225 * 16, -270 * 16);
    pen.setColor(barColor());
    painter.setPen(pen);
    const int sweep = qRound(fraction(this) * 270 * 16);
    if (sweep > 0) painter.drawArc(bounds, 225 * 16, -sweep);
    if (isTextVisible()) drawText(&painter, valText());
}

MultiSegmentProgressRing::MultiSegmentProgressRing(QWidget* parent, bool useAni)
    : ProgressRing(parent, useAni) {
    setFixedSize(100, 100);
    qfw::setFont(this);
    watchTheme(this);
}
QList<MultiSegmentProgressRing::Segment> MultiSegmentProgressRing::segments() const {
    return segments_;
}
void MultiSegmentProgressRing::setSegments(const QList<Segment>& segments) {
    for (const auto& segment : segments) {
        if (!std::isfinite(segment.first) || segment.first < 0 || !segment.second.isValid())
            throw std::invalid_argument(
                "segments require non-negative finite weights and valid colors");
    }
    segments_ = segments;
    update();
}
qreal MultiSegmentProgressRing::gapDegree() const { return gapDegree_; }
void MultiSegmentProgressRing::setGapDegree(qreal degrees) {
    if (!std::isfinite(degrees)) throw std::invalid_argument("gap must be finite");
    gapDegree_ = std::clamp(degrees, 0.0, 360.0);
    update();
}
QString MultiSegmentProgressRing::text() const { return text_; }
void MultiSegmentProgressRing::setText(const QString& text) {
    text_ = text;
    update();
}
void MultiSegmentProgressRing::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    if (!isEnabled()) painter.setOpacity(0.43);
    qreal largest = 0;
    int positiveCount = 0;
    for (const auto& segment : segments_) {
        largest = std::max(largest, segment.first);
        if (segment.first > 0) ++positiveCount;
    }
    if (largest > 0) {
        qreal total = 0;
        for (const auto& segment : segments_) total += segment.first / largest;
        const qreal stroke = ringStroke(this);
        const QRectF bounds = ringRect(this, stroke);
        const qreal radius = bounds.width() / 2;
        const qreal gap =
            positiveCount == 1
                ? 0
                : gapDegree_ + (radius > 0 ? stroke / radius * 180 / 3.14159265358979323846 : 360);
        qreal start = 90;
        for (const auto& segment : segments_) {
            const qreal slot = (segment.first / largest) / total * 360;
            const qreal sweep = std::max(0.0, slot - gap);
            if (sweep > 0) {
                painter.setPen(
                    QPen(segment.second, stroke, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                painter.drawArc(bounds, qRound(start * 16), -qRound(sweep * 16));
            }
            start -= slot;
        }
    }
    if (!text_.isEmpty()) drawText(&painter, text_);
}

StepProgressBarButton::StepProgressBarButton(const QIcon& icon, const QString& text,
                                             QWidget* parent)
    : QPushButton(text, parent),
      hoverAnimation_(new QPropertyAnimation(this, "hoverProgress", this)) {
    QPushButton::setIcon(icon);
    setCheckable(true);
    setFixedSize(34, 34);
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::StrongFocus);
    qfw::setFont(this);
    hoverAnimation_->setDuration(150);
    watchTheme(this);
}
StepProgressBarButton::StepProgressBarButton(const FluentIconBase& icon, const QString& text,
                                             QWidget* parent)
    : StepProgressBarButton(icon.qicon(true), text, parent) {}
void StepProgressBarButton::setCheckedLockEnabled(bool enabled) { locked_ = enabled; }
bool StepProgressBarButton::isCheckedLockEnabled() const { return locked_; }
void StepProgressBarButton::setNonInteractive(bool enabled) {
    nonInteractive_ = enabled;
    setCursor(enabled ? Qt::ArrowCursor : Qt::PointingHandCursor);
    setFocusPolicy(enabled ? Qt::NoFocus : Qt::StrongFocus);
    if (enabled) {
        hoverAnimation_->stop();
        setHoverProgress(0);
        setDown(false);
        clearFocus();
    }
}
bool StepProgressBarButton::isNonInteractive() const { return nonInteractive_; }
qreal StepProgressBarButton::hoverProgress() const { return hoverProgress_; }
void StepProgressBarButton::setHoverProgress(qreal progress) {
    if (!std::isfinite(progress)) return;
    hoverProgress_ = std::clamp(progress, 0.0, 1.0);
    update();
}
bool StepProgressBarButton::event(QEvent* event) {
    if (nonInteractive_ &&
        (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease ||
         event->type() == QEvent::MouseButtonDblClick || event->type() == QEvent::KeyPress ||
         event->type() == QEvent::KeyRelease))
        return true;
    if (event->type() == QEvent::Hide || (event->type() == QEvent::EnabledChange && !isEnabled())) {
        hoverAnimation_->stop();
        setHoverProgress(0);
    } else if (!nonInteractive_ && isEnabled() &&
               (event->type() == QEvent::Enter || event->type() == QEvent::Leave)) {
        hoverAnimation_->stop();
        hoverAnimation_->setStartValue(hoverProgress_);
        hoverAnimation_->setEndValue(event->type() == QEvent::Enter ? 1.0 : 0.0);
        hoverAnimation_->start();
    }
    return QPushButton::event(event);
}
void StepProgressBarButton::nextCheckState() {
    if (nonInteractive_) return;
    if (locked_ && isChecked()) {
        emit checkedClicked();
        return;
    }
    QPushButton::nextCheckState();
}
void StepProgressBarButton::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    if (!isEnabled()) painter.setOpacity(0.43);
    const QRectF bounds = QRectF(rect()).adjusted(1, 1, -1, -1);
    painter.setPen(Qt::NoPen);
    painter.setBrush(isChecked()     ? themeColor()
                     : isDarkTheme() ? QColor(255, 255, 255, 15)
                                     : QColor(255, 255, 255, 178));
    painter.drawEllipse(bounds);
    if (hoverProgress_ > 0) {
        painter.save();
        painter.setOpacity(painter.opacity() * hoverProgress_);
        QLinearGradient gradient(bounds.topLeft(), bounds.bottomRight());
        gradient.setColorAt(0, themeColor());
        gradient.setColorAt(1, themedColor(QConfig::instance().themeColor(), isDarkTheme(),
                                           isDarkTheme() ? "ThemeColorDark3" : "ThemeColorLight3"));
        painter.setBrush(gradient);
        painter.drawEllipse(bounds);
        painter.restore();
    }
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QColor(isDarkTheme() ? 255 : 0, isDarkTheme() ? 255 : 0, isDarkTheme() ? 255 : 0,
                          isDarkTheme() ? 13 : 19));
    painter.drawEllipse(bounds);
    if (isChecked() && !icon().isNull()) {
        icon().paint(&painter, QRect((width() - 16) / 2, (height() - 16) / 2, 16, 16),
                     Qt::AlignCenter, isEnabled() ? QIcon::Normal : QIcon::Disabled, QIcon::On);
    } else {
        painter.setPen((isChecked() || hoverProgress_ > 0)
                           ? (isDarkTheme() ? Qt::black : Qt::white)
                           : (isDarkTheme() ? Qt::white : Qt::black));
        painter.setFont(font());
        painter.drawText(rect(), Qt::AlignCenter, text());
    }
    if (hasFocus() && testAttribute(Qt::WA_KeyboardFocusChange)) {
        painter.setPen(QPen(isDarkTheme() ? Qt::white : Qt::black, 1, Qt::DotLine));
        painter.drawEllipse(bounds.adjusted(3, 3, -3, -3));
    }
}

StepProgressBar::StepProgressBar(QWidget* parent) : StepProgressBar(4, parent) {}
StepProgressBar::StepProgressBar(int count, QWidget* parent) : QWidget(parent), count_(count) {
    checkCount(count);
    hBoxLayout = new QHBoxLayout(this);
    hBoxLayout->setContentsMargins(0, 0, 0, 0);
    hBoxLayout->setSpacing(16);
    watchTheme(this);
    rebuild();
}
StepProgressBar::StepProgressBar(const FluentIconBase& icon, int count, QWidget* parent)
    : StepProgressBar(count, parent) {
    QList<QIcon> icons;
    for (int i = 0; i < count; ++i) icons.append(icon.qicon(true));
    setIcons(icons);
}
int StepProgressBar::current() const { return current_; }
int StepProgressBar::count() const { return count_; }
void StepProgressBar::setCurrent(int index) {
    if (index < 0 || index >= count_ || index == current_) return;
    current_ = index;
    updateStates();
    update();
    emit currentChanged(index);
}
void StepProgressBar::setCount(int count) {
    checkCount(count);
    if (count == count_) return;
    count_ = count;
    const int previous = current_;
    current_ = std::min(current_, count_ - 1);
    rebuild();
    if (current_ != previous) emit currentChanged(current_);
}
void StepProgressBar::rebuild() {
    buttons_.clear();
    labels_.clear();
    while (auto* item = hBoxLayout->takeAt(0)) {
        delete item->widget();
        delete item;
    }
    for (int i = 0; i < count_; ++i) {
        auto* column = new QWidget(this);
        auto* layout = new QVBoxLayout(column);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(8);
        auto* step = new StepProgressBarButton(icon(i), QString::number(i + 1), column);
        step->setCheckedLockEnabled(true);
        step->setNonInteractive(nonInteractive_);
        auto* label = new BodyLabel(stepName(i), column);
        label->setAlignment(Qt::AlignCenter);
        label->setWordWrap(true);
        step->setAccessibleName(stepName(i));
        layout->addWidget(step, 0, Qt::AlignHCenter);
        layout->addWidget(label);
        layout->addStretch();
        hBoxLayout->addWidget(column, 1);
        buttons_.append(step);
        labels_.append(label);
        connect(step, &QAbstractButton::clicked, this, [this, i] {
            if (!nonInteractive_) {
                if (i == current_)
                    updateStates();
                else
                    setCurrent(i);
            }
        });
    }
    updateStates();
    updateGeometry();
    update();
}
void StepProgressBar::updateStates() {
    for (int i = 0; i < buttons_.size(); ++i) {
        const QSignalBlocker blocker(buttons_[i]);
        buttons_[i]->setChecked(i <= current_);
    }
}
void StepProgressBar::setIcon(int index, const QIcon& icon) {
    if (index < 0 || index >= count_) return;
    while (icons_.size() <= index) icons_.append(QIcon());
    icons_[index] = icon;
    buttons_[index]->setIcon(icon);
}
void StepProgressBar::setIcon(int index, const FluentIconBase& icon) {
    setIcon(index, icon.qicon(true));
}
void StepProgressBar::setIcons(const QList<QIcon>& icons) {
    icons_ = icons;
    for (int i = 0; i < count_; ++i) buttons_[i]->setIcon(icon(i));
}
QIcon StepProgressBar::icon(int index) const {
    return index >= 0 && index < icons_.size() ? icons_[index] : QIcon();
}
void StepProgressBar::setStepName(int index, const QString& name) {
    if (index < 0 || index >= count_) return;
    while (names_.size() <= index) names_.append(QString::number(names_.size() + 1));
    names_[index] = name;
    labels_[index]->setText(name);
    buttons_[index]->setAccessibleName(name);
    updateGeometry();
}
void StepProgressBar::setStepNames(const QStringList& names) {
    names_ = names;
    for (int i = 0; i < count_; ++i) {
        labels_[i]->setText(stepName(i));
        buttons_[i]->setAccessibleName(stepName(i));
    }
    updateGeometry();
}
QString StepProgressBar::stepName(int index) const {
    if (index < 0 || index >= count_) return {};
    return index < names_.size() ? names_[index] : QString::number(index + 1);
}
StepProgressBarButton* StepProgressBar::button(int index) const {
    return index >= 0 && index < buttons_.size() ? buttons_[index] : nullptr;
}
void StepProgressBar::setNonInteractive(bool enabled) {
    nonInteractive_ = enabled;
    for (auto* step : buttons_) step->setNonInteractive(enabled);
}
bool StepProgressBar::isNonInteractive() const { return nonInteractive_; }
void StepProgressBar::nextStep() { setCurrent(current_ + 1); }
void StepProgressBar::previousStep() { setCurrent(current_ - 1); }
QSize StepProgressBar::sizeHint() const {
    return QSize(std::max(420, minimumSizeHint().width()), hBoxLayout->sizeHint().height());
}
QSize StepProgressBar::minimumSizeHint() const { return hBoxLayout->minimumSize(); }
void StepProgressBar::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    if (!isEnabled()) painter.setOpacity(0.43);
    painter.setPen(Qt::NoPen);
    for (int i = 0; i + 1 < buttons_.size(); ++i) {
        const QPoint first = buttons_[i]->mapTo(this, buttons_[i]->rect().center());
        const QPoint next = buttons_[i + 1]->mapTo(this, buttons_[i + 1]->rect().center());
        const int left = std::min(first.x(), next.x()) + 17;
        const int right = std::max(first.x(), next.x()) - 16;
        if (right <= left) continue;
        painter.setBrush(i < current_    ? themeColor()
                         : isDarkTheme() ? QColor(255, 255, 255, 17)
                                         : QColor(0, 0, 0, 125));
        painter.drawRect(QRectF(left, first.y() - 1, right - left, 3));
    }
}
}  // namespace qfw
