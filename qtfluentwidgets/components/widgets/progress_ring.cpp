#include "components/widgets/progress_ring.h"

#include <QPaintEvent>
#include <QPainter>

#include "common/color.h"
#include "common/font.h"
#include "common/style_sheet.h"

namespace qfw {

// ============================================================================
// ProgressRing
// ============================================================================

ProgressRing::ProgressRing(QWidget* parent, bool useAni) : ProgressBar(parent, useAni) {
    lightBackgroundColor = QColor(0, 0, 0, 34);
    darkBackgroundColor = QColor(255, 255, 255, 34);
    _strokeWidth = 6;

    setTextVisible(false);
    setFixedSize(80, 80);
}

int ProgressRing::strokeWidth() const { return _strokeWidth; }

void ProgressRing::setStrokeWidth(int w) {
    _strokeWidth = w;
    update();
}

void ProgressRing::drawText(QPainter* painter, const QString& text) {
    painter->setFont(font());
    painter->setPen(isDarkTheme() ? Qt::white : Qt::black);
    painter->drawText(rect(), Qt::AlignCenter, text);
}

void ProgressRing::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    float cw = static_cast<float>(_strokeWidth);
    float w = qMin(height(), width()) - cw;
    QRectF rc(cw / 2.0f, height() / 2.0f - w / 2.0f, w, w);

    // draw background
    QColor bc = isDarkTheme() ? darkBackgroundColor : lightBackgroundColor;
    QPen pen(bc, cw, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);
    painter.drawArc(rc, 0, 360 * 16);

    if (maximum() <= minimum()) {
        return;
    }

    // draw bar
    pen.setColor(barColor());
    painter.setPen(pen);
    int degree = qRound((getVal() / (maximum() - minimum())) * 360.0f);
    painter.drawArc(rc, 90 * 16, -degree * 16);

    // draw text
    if (isTextVisible()) {
        drawText(&painter, valText());
    }
}

// ============================================================================
// IndeterminateProgressRing
// ============================================================================

IndeterminateProgressRing::IndeterminateProgressRing(QWidget* parent, bool startAni)
    : QProgressBar(parent) {
    lightBackgroundColor = QColor(0, 0, 0, 0);
    darkBackgroundColor = QColor(255, 255, 255, 0);
    _strokeWidth = 6;

    _startAngle = -180;
    _spanAngle = 0;

    startAngleAni1 = new QPropertyAnimation(this, "startAngle", this);
    startAngleAni2 = new QPropertyAnimation(this, "startAngle", this);
    spanAngleAni1 = new QPropertyAnimation(this, "spanAngle", this);
    spanAngleAni2 = new QPropertyAnimation(this, "spanAngle", this);

    startAngleAniGroup = new QSequentialAnimationGroup(this);
    spanAngleAniGroup = new QSequentialAnimationGroup(this);
    aniGroup = new QParallelAnimationGroup(this);

    // initialize start angle animation
    startAngleAni1->setDuration(1000);
    startAngleAni1->setStartValue(0);
    startAngleAni1->setEndValue(450);

    startAngleAni2->setDuration(1000);
    startAngleAni2->setStartValue(450);
    startAngleAni2->setEndValue(1080);

    startAngleAniGroup->addAnimation(startAngleAni1);
    startAngleAniGroup->addAnimation(startAngleAni2);

    // initialize span angle animation
    spanAngleAni1->setDuration(1000);
    spanAngleAni1->setStartValue(0);
    spanAngleAni1->setEndValue(180);

    spanAngleAni2->setDuration(1000);
    spanAngleAni2->setStartValue(180);
    spanAngleAni2->setEndValue(0);

    spanAngleAniGroup->addAnimation(spanAngleAni1);
    spanAngleAniGroup->addAnimation(spanAngleAni2);

    aniGroup->addAnimation(startAngleAniGroup);
    aniGroup->addAnimation(spanAngleAniGroup);
    aniGroup->setLoopCount(-1);

    setFixedSize(80, 80);

    if (startAni) {
        start();
    }
}

int IndeterminateProgressRing::startAngle() const { return _startAngle; }

void IndeterminateProgressRing::setStartAngle(int angle) {
    _startAngle = angle;
    update();
}

int IndeterminateProgressRing::spanAngle() const { return _spanAngle; }

void IndeterminateProgressRing::setSpanAngle(int angle) {
    _spanAngle = angle;
    update();
}

int IndeterminateProgressRing::strokeWidth() const { return _strokeWidth; }

void IndeterminateProgressRing::setStrokeWidth(int w) {
    _strokeWidth = w;
    update();
}

void IndeterminateProgressRing::start() {
    _startAngle = 0;
    _spanAngle = 0;
    aniGroup->start();
}

void IndeterminateProgressRing::stop() {
    aniGroup->stop();
    _startAngle = 0;
    _spanAngle = 0;
    update();
}

QColor IndeterminateProgressRing::lightBarColor() const {
    return _lightBarColor.isValid() ? _lightBarColor : themeColor();
}

QColor IndeterminateProgressRing::darkBarColor() const {
    return _darkBarColor.isValid()
               ? _darkBarColor
               : themedColor(themeColor(), true, QStringLiteral("ThemeColorLight1"));
}

void IndeterminateProgressRing::setCustomBarColor(const QColor& light, const QColor& dark) {
    _lightBarColor = light;
    _darkBarColor = dark;
    update();
}

void IndeterminateProgressRing::setCustomBackgroundColor(const QColor& light, const QColor& dark) {
    lightBackgroundColor = light;
    darkBackgroundColor = dark;
    update();
}

void IndeterminateProgressRing::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    float cw = static_cast<float>(_strokeWidth);
    float w = qMin(height(), width()) - cw;
    QRectF rc(cw / 2.0f, height() / 2.0f - w / 2.0f, w, w);

    // draw background
    QColor bc = isDarkTheme() ? darkBackgroundColor : lightBackgroundColor;
    QPen pen(bc, cw, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);
    painter.drawArc(rc, 0, 360 * 16);

    // draw bar
    pen.setColor(isDarkTheme() ? darkBarColor() : lightBarColor());
    painter.setPen(pen);

    int startAngle = -_startAngle + 180;
    painter.drawArc(rc, (startAngle % 360) * 16, -_spanAngle * 16);
}

}  // namespace qfw
