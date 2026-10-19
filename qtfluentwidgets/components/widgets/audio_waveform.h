#pragma once

#include <QColor>
#include <QLineF>
#include <QVector>
#include <QWidget>

namespace qfw {
// Normalized mono PCM visualization only; it does not decode or play audio.
// Call mutators on the GUI thread (use queued connections for producer threads).
class AudioWaveformWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qint64 position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(qreal barWidth READ barWidth WRITE setBarWidth)
    Q_PROPERTY(qreal barSpacing READ barSpacing WRITE setBarSpacing)
    Q_PROPERTY(qreal amplitudeScale READ amplitudeScale WRITE setAmplitudeScale)
    Q_PROPERTY(bool seekEnabled READ isSeekEnabled WRITE setSeekEnabled)
public:
    explicit AudioWaveformWidget(QWidget* parent = nullptr);
    void setSamples(const QVector<float>& samples, int sampleRate = 24000);
    void appendSamples(const QVector<float>& samples);
    void appendSamples(const QVector<float>& samples, int sampleRate);
    void clear();
    QVector<float> samples() const;
    int sampleCount() const;
    int sampleRate() const;
    qint64 duration() const;
    qint64 position() const;
    qreal barWidth() const;
    void setBarWidth(qreal width);
    qreal barSpacing() const;
    void setBarSpacing(qreal spacing);
    qreal amplitudeScale() const;
    void setAmplitudeScale(qreal scale);
    QColor waveformColor() const;
    void setWaveformColor(const QColor& light, const QColor& dark = QColor());
    QColor playedColor() const;
    void setPlayedColor(const QColor& light, const QColor& dark = QColor());
    bool isSeekEnabled() const;
    void setSeekEnabled(bool enabled);
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
public slots:
    void setPosition(qint64 milliseconds);
signals:
    void samplesChanged();
    void durationChanged(qint64 milliseconds);
    void positionChanged(qint64 milliseconds);
    void seekRequested(qint64 milliseconds);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    void cacheBlocks();
    QPair<float, float> extrema(int start, int end) const;
    void rebuildBars();
    void notifySamples(qint64 oldDuration);
    void seek(qint64 position, bool force = false);
    void seekAt(qreal x, bool force = false);
    QVector<float> samples_, blockMin_, blockMax_;
    QVector<QLineF> bars_;
    int sampleRate_ = 24000;
    qint64 position_ = 0;
    qreal barWidth_ = 1.5, barSpacing_ = 2, amplitudeScale_ = 1;
    bool seekEnabled_ = true, dragging_ = false;
    QColor lightWaveformColor_{"#D6D6D6"}, darkWaveformColor_{"#686868"};
    QColor lightPlayedColor_{"#606060"}, darkPlayedColor_{"#F2F2F2"};
};
}  // namespace qfw
