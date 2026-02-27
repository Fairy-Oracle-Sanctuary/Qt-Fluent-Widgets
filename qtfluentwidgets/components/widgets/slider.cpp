#include "components/widgets/slider.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionSlider>
#include <QtMath>

#include "common/color.h"
#include "common/style_sheet.h"

namespace qfw {

// ============================================================================
// SliderHandle
// ============================================================================

SliderHandle::SliderHandle(QSlider* parent) : QWidget(parent) {
    setFixedSize(22, 22);
    _radius = 5;
    radiusAni = new QPropertyAnimation(this, "radius", this);
    radiusAni->setDuration(100);
}

int SliderHandle::radius() const { return _radius; }

void SliderHandle::setRadius(int r) {
    _radius = r;
    update();
}

void SliderHandle::setHandleColor(const QColor& light, const QColor& dark) {
    lightHandleColor = light;
    darkHandleColor = dark;
    update();
}

void SliderHandle::enterEvent(QEnterEvent* e) {
    Q_UNUSED(e);
    startAni(6);
}

void SliderHandle::leaveEvent(QEvent* e) {
    Q_UNUSED(e);
    startAni(5);
}

void SliderHandle::mousePressEvent(QMouseEvent* e) {
    Q_UNUSED(e);
    startAni(4);
    emit pressed();
}

void SliderHandle::mouseReleaseEvent(QMouseEvent* e) {
    Q_UNUSED(e);
    startAni(6);
    emit released();
}

void SliderHandle::startAni(int radius) {
    radiusAni->stop();
    radiusAni->setStartValue(_radius);
    radiusAni->setEndValue(radius);
    radiusAni->start();
}

void SliderHandle::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    bool isDark = isDarkTheme();

    // draw outer circle
    painter.setPen(QColor(0, 0, 0, isDark ? 90 : 25));
    painter.setBrush(isDark ? QColor(69, 69, 69) : Qt::white);
    painter.drawEllipse(rect().adjusted(1, 1, -1, -1));

    // draw inner circle - use ThemeColorDark1 in dark mode
    painter.setPen(Qt::NoPen);
    QColor handleColor;
    if (isDark) {
        handleColor = themedColor(themeColor(), true, QStringLiteral("ThemeColorLight1"));
    } else {
        handleColor = autoFallbackThemeColor(lightHandleColor, darkHandleColor);
    }
    painter.setBrush(handleColor);
    painter.drawEllipse(QPoint(11, 11), _radius, _radius);
}

// ============================================================================
// Slider
// ============================================================================

Slider::Slider(QWidget* parent) : QSlider(parent) { postInit(); }

Slider::Slider(Qt::Orientation orientation, QWidget* parent) : QSlider(orientation, parent) {
    postInit();
}

void Slider::postInit() {
    handle = new SliderHandle(this);
    if (orientation() == Qt::Horizontal) {
        setMinimumHeight(handle->height());
    } else {
        setMinimumWidth(handle->width());
    }

    connect(handle, &SliderHandle::pressed, this, &Slider::sliderPressed);
    connect(handle, &SliderHandle::released, this, &Slider::sliderReleased);
    connect(this, &Slider::valueChanged, this, &Slider::adjustHandlePos);

    adjustHandlePos();
}

void Slider::setThemeColor(const QColor& light, const QColor& dark) {
    lightGrooveColor = light;
    darkGrooveColor = dark;
    handle->setHandleColor(light, dark);
    update();
}

void Slider::mousePressEvent(QMouseEvent* e) {
    _pressedPos = e->pos();
    setValue(posToValue(e->pos()));
    emit clicked(value());
}

void Slider::mouseMoveEvent(QMouseEvent* e) {
    setValue(posToValue(e->pos()));
    _pressedPos = e->pos();
    emit sliderMoved(value());
}

int Slider::grooveLength() const {
    int l = (orientation() == Qt::Horizontal) ? width() : height();
    return l - handle->width();
}

void Slider::adjustHandlePos() {
    int total = qMax(maximum() - minimum(), 1);
    int delta = static_cast<int>(static_cast<double>(value() - minimum()) / total * grooveLength());

    if (orientation() == Qt::Vertical) {
        int x = (width() - handle->width()) / 2;
        handle->move(x, delta);
    } else {
        int y = (height() - handle->height()) / 2;
        handle->move(delta, y);
    }
}

int Slider::posToValue(const QPoint& pos) {
    double pd = handle->width() / 2.0;
    int gs = qMax(grooveLength(), 1);
    int v = (orientation() == Qt::Horizontal) ? pos.x() : pos.y();
    return static_cast<int>((v - pd) / gs * (maximum() - minimum()) + minimum());
}

void Slider::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(isDarkTheme() ? QColor(255, 255, 255, 115) : QColor(0, 0, 0, 100));

    if (orientation() == Qt::Horizontal) {
        drawHorizonGroove(&painter);
        drawHorizonTick(&painter);
    } else {
        drawVerticalGroove(&painter);
        drawVerticalTick(&painter);
    }
}

void Slider::drawHorizonTick(QPainter* painter) { Q_UNUSED(painter); }
void Slider::drawVerticalTick(QPainter* painter) { Q_UNUSED(painter); }

void Slider::drawHorizonGroove(QPainter* painter) {
    int w = width();
    int r = handle->width() / 2;
    int cy = height() / 2;
    painter->drawRoundedRect(QRectF(r, cy - 3, w - r * 2, 5), 2, 2);

    if (maximum() - minimum() == 0) return;

    // Use ThemeColorDark1 in dark mode
    QColor grooveColor;
    if (isDarkTheme()) {
        grooveColor = themedColor(themeColor(), true, QStringLiteral("ThemeColorLight1"));
    } else {
        grooveColor = autoFallbackThemeColor(lightGrooveColor, darkGrooveColor);
    }
    painter->setBrush(grooveColor);
    double aw = static_cast<double>(value() - minimum()) / (maximum() - minimum()) * (w - r * 2);
    painter->drawRoundedRect(QRectF(r, cy - 3, aw, 5), 2, 2);
}

void Slider::drawVerticalGroove(QPainter* painter) {
    int h = height();
    int r = handle->width() / 2;
    int cx = width() / 2;
    painter->drawRoundedRect(QRectF(cx - 3, r, 6, h - 2 * r), 2, 2);

    if (maximum() - minimum() == 0) return;

    // Use ThemeColorDark1 in dark mode
    QColor grooveColor;
    if (isDarkTheme()) {
        grooveColor = themedColor(themeColor(), true, QStringLiteral("ThemeColorLight1"));
    } else {
        grooveColor = autoFallbackThemeColor(lightGrooveColor, darkGrooveColor);
    }
    painter->setBrush(grooveColor);
    double ah = static_cast<double>(value() - minimum()) / (maximum() - minimum()) * (h - r * 2);
    painter->drawRoundedRect(QRectF(cx - 3, r, 6, ah), 3, 3);
}

void Slider::resizeEvent(QResizeEvent* e) {
    QSlider::resizeEvent(e);
    adjustHandlePos();
}

// ============================================================================
// ClickableSlider
// ============================================================================

ClickableSlider::ClickableSlider(QWidget* parent) : QSlider(parent) {}
ClickableSlider::ClickableSlider(Qt::Orientation orientation, QWidget* parent)
    : QSlider(orientation, parent) {}

void ClickableSlider::mousePressEvent(QMouseEvent* e) {
    QSlider::mousePressEvent(e);
    int val;
    if (orientation() == Qt::Horizontal) {
        val = static_cast<int>(static_cast<double>(e->pos().x()) / width() * maximum());
    } else {
        val = static_cast<int>(static_cast<double>(height() - e->pos().y()) / height() * maximum());
    }
    setValue(val);
    emit clicked(value());
}

// ============================================================================
// HollowHandleStyle
// ============================================================================

HollowHandleStyle::HollowHandleStyle(const QMap<QString, QVariant>& config) : QProxyStyle() {
    this->config = {{"groove.height", 10},
                    {"sub-page.color", QColor(255, 255, 255)},
                    {"add-page.color", QColor(255, 255, 255, 64)},
                    {"handle.color", QColor(255, 255, 255)},
                    {"handle.ring-width", 4},
                    {"handle.hollow-radius", 6},
                    {"handle.margin", 4}};

    QMapIterator<QString, QVariant> i(config);
    while (i.hasNext()) {
        i.next();
        this->config[i.key()] = i.value();
    }

    int w = this->config["handle.margin"].toInt() + this->config["handle.ring-width"].toInt() +
            this->config["handle.hollow-radius"].toInt();
    this->config["handle.size"] = QSize(2 * w, 2 * w);
}

QRect HollowHandleStyle::subControlRect(ComplexControl cc, const QStyleOptionComplex* opt,
                                        SubControl sc, const QWidget* widget) const {
    const auto* sopt = qstyleoption_cast<const QStyleOptionSlider*>(opt);
    const QSlider* slider = qobject_cast<const QSlider*>(widget);
    if (cc != CC_Slider || !sopt || !slider || slider->orientation() != Qt::Horizontal ||
        sc == SC_SliderTickmarks) {
        return QProxyStyle::subControlRect(cc, opt, sc, widget);
    }

    QRect rect = widget->rect();

    if (sc == SC_SliderGroove) {
        int h = config["groove.height"].toInt();
        return QRect(0, (rect.height() - h) / 2, rect.width(), h);
    } else if (sc == SC_SliderHandle) {
        QSize size = config["handle.size"].toSize();
        int x = sliderPositionFromValue(sopt->minimum, sopt->maximum, sopt->sliderPosition,
                                        rect.width());
        x = static_cast<int>(x * static_cast<double>(rect.width() - size.width()) / rect.width());
        return QRect(x, 0, size.width(), size.height());
    }

    return QProxyStyle::subControlRect(cc, opt, sc, widget);
}

void HollowHandleStyle::drawComplexControl(ComplexControl cc, const QStyleOptionComplex* opt,
                                           QPainter* painter, const QWidget* widget) const {
    const auto* sopt = qstyleoption_cast<const QStyleOptionSlider*>(opt);
    const QSlider* slider = qobject_cast<const QSlider*>(widget);
    if (cc != CC_Slider || !sopt || !slider || slider->orientation() != Qt::Horizontal) {
        QProxyStyle::drawComplexControl(cc, opt, painter, widget);
        return;
    }

    QRect grooveRect = subControlRect(cc, opt, SC_SliderGroove, widget);
    QRect handleRect = subControlRect(cc, opt, SC_SliderHandle, widget);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);

    // paint groove
    int w = handleRect.x() - grooveRect.x();
    int h = config["groove.height"].toInt();

    painter->setBrush(config["sub-page.color"].value<QColor>());
    painter->drawRect(grooveRect.x(), grooveRect.y(), w, h);

    int x2 = w + config["handle.size"].toSize().width();
    painter->setBrush(config["add-page.color"].value<QColor>());
    painter->drawRect(x2, grooveRect.y(), grooveRect.width() - w, h);

    // paint handle
    int ringWidth = config["handle.ring-width"].toInt();
    int hollowRadius = config["handle.hollow-radius"].toInt();
    int radius = ringWidth + hollowRadius;

    QPainterPath path;
    QPoint center = handleRect.center() + QPoint(1, 1);
    path.addEllipse(center, radius, radius);
    path.addEllipse(center, hollowRadius, hollowRadius);
    path.setFillRule(Qt::OddEvenFill);

    QColor handleColor = config["handle.color"].value<QColor>();
    handleColor.setAlpha((sopt->activeSubControls & SC_SliderHandle) ? 153 : 255);
    painter->setBrush(handleColor);
    painter->drawPath(path);

    if (slider->isSliderDown()) {
        handleColor.setAlpha(255);
        painter->setBrush(handleColor);
        painter->drawEllipse(handleRect);
    }

    painter->restore();
}

}  // namespace qfw
