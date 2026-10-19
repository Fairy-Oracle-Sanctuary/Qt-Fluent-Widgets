#include "components/widgets/tag.h"

#include <QEvent>
#include <QPainter>
#include <QStyle>
#include <algorithm>

#include "common/config.h"
#include "common/font.h"

namespace qfw {

Tag::Tag(QWidget* parent) : QWidget(parent) {
    setFixedHeight(33);
    setFocusPolicy(Qt::NoFocus);
    qfw::setFont(this);
    connect(&QConfig::instance(), &QConfig::themeChanged, this, [this] { update(); });
    connect(&QConfig::instance(), &QConfig::themeColorChanged, this, [this] { update(); });
}
Tag::Tag(const QString& text, QWidget* parent) : Tag(parent) { setText(text); }
Tag::Tag(const QIcon& icon, const QString& text, QWidget* parent) : Tag(text, parent) {
    setIcon(icon);
}
Tag::Tag(const FluentIconBase& icon, const QString& text, QWidget* parent) : Tag(text, parent) {
    setIcon(icon);
}
Tag::~Tag() = default;
QString Tag::text() const { return text_; }
void Tag::setText(const QString& text) {
    if (text_ == text) {
        return;
    }
    text_ = text;
    setAccessibleName(text);
    updateGeometry();
    update();
}
QIcon Tag::icon() const { return fluentIcon_ ? fluentIcon_->icon() : icon_; }
void Tag::setIcon(const QIcon& icon) {
    fluentIcon_.reset();
    icon_ = icon;
    updateGeometry();
    update();
}
void Tag::setIcon(const FluentIconBase& icon) {
    fluentIcon_.reset(icon.clone());
    icon_ = QIcon();
    updateGeometry();
    update();
}
void Tag::setIcon(const QVariant& icon) {
    if (icon.canConvert<QIcon>()) {
        setIcon(icon.value<QIcon>());
    } else if (icon.canConvert<const FluentIconBase*>()) {
        const auto* fluent = icon.value<const FluentIconBase*>();
        if (fluent) {
            setIcon(*fluent);
        } else {
            setIcon(QIcon());
        }
    } else if (icon.canConvert<QString>()) {
        setIcon(QIcon(icon.toString()));
    } else if (!icon.isValid()) {
        setIcon(QIcon());
    }
}
QString Tag::type() const { return type_; }
void Tag::setType(const QString& type) {
    static const QStringList types = {QStringLiteral("information"), QStringLiteral("success"),
                                      QStringLiteral("warning"), QStringLiteral("error"),
                                      QStringLiteral("progress")};
    if (types.contains(type) && type_ != type) {
        type_ = type;
        update();
    }
}
std::array<QColor, 3> Tag::colors() const {
    const bool dark = isDarkTheme();
    if (type_ == QStringLiteral("success")) {
        return {dark ? QColor(22, 35, 18, 254) : QColor(246, 255, 237, 127),
                QColor(104, 207, 86, dark ? 180 : 127),
                dark ? QColor(104, 207, 86) : QColor(0, 176, 80)};
    }
    if (type_ == QStringLiteral("warning")) {
        return {dark ? QColor(43, 22, 24, 254) : QColor(255, 242, 204, 127),
                QColor(255, 192, 0, dark ? 180 : 127),
                dark ? QColor(255, 192, 0) : QColor(252, 173, 20)};
    }
    if (type_ == QStringLiteral("error")) {
        return {dark ? QColor(80, 0, 0, 120) : QColor(255, 0, 0, 20),
                dark ? QColor(255, 80, 80, 180) : QColor(255, 0, 0, 70),
                dark ? QColor(255, 80, 80) : QColor(255, 0, 0)};
    }
    if (type_ == QStringLiteral("progress")) {
        return {dark ? QColor(0, 90, 158, 80) : QColor(230, 243, 255, 200),
                QColor(0, 120, 215, 150), dark ? QColor(100, 180, 255) : QColor(0, 90, 158)};
    }
    return {dark ? QColor(40, 40, 40, 200) : QColor(Qt::white),
            dark ? QColor(255, 255, 255, 80) : QColor(0, 0, 0, 40),
            dark ? QColor(189, 189, 189) : QColor(Qt::black)};
}
QColor Tag::backgroundColor() const { return colors()[0]; }
QColor Tag::borderColor() const { return colors()[1]; }
QColor Tag::textColor() const { return colors()[2]; }
QSize Tag::sizeHint() const {
    return QSize(fontMetrics().horizontalAdvance(text_) + 16 + (icon().isNull() ? 0 : 22), 33);
}
QSize Tag::minimumSizeHint() const { return sizeHint(); }
void Tag::changeEvent(QEvent* event) {
    QWidget::changeEvent(event);
    if (event->type() == QEvent::FontChange) {
        updateGeometry();
    }
    update();
}
void Tag::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing |
                           QPainter::SmoothPixmapTransform);
    painter.setFont(font());
    const auto palette = colors();
    painter.setPen(QPen(palette[1], 1));
    painter.setBrush(palette[0]);
    painter.drawRoundedRect(QRectF(rect()).adjusted(1, 1, -1, -1), 5, 5);
    if (!isEnabled()) {
        painter.setOpacity(0.36);
    }
    int textStart = 8;
    if (!icon().isNull()) {
        const QRect logical(8, (height() - 16) / 2, 16, 16);
        const QIcon drawn = fluentIcon_ ? fluentIcon_->icon(Theme::Auto, palette[2]) : icon_;
        drawn.paint(&painter, QStyle::visualRect(layoutDirection(), rect(), logical),
                    Qt::AlignCenter);
        textStart += 22;
    }
    const int textWidth = std::max(0, width() - textStart - 8);
    const QRect textRect =
        QStyle::visualRect(layoutDirection(), rect(), QRect(textStart, 0, textWidth, height()));
    painter.setPen(palette[2]);
    painter.drawText(textRect, Qt::AlignLeading | Qt::AlignVCenter,
                     fontMetrics().elidedText(text_, Qt::ElideRight, textWidth));
}

}  // namespace qfw
