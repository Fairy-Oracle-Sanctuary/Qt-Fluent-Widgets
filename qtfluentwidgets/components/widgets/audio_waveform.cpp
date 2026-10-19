#include "components/widgets/audio_waveform.h"

#include <QHideEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPointer>
#include <QResizeEvent>
#include <cmath>
#include <limits>
#include <stdexcept>

#include "common/config.h"
#include "common/style_sheet.h"

namespace qfw {
namespace {
constexpr int blockSize = 128;
qreal mouseX(const QMouseEvent* event) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return event->position().x();
#else
    return event->localPos().x();
#endif
}
void validateRate(int rate) {
    if (rate <= 0) throw std::invalid_argument("sampleRate must be positive");
}
QVector<float> copySamples(const QVector<float>& samples) {
    if (samples.size() > std::numeric_limits<int>::max())
        throw std::length_error("Too many audio samples");
    QVector<float> result;
    result.reserve(samples.size());
    for (float value : samples) {
        if (!std::isfinite(value)) throw std::invalid_argument("Audio samples must be finite");
        result.append(qBound(-1.0f, value, 1.0f));
    }
    return result;
}
void validatePositive(qreal value, bool allowZero = false) {
    if (!std::isfinite(value) || (allowZero ? value < 0 : value <= 0))
        throw std::invalid_argument("Waveform geometry must be finite and positive");
}
QColor darkColor(const QColor& light, const QColor& dark) {
    if (!light.isValid()) throw std::invalid_argument("Invalid waveform color");
    return dark.isValid() ? dark : light;
}
}  // namespace
AudioWaveformWidget::AudioWaveformWidget(QWidget* parent) : QWidget(parent) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::PointingHandCursor);
    setAccessibleName(tr("Audio waveform"));
    connect(&QConfig::instance(), &QConfig::themeChangedFinished, this,
            QOverload<>::of(&AudioWaveformWidget::update));
}
void AudioWaveformWidget::setSamples(const QVector<float>& samples, int rate) {
    validateRate(rate);
    auto data = copySamples(samples);
    const auto oldDuration = duration();
    samples_ = std::move(data);
    sampleRate_ = rate;
    blockMin_.clear();
    blockMax_.clear();
    dragging_ = false;
    cacheBlocks();
    rebuildBars();
    QPointer<AudioWaveformWidget> guard(this);
    setPosition(0);
    if (guard) notifySamples(oldDuration);
}
void AudioWaveformWidget::appendSamples(const QVector<float>& samples) {
    appendSamples(samples, sampleRate_);
}
void AudioWaveformWidget::appendSamples(const QVector<float>& samples, int rate) {
    validateRate(rate);
    if (!samples_.isEmpty() && rate != sampleRate_)
        throw std::invalid_argument("Cannot append a different sample rate; clear first");
    auto data = copySamples(samples);
    if (data.isEmpty()) return;
    if (data.size() > std::numeric_limits<int>::max() - samples_.size())
        throw std::length_error("Too many audio samples");
    const auto oldDuration = duration();
    samples_ += data;
    sampleRate_ = rate;
    cacheBlocks();
    rebuildBars();
    notifySamples(oldDuration);
}
void AudioWaveformWidget::clear() { setSamples({}, sampleRate_); }
QVector<float> AudioWaveformWidget::samples() const { return samples_; }
int AudioWaveformWidget::sampleCount() const { return int(samples_.size()); }
int AudioWaveformWidget::sampleRate() const { return sampleRate_; }
qint64 AudioWaveformWidget::duration() const {
    return (qint64(sampleCount()) * 1000 + sampleRate_ - 1) / sampleRate_;
}
qint64 AudioWaveformWidget::position() const { return position_; }
void AudioWaveformWidget::setPosition(qint64 milliseconds) {
    const auto value = qBound<qint64>(0, milliseconds, duration());
    if (position_ == value) return;
    position_ = value;
    update();
    emit positionChanged(value);
}
void AudioWaveformWidget::notifySamples(qint64 oldDuration) {
    setAccessibleDescription(tr("Duration: %1 ms").arg(duration()));
    update();
    QPointer<AudioWaveformWidget> guard(this);
    emit samplesChanged();
    if (guard && oldDuration != duration()) emit durationChanged(duration());
}
void AudioWaveformWidget::cacheBlocks() {
    const int start = int(blockMin_.size()) * blockSize;
    const int stop = sampleCount() / blockSize * blockSize;
    for (int offset = start; offset < stop; offset += blockSize) {
        float low = samples_[offset], high = low;
        for (int i = offset + 1; i < offset + blockSize; ++i) {
            low = qMin(low, samples_[i]);
            high = qMax(high, samples_[i]);
        }
        blockMin_.append(low);
        blockMax_.append(high);
    }
}
QPair<float, float> AudioWaveformWidget::extrema(int start, int end) const {
    float low = 0, high = 0;
    int i = start;
    while (i < end) {
        if (i % blockSize == 0 && end - i >= blockSize) {
            low = qMin(low, blockMin_[i / blockSize]);
            high = qMax(high, blockMax_[i / blockSize]);
            i += blockSize;
        } else {
            low = qMin(low, samples_[i]);
            high = qMax(high, samples_[i]);
            ++i;
        }
    }
    return {low, high};
}
void AudioWaveformWidget::rebuildBars() {
    bars_.clear();
    if (samples_.isEmpty()) return;
    const QRectF rect(contentsRect());
    const qreal width = rect.width() - barWidth_;
    const qreal height = qMax<qreal>(0, rect.height() - barWidth_);
    if (width < 0 || height <= 0) return;
    // Bound in floating point before converting, including extremely small strides.
    const int count = int(qMin<qreal>(
        sampleCount(), qMax<qreal>(1, std::floor(width / (barWidth_ + barSpacing_)) + 1)));
    const qreal step = count > 1 ? width / (count - 1) : 0;
    bars_.reserve(count);
    for (int i = 0; i < count; ++i) {
        const auto range = extrema(int(qint64(i) * sampleCount() / count),
                                   int(qint64(i + 1) * sampleCount() / count));
        const qreal low = qMax<qreal>(-1, range.first * amplitudeScale_);
        const qreal high = qMin<qreal>(1, range.second * amplitudeScale_);
        const qreal x = count > 1 ? rect.left() + barWidth_ / 2 + i * step : rect.center().x();
        bars_.append(QLineF(x, rect.center().y() - high * height / 2 - .005, x,
                            rect.center().y() - low * height / 2 + .005));
    }
}
qreal AudioWaveformWidget::barWidth() const { return barWidth_; }
void AudioWaveformWidget::setBarWidth(qreal width) {
    validatePositive(width);
    barWidth_ = width;
    rebuildBars();
    update();
}
qreal AudioWaveformWidget::barSpacing() const { return barSpacing_; }
void AudioWaveformWidget::setBarSpacing(qreal spacing) {
    validatePositive(spacing, true);
    barSpacing_ = spacing;
    rebuildBars();
    update();
}
qreal AudioWaveformWidget::amplitudeScale() const { return amplitudeScale_; }
void AudioWaveformWidget::setAmplitudeScale(qreal scale) {
    validatePositive(scale);
    amplitudeScale_ = scale;
    rebuildBars();
    update();
}
QColor AudioWaveformWidget::waveformColor() const {
    return isDarkTheme() ? darkWaveformColor_ : lightWaveformColor_;
}
void AudioWaveformWidget::setWaveformColor(const QColor& light, const QColor& dark) {
    const auto validDark = darkColor(light, dark);
    lightWaveformColor_ = light;
    darkWaveformColor_ = validDark;
    update();
}
QColor AudioWaveformWidget::playedColor() const {
    return isDarkTheme() ? darkPlayedColor_ : lightPlayedColor_;
}
void AudioWaveformWidget::setPlayedColor(const QColor& light, const QColor& dark) {
    const auto validDark = darkColor(light, dark);
    lightPlayedColor_ = light;
    darkPlayedColor_ = validDark;
    update();
}
bool AudioWaveformWidget::isSeekEnabled() const { return seekEnabled_; }
void AudioWaveformWidget::setSeekEnabled(bool enabled) {
    seekEnabled_ = enabled;
    dragging_ = false;
    setCursor(enabled ? Qt::PointingHandCursor : Qt::ArrowCursor);
    setFocusPolicy(enabled ? Qt::StrongFocus : Qt::NoFocus);
}
QSize AudioWaveformWidget::sizeHint() const { return {480, 160}; }
QSize AudioWaveformWidget::minimumSizeHint() const { return {32, 32}; }
void AudioWaveformWidget::seek(qint64 position, bool force) {
    const auto previous = position_;
    QPointer<AudioWaveformWidget> guard(this);
    setPosition(position);
    if (guard && (force || previous != position_)) emit seekRequested(position_);
}
void AudioWaveformWidget::seekAt(qreal x, bool force) {
    const auto rect = contentsRect();
    const auto fraction = qBound<qreal>(0, (x - rect.left()) / qMax(1, rect.width() - 1), 1);
    const auto raw = fraction * duration();
    auto rounded = qint64(std::floor(raw));
    const auto remainder = raw - rounded;
    // Python round(): ties go to the nearest even integer.
    if (remainder > .5 || (remainder == .5 && rounded % 2)) ++rounded;
    seek(rounded, force);
}
void AudioWaveformWidget::mousePressEvent(QMouseEvent* event) {
    if (isEnabled() && seekEnabled_ && !samples_.isEmpty() && event->button() == Qt::LeftButton) {
        dragging_ = true;
        setFocus(Qt::MouseFocusReason);
        event->accept();
        seekAt(mouseX(event), true);
    } else
        QWidget::mousePressEvent(event);
}
void AudioWaveformWidget::mouseMoveEvent(QMouseEvent* event) {
    if (dragging_ && event->buttons().testFlag(Qt::LeftButton)) {
        event->accept();
        seekAt(mouseX(event));
    } else
        QWidget::mouseMoveEvent(event);
}
void AudioWaveformWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (dragging_ && event->button() == Qt::LeftButton) {
        dragging_ = false;
        event->accept();
        seekAt(mouseX(event));
    } else
        QWidget::mouseReleaseEvent(event);
}
void AudioWaveformWidget::keyPressEvent(QKeyEvent* event) {
    if (isEnabled() && seekEnabled_ && !samples_.isEmpty()) {
        qint64 next;
        switch (event->key()) {
            case Qt::Key_Left:
                next = position_ - 1000;
                break;
            case Qt::Key_Right:
                next = position_ + 1000;
                break;
            case Qt::Key_Home:
                next = 0;
                break;
            case Qt::Key_End:
                next = duration();
                break;
            default:
                QWidget::keyPressEvent(event);
                return;
        }
        event->accept();
        seek(next, true);
        return;
    }
    QWidget::keyPressEvent(event);
}
void AudioWaveformWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    rebuildBars();
}
void AudioWaveformWidget::hideEvent(QHideEvent* event) {
    dragging_ = false;
    QWidget::hideEvent(event);
}
void AudioWaveformWidget::changeEvent(QEvent* event) {
    if (event->type() == QEvent::EnabledChange) {
        dragging_ = false;
        update();
    } else if (event->type() == QEvent::ContentsRectChange)
        rebuildBars();
    QWidget::changeEvent(event);
}
void AudioWaveformWidget::paintEvent(QPaintEvent*) {
    if (bars_.isEmpty()) return;
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    if (!isEnabled()) painter.setOpacity(.43);
    QPen pen(waveformColor(), barWidth_, Qt::SolidLine, Qt::RoundCap);
    const QRectF rect(contentsRect());
    const qreal fraction = duration() ? qreal(position_) / duration() : 0;
    painter.save();
    if (fraction > 0)
        painter.setClipRect(QRectF(rect.left() + rect.width() * fraction, rect.top(),
                                   rect.width() * (1 - fraction), rect.height()));
    painter.setPen(pen);
    painter.drawLines(bars_);
    painter.restore();
    if (fraction > 0) {
        painter.setClipRect(
            QRectF(rect.left(), rect.top(), rect.width() * fraction, rect.height()));
        pen.setColor(playedColor());
        painter.setPen(pen);
        painter.drawLines(bars_);
    }
}
}  // namespace qfw
