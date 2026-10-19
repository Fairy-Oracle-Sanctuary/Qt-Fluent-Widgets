#include "components/widgets/progress_toast.h"

#include <QCloseEvent>
#include <QHideEvent>
#include <QPainterPath>
#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "common/config.h"
#include "common/style_sheet.h"

namespace qfw {
namespace {
InfoBarIconEnum checkedIcon(InfoBarIconEnum icon) {
    switch (icon) {
        case InfoBarIconEnum::Information:
        case InfoBarIconEnum::Success:
        case InfoBarIconEnum::Warning:
        case InfoBarIconEnum::Error:
            return icon;
    }
    throw std::invalid_argument("Invalid progress toast status");
}
QPair<QColor, QColor> checkedColors(const QColor& light, const QColor& dark) {
    if (!light.isValid() || !dark.isValid())
        throw std::invalid_argument("Invalid progress toast color");
    return {light, dark};
}
class ProgressToastIcon : public QWidget {
public:
    explicit ProgressToastIcon(ProgressToast* toast) : QWidget(toast), toast_(toast) {
        setFixedSize(16, 16);
        setAttribute(Qt::WA_TransparentForMouseEvents);
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        const auto color = toast_->barColor();
        const qreal brightness = color.red() * .2126 + color.green() * .7152 + color.blue() * .0722;
        const auto theme = brightness > 160 ? Theme::Dark : Theme::Light;
        // Only recolor the outer shape; retain the contrasting inner glyph.
        const auto svg =
            writeSvg(InfoBarIcon(toast_->icon()).path(theme), {0},
                     {{QStringLiteral("fill"), color.name()},
                      {QStringLiteral("fill-opacity"), QString::number(color.alphaF())}});
        drawSvgIcon(svg.toUtf8(), &painter, rect());
    }

private:
    ProgressToast* toast_;
};
}  // namespace
ProgressToast::ProgressToast(const QString& content, InfoBarIconEnum icon, int value, int duration,
                             bool isClosable, ToastPosition position, QWidget* parent)
    : Toast({}, content, duration, isClosable, position, Qt::Horizontal, QColor("#0078d4"), parent),
      icon_(checkedIcon(icon)),
      value_(std::clamp(value, 0, 100)),
      progress_(value_) {
    setAttribute(Qt::WA_TranslucentBackground);
    progressAnimation_ = new QPropertyAnimation(this, "progress", this);
    progressAnimation_->setDuration(150);
    iconWidget = new ProgressToastIcon(this);
    configureCompact(iconWidget, QMargins(12, 10, 12, 10), QSize(280, 50));
    connect(&QConfig::instance(), &QConfig::themeChanged, this, [this] { updateAppearance(); });
    connect(&QConfig::instance(), &QConfig::themeColorChanged, this,
            [this] { updateAppearance(); });
    updateAppearance();
}
InfoBarIconEnum ProgressToast::icon() const { return icon_; }
void ProgressToast::setIcon(InfoBarIconEnum icon) {
    icon_ = checkedIcon(icon);
    updateAppearance();
}
int ProgressToast::value() const { return value_; }
void ProgressToast::setValue(int value) {
    value = std::clamp(value, 0, 100);
    if (value == value_) return;
    value_ = value;
    progressAnimation_->stop();
    if (useAni_ && isVisible() && isEnabled()) {
        progressAnimation_->setStartValue(progress_);
        progressAnimation_->setEndValue(value);
        progressAnimation_->start();
    } else
        setProgress(value);
    emit valueChanged(value);
}
qreal ProgressToast::progress() const { return progress_; }
void ProgressToast::setProgress(qreal value) {
    if (!std::isfinite(value)) throw std::invalid_argument("Invalid progress toast progress");
    progress_ = std::clamp(value, qreal(0), qreal(100));
    update();
}
bool ProgressToast::isUseAni() const { return useAni_; }
void ProgressToast::setUseAni(bool enabled) {
    useAni_ = enabled;
    if (!enabled) settleProgress();
}
QColor ProgressToast::barColor() const {
    auto colors = stateColors_.value(icon_, customColors_);
    if (!colors.first.isValid()) {
        switch (icon_) {
            case InfoBarIconEnum::Success:
                colors = {QColor("#0F7B0F"), QColor("#6CCB5F")};
                break;
            case InfoBarIconEnum::Warning:
                colors = {QColor("#9D5D00"), QColor("#FCE100")};
                break;
            case InfoBarIconEnum::Error:
                colors = {QColor("#C42B1C"), QColor("#FF99A4")};
                break;
            case InfoBarIconEnum::Information:
                return themeColor();
        }
    }
    return isDarkTheme() ? colors.second : colors.first;
}
QColor ProgressToast::statusColor() const { return barColor(); }
void ProgressToast::setStateColor(InfoBarIconEnum icon, const QColor& color) {
    setStateColor(icon, color, color);
}
void ProgressToast::setStateColor(InfoBarIconEnum icon, const QColor& light, const QColor& dark) {
    checkedIcon(icon);
    const auto pair = checkedColors(light, dark);
    stateColors_.insert(icon, pair);
    updateAppearance();
}
void ProgressToast::setCustomBarColor(const QColor& color) { setCustomBarColor(color, color); }
void ProgressToast::setCustomBarColor(const QColor& light, const QColor& dark) {
    customColors_ = checkedColors(light, dark);
    updateAppearance();
}
void ProgressToast::updateAppearance() {
    setToastColor(barColor());
    iconWidget->update();
    update();
}
QColor ProgressToast::defaultBackgroundColor() const {
    return QColor(isDarkTheme() ? "#323232" : "#F7F7F7");
}
void ProgressToast::settleProgress() {
    progressAnimation_->stop();
    setProgress(value_);
}
void ProgressToast::hideEvent(QHideEvent* event) {
    settleProgress();
    Toast::hideEvent(event);
}
void ProgressToast::closeEvent(QCloseEvent* event) {
    progressAnimation_->stop();
    Toast::closeEvent(event);
}
void ProgressToast::changeEvent(QEvent* event) {
    Toast::changeEvent(event);
    if (event->type() == QEvent::EnabledChange && !isEnabled() && progressAnimation_)
        settleProgress();
}
void ProgressToast::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QPainterPath shape;
    shape.addRoundedRect(QRectF(rect()).adjusted(.5, .5, -.5, -.5), 8, 8);
    painter.setPen(QPen(QColor(isDarkTheme() ? "#484848" : "#E5E5E5"), 1));
    painter.setBrush(backgroundColor());
    painter.drawPath(shape);
    QPainterPath clip;
    clip.addRoundedRect(QRectF(rect()), 8, 8);
    painter.setClipPath(clip);
    // Like Python, the stripe always fills left-to-right, including RTL text.
    painter.fillRect(QRectF(0, height() - 3, width() * progress_ / 100, 3), barColor());
}
ProgressToast* ProgressToast::newToast(const QString& content, InfoBarIconEnum icon, int value,
                                       int duration, bool isClosable, ToastPosition position,
                                       QWidget* parent) {
    auto* result = new ProgressToast(content, icon, value, duration, isClosable, position, parent);
    result->show();
    return result;
}
ProgressToast* ProgressToast::info(const QString& content, int value, int duration, bool isClosable,
                                   ToastPosition position, QWidget* parent) {
    return newToast(content, InfoBarIconEnum::Information, value, duration, isClosable, position,
                    parent);
}
ProgressToast* ProgressToast::success(const QString& content, int value, int duration,
                                      bool isClosable, ToastPosition position, QWidget* parent) {
    return newToast(content, InfoBarIconEnum::Success, value, duration, isClosable, position,
                    parent);
}
ProgressToast* ProgressToast::warning(const QString& content, int value, int duration,
                                      bool isClosable, ToastPosition position, QWidget* parent) {
    return newToast(content, InfoBarIconEnum::Warning, value, duration, isClosable, position,
                    parent);
}
ProgressToast* ProgressToast::error(const QString& content, int value, int duration,
                                    bool isClosable, ToastPosition position, QWidget* parent) {
    return newToast(content, InfoBarIconEnum::Error, value, duration, isClosable, position, parent);
}
ProgressToast* ProgressToast::custom(const QString& content, const QColor& color, int value,
                                     InfoBarIconEnum icon, int duration, bool isClosable,
                                     ToastPosition position, QWidget* parent) {
    return custom(content, color, color, value, icon, duration, isClosable, position, parent);
}
ProgressToast* ProgressToast::custom(const QString& content, const QColor& light,
                                     const QColor& dark, int value, InfoBarIconEnum icon,
                                     int duration, bool isClosable, ToastPosition position,
                                     QWidget* parent) {
    checkedColors(light, dark);
    auto* result = new ProgressToast(content, icon, value, duration, isClosable, position, parent);
    result->setCustomBarColor(light, dark);
    result->show();
    return result;
}
}  // namespace qfw
