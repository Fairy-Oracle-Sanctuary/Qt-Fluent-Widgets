#include "components/widgets/info_badge.h"

#include <QEvent>
#include <QPainter>
#include <QWidget>

#include "common/color.h"
#include "common/font.h"
#include "common/style_sheet.h"

namespace qfw {

static QString levelToString(InfoLevel level) {
    switch (level) {
        case InfoLevel::Info:
            return QStringLiteral("Info");
        case InfoLevel::Success:
            return QStringLiteral("Success");
        case InfoLevel::Attention:
            return QStringLiteral("Attension");
        case InfoLevel::Warning:
            return QStringLiteral("Warning");
        case InfoLevel::Error:
            return QStringLiteral("Error");
        default:
            return QStringLiteral("Info");
    }
}

static void drawIconVariant(const QVariant& icon, QPainter* painter, const QRect& rect,
                            Theme themeForFluentIcon) {
    if (!painter || !icon.isValid()) {
        return;
    }

    if (icon.canConvert<QIcon>()) {
        const QIcon qicon = icon.value<QIcon>();
        if (!qicon.isNull()) {
            qicon.paint(painter, rect, Qt::AlignCenter, QIcon::Normal, QIcon::Off);
        }
        return;
    }

    if (icon.canConvert<const FluentIconBase*>()) {
        const auto* fluentIcon = icon.value<const FluentIconBase*>();
        if (!fluentIcon) {
            return;
        }
        fluentIcon->render(painter, rect, themeForFluentIcon);
        return;
    }
}

// ============================================================================
// InfoBadge
// ============================================================================
InfoBadge::InfoBadge(QWidget* parent, InfoLevel level) : QLabel(parent) { init(level); }

InfoBadge::InfoBadge(const QString& text, QWidget* parent, InfoLevel level) : QLabel(parent) {
    init(level);
    setText(text);
}

InfoBadge::InfoBadge(int num, QWidget* parent, InfoLevel level) : QLabel(parent) {
    init(level);
    setNum(num);
}

InfoBadge::InfoBadge(double num, QWidget* parent, InfoLevel level) : QLabel(parent) {
    init(level);
    setNum(num);
}

void InfoBadge::init(InfoLevel level) {
    setLevel(level);

    qfw::setFont(this, 11);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    setProperty("qssClass", "InfoBadge");
    qfw::setStyleSheet(this, qfw::FluentStyleSheet::InfoBadge);
}

void InfoBadge::setLevel(InfoLevel level) {
    if (level_ == level) {
        return;
    }

    level_ = level;
    setProperty("level", levelToString(level_));
    update();
}

InfoLevel InfoBadge::level() const { return level_; }

void InfoBadge::setCustomBackgroundColor(const QColor& light, const QColor& dark) {
    lightBackgroundColor_ = QColor(light);
    darkBackgroundColor_ = QColor(dark);
    update();
}

QColor InfoBadge::backgroundColor() const {
    const bool dark = isDarkTheme();

    if (lightBackgroundColor_.isValid() || darkBackgroundColor_.isValid()) {
        return dark
                   ? (darkBackgroundColor_.isValid() ? darkBackgroundColor_ : lightBackgroundColor_)
                   : (lightBackgroundColor_.isValid() ? lightBackgroundColor_
                                                      : darkBackgroundColor_);
    }

    switch (level_) {
        case InfoLevel::Info:
            return dark ? QColor(157, 157, 157) : QColor(138, 138, 138);
        case InfoLevel::Success:
            return dark ? QColor(108, 203, 95) : QColor(15, 123, 15);
        case InfoLevel::Attention:
            return dark ? themedColor(themeColor(), true, QStringLiteral("ThemeColorLight1"))
                        : themeColor();
        case InfoLevel::Warning:
            return dark ? QColor(255, 244, 206) : QColor(157, 93, 0);
        case InfoLevel::Error:
        default:
            return dark ? QColor(255, 153, 164) : QColor(196, 43, 28);
    }
}

void InfoBadge::paintEvent(QPaintEvent* e) {
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(backgroundColor());

    const qreal r = height() / 2.0;
    painter.drawRoundedRect(rect(), r, r);

    QLabel::paintEvent(e);
}

InfoBadge* InfoBadge::make(const QString& text, QWidget* parent, InfoLevel level, QWidget* target,
                           InfoBadgePosition position) {
    auto* w = new InfoBadge(text, parent, level);
    w->adjustSize();

    if (target) {
        w->manager_ = InfoBadgeManager::make(position, target, w);
        if (w->manager_) {
            w->move(w->manager_->position());
        }
    }

    return w;
}

InfoBadge* InfoBadge::info(const QString& text, QWidget* parent, QWidget* target,
                           InfoBadgePosition position) {
    return make(text, parent, InfoLevel::Info, target, position);
}

InfoBadge* InfoBadge::success(const QString& text, QWidget* parent, QWidget* target,
                              InfoBadgePosition position) {
    return make(text, parent, InfoLevel::Success, target, position);
}

InfoBadge* InfoBadge::attention(const QString& text, QWidget* parent, QWidget* target,
                                InfoBadgePosition position) {
    return make(text, parent, InfoLevel::Attention, target, position);
}

InfoBadge* InfoBadge::warning(const QString& text, QWidget* parent, QWidget* target,
                              InfoBadgePosition position) {
    return make(text, parent, InfoLevel::Warning, target, position);
}

InfoBadge* InfoBadge::error(const QString& text, QWidget* parent, QWidget* target,
                            InfoBadgePosition position) {
    return make(text, parent, InfoLevel::Error, target, position);
}

InfoBadge* InfoBadge::custom(const QString& text, const QColor& light, const QColor& dark,
                             QWidget* parent, QWidget* target, InfoBadgePosition position) {
    auto* w = make(text, parent, InfoLevel::Info, target, position);
    w->setCustomBackgroundColor(light, dark);
    return w;
}

// ============================================================================
// DotInfoBadge
// ============================================================================
DotInfoBadge::DotInfoBadge(QWidget* parent, InfoLevel level) : InfoBadge(parent, level) {
    setFixedSize(4, 4);
}

void DotInfoBadge::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(backgroundColor());
    painter.drawEllipse(rect());
}

DotInfoBadge* DotInfoBadge::make(QWidget* parent, InfoLevel level, QWidget* target,
                                 InfoBadgePosition position) {
    auto* w = new DotInfoBadge(parent, level);

    if (target) {
        w->manager_ = InfoBadgeManager::make(position, target, w);
        if (w->manager_) {
            w->move(w->manager_->position());
        }
    }

    return w;
}

// ============================================================================
// IconInfoBadge
// ============================================================================
IconInfoBadge::IconInfoBadge(QWidget* parent, InfoLevel level) : InfoBadge(parent, level) {
    setFixedSize(16, 16);
}

IconInfoBadge::IconInfoBadge(const QVariant& icon, QWidget* parent, InfoLevel level)
    : InfoBadge(parent, level), icon_(icon) {
    setFixedSize(16, 16);
}

void IconInfoBadge::setIcon(const QVariant& icon) {
    icon_ = icon;
    update();
}

QVariant IconInfoBadge::icon() const { return icon_; }

QSize IconInfoBadge::iconSize() const { return iconSize_; }

void IconInfoBadge::setIconSize(const QSize& size) {
    iconSize_ = size;
    update();
}

void IconInfoBadge::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter.setPen(Qt::NoPen);
    painter.setBrush(backgroundColor());
    painter.drawEllipse(rect());

    const int iw = iconSize_.width();
    const int ih = iconSize_.height();
    const int x = (width() - iw) / 2;
    const int y = (height() - ih) / 2;

    const Theme themeForFluentIcon = isDarkTheme() ? Theme::Light : Theme::Dark;
    drawIconVariant(icon_, &painter, QRect(x, y, iw, ih), themeForFluentIcon);
}

IconInfoBadge* IconInfoBadge::make(const QVariant& icon, QWidget* parent, InfoLevel level,
                                   QWidget* target, InfoBadgePosition position) {
    auto* w = new IconInfoBadge(icon, parent, level);

    if (target) {
        w->manager_ = InfoBadgeManager::make(position, target, w);
        if (w->manager_) {
            w->move(w->manager_->position());
        }
    }

    return w;
}

// ============================================================================
// InfoBadgeManager
// ============================================================================
InfoBadgeManager::InfoBadgeManager(QWidget* target, InfoBadge* badge)
    : QObject(), target_(target), badge_(badge) {
    if (target_) {
        target_->installEventFilter(this);
    }
}

InfoBadgeManager* InfoBadgeManager::make(InfoBadgePosition position, QWidget* target,
                                         InfoBadge* badge) {
    if (!target || !badge) {
        return nullptr;
    }

    auto* m = new InfoBadgeManager(target, badge);
    m->position_ = position;
    return m;
}

QRect InfoBadgeManager::targetRectInBadgeParent() const {
    if (!target_ || !badge_) {
        return {};
    }

    QWidget* parent = badge_->parentWidget();
    if (!parent) {
        parent = target_->parentWidget();
    }

    const QPoint topLeft = parent ? target_->mapTo(parent, QPoint(0, 0)) : target_->pos();
    return QRect(topLeft, target_->size());
}

QPoint InfoBadgeManager::position() const {
    if (!target_ || !badge_) {
        return {};
    }

    const QRect tr = targetRectInBadgeParent();

    switch (position_) {
        case InfoBadgePosition::TopRight:
        case InfoBadgePosition::NavigationItem: {
            const QPoint pos = tr.topRight();
            return QPoint(pos.x() - badge_->width() / 2, pos.y() - badge_->height() / 2);
        }
        case InfoBadgePosition::Right:
            return QPoint(tr.right() - badge_->width() / 2, tr.center().y() - badge_->height() / 2);
        case InfoBadgePosition::BottomRight: {
            const QPoint pos = tr.bottomRight();
            return QPoint(pos.x() - badge_->width() / 2, pos.y() - badge_->height() / 2);
        }
        case InfoBadgePosition::TopLeft:
            return QPoint(tr.left() - badge_->width() / 2, tr.top() - badge_->height() / 2);
        case InfoBadgePosition::Left:
            return QPoint(tr.left() - badge_->width() / 2, tr.center().y() - badge_->height() / 2);
        case InfoBadgePosition::BottomLeft: {
            const QPoint pos = tr.bottomLeft();
            return QPoint(pos.x() - badge_->width() / 2, pos.y() - badge_->height() / 2);
        }
        default:
            break;
    }

    return {};
}

bool InfoBadgeManager::eventFilter(QObject* watched, QEvent* e) {
    if (watched == target_ && badge_ && e) {
        if (e->type() == QEvent::Resize || e->type() == QEvent::Move) {
            badge_->move(position());
        }
    }

    return QObject::eventFilter(watched, e);
}

}  // namespace qfw
