#include "components/widgets/card_widget.h"

#include <QEnterEvent>
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

#include "common/config.h"
#include "common/font.h"
#include "common/style_sheet.h"
#include "components/widgets/label.h"

namespace qfw {

// ==========================================================================
// CardWidget
// ==========================================================================
CardWidget::CardWidget(QWidget* parent) : QFrame(parent) {
    setProperty("qssClass", "CardWidget");

    backgroundColor_ = normalBackgroundColor();
    bgAni_ = new QPropertyAnimation(this, "backgroundColor", this);
    bgAni_->setDuration(150);

    setBorderRadius(5);
    setMouseTracking(true);
}

void CardWidget::setClickEnabled(bool enabled) {
    clickEnabled_ = enabled;
    update();
}

bool CardWidget::isClickEnabled() const { return clickEnabled_; }

QColor CardWidget::backgroundColor() const { return backgroundColor_; }

void CardWidget::setBackgroundColor(const QColor& c) {
    backgroundColor_ = c;
    update();
}

int CardWidget::borderRadius() const { return borderRadius_; }

void CardWidget::setBorderRadius(int radius) {
    borderRadius_ = radius;
    update();
}

QColor CardWidget::normalBackgroundColor() const {
    return QColor(255, 255, 255, isDarkTheme() ? 13 : 170);
}

QColor CardWidget::hoverBackgroundColor() const {
    return QColor(255, 255, 255, isDarkTheme() ? 21 : 64);
}

QColor CardWidget::pressedBackgroundColor() const {
    return QColor(255, 255, 255, isDarkTheme() ? 8 : 64);
}

void CardWidget::updateBackgroundColor(const QColor& target) {
    if (!bgAni_) {
        setBackgroundColor(target);
        return;
    }

    bgAni_->stop();
    bgAni_->setEndValue(target);
    bgAni_->start();
}

void CardWidget::enterEvent(QEnterEvent* e) {
    isHover_ = true;
    updateBackgroundColor(hoverBackgroundColor());
    QFrame::enterEvent(e);
}

void CardWidget::leaveEvent(QEvent* e) {
    isHover_ = false;
    isPressed_ = false;
    updateBackgroundColor(normalBackgroundColor());
    QFrame::leaveEvent(e);
}

void CardWidget::mousePressEvent(QMouseEvent* e) {
    isPressed_ = true;
    updateBackgroundColor(pressedBackgroundColor());
    QFrame::mousePressEvent(e);
}

void CardWidget::mouseReleaseEvent(QMouseEvent* e) {
    QFrame::mouseReleaseEvent(e);

    const bool inside = rect().contains(e->pos());
    if (inside) {
        emit clicked();
    }

    isPressed_ = false;
    updateBackgroundColor(isHover_ ? hoverBackgroundColor() : normalBackgroundColor());
}

void CardWidget::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    const int w = width();
    const int h = height();
    const int r = borderRadius_;
    const int d = 2 * r;

    const bool dark = isDarkTheme();

    // draw top border
    {
        QPainterPath path;
        path.arcMoveTo(1, h - d - 1, d, d, 240);
        path.arcTo(1, h - d - 1, d, d, 225, -60);
        path.lineTo(1, r);
        path.arcTo(1, 1, d, d, -180, -90);
        path.lineTo(w - r, 1);
        path.arcTo(w - d - 1, 1, d, d, 90, -90);
        path.lineTo(w - 1, h - r);
        path.arcTo(w - d - 1, h - d - 1, d, d, 0, -60);

        QColor topBorderColor(0, 0, 0, 20);
        if (dark) {
            if (isPressed_) {
                topBorderColor = QColor(255, 255, 255, 18);
            } else if (isHover_) {
                topBorderColor = QColor(255, 255, 255, 13);
            }
        } else {
            topBorderColor = QColor(0, 0, 0, 15);
        }

        painter.strokePath(path, topBorderColor);

        // draw bottom border
        QPainterPath path2;
        path2.arcMoveTo(1, h - d - 1, d, d, 240);
        path2.arcTo(1, h - d - 1, d, d, 240, 30);
        path2.lineTo(w - r - 1, h - 1);
        path2.arcTo(w - d - 1, h - d - 1, d, d, 270, 30);

        QColor bottomBorderColor = topBorderColor;
        if (!dark && isHover_ && !isPressed_) {
            bottomBorderColor = QColor(0, 0, 0, 27);
        }
        painter.strokePath(path2, bottomBorderColor);
    }

    // draw background
    painter.setPen(Qt::NoPen);
    const QRect rectBg = rect().adjusted(1, 1, -1, -1);
    painter.setBrush(backgroundColor_);
    painter.drawRoundedRect(rectBg, r, r);
}

// ==========================================================================
// SimpleCardWidget
// ==========================================================================
SimpleCardWidget::SimpleCardWidget(QWidget* parent) : CardWidget(parent) {
    setProperty("qssClass", "SimpleCardWidget");
}

QColor SimpleCardWidget::hoverBackgroundColor() const { return normalBackgroundColor(); }

QColor SimpleCardWidget::pressedBackgroundColor() const { return normalBackgroundColor(); }

void SimpleCardWidget::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setBrush(backgroundColor());

    if (isDarkTheme()) {
        painter.setPen(QColor(0, 0, 0, 48));
    } else {
        painter.setPen(QColor(0, 0, 0, 12));
    }

    const int r = borderRadius();
    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), r, r);
}

// ==========================================================================
// ElevatedCardWidget
// ==========================================================================
ElevatedCardWidget::ElevatedCardWidget(QWidget* parent) : SimpleCardWidget(parent) {
    setProperty("qssClass", "ElevatedCardWidget");

    // The C++ DropShadowAnimation animates to (normalColor, normalBlurRadius) on hover and
    // to transparent/0 on leave.
    shadowAni_ = new DropShadowAnimation(this, QColor(0, 0, 0, 20), 38);

    if (auto* eff = qobject_cast<QGraphicsDropShadowEffect*>(graphicsEffect())) {
        eff->setOffset(0, 5);
    }

    elevatedAni_ = new QPropertyAnimation(this, QByteArrayLiteral("pos"), this);
    elevatedAni_->setDuration(100);

    originalPos_ = pos();
    setBorderRadius(8);
}

void ElevatedCardWidget::startElevateAnimation(const QPoint& start, const QPoint& end) {
    if (!elevatedAni_) {
        move(end);
        return;
    }
    elevatedAni_->stop();
    elevatedAni_->setStartValue(start);
    elevatedAni_->setEndValue(end);
    elevatedAni_->start();
}

void ElevatedCardWidget::enterEvent(QEnterEvent* e) {
    SimpleCardWidget::enterEvent(e);

    if (elevatedAni_ && elevatedAni_->state() != QAbstractAnimation::Running) {
        originalPos_ = pos();
    }

    startElevateAnimation(pos(), pos() - QPoint(0, 3));
}

void ElevatedCardWidget::leaveEvent(QEvent* e) {
    SimpleCardWidget::leaveEvent(e);
    startElevateAnimation(pos(), originalPos_);
}

void ElevatedCardWidget::mousePressEvent(QMouseEvent* e) {
    SimpleCardWidget::mousePressEvent(e);
    startElevateAnimation(pos(), originalPos_);
}

QColor ElevatedCardWidget::hoverBackgroundColor() const {
    return isDarkTheme() ? QColor(255, 255, 255, 16) : QColor(255, 255, 255);
}

QColor ElevatedCardWidget::pressedBackgroundColor() const {
    return QColor(255, 255, 255, isDarkTheme() ? 6 : 118);
}

// ==========================================================================
// CardSeparator
// ==========================================================================
CardSeparator::CardSeparator(QWidget* parent) : QWidget(parent) { setFixedHeight(3); }

void CardSeparator::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    if (isDarkTheme()) {
        painter.setPen(QColor(255, 255, 255, 46));
    } else {
        painter.setPen(QColor(0, 0, 0, 12));
    }

    painter.drawLine(2, 1, width() - 2, 1);
}

// ==========================================================================
// HeaderCardWidget
// ==========================================================================
HeaderCardWidget::HeaderCardWidget(QWidget* parent) : SimpleCardWidget(parent) {
    headerView_ = new QWidget(this);
    headerLabel_ = new QLabel(this);
    separator_ = new CardSeparator(this);
    view_ = new QWidget(this);

    vBoxLayout_ = new QVBoxLayout(this);
    headerLayout_ = new QHBoxLayout(headerView_);
    viewLayout_ = new QHBoxLayout(view_);

    headerLayout_->addWidget(headerLabel_);
    headerLayout_->setContentsMargins(24, 0, 16, 0);
    headerView_->setFixedHeight(48);

    vBoxLayout_->setSpacing(0);
    vBoxLayout_->setContentsMargins(0, 0, 0, 0);
    vBoxLayout_->addWidget(headerView_);
    vBoxLayout_->addWidget(separator_);
    vBoxLayout_->addWidget(view_);

    viewLayout_->setContentsMargins(24, 24, 24, 24);

    qfw::setFont(headerLabel_, 15, QFont::DemiBold);

    view_->setObjectName(QStringLiteral("view"));
    headerView_->setObjectName(QStringLiteral("headerView"));
    headerLabel_->setObjectName(QStringLiteral("headerLabel"));

    setProperty("qssClass", "HeaderCardWidget");
    qfw::setStyleSheet(this, qfw::FluentStyleSheet::CardWidget);

    postInit();
}

HeaderCardWidget::HeaderCardWidget(const QString& title, QWidget* parent)
    : HeaderCardWidget(parent) {
    setTitle(title);
}

QString HeaderCardWidget::title() const { return headerLabel_ ? headerLabel_->text() : QString(); }

void HeaderCardWidget::setTitle(const QString& title) {
    if (headerLabel_) {
        headerLabel_->setText(title);
    }
}

void HeaderCardWidget::postInit() {}

// ==========================================================================
// CardGroupWidget
// ==========================================================================
CardGroupWidget::CardGroupWidget(const QIcon& icon, const QString& title, const QString& content,
                                 QWidget* parent)
    : QWidget(parent) {
    vBoxLayout_ = new QVBoxLayout(this);
    hBoxLayout_ = new QHBoxLayout();

    iconWidget_ = new qfw::IconWidget(icon, this);
    titleLabel_ = new BodyLabel(title, this);
    contentLabel_ = new CaptionLabel(content, this);
    textLayout_ = new QVBoxLayout();

    separator_ = new CardSeparator(this);

    initWidget();
}

CardGroupWidget::CardGroupWidget(const FluentIconBase& icon, const QString& title,
                                 const QString& content, QWidget* parent)
    : CardGroupWidget(icon.qicon(), title, content, parent) {}

CardGroupWidget::CardGroupWidget(const QString& iconPath, const QString& title,
                                 const QString& content, QWidget* parent)
    : CardGroupWidget(QIcon(iconPath), title, content, parent) {}

void CardGroupWidget::initWidget() {
    if (separator_) {
        separator_->hide();
    }

    if (iconWidget_) {
        iconWidget_->setFixedSize(20, 20);
    }

    if (contentLabel_) {
        contentLabel_->setTextColor(QColor(96, 96, 96), QColor(206, 206, 206));
    }

    vBoxLayout_->setSpacing(0);
    vBoxLayout_->setContentsMargins(0, 0, 0, 0);
    vBoxLayout_->addLayout(hBoxLayout_);
    vBoxLayout_->addWidget(separator_);

    textLayout_->addWidget(titleLabel_);
    textLayout_->addWidget(contentLabel_);

    hBoxLayout_->addWidget(iconWidget_);
    hBoxLayout_->addLayout(textLayout_);
    hBoxLayout_->addStretch(1);

    hBoxLayout_->setSpacing(15);
    hBoxLayout_->setContentsMargins(24, 10, 24, 10);
    textLayout_->setContentsMargins(0, 0, 0, 0);
    textLayout_->setSpacing(0);
    hBoxLayout_->setAlignment(Qt::AlignLeft);
    textLayout_->setAlignment(Qt::AlignCenter);
}

QString CardGroupWidget::title() const { return titleLabel_ ? titleLabel_->text() : QString(); }

void CardGroupWidget::setTitle(const QString& text) {
    if (titleLabel_) {
        titleLabel_->setText(text);
    }
}

QString CardGroupWidget::content() const {
    return contentLabel_ ? contentLabel_->text() : QString();
}

void CardGroupWidget::setContent(const QString& text) {
    if (contentLabel_) {
        contentLabel_->setText(text);
    }
}

QIcon CardGroupWidget::icon() const { return iconWidget_ ? iconWidget_->icon() : QIcon(); }

void CardGroupWidget::setIcon(const QIcon& icon) {
    if (iconWidget_) {
        iconWidget_->setIcon(icon);
    }
}

void CardGroupWidget::setIcon(const FluentIconBase& icon) { setIcon(icon.qicon()); }

void CardGroupWidget::setIcon(const QString& iconPath) { setIcon(QIcon(iconPath)); }

void CardGroupWidget::setIconSize(const QSize& size) {
    if (iconWidget_) {
        iconWidget_->setFixedSize(size);
    }
}

void CardGroupWidget::setSeparatorVisible(bool visible) {
    if (separator_) {
        separator_->setVisible(visible);
    }
}

bool CardGroupWidget::isSeparatorVisible() const { return separator_ && separator_->isVisible(); }

void CardGroupWidget::addWidget(QWidget* widget, int stretch) {
    if (hBoxLayout_) {
        hBoxLayout_->addWidget(widget, stretch);
    }
}

// ==========================================================================
// GroupHeaderCardWidget
// ==========================================================================
void GroupHeaderCardWidget::postInit() {
    HeaderCardWidget::postInit();

    groupLayout_ = new QVBoxLayout();
    groupLayout_->setSpacing(0);
    groupLayout_->setContentsMargins(0, 0, 0, 0);

    if (viewLayout_) {
        viewLayout_->setContentsMargins(0, 0, 0, 0);
        viewLayout_->addLayout(groupLayout_);
    }
}

CardGroupWidget* GroupHeaderCardWidget::addGroup(const QIcon& icon, const QString& title,
                                                 const QString& content, QWidget* widget,
                                                 int stretch) {
    auto* group = new CardGroupWidget(icon, title, content, this);
    group->addWidget(widget, stretch);

    if (!groupWidgets_.isEmpty()) {
        if (groupWidgets_.last()) {
            groupWidgets_.last()->setSeparatorVisible(true);
        }
    }

    if (groupLayout_) {
        groupLayout_->addWidget(group);
    }

    groupWidgets_.append(group);
    return group;
}

CardGroupWidget* GroupHeaderCardWidget::addGroup(const FluentIconBase& icon, const QString& title,
                                                 const QString& content, QWidget* widget,
                                                 int stretch) {
    return addGroup(icon.qicon(), title, content, widget, stretch);
}

CardGroupWidget* GroupHeaderCardWidget::addGroup(const QString& iconPath, const QString& title,
                                                 const QString& content, QWidget* widget,
                                                 int stretch) {
    return addGroup(QIcon(iconPath), title, content, widget, stretch);
}

int GroupHeaderCardWidget::groupCount() const { return groupWidgets_.size(); }

}  // namespace qfw
