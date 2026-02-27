#include "components/widgets/check_box.h"

#include <QEnterEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionButton>

#include "common/color.h"
#include "common/config.h"
#include "common/font.h"
#include "common/style_sheet.h"

namespace qfw {

CheckBox::CheckBox(QWidget* parent) : QCheckBox(parent) {
    setProperty("qssClass", "CheckBox");

    qfw::setFont(this);
    qfw::setStyleSheet(this, FluentStyleSheet::CheckBox);

    setMouseTracking(true);
}

CheckBox::CheckBox(const QString& text, QWidget* parent) : CheckBox(parent) { setText(text); }

void CheckBox::setCheckedColor(const QColor& light, const QColor& dark) {
    lightCheckedColor_ = light;
    darkCheckedColor_ = dark;
    update();
}

void CheckBox::setTextColor(const QColor& light, const QColor& dark) {
    lightTextColor_ = light;
    darkTextColor_ = dark;

    qfw::setCustomStyleSheet(
        this, QStringLiteral("CheckBox{color:%1}").arg(lightTextColor_.name(QColor::HexArgb)),
        QStringLiteral("CheckBox{color:%1}").arg(darkTextColor_.name(QColor::HexArgb)));
}

void CheckBox::mousePressEvent(QMouseEvent* e) {
    isPressed_ = true;
    QCheckBox::mousePressEvent(e);
}

void CheckBox::mouseReleaseEvent(QMouseEvent* e) {
    isPressed_ = false;
    QCheckBox::mouseReleaseEvent(e);
}

void CheckBox::enterEvent(QEnterEvent* e) {
    isHover_ = true;
    update();
    QCheckBox::enterEvent(e);
}

void CheckBox::leaveEvent(QEvent* e) {
    isHover_ = false;
    update();
    QCheckBox::leaveEvent(e);
}

CheckBox::State CheckBox::state() const {
    if (!isEnabled()) {
        return isChecked() ? State::CheckedDisabled : State::Disabled;
    }

    if (isChecked()) {
        if (isPressed_) {
            return State::CheckedPressed;
        }
        if (isHover_) {
            return State::CheckedHover;
        }
        return State::Checked;
    }

    if (isPressed_) {
        return State::Pressed;
    }
    if (isHover_) {
        return State::Hover;
    }

    return State::Normal;
}

QColor CheckBox::borderColor() const {
    const bool dark = isDarkTheme();
    const QColor base = themeColor();

    const QColor light1 = qfw::themedColor(base, false, QStringLiteral("ThemeColorLight1"));
    const QColor light2 = qfw::themedColor(base, false, QStringLiteral("ThemeColorLight2"));
    const QColor dark1 = qfw::themedColor(base, true, QStringLiteral("ThemeColorDark1"));
    const QColor dark2 = qfw::themedColor(base, true, QStringLiteral("ThemeColorDark2"));
    const QColor darkLight1 = qfw::themedColor(base, true, QStringLiteral("ThemeColorLight1"));

    if (dark) {
        switch (state()) {
            case State::Normal:
            case State::Hover:
                return QColor(255, 255, 255, 141);
            case State::Pressed:
                return QColor(255, 255, 255, 40);
            case State::Checked:
                return validColor(darkCheckedColor_, darkLight1);
            case State::CheckedHover:
                return validColor(darkCheckedColor_, dark1);
            case State::CheckedPressed:
                return validColor(darkCheckedColor_, dark2);
            case State::Disabled:
                return QColor(255, 255, 255, 41);
            case State::CheckedDisabled:
                return QColor(0, 0, 0, 0);
        }
    } else {
        switch (state()) {
            case State::Normal:
                return QColor(0, 0, 0, 122);
            case State::Hover:
                return QColor(0, 0, 0, 143);
            case State::Pressed:
                return QColor(0, 0, 0, 69);
            case State::Checked:
                return fallbackThemeColor(lightCheckedColor_);
            case State::CheckedHover:
                return validColor(lightCheckedColor_, light1);
            case State::CheckedPressed:
                return validColor(lightCheckedColor_, light2);
            case State::Disabled:
                return QColor(0, 0, 0, 56);
            case State::CheckedDisabled:
                return QColor(0, 0, 0, 0);
        }
    }

    return QColor();
}

QColor CheckBox::backgroundColor() const {
    const bool dark = isDarkTheme();
    const QColor base = themeColor();

    const QColor light1 = qfw::themedColor(base, false, QStringLiteral("ThemeColorLight1"));
    const QColor light2 = qfw::themedColor(base, false, QStringLiteral("ThemeColorLight2"));
    const QColor dark1 = qfw::themedColor(base, true, QStringLiteral("ThemeColorDark1"));
    const QColor dark2 = qfw::themedColor(base, true, QStringLiteral("ThemeColorDark2"));
    const QColor darkLight1 = qfw::themedColor(base, true, QStringLiteral("ThemeColorLight1"));

    if (dark) {
        switch (state()) {
            case State::Normal:
                return QColor(0, 0, 0, 26);
            case State::Hover:
                return QColor(255, 255, 255, 11);
            case State::Pressed:
                return QColor(255, 255, 255, 18);
            case State::Checked:
                return validColor(darkCheckedColor_, darkLight1);
            case State::CheckedHover:
                return validColor(darkCheckedColor_, dark1);
            case State::CheckedPressed:
                return validColor(darkCheckedColor_, dark2);
            case State::Disabled:
                return QColor(0, 0, 0, 0);
            case State::CheckedDisabled:
                return QColor(255, 255, 255, 41);
        }
    } else {
        switch (state()) {
            case State::Normal:
                return QColor(0, 0, 0, 6);
            case State::Hover:
                return QColor(0, 0, 0, 13);
            case State::Pressed:
                return QColor(0, 0, 0, 31);
            case State::Checked:
                return fallbackThemeColor(lightCheckedColor_);
            case State::CheckedHover:
                return validColor(lightCheckedColor_, light1);
            case State::CheckedPressed:
                return validColor(lightCheckedColor_, light2);
            case State::Disabled:
                return QColor(0, 0, 0, 0);
            case State::CheckedDisabled:
                return QColor(0, 0, 0, 56);
        }
    }

    return QColor();
}

CheckBoxIcon CheckBox::stateIcon(Qt::CheckState state) {
    if (state == Qt::Checked) {
        return CheckBoxIcon(CheckBoxIcon::Accept);
    }

    if (state == Qt::PartiallyChecked) {
        return CheckBoxIcon(CheckBoxIcon::PartialAccept);
    }

    return CheckBoxIcon(CheckBoxIcon::Accept);
}

void CheckBox::paintEvent(QPaintEvent* e) {
    QCheckBox::paintEvent(e);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QStyleOptionButton opt;
    opt.initFrom(this);

    const QRect indicatorRect = style()->subElementRect(QStyle::SE_CheckBoxIndicator, &opt, this);

    painter.setPen(borderColor());
    painter.setBrush(backgroundColor());
    painter.drawRoundedRect(indicatorRect, 4.5, 4.5);

    if (!isEnabled()) {
        painter.setOpacity(0.8);
    }

    if (checkState() == Qt::Checked || checkState() == Qt::PartiallyChecked) {
        CheckBoxIcon icon = stateIcon(checkState());
        icon.render(&painter, indicatorRect);
    }
}

}  // namespace qfw
