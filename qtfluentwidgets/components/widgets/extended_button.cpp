#include "components/widgets/extended_button.h"

#include <QDesktopServices>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QVariantAnimation>
#include <algorithm>

#include "common/config.h"
#include "common/style_sheet.h"

namespace qfw {
namespace {

struct ButtonColors {
    QColor background;
    QColor border;
    QColor text;
};

// Reserve the maximum glow extent even at rest: animating blur must not change
// Qt's effect source/cache rectangle (which causes jitter on hover).
class LuminaGlowEffect final : public QGraphicsDropShadowEffect {
public:
    using QGraphicsDropShadowEffect::QGraphicsDropShadowEffect;
    QRectF boundingRectFor(const QRectF& rect) const override {
        return rect.adjusted(-45, -45, 45, 45);
    }
};

QColor attentionFill() {
    if (!isDarkTheme()) {
        return themeColor();
    }
    QColor_HsvF_type h, s, v, a;
    QConfig::instance().themeColor().getHsvF(&h, &s, &v, &a);
    return QColor::fromHsvF(h, std::min(s * QColor_HsvF_type(0.84 * 0.95), QColor_HsvF_type(1)),
                            QColor_HsvF_type(0.7), a);
}

QColor attentionBorder() {
    QColor_HsvF_type h, s, v, a;
    QConfig::instance().themeColor().getHsvF(&h, &s, &v, &a);
    if (isDarkTheme()) {
        s *= QColor_HsvF_type(0.84 * 0.92);
        v = 1;
    } else {
        v *= QColor_HsvF_type(0.75);
    }
    return QColor::fromHsvF(h, s, v, a);
}

QColor textSchemeColor(const QString& scheme, bool dark) {
    if (scheme == QStringLiteral("information")) {
        return dark ? QColor(119, 119, 119) : QColor(121, 121, 121);
    }
    if (scheme == QStringLiteral("success")) {
        return dark ? QColor(127, 189, 105) : QColor(30, 130, 60);
    }
    if (scheme == QStringLiteral("warning")) {
        return dark ? QColor(254, 254, 51) : QColor(157, 93, 0);
    }
    if (scheme == QStringLiteral("error")) {
        return QColor(200, 50, 50);
    }
    return themeColor();
}

}  // namespace

// A QObject helper avoids multiple QWidget inheritance and keeps identical
// palettes/animations in the push-button and tool-button implementations.
class ExtendedButtonRenderer final : public QObject {
public:
    ExtendedButtonRenderer(QAbstractButton* button, ExtendedButtonKind kind, bool tool)
        : QObject(button), button_(button), kind_(kind), tool_(tool), animation_(this) {
        animation_.setDuration(120);
        connect(&animation_, &QVariantAnimation::valueChanged, this, [this](const QVariant& value) {
            const qreal t = value.toReal();
            colors_ = {mix(start_.background, target_.background, t),
                       mix(start_.border, target_.border, t), mix(start_.text, target_.text, t)};
            button_->update();
        });
        if (kind_ == ExtendedButtonKind::Outlined) {
            button_->setCheckable(true);
        }
        button_->installEventFilter(this);
        connect(button_, &QAbstractButton::pressed, this, [this] { refresh(); });
        connect(button_, &QAbstractButton::released, this, [this] {
            releasedWhileHovered_ = hovered_;
            refresh();
        });
        connect(button_, &QAbstractButton::toggled, this, [this] { refresh(); });
        connect(&QConfig::instance(), &QConfig::themeChanged, this, [this] { refresh(false); });
        connect(&QConfig::instance(), &QConfig::themeColorChanged, this,
                [this] { refresh(false); });
        refresh(false);
    }

    QString scheme() const { return scheme_; }
    void setScheme(const QString& scheme) {
        static const QStringList schemes = {QStringLiteral("information"),
                                            QStringLiteral("success"), QStringLiteral("attention"),
                                            QStringLiteral("warning"), QStringLiteral("error")};
        if (schemes.contains(scheme) && scheme_ != scheme) {
            scheme_ = scheme;
            refresh();
        }
    }
    bool hasBorder() const { return hasBorder_; }
    void setHasBorder(bool value) {
        hasBorder_ = value;
        refresh();
    }
    ButtonColors colors() const { return colors_; }

    void paint(QPainter& painter, const QVariant& icon) {
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing |
                               QPainter::SmoothPixmapTransform);
        QRectF rect = QRectF(button_->rect()).adjusted(0.5, 0.5, -0.5, -0.5);
        const bool circular =
            tool_ && (kind_ == ExtendedButtonKind::Round || kind_ == ExtendedButtonKind::Outlined);
        if (circular) {
            const qreal size = std::min(rect.width(), rect.height());
            rect = QRectF(rect.center().x() - size / 2, rect.center().y() - size / 2, size, size);
        }
        const qreal radius =
            (kind_ == ExtendedButtonKind::Round || kind_ == ExtendedButtonKind::Outlined)
                ? rect.height() / 2
            : kind_ == ExtendedButtonKind::Lumina ? 8
                                                  : 5;
        painter.setPen(colors_.border.alpha() ? QPen(colors_.border, 1) : QPen(Qt::NoPen));
        painter.setBrush(colors_.background);
        if (circular) {
            painter.drawEllipse(rect);
        } else {
            painter.drawRoundedRect(rect, radius, radius);
        }

        const bool fluent = icon.canConvert<const FluentIconBase*>();
        const auto* fluentIcon = fluent ? icon.value<const FluentIconBase*>() : nullptr;
        const QIcon qicon = icon.canConvert<QIcon>() ? icon.value<QIcon>() : QIcon();
        const bool hasIcon = fluentIcon || !qicon.isNull();
        const QString text = tool_ ? QString() : button_->text();
        const int textWidth = button_->fontMetrics().horizontalAdvance(text);
        const int iconWidth = hasIcon ? button_->iconSize().width() : 0;
        const int gap = hasIcon && !text.isEmpty() ? 8 : 0;
        const int contentWidth = iconWidth + gap + textWidth;
        const qreal left = (button_->width() - contentWidth) / 2.0;
        const bool rtl = button_->isRightToLeft();

        if (hasIcon) {
            const qreal x = rtl ? left + textWidth + gap : left;
            const QRect iconRect(qRound(x), (button_->height() - button_->iconSize().height()) / 2,
                                 iconWidth, button_->iconSize().height());
            const bool colored = kind_ == ExtendedButtonKind::Filled ||
                                 kind_ == ExtendedButtonKind::Text ||
                                 kind_ == ExtendedButtonKind::Hyperlink ||
                                 (kind_ == ExtendedButtonKind::Outlined && button_->isChecked());
            painter.save();
            if (!button_->isEnabled()) {
                painter.setOpacity(0.43);
            } else if (button_->isDown()) {
                painter.setOpacity(0.786);
            }
            const QIcon::State state = button_->isChecked() ? QIcon::On : QIcon::Off;
            if (fluentIcon) {
                fluentIcon->icon(Theme::Auto, colored ? colors_.text : QColor())
                    .paint(&painter, iconRect, Qt::AlignCenter, QIcon::Normal, state);
            } else {
                qicon.paint(&painter, iconRect, Qt::AlignCenter, QIcon::Normal, state);
            }
            painter.restore();
        }
        if (!text.isEmpty()) {
            painter.setFont(button_->font());
            painter.setPen(colors_.text);
            const qreal textLeft = rtl ? left : left + iconWidth + gap;
            painter.drawText(QRectF(textLeft, 0, textWidth, button_->height()), Qt::AlignCenter,
                             text);
        }
        // Preserve keyboard focus feedback without changing layout or painting
        // the native push-button background over the custom surface.
        if (button_->hasFocus() && button_->testAttribute(Qt::WA_KeyboardFocusChange)) {
            painter.setBrush(Qt::NoBrush);
            painter.setPen(QPen(colors_.text, 1, Qt::DashLine));
            painter.drawRoundedRect(rect.adjusted(3, 3, -3, -3), radius, radius);
        }
    }

protected:
    bool eventFilter(QObject* obj, QEvent* event) override {
        switch (event->type()) {
            case QEvent::Enter:
                hovered_ = true;
                refresh();
                break;
            case QEvent::Leave:
                hovered_ = false;
                releasedWhileHovered_ = false;
                refresh();
                break;
            case QEvent::EnabledChange:
                refresh(false);
                break;
            case QEvent::PaletteChange:
                refresh(false);
                break;
            case QEvent::Show:
                refresh(false);
                break;
            case QEvent::Hide:
                animation_.stop();
                break;
            default:
                break;
        }
        return QObject::eventFilter(obj, event);
    }

private:
    static QColor mix(const QColor& a, const QColor& b, qreal t) {
        return QColor(qRound(a.red() + (b.red() - a.red()) * t),
                      qRound(a.green() + (b.green() - a.green()) * t),
                      qRound(a.blue() + (b.blue() - a.blue()) * t),
                      qRound(a.alpha() + (b.alpha() - a.alpha()) * t));
    }

    ButtonColors palette() const {
        const bool dark = isDarkTheme();
        const bool disabled = !button_->isEnabled();
        const bool pressed = button_->isDown();
        const QColor foreground = dark ? QColor(Qt::white) : QColor(Qt::black);
        ButtonColors c{Qt::transparent, Qt::transparent, foreground};
        if (kind_ == ExtendedButtonKind::Filled) {
            c.text = Qt::white;
            if (scheme_ == QStringLiteral("information")) {
                c.background = dark ? QColor(255, 255, 255, 100) : QColor("#8A8A8A");
                c.border = dark ? QColor(255, 255, 255, 150) : c.background;
            } else if (scheme_ == QStringLiteral("success")) {
                c.background = dark ? QColor(103, 186, 71, 128) : QColor("#0F7B0F");
                c.border = dark ? QColor(133, 197, 109, 254) : c.background.darker(115);
            } else if (scheme_ == QStringLiteral("warning")) {
                c.background = dark ? QColor(142, 129, 16, 254) : QColor("#9D5D00");
                c.border = dark ? QColor(255, 255, 51, 254) : c.background.darker(115);
            } else if (scheme_ == QStringLiteral("error")) {
                c.background = dark ? QColor(156, 31, 8, 128) : QColor("#C42B1C");
                c.border = dark ? QColor(180, 45, 52, 254) : c.background.darker(115);
            } else {
                c.background = attentionFill();
                c.border = attentionBorder();
            }
            if (disabled) {
                c.background = dark ? QColor(255, 255, 255, 10) : QColor(249, 249, 249, 76);
                c.border = Qt::transparent;
                c.text = dark ? QColor(255, 255, 255, 80) : QColor(0, 0, 0, 80);
            } else if (pressed) {
                c.background = dark ? QColor(255, 255, 255, 30) : QColor(0, 0, 0, 100);
                c.border = dark ? QColor(255, 255, 255, 30) : QColor(Qt::transparent);
            } else if (hovered_) {
                if (!releasedWhileHovered_) {
                    c.background = dark ? QColor(255, 255, 255, 67) : QColor(0, 0, 0, 60);
                }
                c.border = dark ? QColor(255, 255, 255, 150) : QColor(0, 0, 0, 128);
            }
        } else if (kind_ == ExtendedButtonKind::Text || kind_ == ExtendedButtonKind::Hyperlink) {
            c.text = kind_ == ExtendedButtonKind::Hyperlink ? themeColor()
                                                            : textSchemeColor(scheme_, dark);
            if (disabled) {
                c.text = dark ? QColor(255, 255, 255, 100) : QColor(0, 0, 0, 100);
            } else {
                if (hovered_ || pressed) {
                    c.background = dark ? QColor(255, 255, 255, pressed ? 6 : 9)
                                        : QColor(0, 0, 0, pressed ? 6 : 9);
                }
                if (kind_ == ExtendedButtonKind::Text) {
                    QColor_HsvF_type h, s, v, a;
                    c.text.getHsvF(&h, &s, &v, &a);
                    v = std::min(v * QColor_HsvF_type(pressed    ? 0.85
                                                      : hovered_ ? 1.1
                                                                 : 1),
                                 QColor_HsvF_type(1));
                    c.text = QColor::fromHsvF(h, s, v, a);
                }
            }
        } else if (kind_ == ExtendedButtonKind::Outlined) {
            c.border = button_->isChecked() ? themeColor()
                       : dark               ? QColor(255, 255, 255, 18)
                                            : QColor(0, 0, 0, 15);
            if (button_->isChecked()) {
                c.text = themeColor();
            } else if (disabled) {
                c.background = dark ? QColor(255, 255, 255, 11) : QColor(249, 249, 249, 75);
            } else if (hovered_ || pressed) {
                c.background = dark ? QColor(255, 255, 255, pressed ? 8 : 21)
                                    : QColor(0, 0, 0, pressed ? 17 : 11);
            }
        } else if (kind_ == ExtendedButtonKind::Round) {
            c.background = dark ? QColor(255, 255, 255,
                                         disabled   ? 10
                                         : pressed  ? 8
                                         : hovered_ ? 21
                                                    : 15)
                                : QColor(hovered_ || pressed || disabled ? 249 : 255,
                                         hovered_ || pressed || disabled ? 249 : 255,
                                         hovered_ || pressed || disabled ? 249 : 255,
                                         disabled || pressed ? 76
                                         : hovered_          ? 128
                                                             : 178);
            c.border = dark ? QColor(255, 255, 255, 13) : QColor(0, 0, 0, 19);
            if (tool_) {
                if (!hasBorder_ && !hovered_ && !pressed) {
                    c.background = c.border = Qt::transparent;
                } else if (hovered_ || pressed) {
                    c.border = Qt::transparent;
                    c.background = dark ? QColor(255, 255, 255, pressed ? 8 : 21)
                                        : QColor(0, 0, 0, pressed ? 6 : 9);
                }
            }
        } else if (kind_ == ExtendedButtonKind::Lumina) {
            c.background = dark ? QColor(45, 45, 45) : QColor(249, 249, 249);
            c.border =
                dark ? QColor(255, 255, 255, 50) : QColor(0, 0, 0, !disabled && hovered_ ? 1 : 15);
        }
        if (kind_ == ExtendedButtonKind::Round || kind_ == ExtendedButtonKind::Outlined ||
            kind_ == ExtendedButtonKind::Lumina) {
            if (disabled) {
                c.text.setAlphaF(0.36);
            } else if (pressed) {
                c.text.setAlphaF(0.786);
            }
        }
        return c;
    }

    void refresh(bool animate = true) {
        animation_.stop();
        target_ = palette();
        if (animate && button_->isVisible()) {
            start_ = colors_;
            animation_.setStartValue(0.0);
            animation_.setEndValue(1.0);
            animation_.start();
        } else {
            colors_ = target_;
            button_->update();
        }
    }

    QAbstractButton* button_;
    ExtendedButtonKind kind_;
    bool tool_;
    bool hovered_ = false;
    bool releasedWhileHovered_ = false;
    bool hasBorder_ = true;
    QString scheme_ = QStringLiteral("attention");
    ButtonColors colors_, start_, target_;
    QVariantAnimation animation_;
};

ExtendedPushButton::ExtendedPushButton(ExtendedButtonKind kind, QWidget* parent)
    : PushButton(parent), renderer_(new ExtendedButtonRenderer(this, kind, false)) {}
QString ExtendedPushButton::colorScheme() const { return renderer_->scheme(); }
void ExtendedPushButton::setColorScheme(const QString& scheme) { renderer_->setScheme(scheme); }
QColor ExtendedPushButton::backgroundColor() const { return renderer_->colors().background; }
QColor ExtendedPushButton::borderColor() const { return renderer_->colors().border; }
QColor ExtendedPushButton::textColor() const { return renderer_->colors().text; }
QSize ExtendedPushButton::sizeHint() const {
    const bool hasIcon = !icon().isNull();
    return QSize(fontMetrics().horizontalAdvance(text()) + 24 +
                     (hasIcon ? iconSize().width() + (text().isEmpty() ? 0 : 8) : 0),
                 std::max(fontMetrics().height(), hasIcon ? iconSize().height() : 0) + 12);
}
QSize ExtendedPushButton::minimumSizeHint() const { return sizeHint(); }
void ExtendedPushButton::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    renderer_->paint(painter, icon_);
}

ExtendedToolButton::ExtendedToolButton(ExtendedButtonKind kind, QWidget* parent)
    : ToolButton(parent), renderer_(new ExtendedButtonRenderer(this, kind, true)) {}
QString ExtendedToolButton::colorScheme() const { return renderer_->scheme(); }
void ExtendedToolButton::setColorScheme(const QString& scheme) { renderer_->setScheme(scheme); }
bool ExtendedToolButton::hasBorder() const { return renderer_->hasBorder(); }
void ExtendedToolButton::setHasBorder(bool value) { renderer_->setHasBorder(value); }
QColor ExtendedToolButton::backgroundColor() const { return renderer_->colors().background; }
QColor ExtendedToolButton::borderColor() const { return renderer_->colors().border; }
QColor ExtendedToolButton::textColor() const { return renderer_->colors().text; }
QSize ExtendedToolButton::sizeHint() const {
    const int side = std::max(iconSize().width(), iconSize().height()) + 16;
    return QSize(side, side);
}
QSize ExtendedToolButton::minimumSizeHint() const { return sizeHint(); }
void ExtendedToolButton::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    renderer_->paint(painter, icon_);
}

// All variants retain the existing C++ library's text/QIcon/FluentIcon overloads.
#define QFW_PUSH_CONSTRUCTORS(Name, Kind)                                                      \
    Name::Name(QWidget* parent) : ExtendedPushButton(ExtendedButtonKind::Kind, parent) {}      \
    Name::Name(const QString& text, QWidget* parent) : Name(parent) { setText(text); }         \
    Name::Name(const QIcon& icon, const QString& text, QWidget* parent) : Name(text, parent) { \
        setIcon(icon);                                                                         \
    }                                                                                          \
    Name::Name(const FluentIconBase& icon, const QString& text, QWidget* parent)               \
        : Name(text, parent) {                                                                 \
        setIcon(icon);                                                                         \
    }

#define QFW_TOOL_CONSTRUCTORS(Name, Kind)                                                 \
    Name::Name(QWidget* parent) : ExtendedToolButton(ExtendedButtonKind::Kind, parent) {} \
    Name::Name(const QIcon& icon, QWidget* parent) : Name(parent) { setIcon(icon); }      \
    Name::Name(const FluentIconBase& icon, QWidget* parent) : Name(parent) { setIcon(icon); }

QFW_PUSH_CONSTRUCTORS(FilledPushButton, Filled)
QFW_TOOL_CONSTRUCTORS(FilledToolButton, Filled)
QFW_PUSH_CONSTRUCTORS(TextPushButton, Text)
QFW_TOOL_CONSTRUCTORS(TextToolButton, Text)
QFW_PUSH_CONSTRUCTORS(OutlinedPushButton, Outlined)
QFW_TOOL_CONSTRUCTORS(OutlinedToolButton, Outlined)
QFW_PUSH_CONSTRUCTORS(RoundPushButton, Round)
QFW_TOOL_CONSTRUCTORS(RoundToolButton, Round)

#undef QFW_PUSH_CONSTRUCTORS
#undef QFW_TOOL_CONSTRUCTORS

HyperlinkToolButton::HyperlinkToolButton(QWidget* parent)
    : ExtendedToolButton(ExtendedButtonKind::Hyperlink, parent) {
    connect(this, &QAbstractButton::clicked, this, [this] {
        if (!url_.isEmpty()) {
            QDesktopServices::openUrl(url_);
        }
    });
}
HyperlinkToolButton::HyperlinkToolButton(const QIcon& icon, QWidget* parent)
    : HyperlinkToolButton(parent) {
    setIcon(icon);
}
HyperlinkToolButton::HyperlinkToolButton(const FluentIconBase& icon, QWidget* parent)
    : HyperlinkToolButton(parent) {
    setIcon(icon);
}
HyperlinkToolButton::HyperlinkToolButton(const QIcon& icon, const QString& url, QWidget* parent)
    : HyperlinkToolButton(icon, parent) {
    setUrl(url);
}
HyperlinkToolButton::HyperlinkToolButton(const FluentIconBase& icon, const QString& url,
                                         QWidget* parent)
    : HyperlinkToolButton(icon, parent) {
    setUrl(url);
}
QUrl HyperlinkToolButton::url() const { return url_; }
void HyperlinkToolButton::setUrl(const QUrl& url) { url_ = url; }
void HyperlinkToolButton::setUrl(const QString& url) { setUrl(QUrl(url)); }

LuminaPushButton::LuminaPushButton(QWidget* parent)
    : ExtendedPushButton(ExtendedButtonKind::Lumina, parent) {
    glowEffect_ = new LuminaGlowEffect(this);
    glowEffect_->setOffset(0, 0);
    glowEffect_->setBlurRadius(25);
    QColor color = themeColor();
    color.setAlpha(80);
    glowEffect_->setColor(color);
    setGraphicsEffect(glowEffect_);
    alphaAnimation_ = new QPropertyAnimation(this, "glowAlpha", this);
    blurAnimation_ = new QPropertyAnimation(glowEffect_, "blurRadius", this);
    alphaAnimation_->setDuration(150);
    blurAnimation_->setDuration(150);
    connect(this, &QAbstractButton::pressed, this, &LuminaPushButton::updateGlow);
    connect(this, &QAbstractButton::released, this, &LuminaPushButton::updateGlow);
    connect(&QConfig::instance(), &QConfig::themeChanged, this, &LuminaPushButton::updateGlowColor);
    connect(&QConfig::instance(), &QConfig::themeColorChanged, this,
            &LuminaPushButton::updateGlowColor);
}
LuminaPushButton::LuminaPushButton(const QString& text, QWidget* parent)
    : LuminaPushButton(parent) {
    setText(text);
}
LuminaPushButton::LuminaPushButton(const QIcon& icon, const QString& text, QWidget* parent)
    : LuminaPushButton(text, parent) {
    setIcon(icon);
}
LuminaPushButton::LuminaPushButton(const FluentIconBase& icon, const QString& text, QWidget* parent)
    : LuminaPushButton(text, parent) {
    setIcon(icon);
}
QColor LuminaPushButton::glowColor() const { return glowEffect_->color(); }
void LuminaPushButton::setGlowColor(const QColor& color) {
    if (!color.isValid()) {
        return;
    }
    QColor glow = color;
    glow.setAlpha(glowAlpha());
    glowEffect_->setColor(glow);
    customGlowColor_ = true;
}
int LuminaPushButton::glowAlpha() const { return glowEffect_->color().alpha(); }
void LuminaPushButton::setGlowAlpha(int alpha) {
    QColor color = glowEffect_->color();
    color.setAlpha(std::clamp(alpha, 0, 255));
    glowEffect_->setColor(color);
}
void LuminaPushButton::updateGlowColor() {
    if (!customGlowColor_) {
        QColor color = themeColor();
        color.setAlpha(glowAlpha());
        glowEffect_->setColor(color);
    }
}
bool LuminaPushButton::event(QEvent* event) {
    const bool result = ExtendedPushButton::event(event);
    if (glowEffect_) {
        if (event->type() == QEvent::Enter) {
            glowHovered_ = true;
            updateGlow();
        } else if (event->type() == QEvent::Leave) {
            glowHovered_ = false;
            updateGlow();
        } else if (event->type() == QEvent::EnabledChange) {
            updateGlow();
        }
    }
    return result;
}
void LuminaPushButton::updateGlow() {
    const bool expanded = isEnabled() && glowHovered_ && !isDown();
    alphaAnimation_->stop();
    alphaAnimation_->setStartValue(glowAlpha());
    alphaAnimation_->setEndValue(!isEnabled() ? 30 : expanded ? 200 : 80);
    alphaAnimation_->start();
    blurAnimation_->stop();
    blurAnimation_->setStartValue(glowEffect_->blurRadius());
    blurAnimation_->setEndValue(expanded ? 45 : 25);
    blurAnimation_->start();
}

}  // namespace qfw
