#include "components/widgets/progress_bar.h"

#include <QPaintEvent>
#include <QPainter>
#include <QtMath>

#include "common/color.h"
#include "common/style_sheet.h"

namespace qfw {

// ============================================================================
// ProgressBar
// ============================================================================

ProgressBar::ProgressBar(QWidget* parent, bool useAni) : QProgressBar(parent), _useAni(useAni) {
    setFixedHeight(5);
    lightBackgroundColor = QColor(0, 0, 0, 155);
    darkBackgroundColor = QColor(255, 255, 255, 155);

    ani = new QPropertyAnimation(this, "val", this);

    connect(this, &QProgressBar::valueChanged, this, &ProgressBar::_onValueChanged);
    QProgressBar::setValue(0);
}

float ProgressBar::getVal() const { return _val; }

void ProgressBar::setVal(float v) {
    _val = v;
    update();
}

bool ProgressBar::isUseAni() const { return _useAni; }

void ProgressBar::setUseAni(bool isUse) { _useAni = isUse; }

QColor ProgressBar::lightBarColor() const {
    return _lightBarColor.isValid() ? _lightBarColor : themeColor();
}

QColor ProgressBar::darkBarColor() const {
    return _darkBarColor.isValid()
               ? _darkBarColor
               : themedColor(themeColor(), true, QStringLiteral("ThemeColorLight1"));
}

void ProgressBar::setCustomBarColor(const QColor& light, const QColor& dark) {
    _lightBarColor = light;
    _darkBarColor = dark;
    update();
}

void ProgressBar::setCustomBackgroundColor(const QColor& light, const QColor& dark) {
    lightBackgroundColor = light;
    darkBackgroundColor = dark;
    update();
}

void ProgressBar::resume() {
    _isPaused = false;
    _isError = false;
    update();
}

void ProgressBar::pause() {
    _isPaused = true;
    update();
}

void ProgressBar::setPaused(bool isPaused) {
    _isPaused = isPaused;
    update();
}

bool ProgressBar::isPaused() const { return _isPaused; }

void ProgressBar::error() {
    _isError = true;
    update();
}

void ProgressBar::setError(bool isError) {
    _isError = isError;
    if (isError) {
        error();
    } else {
        resume();
    }
}

bool ProgressBar::isError() const { return _isError; }

QColor ProgressBar::barColor() const {
    if (isPaused()) {
        return isDarkTheme() ? QColor(252, 225, 0) : QColor(157, 93, 0);
    }
    if (isError()) {
        return isDarkTheme() ? QColor(255, 153, 164) : QColor(196, 43, 28);
    }
    return isDarkTheme() ? darkBarColor() : lightBarColor();
}

QString ProgressBar::valText() const {
    if (maximum() <= minimum()) {
        return QString();
    }

    int total = maximum() - minimum();
    QString res = format();
    QLocale loc = locale();
    loc.setNumberOptions(loc.numberOptions() | QLocale::OmitGroupSeparator);

    res.replace("%m", loc.toString(total));
    res.replace("%v", loc.toString(static_cast<int>(_val)));

    if (total == 0) {
        return res.replace("%p", loc.toString(100));
    }

    int progress = qRound((_val - minimum()) * 100.0 / total);
    return res.replace("%p", loc.toString(progress));
}

void ProgressBar::setValue(int value) {
    if (!_useAni) {
        _val = value;
        QProgressBar::setValue(value);
        update();
        return;
    }

    ani->stop();
    ani->setEndValue(static_cast<float>(value));
    ani->setDuration(150);
    ani->start();
    QProgressBar::setValue(value);
}

void ProgressBar::_onValueChanged(int value) {
    if (!_useAni) {
        _val = value;
        update();
    }
}

void ProgressBar::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // draw background
    QColor bc = isDarkTheme() ? darkBackgroundColor : lightBackgroundColor;
    painter.setPen(bc);
    int y = qFloor(height() / 2.0);
    painter.drawLine(0, y, width(), y);

    if (minimum() >= maximum()) {
        return;
    }

    // draw bar
    painter.setPen(Qt::NoPen);
    painter.setBrush(barColor());
    float w = (_val / (maximum() - minimum())) * width();
    float r = height() / 2.0;
    painter.drawRoundedRect(QRectF(0, 0, w, height()), r, r);
}

// ============================================================================
// IndeterminateProgressBar
// ============================================================================

IndeterminateProgressBar::IndeterminateProgressBar(QWidget* parent, bool startAni)
    : QProgressBar(parent) {
    setFixedHeight(5);

    shortBarAni = new QPropertyAnimation(this, "shortPos", this);
    longBarAni = new QPropertyAnimation(this, "longPos", this);
    aniGroup = new QParallelAnimationGroup(this);
    longBarAniGroup = new QSequentialAnimationGroup(this);

    shortBarAni->setDuration(833);
    longBarAni->setDuration(1167);
    shortBarAni->setStartValue(0.0f);
    longBarAni->setStartValue(0.0f);
    shortBarAni->setEndValue(1.45f);
    longBarAni->setEndValue(1.75f);
    longBarAni->setEasingCurve(QEasingCurve::OutQuad);

    aniGroup->addAnimation(shortBarAni);
    longBarAniGroup->addPause(785);
    longBarAniGroup->addAnimation(longBarAni);
    aniGroup->addAnimation(longBarAniGroup);
    aniGroup->setLoopCount(-1);

    if (startAni) {
        start();
    }
}

float IndeterminateProgressBar::shortPos() const { return _shortPos; }

void IndeterminateProgressBar::setShortPos(float p) {
    _shortPos = p;
    update();
}

float IndeterminateProgressBar::longPos() const { return _longPos; }

void IndeterminateProgressBar::setLongPos(float p) {
    _longPos = p;
    update();
}

QColor IndeterminateProgressBar::lightBarColor() const {
    return _lightBarColor.isValid() ? _lightBarColor : themeColor();
}

QColor IndeterminateProgressBar::darkBarColor() const {
    return _darkBarColor.isValid()
               ? _darkBarColor
               : themedColor(themeColor(), true, QStringLiteral("ThemeColorLight1"));
}

void IndeterminateProgressBar::setCustomBarColor(const QColor& light, const QColor& dark) {
    _lightBarColor = light;
    _darkBarColor = dark;
    update();
}

void IndeterminateProgressBar::start() {
    _isError = false;
    _shortPos = 0;
    _longPos = 0;
    aniGroup->start();
    update();
}

void IndeterminateProgressBar::stop() {
    aniGroup->stop();
    _shortPos = 0;
    _longPos = 0;
    update();
}

bool IndeterminateProgressBar::isStarted() const {
    return aniGroup->state() == QParallelAnimationGroup::Running;
}

void IndeterminateProgressBar::pause() {
    aniGroup->pause();
    update();
}

void IndeterminateProgressBar::resume() {
    aniGroup->resume();
    update();
}

void IndeterminateProgressBar::setPaused(bool isPaused) {
    if (isPaused) {
        pause();
    } else {
        resume();
    }
}

bool IndeterminateProgressBar::isPaused() const {
    return aniGroup->state() == QParallelAnimationGroup::Paused;
}

void IndeterminateProgressBar::error() {
    _isError = true;
    aniGroup->stop();
    update();
}

void IndeterminateProgressBar::setError(bool isError) {
    _isError = isError;
    if (isError) {
        error();
    } else {
        start();
    }
}

bool IndeterminateProgressBar::isError() const { return _isError; }

QColor IndeterminateProgressBar::barColor() const {
    if (isError()) {
        return isDarkTheme() ? QColor(255, 153, 164) : QColor(196, 43, 28);
    }
    if (isPaused()) {
        return isDarkTheme() ? QColor(252, 225, 0) : QColor(157, 93, 0);
    }
    return isDarkTheme() ? darkBarColor() : lightBarColor();
}

void IndeterminateProgressBar::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(barColor());

    float r = height() / 2.0;

    // draw short bar
    float sx = (_shortPos - 0.4f) * width();
    float sw = 0.4f * width();
    painter.drawRoundedRect(QRectF(sx, 0, sw, height()), r, r);

    // draw long bar
    float lx = (_longPos - 0.6f) * width();
    float lw = 0.6f * width();
    painter.drawRoundedRect(QRectF(lx, 0, lw, height()), r, r);
}

}  // namespace qfw
