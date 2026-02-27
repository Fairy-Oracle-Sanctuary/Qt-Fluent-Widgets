#include "components/settings/expand_setting_card.h"

#include <QApplication>
#include <QEnterEvent>
#include <QEvent>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QScrollBar>
#include <QWheelEvent>

#include "common/config.h"
#include "common/icon.h"
#include "common/style_sheet.h"

namespace qfw {

// ==========================================================================
// ExpandButton
// ==========================================================================
ExpandButton::ExpandButton(QWidget* parent) : QAbstractButton(parent) {
    setFixedSize(30, 30);
    rotateAni_ = new QPropertyAnimation(this, "angle", this);
    connect(this, &QAbstractButton::clicked, this, &ExpandButton::onClicked);
}

qreal ExpandButton::angle() const { return angle_; }

void ExpandButton::setAngle(qreal angle) {
    angle_ = angle;
    update();
}

void ExpandButton::setExpand(bool isExpand) {
    rotateAni_->stop();
    rotateAni_->setEndValue(isExpand ? 180.0 : 0.0);
    rotateAni_->setDuration(200);
    rotateAni_->start();
}

void ExpandButton::onClicked() { setExpand(angle_ < 180.0); }

void ExpandButton::enterEvent(QEnterEvent* event) {
    setHover(true);
    QAbstractButton::enterEvent(event);
}

void ExpandButton::leaveEvent(QEvent* event) {
    setHover(false);
    QAbstractButton::leaveEvent(event);
}

void ExpandButton::mousePressEvent(QMouseEvent* event) {
    QAbstractButton::mousePressEvent(event);
    setPressed(true);
}

void ExpandButton::mouseReleaseEvent(QMouseEvent* event) {
    QAbstractButton::mouseReleaseEvent(event);
    setPressed(false);
}

void ExpandButton::setHover(bool isHover) {
    isHover_ = isHover;
    update();
}

void ExpandButton::setPressed(bool isPressed) {
    isPressed_ = isPressed;
    update();
}

void ExpandButton::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter.setPen(Qt::NoPen);

    // background
    const int r = isDarkTheme() ? 255 : 0;
    QColor color = Qt::transparent;

    if (isEnabled()) {
        if (isPressed_) {
            color = QColor(r, r, r, 10);
        } else if (isHover_) {
            color = QColor(r, r, r, 14);
        }
    } else {
        painter.setOpacity(0.36);
    }

    painter.setBrush(color);
    painter.drawRoundedRect(rect(), 4, 4);

    // icon (ArrowDown)
    painter.translate(width() / 2.0, height() / 2.0);
    painter.rotate(angle_);

    FluentIcon(FluentIconEnum::ArrowDown).render(&painter, QRect(-5, -5, 10, 10));
}

// ==========================================================================
// SpaceWidget
// ==========================================================================
SpaceWidget::SpaceWidget(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedHeight(1);
}

// ==========================================================================
// HeaderSettingCard
// ==========================================================================
HeaderSettingCard::HeaderSettingCard(const QVariant& icon, const QString& title,
                                     const QString& content, QWidget* parent)
    : SettingCard(icon, title, content, parent), expandButton_(new ExpandButton(this)) {
    hBoxLayout_->addWidget(expandButton_, 0, Qt::AlignRight);
    hBoxLayout_->addSpacing(8);

    titleLabel_->setObjectName(QStringLiteral("titleLabel"));
    installEventFilter(this);
}

ExpandButton* HeaderSettingCard::expandButton() const { return expandButton_; }

bool HeaderSettingCard::eventFilter(QObject* obj, QEvent* event) {
    if (obj == this) {
        if (event->type() == QEvent::Enter) {
            expandButton_->setProperty("hover", true);
            expandButton_->update();
        } else if (event->type() == QEvent::Leave) {
            expandButton_->setProperty("hover", false);
            expandButton_->update();
        } else if (event->type() == QEvent::MouseButtonPress) {
            auto* me = static_cast<QMouseEvent*>(event);
            if (me->button() == Qt::LeftButton) {
                expandButton_->setDown(true);
                expandButton_->update();
            }
        } else if (event->type() == QEvent::MouseButtonRelease) {
            auto* me = static_cast<QMouseEvent*>(event);
            if (me->button() == Qt::LeftButton) {
                expandButton_->setDown(false);
                expandButton_->update();
                expandButton_->click();
            }
        }
    }

    return SettingCard::eventFilter(obj, event);
}

void HeaderSettingCard::addWidget(QWidget* widget) {
    const int n = hBoxLayout_->count();
    if (n > 0) {
        hBoxLayout_->removeItem(hBoxLayout_->itemAt(n - 1));
    }

    hBoxLayout_->addWidget(widget, 0, Qt::AlignRight);
    hBoxLayout_->addSpacing(19);
    hBoxLayout_->addWidget(expandButton_, 0, Qt::AlignRight);
    hBoxLayout_->addSpacing(8);
}

void HeaderSettingCard::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    if (isDarkTheme()) {
        painter.setBrush(QColor(255, 255, 255, 13));
    } else {
        painter.setBrush(QColor(255, 255, 255, 170));
    }

    auto* p = qobject_cast<ExpandSettingCard*>(parentWidget());

    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    path.addRoundedRect(QRectF(rect().adjusted(1, 1, -1, -1)), 6, 6);

    if (p && p->property("isExpand").toBool()) {
        path.addRect(1, height() - 8, width() - 2, 8);
    }

    painter.drawPath(path.simplified());
}

// ==========================================================================
// ExpandBorderWidget
// ==========================================================================
ExpandBorderWidget::ExpandBorderWidget(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_TransparentForMouseEvents);
    if (parent) {
        parent->installEventFilter(this);
    }
}

bool ExpandBorderWidget::eventFilter(QObject* obj, QEvent* event) {
    if (obj == parent() && event->type() == QEvent::Resize) {
        auto* re = static_cast<QResizeEvent*>(event);
        resize(re->size());
    }
    return QWidget::eventFilter(obj, event);
}

void ExpandBorderWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setBrush(Qt::NoBrush);

    if (isDarkTheme()) {
        painter.setPen(QColor(0, 0, 0, 50));
    } else {
        painter.setPen(QColor(0, 0, 0, 19));
    }

    auto* p = qobject_cast<ExpandSettingCard*>(parentWidget());
    const int ch = p ? p->headerHeight() : 0;

    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 6, 6);

    if (p && ch < height()) {
        painter.drawLine(1, ch, width() - 1, ch);
    }
}

// ==========================================================================
// ExpandSettingCard
// ==========================================================================
ExpandSettingCard::ExpandSettingCard(const QVariant& icon, const QString& title,
                                     const QString& content, QWidget* parent)
    : QScrollArea(parent) {
    setProperty("qssClass", "ExpandSettingCard");

    scrollWidget_ = new QFrame(this);
    view_ = new QFrame(scrollWidget_);
    card_ = new HeaderSettingCard(icon, title, content, this);

    scrollLayout_ = new QVBoxLayout(scrollWidget_);
    viewLayout_ = new QVBoxLayout(view_);

    spaceWidget_ = new SpaceWidget(scrollWidget_);
    borderWidget_ = new ExpandBorderWidget(this);

    expandAni_ = new QPropertyAnimation(verticalScrollBar(), "value", this);

    // init widget
    setWidget(scrollWidget_);
    setWidgetResizable(true);
    setFixedHeight(card_->height());
    setViewportMargins(0, card_->height(), 0, 0);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    scrollLayout_->setContentsMargins(0, 0, 0, 0);
    scrollLayout_->setSpacing(0);
    scrollLayout_->addWidget(view_);
    scrollLayout_->addWidget(spaceWidget_);

    expandAni_->setEasingCurve(QEasingCurve::OutQuad);
    expandAni_->setDuration(200);

    view_->setObjectName(QStringLiteral("view"));
    scrollWidget_->setObjectName(QStringLiteral("scrollWidget"));

    setProperty("isExpand", false);

    qfw::setStyleSheet(card_, FluentStyleSheet::ExpandSettingCard);
    qfw::setStyleSheet(this, FluentStyleSheet::ExpandSettingCard);

    card_->installEventFilter(this);
    connect(expandAni_, &QPropertyAnimation::valueChanged, this,
            &ExpandSettingCard::onExpandValueChanged);
    connect(card_->expandButton(), &QAbstractButton::clicked, this,
            &ExpandSettingCard::toggleExpand);
}

int ExpandSettingCard::headerHeight() const { return card_ ? card_->height() : 0; }

void ExpandSettingCard::addWidget(QWidget* widget) {
    card_->addWidget(widget);
    adjustViewSize();
}

void ExpandSettingCard::wheelEvent(QWheelEvent* event) { event->ignore(); }

void ExpandSettingCard::setExpand(bool isExpand) {
    if (isExpand_ == isExpand) {
        return;
    }

    adjustViewSize();

    isExpand_ = isExpand;
    setProperty("isExpand", isExpand);
    setStyle(QApplication::style());

    if (isExpand) {
        const int h = viewLayout_->sizeHint().height();
        verticalScrollBar()->setValue(h);
        expandAni_->setStartValue(h);
        expandAni_->setEndValue(0);
    } else {
        expandAni_->setStartValue(0);
        expandAni_->setEndValue(verticalScrollBar()->maximum());
    }

    expandAni_->start();
    card_->expandButton()->setExpand(isExpand);
}

void ExpandSettingCard::toggleExpand() { setExpand(!isExpand_); }

void ExpandSettingCard::resizeEvent(QResizeEvent* event) {
    QScrollArea::resizeEvent(event);
    card_->resize(width(), card_->height());
    scrollWidget_->resize(width(), scrollWidget_->height());
}

void ExpandSettingCard::onExpandValueChanged() {
    const int vh = viewLayout_->sizeHint().height();
    const int h = viewportMargins().top();
    setFixedHeight(qMax(h + vh - verticalScrollBar()->value(), h));
}

void ExpandSettingCard::adjustViewSize() {
    const int h = viewLayout_->sizeHint().height();
    spaceWidget_->setFixedHeight(h);

    if (isExpand_) {
        setFixedHeight(card_->height() + h);
    }
}

// ==========================================================================
// GroupSeparator
// ==========================================================================
GroupSeparator::GroupSeparator(QWidget* parent) : QWidget(parent) { setFixedHeight(3); }

void GroupSeparator::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    if (isDarkTheme()) {
        painter.setPen(QColor(0, 0, 0, 50));
    } else {
        painter.setPen(QColor(0, 0, 0, 19));
    }

    painter.drawLine(0, 1, width(), 1);
}

// ==========================================================================
// GroupWidget
// ==========================================================================
GroupWidget::GroupWidget(const QVariant& icon, const QString& title, const QString& content,
                         QWidget* widget, int stretch, QWidget* parent)
    : QWidget(parent),
      iconWidget_(new SettingIconWidget(icon, this)),
      titleLabel_(new QLabel(title, this)),
      contentLabel_(new QLabel(content, this)),
      widget_(widget),
      hBoxLayout_(new QHBoxLayout(this)),
      vBoxLayout_(new QVBoxLayout()) {
    if (content.isEmpty()) {
        contentLabel_->hide();
    }

    iconWidget_->setFixedSize(16, 16);
    setMinimumHeight(60);
    setIcon(icon);

    hBoxLayout_->setSpacing(16);
    hBoxLayout_->setContentsMargins(48, 12, 48, 12);
    hBoxLayout_->setAlignment(Qt::AlignVCenter);

    vBoxLayout_->setSpacing(0);
    vBoxLayout_->setContentsMargins(0, 0, 0, 0);
    vBoxLayout_->setAlignment(Qt::AlignVCenter);

    hBoxLayout_->addWidget(iconWidget_, 0, Qt::AlignLeft);
    hBoxLayout_->addLayout(vBoxLayout_);
    vBoxLayout_->addWidget(titleLabel_, 0, Qt::AlignLeft);
    vBoxLayout_->addWidget(contentLabel_, 0, Qt::AlignLeft);

    hBoxLayout_->addStretch(1);

    if (widget_) {
        widget_->setParent(this);
        hBoxLayout_->addWidget(widget_, stretch);
    }

    titleLabel_->setObjectName(QStringLiteral("titleLabel"));
    contentLabel_->setObjectName(QStringLiteral("contentLabel"));
}

void GroupWidget::setTitle(const QString& title) { titleLabel_->setText(title); }

void GroupWidget::setContent(const QString& content) {
    contentLabel_->setText(content);
    contentLabel_->setVisible(!content.isEmpty());
}

void GroupWidget::setIconSize(int width, int height) { iconWidget_->setFixedSize(width, height); }

void GroupWidget::setIcon(const QVariant& icon) {
    iconWidget_->setIcon(icon);
    iconWidget_->setHidden(iconWidget_->icon().isNull());
}

// ==========================================================================
// ExpandGroupSettingCard
// ==========================================================================
ExpandGroupSettingCard::ExpandGroupSettingCard(const QVariant& icon, const QString& title,
                                               const QString& content, QWidget* parent)
    : ExpandSettingCard(icon, title, content, parent) {
    viewLayout_->setContentsMargins(0, 0, 0, 0);
    viewLayout_->setSpacing(0);
}

void ExpandGroupSettingCard::addGroupWidget(QWidget* widget) {
    if (!widget) {
        return;
    }

    if (viewLayout_->count() >= 1) {
        viewLayout_->addWidget(new GroupSeparator(view_));
    }

    widget->setParent(view_);
    widgets_.append(widget);
    viewLayout_->addWidget(widget);
    adjustViewSize();
}

GroupWidget* ExpandGroupSettingCard::addGroup(const QVariant& icon, const QString& title,
                                              const QString& content, QWidget* widget,
                                              int stretch) {
    auto* group = new GroupWidget(icon, title, content, widget, stretch);
    addGroupWidget(group);
    return group;
}

void ExpandGroupSettingCard::removeGroupWidget(QWidget* widget) {
    if (!widget || !widgets_.contains(widget)) {
        return;
    }

    const int layoutIndex = viewLayout_->indexOf(widget);
    const int index = widgets_.indexOf(widget);

    viewLayout_->removeWidget(widget);
    widgets_.removeAll(widget);

    if (widgets_.isEmpty()) {
        adjustViewSize();
        return;
    }

    // remove separator
    if (layoutIndex >= 1) {
        if (auto* sep = viewLayout_->itemAt(layoutIndex - 1)->widget()) {
            viewLayout_->removeWidget(sep);
            sep->deleteLater();
        }
    } else if (index == 0) {
        if (auto* sep = viewLayout_->itemAt(0)->widget()) {
            viewLayout_->removeWidget(sep);
            sep->deleteLater();
        }
    }

    adjustViewSize();
}

void ExpandGroupSettingCard::adjustViewSize() {
    int h = 0;
    for (const auto& wptr : widgets_) {
        QWidget* w = wptr.data();
        if (w) {
            h += w->sizeHint().height() + 3;
        }
    }

    spaceWidget_->setFixedHeight(h);

    if (isExpand_) {
        setFixedHeight(card_->height() + h);
    }
}

// ==========================================================================
// SimpleExpandGroupSettingCard
// ==========================================================================
void SimpleExpandGroupSettingCard::adjustViewSize() {
    const int h = viewLayout_->sizeHint().height();
    spaceWidget_->setFixedHeight(h);

    if (isExpand_) {
        setFixedHeight(card_->height() + h);
    }
}

}  // namespace qfw
