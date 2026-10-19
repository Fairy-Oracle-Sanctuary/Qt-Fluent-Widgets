#include "components/widgets/round_progress_toast.h"

#include <QCloseEvent>
#include <QGraphicsDropShadowEffect>
#include <QPainter>

#include "common/config.h"
#include "common/style_sheet.h"
#include "components/widgets/progress_ring.h"

namespace qfw {
RoundProgressToast::RoundProgressToast(QWidget* parent)
    : RoundProgressToast(QString(), -1, ToastPosition::TopRight, parent) {}
RoundProgressToast::RoundProgressToast(const QString& content, int duration, ToastPosition position,
                                       QWidget* parent)
    : Toast({}, content.isNull() ? tr("Loading, please wait") : content, duration, false, position,
            Qt::Horizontal, themeColor(), parent) {
    setAttribute(Qt::WA_TranslucentBackground);
    progressRing = new IndeterminateProgressRing(this, false);
    progressRing->setFixedSize(20, 20);
    progressRing->setStrokeWidth(3);
    progressRing->setAttribute(Qt::WA_TransparentForMouseEvents);
    configureCompact(progressRing, QMargins(20, 14, 30, 14), QSize(180, 48), false);
    // The ring handles visibility pause/resume itself, preserving its phase.
    progressRing->start();
    if (auto* shadow = qobject_cast<QGraphicsDropShadowEffect*>(graphicsEffect())) {
        shadow->setBlurRadius(16);
        shadow->setOffset(0, 3);
        shadow->setColor(QColor(0, 0, 0, 32));
    }
    connect(&QConfig::instance(), &QConfig::themeChanged, progressRing,
            [this] { progressRing->update(); });
    connect(&QConfig::instance(), &QConfig::themeColorChanged, progressRing,
            [this] { progressRing->update(); });
}
QColor RoundProgressToast::defaultBackgroundColor() const {
    return QColor(isDarkTheme() ? "#272727" : "#F7F7F7");
}
void RoundProgressToast::closeEvent(QCloseEvent* event) {
    progressRing->stop();
    Toast::closeEvent(event);
}
void RoundProgressToast::changeEvent(QEvent* event) {
    Toast::changeEvent(event);
    if (event->type() == QEvent::EnabledChange && progressRing) {
        if (isEnabled())
            progressRing->start();
        else
            progressRing->stop();
    }
}
void RoundProgressToast::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(QColor(isDarkTheme() ? "#1D1D1D" : "#E5E5E5"), 1));
    painter.setBrush(backgroundColor());
    const auto shape = QRectF(rect()).adjusted(.5, .5, -.5, -.5);
    painter.drawRoundedRect(shape, shape.height() / 2, shape.height() / 2);
}
RoundProgressToast* RoundProgressToast::newToast(const QString& content, int duration,
                                                 ToastPosition position, QWidget* parent) {
    auto* result = new RoundProgressToast(content, duration, position, parent);
    result->show();
    return result;
}
RoundProgressToast* RoundProgressToast::info(const QString& content, int duration,
                                             ToastPosition position, QWidget* parent) {
    return newToast(content, duration, position, parent);
}
}  // namespace qfw
