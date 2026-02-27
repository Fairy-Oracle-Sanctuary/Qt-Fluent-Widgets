#include "components/widgets/button.h"

#include <QApplication>
#include <QDesktopServices>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QMenu>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QSizePolicy>
#include <QStyle>
#include <QStyleOptionButton>
#include <QUrl>
#include <algorithm>
#include <climits>

#include "common/animation.h"
#include "common/color.h"
#include "common/font.h"
#include "common/icon.h"
#include "common/style_sheet.h"
#include "components/widgets/menu.h"

namespace qfw {

static bool execRoundMenuAtButton(QMenu* menu, QWidget* anchor, int anchorWidth, int yDown,
                                  int yUp) {
    auto* roundMenu = qobject_cast<qfw::RoundMenu*>(menu);
    if (!roundMenu || !anchor) {
        return false;
    }

    if (roundMenu->isVisible()) {
        roundMenu->hide();
    }

    if (roundMenu->view()) {
        roundMenu->view()->setMinimumWidth(anchorWidth);
        roundMenu->view()->adjustSizeForMenu();
    }
    roundMenu->adjustSize();

    const int leftMargin =
        (roundMenu->layout() ? roundMenu->layout()->contentsMargins().left() : 0);

    const int x = -roundMenu->width() / 2 + leftMargin + anchorWidth / 2;
    const QPoint pd = anchor->mapToGlobal(QPoint(x, yDown));
    const QPoint pu = anchor->mapToGlobal(QPoint(x, yUp));

    const int hd = roundMenu->view()
                       ? roundMenu->view()->heightForAnimation(pd, MenuAnimationType::DropDown)
                       : INT_MAX;
    const int hu = roundMenu->view()
                       ? roundMenu->view()->heightForAnimation(pu, MenuAnimationType::PullUp)
                       : INT_MAX;

    if (hd >= hu) {
        roundMenu->execAt(pd, true, MenuAnimationType::DropDown);
    } else {
        roundMenu->execAt(pu, true, MenuAnimationType::PullUp);
    }
    return true;
}

FluentPushButton::FluentPushButton(QWidget* parent) : QPushButton(parent) { init(); }

FluentPushButton::~FluentPushButton() {
    if (ownedIcon_) {
        delete ownedIcon_;
        ownedIcon_ = nullptr;
    }
}

PushButton::PushButton(QWidget* parent) : FluentPushButton(parent) {}

PushButton::PushButton(const QString& text, QWidget* parent) : FluentPushButton(text, parent) {}

PushButton::PushButton(const QIcon& icon, const QString& text, QWidget* parent)
    : FluentPushButton(icon, text, parent) {}

PushButton::PushButton(const FluentIconBase& icon, const QString& text, QWidget* parent)
    : FluentPushButton(icon, text, parent) {}

PillPushButton::PillPushButton(QWidget* parent) : ToggleButton(parent) {
    setProperty("qssClass", "PillPushButton");
}

PillPushButton::PillPushButton(const QString& text, QWidget* parent) : ToggleButton(text, parent) {
    setProperty("qssClass", "PillPushButton");
}

PillPushButton::PillPushButton(const QIcon& icon, const QString& text, QWidget* parent)
    : ToggleButton(icon, text, parent) {
    setProperty("qssClass", "PillPushButton");
}

PillPushButton::PillPushButton(const FluentIconBase& icon, const QString& text, QWidget* parent)
    : ToggleButton(icon, text, parent) {
    setProperty("qssClass", "PillPushButton");
}

FluentPushButton::FluentPushButton(const QString& text, QWidget* parent)
    : QPushButton(text, parent) {
    init();
}

FluentPushButton::FluentPushButton(const QIcon& icon, const QString& text, QWidget* parent)
    : QPushButton(text, parent) {
    init();
    setIcon(icon);
}

FluentPushButton::FluentPushButton(const FluentIconBase& icon, const QString& text, QWidget* parent)
    : QPushButton(text, parent) {
    init();
    setIcon(icon);
}

void FluentPushButton::init() {
    qfw::setStyleSheet(this, qfw::FluentStyleSheet::Button);
    isPressed = false;
    isHover = false;

    setProperty("qssClass", "PushButton");

    setIconSize(QSize(16, 16));
    setIcon(QIcon());

    qfw::setFont(this);
}

void FluentPushButton::setIcon(const QIcon& icon) {
    if (ownedIcon_) {
        delete ownedIcon_;
        ownedIcon_ = nullptr;
    }
    setProperty("hasIcon", !icon.isNull());
    setStyle(QApplication::style());
    QPushButton::setIcon(icon);
    icon_ = icon;
    update();
}

void FluentPushButton::setIcon(const FluentIconBase& icon) {
    if (ownedIcon_) {
        delete ownedIcon_;
    }
    ownedIcon_ = icon.clone();
    setProperty("hasIcon", true);
    setStyle(QApplication::style());
    QPushButton::setIcon(icon.qicon());
    icon_ = QVariant::fromValue(static_cast<const FluentIconBase*>(ownedIcon_));
    update();
}

void FluentPushButton::setIcon(const QVariant& icon) {
    if (icon.canConvert<QIcon>()) {
        setIcon(icon.value<QIcon>());
    } else if (icon.canConvert<const FluentIconBase*>()) {
        auto fluentIcon = icon.value<const FluentIconBase*>();
        if (fluentIcon) {
            setIcon(*fluentIcon);
        }
    }
}

QSize FluentPushButton::sizeHint() const { return QPushButton::sizeHint(); }

QSize FluentPushButton::minimumSizeHint() const { return sizeHint(); }

QIcon FluentPushButton::icon() const {
    if (icon_.canConvert<QIcon>()) {
        return icon_.value<QIcon>();
    } else if (icon_.canConvert<const FluentIconBase*>()) {
        auto fluentIcon = icon_.value<const FluentIconBase*>();
        if (fluentIcon) {
            return fluentIcon->icon();
        }
    }
    return QIcon();
}

void FluentPushButton::drawIcon(QPainter* painter, const QRectF& rect, QIcon::State state) {
    if (icon_.canConvert<QIcon>()) {
        QIcon qicon = icon_.value<QIcon>();
        qicon.paint(painter, rect.toRect(), Qt::AlignCenter, QIcon::Normal, state);
    } else if (icon_.canConvert<const FluentIconBase*>()) {
        auto fluentIcon = icon_.value<const FluentIconBase*>();
        if (fluentIcon) {
            fluentIcon->render(painter, rect.toRect());
        }
    }
}

void FluentPushButton::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QStyleOptionButton opt;
    initStyleOption(&opt);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform |
                           QPainter::TextAntialiasing);

    // draw button background
    style()->drawControl(QStyle::CE_PushButtonBevel, &opt, &painter, this);

    paintContent(painter, opt);
}

void FluentPushButton::paintContent(QPainter& painter, QStyleOptionButton& opt) {
    // If we have text, draw icon + text with custom spacing
    if (!opt.text.isEmpty()) {
        // match Python/QSS behavior: pressed/disabled dim both icon and text
        if (!isEnabled()) {
            painter.setOpacity(0.3628);
        } else if (isPressed) {
            painter.setOpacity(0.786);
        }

        // Get content rect - this includes padding/margins from stylesheet.
        // We should center within this rect so QSS padding (e.g. reserved drop-down arrow area)
        // affects the icon/text position.
        const QRect contentRect =
            style()->subElementRect(QStyle::SE_PushButtonContents, &opt, this);

        const int iconW = opt.icon.isNull() ? 0 : opt.iconSize.width();
        const int iconH = opt.icon.isNull() ? 0 : opt.iconSize.height();
        const int gap = (opt.icon.isNull() ? 0 : 10);

        const QFontMetrics fm(font());
        const int textW = fm.horizontalAdvance(opt.text);
        const int textH = fm.height();

        const int totalW = iconW + gap + textW;

        // Center the content within the entire button.
        const int x0 = (width() - totalW) / 2;
        const int iconY = (height() - iconH) / 2;
        const int textY = (height() - textH) / 2 + fm.ascent();

        // icon
        if (!opt.icon.isNull()) {
            const QRectF iconRect(x0, iconY, iconW, iconH);
            QIcon::State iconState = isChecked() ? QIcon::On : QIcon::Off;

            // In checked+pressed state, icon should also look dim like text.
            // QSS uses rgba(..., 0.63) for pressed text; approximate via painter opacity.
            if (isChecked() && isPressed) {
                painter.save();
                painter.setOpacity(painter.opacity() * 0.63);
                drawIcon(&painter, iconRect, iconState);
                painter.restore();
            } else {
                drawIcon(&painter, iconRect, iconState);
            }
        }

        // text
        QColor textColor = opt.palette.buttonText().color();
        if (isChecked()) {
            // Match QSS group:
            // - light theme checked text is white (pressed: rgba(255,255,255,0.63), disabled:
            // rgba(255,255,255,0.9))
            // - dark theme checked text is black (pressed: rgba(0,0,0,0.63), disabled:
            // rgba(255,255,255,0.43))
            if (isDarkTheme()) {
                if (!isEnabled()) {
                    textColor = QColor(255, 255, 255, static_cast<int>(0.43 * 255));
                } else if (isPressed) {
                    textColor = QColor(0, 0, 0, static_cast<int>(0.63 * 255));
                } else {
                    textColor = Qt::black;
                }
            } else {
                if (!isEnabled()) {
                    textColor = QColor(255, 255, 255, static_cast<int>(0.9 * 255));
                } else if (isPressed) {
                    textColor = QColor(255, 255, 255, static_cast<int>(0.63 * 255));
                } else {
                    textColor = Qt::white;
                }
            }
        }
        painter.setPen(textColor);
        painter.drawText(QPoint(x0 + iconW + gap, textY), opt.text);

        return;
    }

    bool hasIcon = false;
    if (icon_.canConvert<QIcon>()) {
        hasIcon = !icon_.value<QIcon>().isNull();
    } else if (icon_.canConvert<const FluentIconBase*>()) {
        hasIcon = (icon_.value<const FluentIconBase*>() != nullptr);
    }

    if (!hasIcon) {
        return;
    }

    if (!isEnabled()) {
        painter.setOpacity(0.3628);
    } else if (isPressed) {
        painter.setOpacity(0.786);
    }

    const int w = iconSize().width();
    const int h = iconSize().height();
    const qreal y = (height() - h) / 2.0;

    // Calculate x position with 12px left margin, matching Python behavior
    int mw = minimumSizeHint().width();
    qreal x;
    if (mw > width()) {
        x = 12;
    } else {
        x = (width() - w) / 2.0;
    }

    if (isRightToLeft()) {
        x = width() - w - x;
    }

    const QRectF rect(x, y, w, h);
    QIcon::State state = isChecked() ? QIcon::On : QIcon::Off;

    if (isChecked() && isPressed) {
        painter.save();
        painter.setOpacity(painter.opacity() * 0.63);
        drawIcon(&painter, rect, state);
        painter.restore();
    } else {
        drawIcon(&painter, rect, state);
    }
}

void FluentPushButton::mousePressEvent(QMouseEvent* event) {
    isPressed = true;
    QPushButton::mousePressEvent(event);
}

void FluentPushButton::mouseReleaseEvent(QMouseEvent* event) {
    isPressed = false;
    QPushButton::mouseReleaseEvent(event);
}

void FluentPushButton::enterEvent(QEnterEvent* event) {
    isHover = true;
    update();
    QPushButton::enterEvent(event);
}

void FluentPushButton::leaveEvent(QEvent* event) {
    isHover = false;
    update();
    QPushButton::leaveEvent(event);
}

PrimaryPushButton::PrimaryPushButton(QWidget* parent) : FluentPushButton(parent) {
    setProperty("qssClass", "PrimaryPushButton");
}

PrimaryPushButton::PrimaryPushButton(const QString& text, QWidget* parent)
    : FluentPushButton(text, parent) {
    setProperty("qssClass", "PrimaryPushButton");
}

PrimaryPushButton::PrimaryPushButton(const QIcon& icon, const QString& text, QWidget* parent)
    : FluentPushButton(icon, text, parent) {
    setProperty("qssClass", "PrimaryPushButton");
}

PrimaryPushButton::PrimaryPushButton(const FluentIconBase& icon, const QString& text,
                                     QWidget* parent)
    : FluentPushButton(icon, text, parent) {
    setProperty("qssClass", "PrimaryPushButton");
}

void PrimaryPushButton::paintEvent(QPaintEvent* event) { FluentPushButton::paintEvent(event); }

void PrimaryPushButton::drawIcon(QPainter* painter, const QRectF& rect, QIcon::State state) {
    if (!painter) {
        return;
    }

    if (icon_.canConvert<QIcon>()) {
        const QIcon qicon = icon_.value<QIcon>();
        if (qicon.isNull()) {
            return;
        }

        if (isEnabled()) {
            const QSize size = rect.size().toSize();
            QPixmap pm = qicon.pixmap(size, QIcon::Normal, state);
            if (!pm.isNull()) {
                QImage img = pm.toImage();
                img = img.convertToFormat(QImage::Format_ARGB32);

                // Decide whether we need to invert.
                // Light theme primary buttons expect a light (white) icon;
                // dark theme primary buttons expect a dark (black) icon.
                const bool wantLightIcon = !isDarkTheme();

                qint64 sum = 0;
                qint64 cnt = 0;
                const int stepX = std::max(1, img.width() / 16);
                const int stepY = std::max(1, img.height() / 16);
                for (int y = 0; y < img.height(); y += stepY) {
                    const QRgb* line = reinterpret_cast<const QRgb*>(img.constScanLine(y));
                    for (int x = 0; x < img.width(); x += stepX) {
                        const QRgb px = line[x];
                        const int a = qAlpha(px);
                        if (a < 10) {
                            continue;
                        }
                        // Luma approximation
                        sum += (qRed(px) * 299 + qGreen(px) * 587 + qBlue(px) * 114) / 1000;
                        cnt++;
                    }
                }

                const int avg = cnt > 0 ? static_cast<int>(sum / cnt) : 0;
                const bool isCurrentlyLight = avg >= 128;
                const bool shouldInvert = wantLightIcon ? !isCurrentlyLight : isCurrentlyLight;

                if (shouldInvert) {
                    img.invertPixels(QImage::InvertRgb);
                }

                painter->drawPixmap(rect.toRect(), QPixmap::fromImage(img));
                return;
            }
        } else {
            painter->setOpacity(isDarkTheme() ? 0.786 : 0.9);
            qicon.paint(painter, rect.toRect(), Qt::AlignCenter, QIcon::Disabled, state);
            return;
        }
    }

    if (icon_.canConvert<const FluentIconBase*>()) {
        auto fluentIcon = icon_.value<const FluentIconBase*>();
        if (!fluentIcon) {
            return;
        }

        if (isEnabled()) {
            QIcon reversed = fluentIcon->qicon(true);
            reversed.paint(painter, rect.toRect(), Qt::AlignCenter, QIcon::Normal, state);
        } else {
            painter->setOpacity(isDarkTheme() ? 0.786 : 0.9);
            fluentIcon->render(painter, rect.toRect(), Theme::Dark);
        }
        return;
    }

    FluentPushButton::drawIcon(painter, rect, state);
}

TransparentPushButton::TransparentPushButton(QWidget* parent) : FluentPushButton(parent) {
    setProperty("qssClass", "TransparentPushButton");
}

TransparentPushButton::TransparentPushButton(const QString& text, QWidget* parent)
    : FluentPushButton(text, parent) {
    setProperty("qssClass", "TransparentPushButton");
}

TransparentPushButton::TransparentPushButton(const QIcon& icon, const QString& text,
                                             QWidget* parent)
    : FluentPushButton(icon, text, parent) {
    setProperty("qssClass", "TransparentPushButton");
}

TransparentPushButton::TransparentPushButton(const FluentIconBase& icon, const QString& text,
                                             QWidget* parent)
    : FluentPushButton(icon, text, parent) {
    setProperty("qssClass", "TransparentPushButton");
}

ToggleButton::ToggleButton(QWidget* parent) : FluentPushButton(parent) {
    setProperty("qssClass", "ToggleButton");
    setCheckable(true);
    setChecked(false);
}

ToggleButton::ToggleButton(const QString& text, QWidget* parent) : FluentPushButton(text, parent) {
    setProperty("qssClass", "ToggleButton");
    setCheckable(true);
    setChecked(false);
}

ToggleButton::ToggleButton(const QIcon& icon, const QString& text, QWidget* parent)
    : FluentPushButton(icon, text, parent) {
    setProperty("qssClass", "ToggleButton");
    setCheckable(true);
    setChecked(false);
}

ToggleButton::ToggleButton(const FluentIconBase& icon, const QString& text, QWidget* parent)
    : FluentPushButton(icon, text, parent) {
    setProperty("qssClass", "ToggleButton");
    setCheckable(true);
    setChecked(false);
}

void ToggleButton::drawIcon(QPainter* painter, const QRectF& rect, QIcon::State state) {
    if (isChecked()) {
        if (!painter) {
            return;
        }

        if (icon_.canConvert<QIcon>()) {
            const QIcon qicon = icon_.value<QIcon>();
            if (qicon.isNull()) {
                return;
            }

            const QSize size = rect.size().toSize();
            QPixmap pm = qicon.pixmap(size, QIcon::Normal, state);
            if (!pm.isNull()) {
                QImage img = pm.toImage();
                img = img.convertToFormat(QImage::Format_ARGB32);
                img.invertPixels(QImage::InvertRgb);
                painter->drawPixmap(rect.toRect(), QPixmap::fromImage(img));
                return;
            }
        }

        if (icon_.canConvert<const FluentIconBase*>()) {
            auto fluentIcon = icon_.value<const FluentIconBase*>();
            if (!fluentIcon) {
                return;
            }

            if (isEnabled()) {
                QIcon reversed = fluentIcon->qicon(true);
                reversed.paint(painter, rect.toRect(), Qt::AlignCenter, QIcon::Normal, state);
            } else {
                painter->setOpacity(isDarkTheme() ? 0.786 : 0.9);
                fluentIcon->render(painter, rect.toRect(), Theme::Dark);
            }
            return;
        }

        FluentPushButton::drawIcon(painter, rect, state);
        return;
    }

    FluentPushButton::drawIcon(painter, rect, state);
}

TransparentTogglePushButton::TransparentTogglePushButton(QWidget* parent) : ToggleButton(parent) {
    setProperty("qssClass", "TransparentTogglePushButton");
}

TransparentTogglePushButton::TransparentTogglePushButton(const QString& text, QWidget* parent)
    : ToggleButton(text, parent) {
    setProperty("qssClass", "TransparentTogglePushButton");
}

TransparentTogglePushButton::TransparentTogglePushButton(const QIcon& icon, const QString& text,
                                                         QWidget* parent)
    : ToggleButton(icon, text, parent) {
    setProperty("qssClass", "TransparentTogglePushButton");
}

TransparentTogglePushButton::TransparentTogglePushButton(const FluentIconBase& icon,
                                                         const QString& text, QWidget* parent)
    : ToggleButton(icon, text, parent) {
    setProperty("qssClass", "TransparentTogglePushButton");
}

HyperlinkButton::HyperlinkButton(QWidget* parent) : FluentPushButton(parent), url_() {
    setProperty("qssClass", "HyperlinkButton");
    setCursor(Qt::PointingHandCursor);
    QObject::connect(this, &QPushButton::clicked, this, &HyperlinkButton::onClicked);
}

HyperlinkButton::HyperlinkButton(const QString& url, const QString& text, QWidget* parent,
                                 const QVariant& icon)
    : HyperlinkButton(parent) {
    setText(text);
    setUrl(url);
    if (icon.isValid()) {
        setIcon(icon);
    }
}

HyperlinkButton::HyperlinkButton(const QIcon& icon, const QString& url, const QString& text,
                                 QWidget* parent)
    : HyperlinkButton(url, text, parent, QVariant::fromValue(icon)) {}

HyperlinkButton::HyperlinkButton(const FluentIconBase& icon, const QString& url,
                                 const QString& text, QWidget* parent)
    : HyperlinkButton(url, text, parent, QVariant::fromValue(&icon)) {}

QUrl HyperlinkButton::url() const { return url_; }

void HyperlinkButton::setUrl(const QUrl& url) { url_ = url; }

void HyperlinkButton::setUrl(const QString& url) { url_ = QUrl(url); }

void HyperlinkButton::onClicked() {
    if (url_.isValid()) {
        QDesktopServices::openUrl(url_);
    }
}

void HyperlinkButton::drawIcon(QPainter* painter, const QRectF& rect, QIcon::State state) {
    if (!painter) {
        return;
    }

    if (icon_.canConvert<const FluentIconBase*>()) {
        auto fluentIcon = icon_.value<const FluentIconBase*>();
        if (!fluentIcon) {
            return;
        }

        if (isEnabled()) {
            QIcon colored = fluentIcon->icon(Theme::Auto, themeColor());
            colored.paint(painter, rect.toRect(), Qt::AlignCenter, QIcon::Normal, state);
        } else {
            painter->setOpacity(isDarkTheme() ? 0.3628 : 0.36);
            fluentIcon->render(painter, rect.toRect());
        }
        return;
    }

    if (!isEnabled()) {
        painter->setOpacity(isDarkTheme() ? 0.3628 : 0.36);
    }
    FluentPushButton::drawIcon(painter, rect, state);
}

RadioButton::RadioButton(QWidget* parent)
    : QRadioButton(parent),
      lightTextColor_(QColor(0, 0, 0)),
      darkTextColor_(QColor(255, 255, 255)),
      lightIndicatorColor_(QColor()),
      darkIndicatorColor_(QColor()),
      indicatorPos_(QPoint(11, 12)),
      isHover_(false) {
    qfw::setStyleSheet(this, qfw::FluentStyleSheet::Button);
    setProperty("qssClass", "RadioButton");
    setAttribute(Qt::WA_MacShowFocusRect, false);
    qfw::setFont(this);
}

RadioButton::RadioButton(const QString& text, QWidget* parent) : RadioButton(parent) {
    setText(text);
}

QColor RadioButton::lightTextColor() const { return lightTextColor_; }

QColor RadioButton::darkTextColor() const { return darkTextColor_; }

void RadioButton::setLightTextColor(const QColor& color) {
    lightTextColor_ = QColor(color);
    update();
}

void RadioButton::setDarkTextColor(const QColor& color) {
    darkTextColor_ = QColor(color);
    update();
}

void RadioButton::setIndicatorColor(const QColor& light, const QColor& dark) {
    lightIndicatorColor_ = QColor(light);
    darkIndicatorColor_ = QColor(dark);
    update();
}

void RadioButton::setTextColor(const QColor& light, const QColor& dark) {
    setLightTextColor(light);
    setDarkTextColor(dark);
}

void RadioButton::enterEvent(QEnterEvent* event) {
    isHover_ = true;
    update();
    QRadioButton::enterEvent(event);
}

void RadioButton::leaveEvent(QEvent* event) {
    isHover_ = false;
    update();
    QRadioButton::leaveEvent(event);
}

void RadioButton::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    drawIndicator(&painter);
    drawText(&painter);
}

void RadioButton::drawText(QPainter* painter) {
    if (!painter) {
        return;
    }

    if (!isEnabled()) {
        painter->setOpacity(0.36);
    }

    painter->setFont(font());
    painter->setPen(textColor());
    painter->drawText(QRect(29, 0, width(), height()), Qt::AlignVCenter, text());
}

void RadioButton::drawIndicator(QPainter* painter) {
    if (!painter) {
        return;
    }

    const bool dark = isDarkTheme();

    auto themeColorDark1 = []() {
        const QColor base = themeColor();
        float h = 0, s = 0, v = 0, a = 0;
        base.getHsvF(&h, &s, &v, &a);

        // Keep consistent with common/style_sheet.cpp themedColor(..., darkTheme=true,
        // "ThemeColorDark1")
        s *= 0.84f;
        v = 1.0f;
        v *= 0.9f;

        return QColor::fromHsvF(static_cast<qreal>(h), static_cast<qreal>(std::min(s, 1.0f)),
                                static_cast<qreal>(std::min(v, 1.0f)), static_cast<qreal>(a));
    };

    if (isChecked()) {
        QColor borderColor;
        if (isEnabled()) {
            // Python uses autoFallbackThemeColor, which falls back to themeColor() when no custom
            // indicator colors are provided. For C++ migration, the expected visual in dark theme
            // is ThemeColorDark1 (same token as PrimaryPushButton background).
            const QColor custom = dark ? darkIndicatorColor_ : lightIndicatorColor_;
            if (dark && !custom.isValid()) {
                borderColor = themeColorDark1();
            } else {
                borderColor = fallbackThemeColor(custom);
            }
        } else {
            borderColor = dark ? QColor(255, 255, 255, 40) : QColor(0, 0, 0, 55);
        }

        const QColor filledColor = dark ? QColor(Qt::black) : QColor(Qt::white);
        const int thickness = (isHover_ && !isDown()) ? 4 : 5;
        drawCircle(painter, indicatorPos_, 10, thickness, borderColor, filledColor);
        return;
    }

    QColor filledColor;
    QColor borderColor;

    if (isEnabled()) {
        if (!isDown()) {
            borderColor = dark ? QColor(255, 255, 255, 153) : QColor(0, 0, 0, 153);
        } else {
            borderColor = dark ? QColor(255, 255, 255, 40) : QColor(0, 0, 0, 55);
        }

        if (isDown()) {
            filledColor = dark ? QColor(Qt::black) : QColor(Qt::white);
        } else if (isHover_) {
            filledColor = dark ? QColor(255, 255, 255, 11) : QColor(0, 0, 0, 15);
        } else {
            filledColor = dark ? QColor(0, 0, 0, 26) : QColor(0, 0, 0, 6);
        }
    } else {
        filledColor = QColor(Qt::transparent);
        borderColor = dark ? QColor(255, 255, 255, 40) : QColor(0, 0, 0, 55);
    }

    drawCircle(painter, indicatorPos_, 10, 1, borderColor, filledColor);

    if (isEnabled() && isDown()) {
        borderColor = dark ? QColor(255, 255, 255, 40) : QColor(0, 0, 0, 24);
        drawCircle(painter, indicatorPos_, 9, 4, borderColor, QColor(Qt::transparent));
    }
}

void RadioButton::drawCircle(QPainter* painter, const QPoint& center, int radius, int thickness,
                             const QColor& borderColor, const QColor& filledColor) {
    if (!painter) {
        return;
    }

    QPainterPath path;
    path.setFillRule(Qt::WindingFill);

    const QRectF outerRect(center.x() - radius, center.y() - radius, 2 * radius, 2 * radius);
    path.addEllipse(outerRect);

    const int ir = radius - thickness;
    const QRectF innerRect(center.x() - ir, center.y() - ir, 2 * ir, 2 * ir);
    QPainterPath innerPath;
    innerPath.addEllipse(innerRect);

    path = path.subtracted(innerPath);

    painter->setPen(Qt::NoPen);
    painter->fillPath(path, borderColor);
    painter->fillPath(innerPath, filledColor);
}

QColor RadioButton::textColor() const { return isDarkTheme() ? darkTextColor_ : lightTextColor_; }

ToolButton::ToolButton(QWidget* parent) : QToolButton(parent) { init(); }

ToolButton::ToolButton(const QIcon& icon, QWidget* parent) : QToolButton(parent) {
    init();
    setIcon(icon);
}

ToolButton::ToolButton(const FluentIconBase& icon, QWidget* parent) : QToolButton(parent) {
    init();
    setIcon(icon);
}

void ToolButton::init() {
    qfw::setStyleSheet(this, qfw::FluentStyleSheet::Button);
    isPressed = false;
    isHover = false;

    setProperty("qssClass", "ToolButton");
    setAutoRaise(true);
    setToolButtonStyle(Qt::ToolButtonIconOnly);
    setIconSize(QSize(16, 16));
    setIcon(QIcon());
    qfw::setFont(this);
}

ToolButton::~ToolButton() {
    if (ownedIcon_) {
        delete ownedIcon_;
        ownedIcon_ = nullptr;
    }
}

void ToolButton::setIcon(const QIcon& icon) {
    if (ownedIcon_) {
        delete ownedIcon_;
        ownedIcon_ = nullptr;
    }
    icon_ = icon;
    update();
}

void ToolButton::setIcon(const FluentIconBase& icon) {
    if (ownedIcon_) {
        delete ownedIcon_;
    }
    ownedIcon_ = icon.clone();
    icon_ = QVariant::fromValue(static_cast<const FluentIconBase*>(ownedIcon_));
    update();
}

void ToolButton::setIcon(const QVariant& icon) {
    if (icon.canConvert<QIcon>()) {
        setIcon(icon.value<QIcon>());
    } else if (icon.canConvert<const FluentIconBase*>()) {
        auto fluentIcon = icon.value<const FluentIconBase*>();
        if (fluentIcon) {
            setIcon(*fluentIcon);
        }
    } else if (icon.canConvert<QString>()) {
        setIcon(QIcon(icon.toString()));
    }
}

QIcon ToolButton::icon() const {
    if (icon_.canConvert<QIcon>()) {
        return icon_.value<QIcon>();
    } else if (icon_.canConvert<const FluentIconBase*>()) {
        auto fluentIcon = icon_.value<const FluentIconBase*>();
        if (fluentIcon) {
            return fluentIcon->icon();
        }
    }
    return QIcon();
}

QSize ToolButton::sizeHint() const {
    // Match QSS padding (ToolButton: 5px 9px 6px 8px) + 16x16 icon.
    // Note: keep it deterministic so layout allocates enough area for hover/pressed background.
    const int w = iconSize().width() + 11 + 11;
    const int h = iconSize().height() + 8 + 8;
    return QSize(w, h);
}

QSize ToolButton::minimumSizeHint() const { return sizeHint(); }

void ToolButton::drawIcon(QPainter* painter, const QRectF& rect, QIcon::State state) {
    if (!painter) {
        return;
    }

    if (icon_.canConvert<QIcon>()) {
        QIcon qicon = icon_.value<QIcon>();
        qicon.paint(painter, rect.toRect(), Qt::AlignCenter, QIcon::Normal, state);
    } else if (icon_.canConvert<const FluentIconBase*>()) {
        auto fluentIcon = icon_.value<const FluentIconBase*>();
        if (fluentIcon) {
            fluentIcon->render(painter, rect.toRect());
        }
    }
}

void ToolButton::paintEvent(QPaintEvent* event) {
    QToolButton::paintEvent(event);

    bool hasIcon = false;
    if (icon_.canConvert<QIcon>()) {
        hasIcon = !icon_.value<QIcon>().isNull();
    } else if (icon_.canConvert<const FluentIconBase*>()) {
        hasIcon = (icon_.value<const FluentIconBase*>() != nullptr);
    }

    if (!hasIcon) {
        return;
    }

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    if (!isEnabled()) {
        painter.setOpacity(0.43);
    } else if (isPressed) {
        painter.setOpacity(0.63);
    }

    const int w = iconSize().width();
    const int h = iconSize().height();
    const qreal y = (height() - h) / 2.0;
    const qreal x = (width() - w) / 2.0;
    const QRectF rect(x, y, w, h);
    drawIcon(&painter, rect, QIcon::Off);
}

void ToolButton::mousePressEvent(QMouseEvent* event) {
    isPressed = true;
    QToolButton::mousePressEvent(event);
}

void ToolButton::mouseReleaseEvent(QMouseEvent* event) {
    isPressed = false;
    QToolButton::mouseReleaseEvent(event);
}

void ToolButton::enterEvent(QEnterEvent* event) {
    isHover = true;
    update();
    QToolButton::enterEvent(event);
}

void ToolButton::leaveEvent(QEvent* event) {
    isHover = false;
    update();
    QToolButton::leaveEvent(event);
}

TransparentToolButton::TransparentToolButton(QWidget* parent) : ToolButton(parent) {
    setProperty("qssClass", "TransparentToolButton");
}

TransparentToolButton::TransparentToolButton(const QIcon& icon, QWidget* parent)
    : ToolButton(icon, parent) {
    setProperty("qssClass", "TransparentToolButton");
}

TransparentToolButton::TransparentToolButton(const FluentIconBase& icon, QWidget* parent)
    : ToolButton(icon, parent) {
    setProperty("qssClass", "TransparentToolButton");
}

TransparentToolButton::TransparentToolButton(FluentIconEnum icon, QWidget* parent)
    : ToolButton(FluentIcon(icon), parent) {
    setProperty("qssClass", "TransparentToolButton");
}

PrimaryToolButton::PrimaryToolButton(QWidget* parent) : ToolButton(parent) {
    setProperty("qssClass", "PrimaryToolButton");
}

PrimaryToolButton::PrimaryToolButton(const QIcon& icon, QWidget* parent)
    : ToolButton(icon, parent) {
    setProperty("qssClass", "PrimaryToolButton");
}

PrimaryToolButton::PrimaryToolButton(const FluentIconBase& icon, QWidget* parent)
    : ToolButton(icon, parent) {
    setProperty("qssClass", "PrimaryToolButton");
}

void PrimaryToolButton::drawIcon(QPainter* painter, const QRectF& rect, QIcon::State state) {
    if (!painter) {
        return;
    }

    if (icon_.canConvert<const FluentIconBase*>()) {
        auto fluentIcon = icon_.value<const FluentIconBase*>();
        if (!fluentIcon) {
            return;
        }

        if (isEnabled()) {
            QIcon reversed = fluentIcon->qicon(true);
            reversed.paint(painter, rect.toRect(), Qt::AlignCenter, QIcon::Normal, state);
        } else {
            painter->setOpacity(isDarkTheme() ? 0.786 : 0.9);
            fluentIcon->render(painter, rect.toRect(), Theme::Dark);
        }
        return;
    }

    if (icon_.canConvert<QIcon>()) {
        QIcon qicon = icon_.value<QIcon>();
        if (isEnabled()) {
            qicon.paint(painter, rect.toRect(), Qt::AlignCenter, QIcon::Normal, state);
        } else {
            painter->setOpacity(isDarkTheme() ? 0.786 : 0.9);
            qicon.paint(painter, rect.toRect(), Qt::AlignCenter, QIcon::Disabled, state);
        }
        return;
    }
}

SplitDropButton::SplitDropButton(QWidget* parent) : ToolButton(parent) {
    setProperty("qssClass", "SplitDropButton");
    setToolButtonStyle(Qt::ToolButtonIconOnly);
    setText(QString());
    setIconSize(QSize(10, 10));
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    arrowAni_ = new TranslateYAnimation(this);
}

void SplitDropButton::paintEvent(QPaintEvent* event) {
    QToolButton::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    if (!isEnabled()) {
        painter.setOpacity(0.43);
    } else if (isPressed) {
        painter.setOpacity(0.5);
    } else if (isHover) {
        painter.setOpacity(1.0);
    } else {
        painter.setOpacity(0.63);
    }

    const int w = iconSize().width();
    const int h = iconSize().height();
    const qreal x = (width() - w) / 2.0;
    const qreal yOffset = arrowAni_ ? arrowAni_->getY() : 0.0;
    const qreal y = (height() - h) / 2.0 + yOffset;
    const QRectF rect(x, y, w, h);

    const FluentIcon arrow(FluentIconEnum::ChevronDown);
    arrow.render(&painter, rect.toRect(), Theme::Auto);
}

PrimarySplitDropButton::PrimarySplitDropButton(QWidget* parent) : PrimaryToolButton(parent) {
    setProperty("qssClass", "PrimarySplitDropButton");
    setToolButtonStyle(Qt::ToolButtonIconOnly);
    setText(QString());
    setIconSize(QSize(10, 10));
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    arrowAni_ = new TranslateYAnimation(this);
}

void PrimarySplitDropButton::paintEvent(QPaintEvent* event) {
    QToolButton::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    if (!isEnabled()) {
        painter.setOpacity(0.43);
    } else if (isPressed) {
        painter.setOpacity(0.7);
    } else if (isHover) {
        painter.setOpacity(0.9);
    } else {
        painter.setOpacity(1.0);
    }

    const int w = iconSize().width();
    const int h = iconSize().height();
    const qreal x = (width() - w) / 2.0;
    const qreal yOffset = arrowAni_ ? arrowAni_->getY() : 0.0;
    const qreal y = (height() - h) / 2.0 + yOffset;
    const QRectF rect(x, y, w, h);

    // Match Python: theme = DARK if not isDarkTheme() else LIGHT
    const Theme theme = isDarkTheme() ? Theme::Light : Theme::Dark;
    const FluentIcon arrow(FluentIconEnum::ChevronDown);
    arrow.render(&painter, rect.toRect(), theme);
}

SplitWidgetBase::SplitWidgetBase(QWidget* parent) : QWidget(parent) {
    dropButton_ = new SplitDropButton(this);
    hBoxLayout_ = new QHBoxLayout(this);
    hBoxLayout_->setSpacing(0);
    hBoxLayout_->setContentsMargins(0, 0, 0, 0);
    hBoxLayout_->addWidget(dropButton_);

    connect(dropButton_, &QToolButton::clicked, this, &SplitWidgetBase::dropDownClicked);
    connect(dropButton_, &QToolButton::clicked, this, &SplitWidgetBase::showFlyout);

    setAttribute(Qt::WA_TranslucentBackground);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void SplitWidgetBase::setWidget(QWidget* widget) {
    if (!widget || !hBoxLayout_) {
        return;
    }
    hBoxLayout_->insertWidget(0, widget, 1, Qt::AlignLeft);
}

void SplitWidgetBase::setDropButton(ToolButton* button) {
    if (!button || !hBoxLayout_) {
        return;
    }

    if (dropButton_) {
        hBoxLayout_->removeWidget(dropButton_);
        dropButton_->deleteLater();
    }

    dropButton_ = button;
    dropButton_->setParent(this);

    connect(dropButton_, &QToolButton::clicked, this, &SplitWidgetBase::dropDownClicked);
    connect(dropButton_, &QToolButton::clicked, this, &SplitWidgetBase::showFlyout);
    hBoxLayout_->addWidget(dropButton_);
}

ToolButton* SplitWidgetBase::dropButton() const { return dropButton_; }

void SplitWidgetBase::setFlyout(QMenu* menu) { flyout_ = menu; }

QMenu* SplitWidgetBase::flyout() const { return flyout_; }

void SplitWidgetBase::showFlyout() {
    if (!flyout_) {
        return;
    }

    if (execRoundMenuAtButton(flyout_, this, width(), height(), 0)) {
        return;
    }

    flyout_->setMinimumWidth(width());

    const QPoint globalDown = mapToGlobal(QPoint(0, height()));
    const QPoint globalUp = mapToGlobal(QPoint(0, 0));

    QScreen* screen = window() ? window()->screen() : QGuiApplication::primaryScreen();
    const QRect avail = screen ? screen->availableGeometry() : QRect();
    const QSize hint = flyout_->sizeHint();

    const int downSpace = avail.isValid() ? (avail.bottom() - globalDown.y()) : INT_MAX;
    const int upSpace = avail.isValid() ? (globalUp.y() - avail.top()) : INT_MAX;

    if (downSpace >= upSpace) {
        flyout_->popup(globalDown);
    } else {
        flyout_->popup(QPoint(globalUp.x(), globalUp.y() - hint.height()));
    }
}

SplitPushButton::SplitPushButton(QWidget* parent) : SplitWidgetBase(parent) {
    button_ = new FluentPushButton(QString(), this);
    button_->setObjectName(QStringLiteral("splitPushButton"));
    setWidget(button_);
}

SplitPushButton::SplitPushButton(const QString& text, QWidget* parent, const QVariant& icon)
    : SplitPushButton(parent) {
    setText(text);
    if (icon.isValid()) {
        setIcon(icon);
    }
}

SplitPushButton::SplitPushButton(const QIcon& icon, const QString& text, QWidget* parent)
    : SplitPushButton(text, parent, QVariant::fromValue(icon)) {}

SplitPushButton::SplitPushButton(const FluentIconBase& icon, const QString& text, QWidget* parent)
    : SplitPushButton(text, parent, QVariant::fromValue(&icon)) {}

void SplitPushButton::setText(const QString& text) {
    if (button_) {
        button_->setText(text);
    }
}

QString SplitPushButton::text() const { return button_ ? button_->text() : QString(); }

void SplitPushButton::setIcon(const QIcon& icon) {
    if (button_) {
        button_->setIcon(icon);
    }
}

void SplitPushButton::setIcon(const FluentIconBase& icon) {
    if (button_) {
        button_->setIcon(icon);
    }
}

void SplitPushButton::setIcon(const QVariant& icon) {
    if (button_) {
        button_->setIcon(icon);
    }
}

FluentPushButton* SplitPushButton::button() const { return button_; }

PrimarySplitPushButton::PrimarySplitPushButton(QWidget* parent) : SplitWidgetBase(parent) {
    setDropButton(new PrimarySplitDropButton(this));

    button_ = new PrimaryPushButton(QString(), this);
    button_->setObjectName(QStringLiteral("primarySplitPushButton"));
    setWidget(button_);
}

PrimarySplitPushButton::PrimarySplitPushButton(const QString& text, QWidget* parent,
                                               const QVariant& icon)
    : PrimarySplitPushButton(parent) {
    setText(text);
    if (icon.isValid()) {
        setIcon(icon);
    }
}

PrimarySplitPushButton::PrimarySplitPushButton(const QIcon& icon, const QString& text,
                                               QWidget* parent)
    : PrimarySplitPushButton(text, parent, QVariant::fromValue(icon)) {}

PrimarySplitPushButton::PrimarySplitPushButton(const FluentIconBase& icon, const QString& text,
                                               QWidget* parent)
    : PrimarySplitPushButton(text, parent, QVariant::fromValue(&icon)) {}

void PrimarySplitPushButton::setText(const QString& text) {
    if (button_) {
        button_->setText(text);
    }
}

QString PrimarySplitPushButton::text() const { return button_ ? button_->text() : QString(); }

void PrimarySplitPushButton::setIcon(const QIcon& icon) {
    if (button_) {
        button_->setIcon(icon);
    }
}

void PrimarySplitPushButton::setIcon(const FluentIconBase& icon) {
    if (button_) {
        button_->setIcon(icon);
    }
}

void PrimarySplitPushButton::setIcon(const QVariant& icon) {
    if (button_) {
        button_->setIcon(icon);
    }
}

PrimaryPushButton* PrimarySplitPushButton::button() const { return button_; }

SplitToolButton::SplitToolButton(QWidget* parent) : SplitWidgetBase(parent) {
    button_ = new ToolButton(this);
    button_->setObjectName(QStringLiteral("splitToolButton"));
    connect(button_, &QToolButton::clicked, this, &SplitToolButton::clicked);
    setWidget(button_);
}

SplitToolButton::SplitToolButton(const QVariant& icon, QWidget* parent) : SplitToolButton(parent) {
    if (icon.isValid()) {
        setIcon(icon);
    }
}

SplitToolButton::SplitToolButton(const QIcon& icon, QWidget* parent)
    : SplitToolButton(QVariant::fromValue(icon), parent) {}

SplitToolButton::SplitToolButton(const FluentIconBase& icon, QWidget* parent)
    : SplitToolButton(QVariant::fromValue(&icon), parent) {}

void SplitToolButton::setIcon(const QIcon& icon) {
    if (button_) {
        button_->setIcon(icon);
    }
}

void SplitToolButton::setIcon(const FluentIconBase& icon) {
    if (button_) {
        button_->setIcon(icon);
    }
}

void SplitToolButton::setIcon(const QVariant& icon) {
    if (button_) {
        button_->setIcon(icon);
    }
}

QIcon SplitToolButton::icon() const { return button_ ? button_->icon() : QIcon(); }

void SplitToolButton::setIconSize(const QSize& size) {
    if (button_) {
        button_->setIconSize(size);
    }
}

QSize SplitToolButton::iconSize() const { return button_ ? button_->iconSize() : QSize(); }

PrimarySplitToolButton::PrimarySplitToolButton(QWidget* parent) : SplitToolButton(parent) {
    initPrimary();
}

PrimarySplitToolButton::PrimarySplitToolButton(const QVariant& icon, QWidget* parent)
    : PrimarySplitToolButton(parent) {
    if (icon.isValid()) {
        setIcon(icon);
    }
}

PrimarySplitToolButton::PrimarySplitToolButton(const QIcon& icon, QWidget* parent)
    : PrimarySplitToolButton(QVariant::fromValue(icon), parent) {}

PrimarySplitToolButton::PrimarySplitToolButton(const FluentIconBase& icon, QWidget* parent)
    : PrimarySplitToolButton(QVariant::fromValue(&icon), parent) {}

void PrimarySplitToolButton::initPrimary() {
    setDropButton(new PrimarySplitDropButton(this));

    // Replace left side button with PrimaryToolButton
    if (button_) {
        if (hBoxLayout_) {
            hBoxLayout_->removeWidget(button_);
        }
        button_->deleteLater();
        button_ = nullptr;
    }

    auto* primaryBtn = new PrimaryToolButton(this);
    primaryBtn->setObjectName(QStringLiteral("primarySplitToolButton"));
    connect(primaryBtn, &QToolButton::clicked, this, &SplitToolButton::clicked);
    setWidget(primaryBtn);
    button_ = primaryBtn;
}

void PillButtonBase::paintPill(QPainter* painter, const QRect& rect, bool checked, bool enabled,
                               bool pressed, bool hover) {
    if (!painter) {
        return;
    }

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing);

    const bool dark = isDarkTheme();
    QRect r = rect;
    QColor borderColor;
    QColor bgColor;

    auto themed = [](const QColor& base, bool darkTheme, const QString& token) {
        float h = 0, s = 0, v = 0, a = 0;
        base.getHsvF(&h, &s, &v, &a);

        if (darkTheme) {
            s *= 0.84f;
            v = 1.0f;
            if (token == QStringLiteral("ThemeColorDark1")) {
                v *= 0.9f;
            } else if (token == QStringLiteral("ThemeColorDark2")) {
                s *= 0.977f;
                v *= 0.82f;
            } else if (token == QStringLiteral("ThemeColorDark3")) {
                s *= 0.95f;
                v *= 0.7f;
            } else if (token == QStringLiteral("ThemeColorLight1")) {
                s *= 0.92f;
            } else if (token == QStringLiteral("ThemeColorLight2")) {
                s *= 0.78f;
            } else if (token == QStringLiteral("ThemeColorLight3")) {
                s *= 0.65f;
            }
        } else {
            if (token == QStringLiteral("ThemeColorDark1")) {
                v *= 0.75f;
            } else if (token == QStringLiteral("ThemeColorDark2")) {
                s *= 1.05f;
                v *= 0.5f;
            } else if (token == QStringLiteral("ThemeColorDark3")) {
                s *= 1.1f;
                v *= 0.4f;
            } else if (token == QStringLiteral("ThemeColorLight1")) {
                v *= 1.05f;
            } else if (token == QStringLiteral("ThemeColorLight2")) {
                s *= 0.75f;
                v *= 1.05f;
            } else if (token == QStringLiteral("ThemeColorLight3")) {
                s *= 0.65f;
                v *= 1.05f;
            }
        }

        return QColor::fromHsvF(static_cast<qreal>(h), static_cast<qreal>(std::min(s, 1.0f)),
                                static_cast<qreal>(std::min(v, 1.0f)), static_cast<qreal>(a));
    };

    if (!checked) {
        r = rect.adjusted(1, 1, -1, -1);
        borderColor = dark ? QColor(255, 255, 255, 18) : QColor(0, 0, 0, 15);

        if (!enabled) {
            bgColor = dark ? QColor(255, 255, 255, 11) : QColor(249, 249, 249, 75);
        } else if (pressed || hover) {
            bgColor = dark ? QColor(255, 255, 255, 21) : QColor(249, 249, 249, 128);
        } else {
            bgColor = dark ? QColor(255, 255, 255, 15) : QColor(243, 243, 243, 194);
        }
    } else {
        // checked = true - align with PrimaryPushButton group QSS in button.qss
        if (!enabled) {
            bgColor = dark ? QColor(52, 52, 52) : QColor(205, 205, 205);
            borderColor = bgColor;
        } else if (pressed) {
            bgColor = dark ? themed(themeColor(), true, QStringLiteral("ThemeColorDark2"))
                           : themed(themeColor(), false, QStringLiteral("ThemeColorLight3"));
            borderColor = bgColor;
        } else if (hover) {
            bgColor = dark ? themed(themeColor(), true, QStringLiteral("ThemeColorDark1"))
                           : themed(themeColor(), false, QStringLiteral("ThemeColorLight2"));
            borderColor = dark ? themed(themeColor(), true, QStringLiteral("ThemeColorLight1"))
                               : themed(themeColor(), false, QStringLiteral("ThemeColorLight1"));
        } else {
            bgColor = dark ? themed(themeColor(), true, QStringLiteral("ThemeColorLight1"))
                           : themeColor();
            borderColor = dark ? themed(themeColor(), true, QStringLiteral("ThemeColorLight1"))
                               : themed(themeColor(), false, QStringLiteral("ThemeColorLight1"));
        }

        r = rect;
    }

    painter->setPen(borderColor);
    painter->setBrush(bgColor);

    const qreal radius = r.height() / 2.0;
    painter->drawRoundedRect(r, radius, radius);
    painter->restore();
}

void PillPushButton::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    paintPill(&painter, rect(), isChecked(), isEnabled(), isPressed, isHover);

    QStyleOptionButton opt;
    initStyleOption(&opt);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    paintContent(painter, opt);
}

PillToolButton::PillToolButton(QWidget* parent) : ToggleToolButton(parent) {
    setProperty("qssClass", "PillToolButton");
}

PillToolButton::PillToolButton(const QIcon& icon, QWidget* parent)
    : ToggleToolButton(icon, parent) {
    setProperty("qssClass", "PillToolButton");
}

PillToolButton::PillToolButton(const FluentIconBase& icon, QWidget* parent)
    : ToggleToolButton(icon, parent) {
    setProperty("qssClass", "PillToolButton");
}

void PillToolButton::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    paintPill(&painter, rect(), isChecked(), isEnabled(), isPressed, isHover);
    ToggleToolButton::paintEvent(event);
}

ToggleToolButton::ToggleToolButton(QWidget* parent) : ToolButton(parent) {
    setProperty("qssClass", "ToggleToolButton");
    setCheckable(true);
    setChecked(false);
}

ToggleToolButton::ToggleToolButton(const QIcon& icon, QWidget* parent) : ToolButton(icon, parent) {
    setProperty("qssClass", "ToggleToolButton");
    setCheckable(true);
    setChecked(false);
}

ToggleToolButton::ToggleToolButton(const FluentIconBase& icon, QWidget* parent)
    : ToolButton(icon, parent) {
    setProperty("qssClass", "ToggleToolButton");
    setCheckable(true);
    setChecked(false);
}

void ToggleToolButton::paintEvent(QPaintEvent* event) {
    QToolButton::paintEvent(event);

    bool hasIcon = false;
    if (icon_.canConvert<QIcon>()) {
        hasIcon = !icon_.value<QIcon>().isNull();
    } else if (icon_.canConvert<const FluentIconBase*>()) {
        hasIcon = (icon_.value<const FluentIconBase*>() != nullptr);
    }

    if (!hasIcon) {
        return;
    }

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    if (!isEnabled()) {
        painter.setOpacity(0.43);
    } else if (isPressed) {
        painter.setOpacity(0.63);
    }

    const int w = iconSize().width();
    const int h = iconSize().height();
    const qreal y = (height() - h) / 2.0;
    const qreal x = (width() - w) / 2.0;
    const QRectF rect(x, y, w, h);
    const QIcon::State state = isChecked() ? QIcon::On : QIcon::Off;
    drawIcon(&painter, rect, state);
}

void ToggleToolButton::drawIcon(QPainter* painter, const QRectF& rect, QIcon::State state) {
    if (!isChecked()) {
        ToolButton::drawIcon(painter, rect, state);
        return;
    }

    // Checked: follow PrimaryToolButton icon behavior
    if (!painter) {
        return;
    }

    if (icon_.canConvert<const FluentIconBase*>()) {
        auto fluentIcon = icon_.value<const FluentIconBase*>();
        if (!fluentIcon) {
            return;
        }

        if (isEnabled()) {
            QIcon reversed = fluentIcon->qicon(true);
            reversed.paint(painter, rect.toRect(), Qt::AlignCenter, QIcon::Normal, state);
        } else {
            painter->setOpacity(isDarkTheme() ? 0.786 : 0.9);
            fluentIcon->render(painter, rect.toRect(), Theme::Dark);
        }
        return;
    }

    ToolButton::drawIcon(painter, rect, state);
}

TransparentToggleToolButton::TransparentToggleToolButton(QWidget* parent)
    : ToggleToolButton(parent) {
    setProperty("qssClass", "TransparentToggleToolButton");
}

TransparentToggleToolButton::TransparentToggleToolButton(const FluentIconBase& icon,
                                                         QWidget* parent)
    : ToggleToolButton(icon, parent) {
    setProperty("qssClass", "TransparentToggleToolButton");
}

DropDownPushButton::DropDownPushButton(QWidget* parent) : FluentPushButton(parent) {
    setProperty("qssClass", "DropDownPushButton");
    arrowAni_ = new TranslateYAnimation(this);
}

DropDownPushButton::DropDownPushButton(const QString& text, QWidget* parent)
    : FluentPushButton(text, parent) {
    setProperty("qssClass", "DropDownPushButton");
    arrowAni_ = new TranslateYAnimation(this);
}

DropDownPushButton::DropDownPushButton(const QIcon& icon, const QString& text, QWidget* parent)
    : FluentPushButton(icon, text, parent) {
    setProperty("qssClass", "DropDownPushButton");
    arrowAni_ = new TranslateYAnimation(this);
}

DropDownPushButton::DropDownPushButton(const FluentIconBase& icon, const QString& text,
                                       QWidget* parent)
    : FluentPushButton(icon, text, parent) {
    setProperty("qssClass", "DropDownPushButton");
    arrowAni_ = new TranslateYAnimation(this);
}

void DropDownPushButton::setMenu(QMenu* menu) { menu_ = menu; }

QMenu* DropDownPushButton::menu() const { return menu_; }

void DropDownPushButton::mouseReleaseEvent(QMouseEvent* event) {
    FluentPushButton::mouseReleaseEvent(event);
    showMenu();
}

void DropDownPushButton::showMenu() {
    if (!menu_) {
        return;
    }

    if (execRoundMenuAtButton(menu_, this, width(), height(), 0)) {
        return;
    }

    // Fallback: plain QMenu without animation
    menu_->setMinimumWidth(width());

    const QPoint globalDown = mapToGlobal(QPoint(0, height()));
    const QPoint globalUp = mapToGlobal(QPoint(0, 0));

    QScreen* screen = window() ? window()->screen() : QGuiApplication::primaryScreen();
    const QRect avail = screen ? screen->availableGeometry() : QRect();

    const QSize hint = menu_->sizeHint();
    const int downSpace = avail.isValid() ? (avail.bottom() - globalDown.y()) : INT_MAX;
    const int upSpace = avail.isValid() ? (globalUp.y() - avail.top()) : INT_MAX;

    if (downSpace >= upSpace) {
        menu_->popup(globalDown);
    } else {
        menu_->popup(QPoint(globalUp.x(), globalUp.y() - hint.height()));
    }
}

void PrimaryDropDownPushButton::showMenu() {
    if (!menu_) {
        return;
    }

    if (execRoundMenuAtButton(menu_, this, width(), height(), 0)) {
        return;
    }

    // Fallback: plain QMenu without animation
    const QPoint globalDown = mapToGlobal(QPoint(0, height()));
    const QPoint globalUp = mapToGlobal(QPoint(0, 0));

    QScreen* screen = window() ? window()->screen() : QGuiApplication::primaryScreen();
    const QRect avail = screen ? screen->availableGeometry() : QRect();

    const QSize hint = menu_->sizeHint();
    const int downSpace = avail.isValid() ? (avail.bottom() - globalDown.y()) : INT_MAX;
    const int upSpace = avail.isValid() ? (globalUp.y() - avail.top()) : INT_MAX;

    if (downSpace >= upSpace) {
        menu_->popup(globalDown);
    } else {
        menu_->popup(QPoint(globalUp.x(), globalUp.y() - hint.height()));
    }
}

void PrimaryDropDownPushButton::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QStyleOptionButton opt;
    initStyleOption(&opt);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    // draw button background
    style()->drawControl(QStyle::CE_PushButtonBevel, &opt, &painter, this);

    // Reserve right area for the drop-down arrow by shifting main content left.
    painter.save();
    painter.translate(-11.0, 0.0);
    paintContent(painter, opt);
    painter.restore();

    if (isHover) {
        painter.setOpacity(0.8);
    } else if (isPressed) {
        painter.setOpacity(0.7);
    }

    const qreal yOffset = arrowAni_ ? arrowAni_->getY() : 0.0;
    const QRectF rect(width() - 22, height() / 2.0 - 5 + yOffset, 10, 10);
    drawPrimaryDropDownIcon(&painter, rect);
}

void DropDownPushButton::drawDropDownIcon(QPainter* painter, const QRectF& rect) {
    if (!painter) {
        return;
    }

    const FluentIcon arrow(qfw::FluentIconEnum::ChevronDown);
    if (isDarkTheme()) {
        arrow.render(painter, rect.toRect());
    } else {
        arrow.render(painter, rect.toRect(), Theme::Auto,
                     QVariantMap{{QStringLiteral("fill"), QStringLiteral("#646464")},
                                 {QStringLiteral("stroke"), QStringLiteral("#646464")}});
    }
}

void DropDownPushButton::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QStyleOptionButton opt;
    initStyleOption(&opt);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    style()->drawControl(QStyle::CE_PushButtonBevel, &opt, &painter, this);

    painter.save();
    painter.translate(-11.0, 0.0);
    paintContent(painter, opt);
    painter.restore();

    if (isHover) {
        painter.setOpacity(0.8);
    } else if (isPressed) {
        painter.setOpacity(0.7);
    }

    const qreal yOffset = arrowAni_ ? arrowAni_->getY() : 0.0;
    const QRectF arrowRect(width() - 22, height() / 2.0 - 5 + yOffset, 10, 10);
    drawDropDownIcon(&painter, arrowRect);
}

TransparentDropDownPushButton::TransparentDropDownPushButton(QWidget* parent)
    : DropDownPushButton(parent) {
    setProperty("qssClass", "TransparentDropDownPushButton");
}

TransparentDropDownPushButton::TransparentDropDownPushButton(const QString& text, QWidget* parent)
    : DropDownPushButton(text, parent) {
    setProperty("qssClass", "TransparentDropDownPushButton");
}

TransparentDropDownPushButton::TransparentDropDownPushButton(const QIcon& icon, const QString& text,
                                                             QWidget* parent)
    : DropDownPushButton(icon, text, parent) {
    setProperty("qssClass", "TransparentDropDownPushButton");
}

TransparentDropDownPushButton::TransparentDropDownPushButton(const FluentIconBase& icon,
                                                             const QString& text, QWidget* parent)
    : DropDownPushButton(icon, text, parent) {
    setProperty("qssClass", "TransparentDropDownPushButton");
}

PrimaryDropDownPushButton::PrimaryDropDownPushButton(QWidget* parent) : PrimaryPushButton(parent) {
    setProperty("qssClass", "PrimaryDropDownPushButton");
    arrowAni_ = new TranslateYAnimation(this);
}

PrimaryDropDownPushButton::PrimaryDropDownPushButton(const QString& text, QWidget* parent)
    : PrimaryPushButton(text, parent) {
    setProperty("qssClass", "PrimaryDropDownPushButton");
    arrowAni_ = new TranslateYAnimation(this);
}

PrimaryDropDownPushButton::PrimaryDropDownPushButton(const QIcon& icon, const QString& text,
                                                     QWidget* parent)
    : PrimaryPushButton(icon, text, parent) {
    setProperty("qssClass", "PrimaryDropDownPushButton");
    arrowAni_ = new TranslateYAnimation(this);
}

PrimaryDropDownPushButton::PrimaryDropDownPushButton(const FluentIconBase& icon,
                                                     const QString& text, QWidget* parent)
    : PrimaryPushButton(icon, text, parent) {
    setProperty("qssClass", "PrimaryDropDownPushButton");
    arrowAni_ = new TranslateYAnimation(this);
}

void PrimaryDropDownPushButton::setMenu(QMenu* menu) { menu_ = menu; }

QMenu* PrimaryDropDownPushButton::menu() const { return menu_; }

void PrimaryDropDownPushButton::mouseReleaseEvent(QMouseEvent* event) {
    PrimaryPushButton::mouseReleaseEvent(event);
    showMenu();
}

void PrimaryDropDownButtonBase::drawPrimaryDropDownIcon(QPainter* painter, const QRectF& rect) {
    if (!painter) {
        return;
    }

    const Theme theme = isDarkTheme() ? Theme::Light : Theme::Dark;
    const FluentIcon arrow(qfw::FluentIconEnum::ChevronDown);
    arrow.render(painter, rect.toRect(), theme);
}

DropDownToolButton::DropDownToolButton(QWidget* parent) : ToolButton(parent) {
    setProperty("qssClass", "DropDownToolButton");
    arrowAni_ = new TranslateYAnimation(this);
}

DropDownToolButton::DropDownToolButton(const QIcon& icon, QWidget* parent)
    : ToolButton(icon, parent) {
    setProperty("qssClass", "DropDownToolButton");
    arrowAni_ = new TranslateYAnimation(this);
}

DropDownToolButton::DropDownToolButton(const FluentIconBase& icon, QWidget* parent)
    : ToolButton(icon, parent) {
    setProperty("qssClass", "DropDownToolButton");
    arrowAni_ = new TranslateYAnimation(this);
}

void DropDownToolButton::setMenu(QMenu* menu) { menu_ = menu; }

QMenu* DropDownToolButton::menu() const { return menu_; }

void DropDownToolButton::mouseReleaseEvent(QMouseEvent* event) {
    ToolButton::mouseReleaseEvent(event);
    showMenu();
}

void DropDownToolButton::showMenu() {
    if (!menu_) {
        return;
    }

    if (execRoundMenuAtButton(menu_, this, width(), height(), 0)) {
        return;
    }

    menu_->setMinimumWidth(width());

    const QPoint globalDown = mapToGlobal(QPoint(0, height()));
    const QPoint globalUp = mapToGlobal(QPoint(0, 0));

    QScreen* screen = window() ? window()->screen() : QGuiApplication::primaryScreen();
    const QRect avail = screen ? screen->availableGeometry() : QRect();
    const QSize hint = menu_->sizeHint();

    const int downSpace = avail.isValid() ? (avail.bottom() - globalDown.y()) : INT_MAX;
    const int upSpace = avail.isValid() ? (globalUp.y() - avail.top()) : INT_MAX;

    if (downSpace >= upSpace) {
        menu_->popup(globalDown);
    } else {
        menu_->popup(QPoint(globalUp.x(), globalUp.y() - hint.height()));
    }
}

void DropDownToolButton::drawIcon(QPainter* painter, const QRectF& rect, QIcon::State state) {
    ToolButton::drawIcon(painter, rect, state);
}

void DropDownToolButton::drawDropDownIcon(QPainter* painter, const QRectF& rect) {
    if (!painter) {
        return;
    }

    const FluentIcon arrow(qfw::FluentIconEnum::ChevronDown);
    if (isDarkTheme()) {
        arrow.render(painter, rect.toRect());
    } else {
        arrow.render(painter, rect.toRect(), Theme::Auto,
                     QVariantMap{{QStringLiteral("fill"), QStringLiteral("#646464")},
                                 {QStringLiteral("stroke"), QStringLiteral("#646464")}});
    }
}

QSize DropDownToolButton::sizeHint() const {
    const QSize base = ToolButton::sizeHint();
    return QSize(base.width() + 21, base.height());
}

QSize DropDownToolButton::minimumSizeHint() const { return sizeHint(); }

void DropDownToolButton::paintEvent(QPaintEvent* event) {
    QToolButton::paintEvent(event);

    bool hasIcon = false;
    if (icon_.canConvert<QIcon>()) {
        hasIcon = !icon_.value<QIcon>().isNull();
    } else if (icon_.canConvert<const FluentIconBase*>()) {
        hasIcon = (icon_.value<const FluentIconBase*>() != nullptr);
    }

    if (!hasIcon) {
        return;
    }

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    if (!isEnabled()) {
        painter.setOpacity(0.43);
    } else if (isPressed) {
        painter.setOpacity(0.63);
    }

    const int w = iconSize().width();
    const int h = iconSize().height();
    const qreal y = (height() - h) / 2.0;
    const qreal x = 12;

    const qreal iconRightLimit = width() - 22 - 2;
    const qreal iconX = std::min(x, iconRightLimit - w);
    const QRectF iconRect(iconX, y, w, h);
    drawIcon(&painter, iconRect, QIcon::Off);

    painter.setOpacity(1.0);
    if (isHover) {
        painter.setOpacity(0.8);
    } else if (isPressed) {
        painter.setOpacity(0.7);
    }

    const qreal yOffset = arrowAni_ ? arrowAni_->getY() : 0.0;
    const QRectF arrowRect(width() - 22, height() / 2.0 - 5 + yOffset, 10, 10);
    drawDropDownIcon(&painter, arrowRect);
}

TransparentDropDownToolButton::TransparentDropDownToolButton(QWidget* parent)
    : DropDownToolButton(parent) {
    setProperty("qssClass", "TransparentDropDownToolButton");
}

TransparentDropDownToolButton::TransparentDropDownToolButton(const QIcon& icon, QWidget* parent)
    : DropDownToolButton(icon, parent) {
    setProperty("qssClass", "TransparentDropDownToolButton");
}

TransparentDropDownToolButton::TransparentDropDownToolButton(const FluentIconBase& icon,
                                                             QWidget* parent)
    : DropDownToolButton(icon, parent) {
    setProperty("qssClass", "TransparentDropDownToolButton");
}

}  // namespace qfw
