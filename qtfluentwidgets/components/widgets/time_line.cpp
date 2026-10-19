#include "components/widgets/time_line.h"

#include <QHideEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <algorithm>

#include "common/config.h"
#include "common/style_sheet.h"

namespace qfw {
namespace {
class TimeLineStatusIcon : public QWidget {
public:
    TimeLineStatusIcon(InfoBarIconEnum icon, QWidget* parent) : QWidget(parent), icon_(icon) {
        setFixedSize(36, 36);
        connect(&QConfig::instance(), &QConfig::themeChanged, this, [this] { update(); });
        connect(&QConfig::instance(), &QConfig::themeColorChanged, this, [this] { update(); });
    }
    void setIcon(InfoBarIconEnum icon) {
        icon_ = icon;
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        InfoBarIcon(icon_).render(&painter, QRect(10, 10, 16, 16));
    }

private:
    InfoBarIconEnum icon_;
};
}  // namespace

TimeLineItem::TimeLineItem(const QString& text, InfoBarIconEnum icon, QWidget* parent)
    : CardWidget(parent), icon_(icon) {
    setClickEnabled(true);
    setBorderRadius(10);
    setMinimumHeight(50);
    iconWidget = new TimeLineStatusIcon(icon, this);
    textLabel = new BodyLabel(text, this);
    textLabel->setWordWrap(true);
    textLabel->setFocusPolicy(Qt::NoFocus);
    iconWidget->setAttribute(Qt::WA_TransparentForMouseEvents);
    textLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    auto* row = new QHBoxLayout(this);
    row->setContentsMargins(4, 7, 14, 7);
    row->setSpacing(0);
    row->addWidget(iconWidget, 0, Qt::AlignVCenter);
    row->addWidget(textLabel, 1);
    connect(&QConfig::instance(), &QConfig::themeChanged, this, [this] { resetPress(); });
}
QString TimeLineItem::text() const { return textLabel->text(); }
void TimeLineItem::setText(const QString& text) { textLabel->setText(text); }
InfoBarIconEnum TimeLineItem::icon() const { return icon_; }
void TimeLineItem::setIcon(InfoBarIconEnum icon) {
    icon_ = icon;
    static_cast<TimeLineStatusIcon*>(iconWidget)->setIcon(icon);
}
void TimeLineItem::setClickEnabled(bool enabled) {
    CardWidget::setClickEnabled(enabled);
    setCursor(enabled ? Qt::PointingHandCursor : Qt::ArrowCursor);
    setFocusPolicy(enabled ? Qt::StrongFocus : Qt::NoFocus);
    if (!enabled) resetPress();
}
void TimeLineItem::resetPress(bool animate) {
    mousePending_ = false;
    isPressed_ = false;
    // CardWidget's animator is private. Stop it before clearing pressed/theme state.
    const QColor target =
        isHover_ && isEnabled() ? hoverBackgroundColor() : normalBackgroundColor();
    bool animated = false;
    for (auto* animation : findChildren<QPropertyAnimation*>())
        if (animation->targetObject() == this && animation->propertyName() == "backgroundColor") {
            animation->stop();
            if (animate) {
                animation->setStartValue(backgroundColor());
                animation->setEndValue(target);
                animation->start();
                animated = true;
            }
        }
    if (!animated) setBackgroundColor(target);
}
void TimeLineItem::mousePressEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton || !isEnabled() || !isClickEnabled()) {
        event->ignore();
        return;
    }
    mousePending_ = true;
    setFocus(Qt::MouseFocusReason);
    CardWidget::mousePressEvent(event);
    event->accept();
}
void TimeLineItem::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) {
        event->ignore();
        return;
    }
    const bool activate =
        mousePending_ && isEnabled() && isClickEnabled() && rect().contains(event->pos());
    resetPress(true);
    event->accept();
    // Finish all state work before callbacks, which may delete the card.
    if (activate) emit clicked();
}
void TimeLineItem::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Space || event->key() == Qt::Key_Return ||
        event->key() == Qt::Key_Enter) {
        event->accept();
        if (!event->isAutoRepeat() && isEnabled() && isClickEnabled()) emit clicked();
        return;
    }
    CardWidget::keyPressEvent(event);
}
void TimeLineItem::hideEvent(QHideEvent* event) {
    isHover_ = false;
    resetPress();
    CardWidget::hideEvent(event);
}
void TimeLineItem::changeEvent(QEvent* event) {
    if (event->type() == QEvent::EnabledChange && !isEnabled()) resetPress();
    CardWidget::changeEvent(event);
}

TimeLineGroup::TimeLineGroup(const QString& title, InfoBarIconEnum icon, QWidget* parent)
    : QWidget(parent), icon_(icon) {
    header = new QWidget(this);
    header->setFixedHeight(36);
    iconWidget = new TimeLineStatusIcon(icon, header);
    titleLabel = new StrongBodyLabel(title, header);
    auto* headerRow = new QHBoxLayout(header);
    headerRow->setContentsMargins(0, 0, 0, 0);
    headerRow->setSpacing(4);
    headerRow->addWidget(iconWidget);
    headerRow->addWidget(titleLabel, 1);
    cards = new QWidget(this);
    cardsLayout = new QVBoxLayout(cards);
    cardsLayout->setContentsMargins(layoutDirection() == Qt::RightToLeft ? 0 : 42, 0,
                                    layoutDirection() == Qt::RightToLeft ? 42 : 0, 12);
    cardsLayout->setSpacing(19);
    layoutBox = new QVBoxLayout(this);
    layoutBox->setContentsMargins(0, 0, 0, 0);
    layoutBox->setSpacing(11);
    layoutBox->addWidget(header);
    layoutBox->addWidget(cards);
    cards->hide();
    connect(&QConfig::instance(), &QConfig::themeChanged, this, [this] { update(); });
}
TimeLineGroup::~TimeLineGroup() {
    for (const auto& item : items_)
        if (item) disconnect(item, nullptr, this, nullptr);
}
QString TimeLineGroup::title() const { return titleLabel->text(); }
void TimeLineGroup::setTitle(const QString& title) { titleLabel->setText(title); }
InfoBarIconEnum TimeLineGroup::icon() const { return icon_; }
void TimeLineGroup::setIcon(InfoBarIconEnum icon) {
    icon_ = icon;
    static_cast<TimeLineStatusIcon*>(iconWidget)->setIcon(icon);
}
TimeLineItem* TimeLineGroup::addItem(const QString& text, InfoBarIconEnum icon) {
    auto* item = new TimeLineItem(text, icon, cards);
    items_.append(item);
    cardsLayout->addWidget(item);
    connect(item, &QObject::destroyed, this, [this, item] { forgetItem(item); });
    cards->show();
    update();
    return item;
}
QList<TimeLineItem*> TimeLineGroup::items() const {
    QList<TimeLineItem*> result;
    for (const auto& item : items_)
        if (item) result.append(item);
    return result;
}
void TimeLineGroup::forgetItem(TimeLineItem* item) {
    items_.erase(
        std::remove_if(items_.begin(), items_.end(),
                       [item](const auto& entry) { return !entry || entry.data() == item; }),
        items_.end());
    cards->setVisible(!items_.isEmpty());
    update();
}
void TimeLineGroup::removeItem(TimeLineItem* item) {
    if (!item || !items().contains(item)) return;
    disconnect(item, nullptr, this, nullptr);
    cardsLayout->removeWidget(item);
    forgetItem(item);
    item->hide();
    item->deleteLater();
}
void TimeLineGroup::paintEvent(QPaintEvent*) {
    if (items_.isEmpty()) return;
    QPainter painter(this);
    painter.setPen(QPen(isDarkTheme() ? QColor(255, 255, 255, 90) : QColor(0, 0, 0, 100), 1));
    const int x = layoutDirection() == Qt::RightToLeft ? width() - 19 : 18;
    painter.drawLine(x, header->height() + 6, x, height() - 1);
}
void TimeLineGroup::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LayoutDirectionChange && cardsLayout) {
        const bool rtl = layoutDirection() == Qt::RightToLeft;
        cardsLayout->setContentsMargins(rtl ? 0 : 42, 0, rtl ? 42 : 0, 12);
        update();
    }
    QWidget::changeEvent(event);
}
TimeLineWidget::TimeLineWidget(QWidget* parent) : QWidget(parent) {
    setMaximumWidth(370);
    vBoxLayout = new QVBoxLayout(this);
    vBoxLayout->setContentsMargins(0, 0, 0, 0);
    vBoxLayout->setSpacing(6);
    // A trailing stretch keeps groups at the top while allowing Qt to honor
    // their height-for-width. Global AlignTop would shrink to the stale hint.
    vBoxLayout->addStretch(1);
    connect(&QConfig::instance(), &QConfig::themeChanged, this, [this] { update(); });
}
TimeLineWidget::~TimeLineWidget() {
    for (const auto& group : groups_)
        if (group) disconnect(group, nullptr, this, nullptr);
}
TimeLineGroup* TimeLineWidget::addGroup(const QString& title, InfoBarIconEnum icon) {
    auto* group = new TimeLineGroup(title, icon, this);
    groups_.append(group);
    vBoxLayout->insertWidget(vBoxLayout->count() - 1, group);
    connect(group, &QObject::destroyed, this, [this, group] {
        groups_.erase(
            std::remove_if(groups_.begin(), groups_.end(),
                           [group](const auto& entry) { return !entry || entry.data() == group; }),
            groups_.end());
    });
    return group;
}
QList<TimeLineGroup*> TimeLineWidget::groups() const {
    QList<TimeLineGroup*> result;
    for (const auto& group : groups_)
        if (group) result.append(group);
    return result;
}
void TimeLineWidget::removeGroup(TimeLineGroup* group) {
    if (!group || !groups().contains(group)) return;
    disconnect(group, nullptr, this, nullptr);
    groups_.removeAll(group);
    vBoxLayout->removeWidget(group);
    group->hide();
    group->deleteLater();
}
void TimeLineWidget::clear() {
    for (auto* group : groups()) removeGroup(group);
}
QSize TimeLineWidget::sizeHint() const {
    auto size = QWidget::sizeHint();
    size.setWidth(370);
    return size;
}
}  // namespace qfw
