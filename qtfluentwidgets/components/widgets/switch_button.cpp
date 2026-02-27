#include "components/widgets/switch_button.h"

#include <QApplication>
#include <QPainter>
#include <QPainterPath>

#include "common/color.h"
#include "common/style_sheet.h"

namespace qfw {

// ============================================================================
// Indicator
// ============================================================================

Indicator::Indicator(QWidget* parent) : ToolButton(parent) {
    setCheckable(true);
    setFixedSize(42, 22);

    slideAni_ = new QPropertyAnimation(this, "sliderX", this);
    slideAni_->setDuration(120);

    connect(this, &QToolButton::toggled, this, &Indicator::toggleSlider);
}

void Indicator::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    drawBackground(&painter);
    drawCircle(&painter);
}

void Indicator::mouseReleaseEvent(QMouseEvent* event) {
    ToolButton::mouseReleaseEvent(event);
    emit checkedChanged(isChecked());
}

void Indicator::toggle() { setChecked(!isChecked()); }

void Indicator::setDown(bool isDown) {
    isPressed = isDown;
    ToolButton::setDown(isDown);
}

void Indicator::setHover(bool isHover) {
    isHover = isHover;
    update();
}

void Indicator::setCheckedColor(const QColor& light, const QColor& dark) {
    lightCheckedColor_ = light;
    darkCheckedColor_ = dark;
    update();
}

qreal Indicator::sliderX() const { return sliderX_; }

void Indicator::setSliderX(qreal x) {
    sliderX_ = qMax(x, 5.0);
    update();
}

void Indicator::toggleSlider() {
    slideAni_->stop();
    slideAni_->setStartValue(sliderX_);
    slideAni_->setEndValue(isChecked() ? 25 : 5);
    slideAni_->start();
}

void Indicator::drawBackground(QPainter* painter) {
    qreal r = height() / 2.0;
    painter->setPen(borderColor());
    painter->setBrush(backgroundColor());
    painter->drawRoundedRect(rect().adjusted(1, 1, -1, -1), r, r);
}

void Indicator::drawCircle(QPainter* painter) {
    painter->setPen(Qt::NoPen);
    painter->setBrush(sliderColor());
    painter->drawEllipse(QPointF(sliderX_ + 6, 11), 6, 6);
}

QColor Indicator::backgroundColor() const {
    bool isDark = isDarkTheme();

    if (isChecked()) {
        QColor color = isDark ? darkCheckedColor_ : lightCheckedColor_;
        if (!isEnabled()) {
            return isDark ? QColor(255, 255, 255, 41) : QColor(0, 0, 0, 56);
        }
        // In dark mode, checked state should always use hover color (ThemeColorDark1)
        if (isDark) {
            return validColor(color,
                              themedColor(themeColor(), isDark, QStringLiteral("ThemeColorLight1")));
        }
        if (isPressed) {
            return validColor(
                color, themedColor(themeColor(), isDark, QStringLiteral("ThemeColorLight2")));
        } else if (isHover) {
            return validColor(
                color, themedColor(themeColor(), isDark, QStringLiteral("ThemeColorLight1")));
        }
        return fallbackThemeColor(color);
    } else {
        if (!isEnabled()) {
            return QColor(0, 0, 0, 0);
        }
        if (isPressed) {
            return isDark ? QColor(255, 255, 255, 18) : QColor(0, 0, 0, 23);
        } else if (isHover) {
            return isDark ? QColor(255, 255, 255, 10) : QColor(0, 0, 0, 15);
        }
        return QColor(0, 0, 0, 0);
    }
}

QColor Indicator::borderColor() const {
    bool isDark = isDarkTheme();

    if (isChecked()) {
        return isEnabled() ? backgroundColor() : QColor(0, 0, 0, 0);
    } else {
        if (isEnabled()) {
            return isDark ? QColor(255, 255, 255, 153) : QColor(0, 0, 0, 133);
        }
        return isDark ? QColor(255, 255, 255, 41) : QColor(0, 0, 0, 56);
    }
}

QColor Indicator::sliderColor() const {
    bool isDark = isDarkTheme();

    if (isChecked()) {
        if (isEnabled()) {
            return isDark ? QColor(Qt::black) : QColor(Qt::white);
        }
        return isDark ? QColor(255, 255, 255, 77) : QColor(255, 255, 255);
    } else {
        if (isEnabled()) {
            return isDark ? QColor(255, 255, 255, 201) : QColor(0, 0, 0, 156);
        }
        return isDark ? QColor(255, 255, 255, 96) : QColor(0, 0, 0, 91);
    }
}

// ============================================================================
// SwitchButton
// ============================================================================

SwitchButton::SwitchButton(QWidget* parent, IndicatorPosition indicatorPos)
    : QWidget(parent),
      offText_(tr("Off")),
      onText_(tr("On")),
      indicatorPos_(indicatorPos),
      lightTextColor_(0, 0, 0),
      darkTextColor_(255, 255, 255) {
    text_ = offText_;
    hBox_ = new QHBoxLayout(this);
    indicator_ = new Indicator(this);
    label_ = new QLabel(text_, this);
    initWidget();
}

SwitchButton::SwitchButton(const QString& text, QWidget* parent, IndicatorPosition indicatorPos)
    : SwitchButton(parent, indicatorPos) {
    offText_ = text;
    setText(text);
}

void SwitchButton::initWidget() {
    setAttribute(Qt::WA_StyledBackground);
    installEventFilter(this);
    setFixedHeight(22);

    hBox_->setSpacing(spacing_);
    hBox_->setContentsMargins(2, 0, 0, 0);

    if (indicatorPos_ == IndicatorPosition::LEFT) {
        hBox_->addWidget(indicator_);
        hBox_->addWidget(label_);
        hBox_->setAlignment(Qt::AlignLeft);
    } else {
        hBox_->addWidget(label_, 0, Qt::AlignRight);
        hBox_->addWidget(indicator_, 0, Qt::AlignRight);
        hBox_->setAlignment(Qt::AlignRight);
    }

    qfw::setStyleSheet(this, FluentStyleSheet::SwitchButton);
    qfw::setStyleSheet(label_, FluentStyleSheet::SwitchButton);

    // Set default theme-aware text color and update on theme change
    setTextColor(lightTextColor_, darkTextColor_);
    connect(&QConfig::instance(), &QConfig::themeChanged, this,
            [this](qfw::Theme) { setTextColor(lightTextColor_, darkTextColor_); });

    connect(indicator_, &QToolButton::toggled, this, &SwitchButton::updateText);
    connect(indicator_, &QToolButton::toggled, this, &SwitchButton::checkedChanged);
}

bool SwitchButton::eventFilter(QObject* obj, QEvent* event) {
    if (obj == this && isEnabled()) {
        if (event->type() == QEvent::MouseButtonPress) {
            indicator_->setDown(true);
        } else if (event->type() == QEvent::MouseButtonRelease) {
            indicator_->setDown(false);
            indicator_->toggle();
        } else if (event->type() == QEvent::Enter) {
            indicator_->setHover(true);
        } else if (event->type() == QEvent::Leave) {
            indicator_->setHover(false);
        }
    }
    return QWidget::eventFilter(obj, event);
}

bool SwitchButton::isChecked() const { return indicator_->isChecked(); }

void SwitchButton::setChecked(bool isChecked) {
    updateText();
    indicator_->setChecked(isChecked);
}

void SwitchButton::toggleChecked() { indicator_->setChecked(!indicator_->isChecked()); }

void SwitchButton::setTextColor(const QColor& light, const QColor& dark) {
    lightTextColor_ = light;
    darkTextColor_ = dark;

    setCustomStyleSheet(
        label_, QString("SwitchButton>QLabel{color:%1}").arg(lightTextColor_.name(QColor::HexArgb)),
        QString("SwitchButton>QLabel{color:%1}").arg(darkTextColor_.name(QColor::HexArgb)));
}

void SwitchButton::setCheckedIndicatorColor(const QColor& light, const QColor& dark) {
    indicator_->setCheckedColor(light, dark);
}

QString SwitchButton::text() const { return text_; }

void SwitchButton::setText(const QString& text) {
    text_ = text;
    label_->setText(text);
    adjustSize();
}

QString SwitchButton::onText() const { return onText_; }

void SwitchButton::setOnText(const QString& text) {
    onText_ = text;
    updateText();
}

QString SwitchButton::offText() const { return offText_; }

void SwitchButton::setOffText(const QString& text) {
    offText_ = text;
    updateText();
}

int SwitchButton::spacing() const { return spacing_; }

void SwitchButton::setSpacing(int spacing) {
    spacing_ = spacing;
    hBox_->setSpacing(spacing);
    update();
}

void SwitchButton::updateText() {
    setText(isChecked() ? onText_ : offText_);
    adjustSize();
}

}  // namespace qfw
