#include "components/navigation/navigation_widget.h"

#include <QApplication>
#include <QCursor>
#include <QEnterEvent>
#include <QFont>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <deque>

#include "common/color.h"
#include "common/config.h"
#include "common/font.h"
#include "common/icon.h"
#include "common/style_sheet.h"
#include "components/widgets/label.h"

namespace qfw {

namespace {

static void drawIconVariant(const QVariant& icon, QPainter* painter, const QRectF& rect) {
    if (!painter || !icon.isValid()) {
        return;
    }

    if (icon.canConvert<QIcon>()) {
        const QIcon qicon = icon.value<QIcon>();
        if (!qicon.isNull()) {
            qicon.paint(painter, rect.toRect(), Qt::AlignCenter, QIcon::Normal, QIcon::Off);
        }
        return;
    }

    if (icon.canConvert<const FluentIconBase*>()) {
        const auto* fluentIcon = icon.value<const FluentIconBase*>();
        if (!fluentIcon) {
            return;
        }
        fluentIcon->render(painter, rect.toRect());
        return;
    }
}

}  // namespace

NavigationWidget::NavigationWidget(bool isSelectable, QWidget* parent)
    : QWidget(parent), isSelectable_(isSelectable) {
    setFixedSize(40, 36);
    setCursor(Qt::PointingHandCursor);
}

void NavigationWidget::enterEvent(QEnterEvent* e) {
    QWidget::enterEvent(e);
    isEnter_ = true;
    update();
}

void NavigationWidget::leaveEvent(QEvent* e) {
    QWidget::leaveEvent(e);
    isEnter_ = false;
    isPressed_ = false;
    update();
}

void NavigationWidget::mousePressEvent(QMouseEvent* e) {
    QWidget::mousePressEvent(e);
    isPressed_ = true;
    update();
}

void NavigationWidget::mouseReleaseEvent(QMouseEvent* e) {
    QWidget::mouseReleaseEvent(e);
    isPressed_ = false;
    update();
    emit clicked(true);
}

void NavigationWidget::setCompacted(bool compacted) {
    if (compacted == isCompacted_) {
        return;
    }

    isCompacted_ = compacted;
    if (compacted) {
        setFixedSize(40, 36);
    } else {
        setFixedSize(EXPAND_WIDTH, 36);
    }

    update();
}

void NavigationWidget::setSelected(bool selected) {
    if (!isSelectable_) {
        return;
    }

    isSelected_ = selected;
    isAboutSelected_ = false;
    update();
    emit selectedChanged(selected);
}

QColor NavigationWidget::textColor() const {
    return isDarkTheme() ? darkTextColor_ : lightTextColor_;
}

void NavigationWidget::setLightTextColor(const QColor& color) {
    lightTextColor_ = QColor(color);
    update();
}

void NavigationWidget::setDarkTextColor(const QColor& color) {
    darkTextColor_ = QColor(color);
    update();
}

void NavigationWidget::setTextColor(const QColor& light, const QColor& dark) {
    setLightTextColor(light);
    setDarkTextColor(dark);
}

void NavigationWidget::setAboutSelected(bool selected) {
    isAboutSelected_ = selected;
    update();
}

QMargins NavigationWidget::marginsForPaint() const { return QMargins(0, 0, 0, 0); }

QRectF NavigationWidget::indicatorRect() const {
    const QMargins m = marginsForPaint();
    return QRectF(m.left(), 10, 3, 16);
}

void NavigationWidget::setIndicatorColor(const QColor& light, const QColor& dark) {
    lightIndicatorColor_ = QColor(light);
    darkIndicatorColor_ = QColor(dark);
    update();
}

// ==========================================================================
// NavigationPushButton
// ==========================================================================
NavigationPushButton::NavigationPushButton(const QVariant& icon, const QString& text,
                                           bool isSelectable, QWidget* parent)
    : NavigationWidget(isSelectable, parent), icon_(icon), text_(text) {
    qfw::setFont(this);
}

void NavigationPushButton::setText(const QString& text) {
    text_ = text;
    update();
}

void NavigationPushButton::setIcon(const QVariant& icon) {
    icon_ = icon;
    update();
}

bool NavigationPushButton::canDrawIndicator() const { return isSelected_; }

void NavigationPushButton::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing |
                           QPainter::SmoothPixmapTransform);
    painter.setPen(Qt::NoPen);

    if (isPressed_) {
        painter.setOpacity(0.7);
    }
    if (!isEnabled()) {
        painter.setOpacity(0.4);
    }

    const int c = isDarkTheme() ? 255 : 0;
    const QMargins m = marginsForPaint();
    const int pl = m.left();
    const int pr = m.right();

    const QRect globalRect(mapToGlobal(QPoint()), size());

    if ((isSelected_ || isAboutSelected_) && isEnabled()) {
        painter.setBrush(QColor(c, c, c, isEnter_ ? 6 : 10));
        painter.drawRoundedRect(rect(), 5, 5);

        if (isSelected_) {
            // Use ThemeColorDark1 in dark mode for better visibility
            QColor indicatorColor;
            if (isDarkTheme()) {
                indicatorColor =
                    darkIndicatorColor_.isValid()
                        ? darkIndicatorColor_
                        : themedColor(themeColor(), true, QStringLiteral("ThemeColorDark1"));
            } else {
                indicatorColor =
                    lightIndicatorColor_.isValid() ? lightIndicatorColor_ : themeColor();
            }
            painter.setBrush(indicatorColor);
            painter.drawRoundedRect(indicatorRect(), 1.5, 1.5);
        }
    } else if ((isEnter_ && globalRect.contains(QCursor::pos())) && isEnabled()) {
        painter.setBrush(QColor(c, c, c, 10));
        painter.drawRoundedRect(rect(), 5, 5);
    }

    drawIconVariant(icon_, &painter, QRectF(11.5 + pl, 10, 16, 16));

    if (isCompacted_) {
        return;
    }

    painter.setFont(font());
    painter.setPen(textColor());

    const bool hasIcon =
        icon_.isValid() && ((icon_.canConvert<QIcon>() && !icon_.value<QIcon>().isNull()) ||
                            icon_.canConvert<const FluentIconBase*>());

    const int left = hasIcon ? (44 + pl) : (pl + 16);
    painter.drawText(QRectF(left, 0, width() - 13 - left - pr, height()), Qt::AlignVCenter, text_);
}

// ==========================================================================
// NavigationToolButton
// ==========================================================================
NavigationToolButton::NavigationToolButton(const QVariant& icon, QWidget* parent)
    : NavigationPushButton(icon, QString(), false, parent) {}

void NavigationToolButton::setCompacted(bool compacted) {
    Q_UNUSED(compacted);
    setFixedSize(40, 36);
}

// ==========================================================================
// NavigationSeparator
// ==========================================================================
NavigationSeparator::NavigationSeparator(QWidget* parent) : NavigationWidget(false, parent) {
    setCompacted(true);
}

void NavigationSeparator::setCompacted(bool compacted) {
    isCompacted_ = compacted;

    if (compacted) {
        setFixedSize(48, 3);
    } else {
        setFixedSize(EXPAND_WIDTH + 10, 3);
    }

    update();
}

void NavigationSeparator::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);

    QPainter painter(this);
    const int c = isDarkTheme() ? 255 : 0;
    QPen pen(QColor(c, c, c, 15));
    pen.setCosmetic(true);
    painter.setPen(pen);
    painter.drawLine(0, 1, width(), 1);
}

// ==========================================================================
// NavigationItemHeader
// ==========================================================================
NavigationItemHeader::NavigationItemHeader(const QString& text, QWidget* parent)
    : NavigationWidget(false, parent), text_(text) {
    qfw::setFont(this, 12);

    lightTextColor_ = QColor(96, 96, 96);
    darkTextColor_ = QColor(160, 160, 160);

    setCursor(Qt::ArrowCursor);

    heightAni_ = new QPropertyAnimation(this, QByteArrayLiteral("maximumHeight"), this);
    heightAni_->setDuration(150);
    heightAni_->setEasingCurve(QEasingCurve::OutQuad);
    connect(heightAni_, &QPropertyAnimation::valueChanged, this,
            &NavigationItemHeader::onHeightChanged);

    setFixedHeight(0);
}

void NavigationItemHeader::setText(const QString& text) {
    text_ = text;
    update();
}

void NavigationItemHeader::setCompacted(bool compacted) {
    isCompacted_ = compacted;

    if (heightAni_) {
        heightAni_->stop();
    }

    if (compacted) {
        setFixedWidth(40);
        if (heightAni_) {
            heightAni_->setStartValue(height());
            heightAni_->setEndValue(0);
            heightAni_->start();
        } else {
            setFixedHeight(0);
        }
    } else {
        setFixedWidth(EXPAND_WIDTH);
        setVisible(true);
        if (heightAni_) {
            heightAni_->setStartValue(height());
            heightAni_->setEndValue(targetHeight_);
            heightAni_->start();
        } else {
            setFixedHeight(targetHeight_);
        }
    }

    update();
}

void NavigationItemHeader::mousePressEvent(QMouseEvent* e) {
    if (e) {
        e->ignore();
    }
}

void NavigationItemHeader::mouseReleaseEvent(QMouseEvent* e) {
    if (e) {
        e->ignore();
    }
}

void NavigationItemHeader::enterEvent(QEnterEvent* e) { Q_UNUSED(e); }

void NavigationItemHeader::leaveEvent(QEvent* e) { Q_UNUSED(e); }

void NavigationItemHeader::onHeightChanged(const QVariant& value) { setFixedHeight(value.toInt()); }

void NavigationItemHeader::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);

    if (height() == 0 || !isVisible()) {
        return;
    }

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    const qreal opacity = qMin(1.0, static_cast<qreal>(height()) / qMax(1, targetHeight_));
    painter.setOpacity(opacity);

    if (!isCompacted_) {
        painter.setFont(font());
        painter.setPen(textColor());
        painter.drawText(QRect(12, 0, width() - 24, height()), Qt::AlignVCenter, text_);
    }
}

// ==========================================================================
// NavigationAvatarWidget
// ==========================================================================
NavigationAvatarWidget::NavigationAvatarWidget(const QString& name, QWidget* parent)
    : NavigationWidget(false, parent), name_(name) {
    avatar_ = new AvatarWidget(this);
    avatar_->setRadius(12);
    avatar_->setText(name);
    avatar_->move(8, 6);
    qfw::setFont(this);
}

void NavigationAvatarWidget::setName(const QString& name) {
    name_ = name;
    if (avatar_) {
        avatar_->setText(name);
    }
    update();
}

void NavigationAvatarWidget::setAvatar(const QString& avatarPath) {
    if (avatar_) {
        avatar_->setImage(avatarPath);
        avatar_->setRadius(12);
    }
    update();
}

void NavigationAvatarWidget::setAvatar(const QPixmap& avatar) {
    if (avatar_) {
        avatar_->setImage(avatar);
        avatar_->setRadius(12);
    }
    update();
}

void NavigationAvatarWidget::setAvatar(const QImage& avatar) {
    if (avatar_) {
        avatar_->setImage(avatar);
        avatar_->setRadius(12);
    }
    update();
}

void NavigationAvatarWidget::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e)

    QPainter painter(this);
    painter.setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing |
                           QPainter::TextAntialiasing);

    if (isPressed_) {
        painter.setOpacity(0.7);
    }

    drawBackground(&painter);
    drawText(&painter);
}

void NavigationAvatarWidget::drawText(QPainter* painter) {
    if (!painter || isCompacted_) {
        return;
    }

    painter->setPen(textColor());
    painter->setFont(font());
    painter->drawText(QRect(44, 0, 255, 36), Qt::AlignVCenter, name_);
}

void NavigationAvatarWidget::drawBackground(QPainter* painter) {
    if (!painter || !isEnter_) {
        return;
    }

    const int c = isDarkTheme() ? 255 : 0;
    painter->setBrush(QColor(c, c, c, 10));
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(rect(), 5, 5);
}

// ==========================================================================
// NavigationUserCard
// ==========================================================================
NavigationUserCard::NavigationUserCard(QWidget* parent)
    : NavigationAvatarWidget(QString(), parent) {
    animationGroup_ = new QParallelAnimationGroup(this);

    radiusAni_ = new QPropertyAnimation(avatar_, "radius", this);
    radiusAni_->setDuration(animationDuration_);
    radiusAni_->setEasingCurve(QEasingCurve::OutCubic);
    connect(radiusAni_, &QPropertyAnimation::valueChanged, this,
            [this](const QVariant&) { updateAvatarPosition(); });

    opacityAni_ = new QPropertyAnimation(this, "textOpacity", this);
    opacityAni_->setDuration(static_cast<int>(animationDuration_ * 0.8));
    opacityAni_->setEasingCurve(QEasingCurve::InOutQuad);

    animationGroup_->addAnimation(radiusAni_);
    animationGroup_->addAnimation(opacityAni_);
    connect(animationGroup_, &QParallelAnimationGroup::finished, this, [this]() {
        if (isCompacted_) {
            setFixedSize(40, 36);
        }
        update();
    });

    setFixedSize(40, 36);
}

void NavigationUserCard::setAvatarIcon(const QVariant& icon) {
    if (!avatar_) {
        return;
    }

    QPixmap pix;
    if (icon.canConvert<QIcon>()) {
        const QIcon qicon = icon.value<QIcon>();
        pix = qicon.pixmap(64, 64);
    } else if (icon.canConvert<const FluentIconBase*>()) {
        const auto* fluentIcon = icon.value<const FluentIconBase*>();
        if (fluentIcon) {
            QPixmap p(64, 64);
            p.fill(Qt::transparent);
            QPainter painter(&p);
            painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
            fluentIcon->render(&painter, QRect(0, 0, 64, 64));
            pix = p;
        }
    }

    if (!pix.isNull()) {
        avatar_->setImage(pix);
        update();
    }
}

void NavigationUserCard::setAvatarBackgroundColor(const QColor& light, const QColor& dark) {
    if (avatar_) {
        avatar_->setBackgroundColor(light, dark);
    }
    update();
}

void NavigationUserCard::setTitle(const QString& title) {
    title_ = title;
    setName(title);
    update();
}

void NavigationUserCard::setSubtitle(const QString& subtitle) {
    subtitle_ = subtitle;
    update();
}

void NavigationUserCard::setTitleFontSize(int size) {
    titleSize_ = size;
    update();
}

void NavigationUserCard::setSubtitleFontSize(int size) {
    subtitleSize_ = size;
    update();
}

void NavigationUserCard::setAnimationDuration(int duration) {
    animationDuration_ = duration;
    if (radiusAni_) {
        radiusAni_->setDuration(duration);
    }
    if (opacityAni_) {
        opacityAni_->setDuration(static_cast<int>(duration * 0.8));
    }
}

void NavigationUserCard::setCompacted(bool compacted) {
    if (compacted == isCompacted_) {
        return;
    }

    isCompacted_ = compacted;
    animationGroup_->stop();

    radiusAni_->setStartValue(avatar_->radius());
    opacityAni_->setStartValue(textOpacity_);

    if (compacted) {
        radiusAni_->setEndValue(12);
        opacityAni_->setEndValue(0.0);
    } else {
        setFixedSize(EXPAND_WIDTH, 80);
        if (parentWidget() && parentWidget()->layout()) {
            parentWidget()->layout()->activate();
        }
        radiusAni_->setEndValue(32);
        opacityAni_->setEndValue(1.0);
    }

    animationGroup_->start();
}

void NavigationUserCard::setTextOpacity(float value) {
    textOpacity_ = value;
    update();
}

void NavigationUserCard::setSubtitleColor(const QColor& color) {
    subtitleColor_ = color;
    update();
}

void NavigationUserCard::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e)

    QPainter painter(this);
    painter.setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing |
                           QPainter::TextAntialiasing);

    if (isPressed_) {
        painter.setOpacity(0.7);
    }

    // hover background same as NavigationAvatarWidget
    {
        const int c = isDarkTheme() ? 255 : 0;
        if (isEnter_) {
            painter.setBrush(QColor(c, c, c, 10));
            painter.setPen(Qt::NoPen);
            painter.drawRoundedRect(rect(), 5, 5);
        }
    }

    if (!isCompacted_ && textOpacity_ > 0.0f) {
        drawText(&painter);
    }
}

void NavigationUserCard::drawText(QPainter* painter) {
    if (!painter || !avatar_) {
        return;
    }

    const int textX = 16 + static_cast<int>(avatar_->radius() * 2) + 12;
    const int textWidth = width() - textX - 16;

    // title
    painter->setFont(qfw::getFont(titleSize_, QFont::Bold));
    QColor c = textColor();
    c.setAlpha(static_cast<int>(255 * textOpacity_));
    painter->setPen(c);

    const int titleY = height() / 2 - 2;
    painter->drawText(QRectF(textX, 0, textWidth, titleY), Qt::AlignLeft | Qt::AlignBottom, title_);

    // subtitle
    if (!subtitle_.isEmpty()) {
        painter->setFont(qfw::getFont(subtitleSize_));

        QColor sc = subtitleColor_.isValid() ? subtitleColor_ : textColor();
        sc.setAlpha(static_cast<int>(150 * textOpacity_));
        painter->setPen(sc);

        const int subtitleY = height() / 2 + 2;
        painter->drawText(QRectF(textX, subtitleY, textWidth, height() - subtitleY),
                          Qt::AlignLeft | Qt::AlignTop, subtitle_);
    }
}

void NavigationUserCard::updateAvatarPosition() {
    if (!avatar_) {
        return;
    }

    if (isCompacted_) {
        // In compact mode: small avatar at top-left corner
        avatar_->move(8, 6);
    } else {
        // In expanded mode: larger avatar vertically centered, with left margin
        const int y = (height() - avatar_->height()) / 2;
        avatar_->move(16, qMax(0, y));
    }
}

// ==========================================================================
// NavigationFlyoutMenu
// ==========================================================================
NavigationFlyoutMenu::NavigationFlyoutMenu(NavigationTreeWidget* tree, QWidget* parent)
    : ScrollArea(parent), treeWidget_(tree) {
    view_ = new QWidget(this);
    vBoxLayout_ = new QVBoxLayout(view_);

    setWidget(view_);
    setWidgetResizable(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    enableTransparentBackground();

    vBoxLayout_->setSpacing(5);
    vBoxLayout_->setContentsMargins(5, 8, 5, 8);

    if (treeWidget_) {
        for (auto* child : treeWidget_->childItems()) {
            auto* tw = qobject_cast<NavigationTreeWidget*>(child);
            if (!tw) {
                continue;
            }

            auto* node = tw->clone();
            if (!node) {
                continue;
            }

            connect(node, &NavigationTreeWidget::expanded, this,
                    [this]() { adjustViewSize(true); });

            treeChildren_.append(node);
            vBoxLayout_->addWidget(node);
        }
    }

    for (auto* c : treeChildren_) {
        initNode(c);
    }

    adjustViewSize(false);
}

void NavigationFlyoutMenu::initNode(NavigationTreeWidget* root) {
    if (!root) {
        return;
    }

    // normalize root node for flyout display
    root->setNodeDepth(qMax(0, root->nodeDepth() - 1));
    root->setCompacted(false);

    for (auto* child : root->childItems()) {
        auto* c = qobject_cast<NavigationTreeWidget*>(child);
        if (!c) {
            continue;
        }

        c->setNodeDepth(c->nodeDepth() - 1);
        c->setCompacted(false);

        if (c->isLeaf()) {
            connect(c, &NavigationWidget::clicked, this, [this](bool) {
                QObject* w = window();
                if (!w) {
                    return;
                }
                const int idx = w->metaObject()->indexOfMethod("fadeOut()");
                if (idx >= 0) {
                    QMetaObject::invokeMethod(w, "fadeOut", Qt::DirectConnection);
                } else if (auto* ww = qobject_cast<QWidget*>(w)) {
                    ww->close();
                }
            });
        }

        initNode(c);
    }
}

int NavigationFlyoutMenu::suitableWidth() const {
    int w = 0;
    for (auto* node : visibleTreeNodes()) {
        if (!node || node->isHidden()) {
            continue;
        }
        w = qMax(w, node->suitableWidth() + 10);
    }

    QWidget* window = nullptr;
    if (this->window()) {
        window = this->window()->parentWidget();
        if (!window) {
            window = this->window();
        }
    }

    if (!window) {
        return w + 10;
    }

    return qMin(window->width() / 2 - 25, w) + 10;
}

QList<NavigationTreeWidget*> NavigationFlyoutMenu::visibleTreeNodes() const {
    QList<NavigationTreeWidget*> nodes;
    std::deque<NavigationTreeWidget*> queue;

    for (auto* c : treeChildren_) {
        if (c) {
            queue.push_back(c);
        }
    }

    while (!queue.empty()) {
        NavigationTreeWidget* node = queue.front();
        queue.pop_front();
        nodes.append(node);

        for (auto* child : node->childItems()) {
            auto* c = qobject_cast<NavigationTreeWidget*>(child);
            if (c && !c->isHidden()) {
                queue.push_back(c);
            }
        }
    }

    return nodes;
}

void NavigationFlyoutMenu::adjustViewSize(bool emitSignal) {
    const int w = suitableWidth();

    for (auto* node : visibleTreeNodes()) {
        if (!node) {
            continue;
        }

        node->setFixedWidth(w - 10);
        if (node->itemWidget()) {
            node->itemWidget()->setFixedWidth(w - 10);
        }
    }

    if (view_) {
        view_->setFixedSize(w, view_->sizeHint().height());
    }

    QWidget* window = nullptr;
    if (this->window()) {
        window = this->window()->parentWidget();
        if (!window) {
            window = this->window();
        }
    }

    const int hLimit = window ? (window->height() - 48) : view_->height();
    const int h = qMin(hLimit, view_ ? view_->height() : 0);
    setFixedSize(w, h);

    if (emitSignal) {
        emit expanded();
    }
}

// ==========================================================================
// NavigationTreeItem
// ==========================================================================
NavigationTreeItem::NavigationTreeItem(const QVariant& icon, const QString& text, bool isSelectable,
                                       QWidget* parent)
    : NavigationPushButton(icon, text, isSelectable, parent) {
    rotateAni_ = new QPropertyAnimation(this, QByteArrayLiteral("arrowAngle"), this);
}

void NavigationTreeItem::setExpanded(bool expanded) {
    if (!rotateAni_) {
        return;
    }

    rotateAni_->stop();
    rotateAni_->setEndValue(expanded ? 180.0 : 0.0);
    rotateAni_->setDuration(150);
    rotateAni_->start();
}

void NavigationTreeItem::setArrowAngle(float angle) {
    arrowAngle_ = angle;
    update();
}

void NavigationTreeItem::mouseReleaseEvent(QMouseEvent* e) {
    NavigationPushButton::mouseReleaseEvent(e);

    bool clickArrow = false;
    if (e) {
        clickArrow = QRectF(width() - 30, 8, 20, 20).contains(e->pos());
    }

    bool arrowTrigger = false;
    if (auto* p = qobject_cast<NavigationTreeWidget*>(parentWidget())) {
        arrowTrigger = clickArrow && !p->isLeaf();
    }

    emit itemClicked(true, arrowTrigger);
    update();
}

bool NavigationTreeItem::canDrawIndicator() const {
    auto* p = qobject_cast<NavigationTreeWidget*>(parentWidget());
    if (!p) {
        return isSelected_;
    }

    if (p->isLeaf() || p->isSelected()) {
        return p->isSelected();
    }

    for (auto* child : p->childItems()) {
        if (!child) {
            continue;
        }

        auto* treeChild = qobject_cast<NavigationTreeWidget*>(child);
        if (!treeChild) {
            continue;
        }

        if (treeChild->itemWidget() && treeChild->itemWidget()->canDrawIndicator() &&
            !treeChild->isVisible()) {
            return true;
        }
    }

    return false;
}

QMargins NavigationTreeItem::marginsForPaint() const {
    auto* p = qobject_cast<NavigationTreeWidget*>(parentWidget());
    if (!p) {
        return NavigationWidget::marginsForPaint();
    }

    const int right = 20 * static_cast<int>(!p->isLeaf());
    return QMargins(p->nodeDepth() * 28, 0, right, 0);
}

void NavigationTreeItem::paintEvent(QPaintEvent* e) {
    NavigationPushButton::paintEvent(e);
    drawDropDownArrow();
}

void NavigationTreeItem::drawDropDownArrow() {
    auto* p = qobject_cast<NavigationTreeWidget*>(parentWidget());
    if (!p) {
        return;
    }

    if (isCompacted_ || p->isLeaf()) {
        return;
    }

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    if (isPressed_) {
        painter.setOpacity(0.7);
    }
    if (!isEnabled()) {
        painter.setOpacity(0.4);
    }

    painter.translate(width() - 20, 18);
    painter.rotate(arrowAngle_);

    FluentIcon(FluentIconEnum::ArrowDown).render(&painter, QRect(-5, -5, 10, 10));
}

// ==========================================================================
// NavigationTreeWidget
// ==========================================================================
NavigationTreeWidget::NavigationTreeWidget(const QVariant& icon, const QString& text,
                                           bool isSelectable, QWidget* parent)
    : NavigationTreeWidgetBase(isSelectable, parent), icon_(icon) {
    itemWidget_ = new NavigationTreeItem(icon, text, isSelectable, this);
    vBoxLayout_ = new QVBoxLayout(this);
    expandAni_ = new QPropertyAnimation(this, QByteArrayLiteral("geometry"), this);

    initWidget();
}

void NavigationTreeWidget::initWidget() {
    vBoxLayout_->setSpacing(4);
    vBoxLayout_->setContentsMargins(0, 0, 0, 0);
    vBoxLayout_->addWidget(itemWidget_, 0, Qt::AlignTop);

    connect(itemWidget_, &NavigationTreeItem::itemClicked, this, &NavigationTreeWidget::onClicked);

    setAttribute(Qt::WA_TranslucentBackground);

    connect(expandAni_, &QPropertyAnimation::valueChanged, this, [this](const QVariant& v) {
        const QRect g = v.toRect();
        setFixedSize(g.size());
    });
    connect(expandAni_, &QPropertyAnimation::valueChanged, this, [this]() { emit expanded(); });

    connect(expandAni_, &QPropertyAnimation::finished, this, [this]() {
        if (parentWidget() && parentWidget()->layout()) {
            parentWidget()->layout()->invalidate();
        }
    });
}

QString NavigationTreeWidget::text() const { return itemWidget_ ? itemWidget_->text() : QString(); }

void NavigationTreeWidget::setText(const QString& text) {
    if (itemWidget_) {
        itemWidget_->setText(text);
    }
}

QVariant NavigationTreeWidget::iconVariant() const {
    return itemWidget_ ? itemWidget_->iconVariant() : QVariant();
}

void NavigationTreeWidget::setIcon(const QVariant& icon) {
    icon_ = icon;
    if (itemWidget_) {
        itemWidget_->setIcon(icon);
    }
}

void NavigationTreeWidget::setFont(const QFont& font) {
    QWidget::setFont(font);
    if (itemWidget_) {
        itemWidget_->setFont(font);
    }
}

NavigationTreeWidget* NavigationTreeWidget::clone() const {
    auto* root = new NavigationTreeWidget(icon_, text(), isSelectable_, parentWidget());
    root->setSelected(isSelected_);
    root->setFixedSize(size());
    root->setTextColor(lightTextColor_, darkTextColor_);
    if (itemWidget_) {
        root->setIndicatorColor(itemWidget_->lightIndicatorColor(),
                                itemWidget_->darkIndicatorColor());
    }
    root->nodeDepth_ = nodeDepth_;

    connect(root, &NavigationTreeWidget::clicked, this, &NavigationTreeWidget::clicked);
    connect(this, &NavigationTreeWidget::selectedChanged, root, &NavigationTreeWidget::setSelected);

    for (auto* child : treeChildren_) {
        if (child) {
            root->addChild(child->clone());
        }
    }

    return root;
}

int NavigationTreeWidget::suitableWidth() const {
    if (!itemWidget_) {
        return 0;
    }

    const QMargins m = itemWidget_->marginsForPaint();

    const bool hasIcon =
        icon_.isValid() && ((icon_.canConvert<QIcon>() && !icon_.value<QIcon>().isNull()) ||
                            icon_.canConvert<const FluentIconBase*>());

    const int left = hasIcon ? (57 + m.left()) : (m.left() + 29);
    const int tw = itemWidget_->fontMetrics().boundingRect(text()).width();
    return left + tw + m.right();
}

void NavigationTreeWidget::addChild(NavigationTreeWidgetBase* child) { insertChild(-1, child); }

void NavigationTreeWidget::insertChild(int index, NavigationTreeWidgetBase* child) {
    auto* c = qobject_cast<NavigationTreeWidget*>(child);
    if (!c) {
        return;
    }

    if (treeChildren_.contains(c)) {
        return;
    }

    c->treeParent_ = this;
    c->nodeDepth_ = nodeDepth_ + 1;

    c->setVisible(isExpanded_);
    connect(c->expandAni_, &QPropertyAnimation::valueChanged, this, [this]() {
        setFixedSize(sizeHint());
        emit expanded();
    });

    NavigationTreeWidget* p = treeParent_;
    while (p) {
        connect(c->expandAni_, &QPropertyAnimation::valueChanged, p,
                [p]() { p->setFixedSize(p->sizeHint()); });
        p = p->treeParent_;
    }

    if (index < 0) {
        index = treeChildren_.size();
    }

    const int insertPos = index + 1;
    treeChildren_.insert(index, c);
    vBoxLayout_->insertWidget(insertPos, c, 0, Qt::AlignTop);

    if (isExpanded_) {
        setFixedHeight(height() + c->height() + vBoxLayout_->spacing());

        NavigationTreeWidget* pp = treeParent_;
        while (pp) {
            pp->setFixedSize(pp->sizeHint());
            pp = pp->treeParent_;
        }
    }

    update();
}

void NavigationTreeWidget::removeChild(NavigationTreeWidgetBase* child) {
    auto* c = qobject_cast<NavigationTreeWidget*>(child);
    if (!c) {
        return;
    }

    treeChildren_.removeOne(c);
    if (vBoxLayout_) {
        vBoxLayout_->removeWidget(c);
    }

    setFixedHeight(sizeHint().height());
}

QList<NavigationTreeWidgetBase*> NavigationTreeWidget::childItems() const {
    QList<NavigationTreeWidgetBase*> res;
    for (auto* c : treeChildren_) {
        res.append(c);
    }
    return res;
}

void NavigationTreeWidget::setExpanded(bool expanded, bool ani) {
    if (expanded == isExpanded_) {
        return;
    }

    isExpanded_ = expanded;
    if (itemWidget_) {
        itemWidget_->setExpanded(expanded);
    }

    for (auto* child : treeChildren_) {
        if (!child) {
            continue;
        }
        child->setVisible(expanded);
        child->setFixedSize(child->sizeHint());
    }

    if (ani && expandAni_) {
        expandAni_->stop();
        expandAni_->setStartValue(geometry());
        expandAni_->setEndValue(QRect(pos(), sizeHint()));
        expandAni_->setDuration(120);
        expandAni_->setEasingCurve(QEasingCurve::OutQuad);
        expandAni_->start();
    } else {
        setFixedSize(sizeHint());
    }
}

bool NavigationTreeWidget::isRoot() const { return treeParent_ == nullptr; }

bool NavigationTreeWidget::isLeaf() const { return treeChildren_.isEmpty(); }

void NavigationTreeWidget::setSelected(bool selected) {
    NavigationTreeWidgetBase::setSelected(selected);
    if (itemWidget_) {
        itemWidget_->setSelected(selected);
    }
}

void NavigationTreeWidget::setCompacted(bool compacted) {
    NavigationTreeWidgetBase::setCompacted(compacted);
    if (itemWidget_) {
        itemWidget_->setCompacted(compacted);
    }

    // Fix: when compacting, also collapse children to prevent layout issues on re-expand
    if (compacted && isExpanded_) {
        // Save current expand state before collapsing
        for (auto* child : treeChildren_) {
            if (child) {
                child->saveExpandState();
            }
        }
        // Collapse without animation
        setExpanded(false, false);
    }
}

void NavigationTreeWidget::setAboutSelected(bool selected) {
    isAboutSelected_ = selected;
    if (itemWidget_) {
        itemWidget_->setAboutSelected(selected);
    }
}

void NavigationTreeWidget::onClicked(bool triggerByUser, bool clickArrow) {
    if (!isCompacted_) {
        if (isSelectable_ && !isSelected_ && !clickArrow) {
            setExpanded(true, true);
        } else {
            setExpanded(!isExpanded_, true);
        }
    }

    if (!clickArrow || isCompacted_) {
        emit clicked(triggerByUser);
    }
}

void NavigationTreeWidget::setRememberExpandState(bool remember) {
    rememberExpandState_ = remember;
}

void NavigationTreeWidget::saveExpandState() {
    wasExpanded_ = rememberExpandState_ ? isExpanded_ : false;
}

void NavigationTreeWidget::restoreExpandState(bool ani) {
    if (wasExpanded_) {
        setExpanded(true, ani);
    }
}

// ==========================================================================
// NavigationIndicator
// ==========================================================================
NavigationIndicator::NavigationIndicator(QWidget* parent) : QWidget(parent) {
    scaleSlideAni_ = new qfw::ScaleSlideAnimation(this, Qt::Vertical);

    // standard indicator size
    resize(3, 16);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_TranslucentBackground);
    hide();

    connect(scaleSlideAni_, &qfw::ScaleSlideAnimation::valueChanged, this,
            [this](const QRectF& g) { setGeometry(g.toRect()); });

    // Python version just emits aniFinished, no geometry normalization
    connect(scaleSlideAni_, &qfw::ScaleSlideAnimation::finished, this,
            [this]() { emit aniFinished(); });
}

void NavigationIndicator::startAnimation(const QRectF& startRect, const QRectF& endRect,
                                         bool useCrossFade) {
    // stop any ongoing animation to avoid accumulating length from previous state
    scaleSlideAni_->stopAnimation();

    // Do NOT normalize rect size - let ScaleSlideAnimation handle the geometry
    // Python version passes startRect/endRect directly without normalization
    setGeometry(startRect.toRect());
    show();

    scaleSlideAni_->setGeometry(startRect);
    scaleSlideAni_->startAnimation(endRect, useCrossFade);
}

void NavigationIndicator::stopAnimation() {
    scaleSlideAni_->stopAnimation();
    hide();
}

void NavigationIndicator::setIndicatorColor(const QColor& light, const QColor& dark) {
    lightColor_ = light;
    darkColor_ = dark;
    update();
}

void NavigationIndicator::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e)

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    // Use ThemeColorDark1 in dark mode for better visibility
    QColor color;
    if (isDarkTheme()) {
        color = darkColor_.isValid()
                    ? darkColor_
                    : themedColor(themeColor(), true, QStringLiteral("ThemeColorDark1"));
    } else {
        color = lightColor_.isValid() ? lightColor_ : themeColor();
    }
    painter.setBrush(color);
    painter.drawRoundedRect(rect(), 1.5, 1.5);
}

}  // namespace qfw
