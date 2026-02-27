#pragma once

#include <QColor>
#include <QParallelAnimationGroup>
#include <QProgressBar>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>

namespace qfw {

class ProgressBar : public QProgressBar {
    Q_OBJECT
    Q_PROPERTY(float val READ getVal WRITE setVal)
    Q_PROPERTY(bool useAni READ isUseAni WRITE setUseAni)

public:
    explicit ProgressBar(QWidget* parent = nullptr, bool useAni = true);

    float getVal() const;
    void setVal(float v);

    bool isUseAni() const;
    void setUseAni(bool isUse);

    QColor lightBarColor() const;
    QColor darkBarColor() const;
    void setCustomBarColor(const QColor& light, const QColor& dark);

    void setCustomBackgroundColor(const QColor& light, const QColor& dark);

    void resume();
    void pause();
    void setPaused(bool isPaused);
    bool isPaused() const;

    void error();
    void setError(bool isError);
    bool isError() const;

    QColor barColor() const;

    QString valText() const;

    void setValue(int value);

protected:
    void paintEvent(QPaintEvent* e) override;

    QColor lightBackgroundColor;
    QColor darkBackgroundColor;
    QColor _lightBarColor;
    QColor _darkBarColor;

    float _val = 0;
    bool _useAni = true;
    bool _isPaused = false;
    bool _isError = false;

private slots:
    void _onValueChanged(int value);

private:
    QPropertyAnimation* ani = nullptr;
};

class IndeterminateProgressBar : public QProgressBar {
    Q_OBJECT
    Q_PROPERTY(float shortPos READ shortPos WRITE setShortPos)
    Q_PROPERTY(float longPos READ longPos WRITE setLongPos)

public:
    explicit IndeterminateProgressBar(QWidget* parent = nullptr, bool start = true);

    float shortPos() const;
    void setShortPos(float p);

    float longPos() const;
    void setLongPos(float p);

    QColor lightBarColor() const;
    QColor darkBarColor() const;
    void setCustomBarColor(const QColor& light, const QColor& dark);

    void start();
    void stop();
    bool isStarted() const;

    void pause();
    void resume();
    void setPaused(bool isPaused);
    bool isPaused() const;

    void error();
    void setError(bool isError);
    bool isError() const;

    QColor barColor() const;

protected:
    void paintEvent(QPaintEvent* e) override;

private:
    float _shortPos = 0;
    float _longPos = 0;
    bool _isError = false;

    QColor _lightBarColor;
    QColor _darkBarColor;

    QPropertyAnimation* shortBarAni = nullptr;
    QPropertyAnimation* longBarAni = nullptr;
    QParallelAnimationGroup* aniGroup = nullptr;
    QSequentialAnimationGroup* longBarAniGroup = nullptr;
};

}  // namespace qfw
