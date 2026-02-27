#include "components/widgets/state_tool_tip.h"

#include <QEnterEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

#include "common/icon.h"
#include "common/style_sheet.h"

namespace qfw {

// ============================================================================
// StateCloseButton
// ============================================================================

StateCloseButton::StateCloseButton(QWidget* parent) : QToolButton(parent) { setFixedSize(12, 12); }

void StateCloseButton::enterEvent(QEnterEvent* e) {
    Q_UNUSED(e);
    isEnter_ = true;
    update();
}

void StateCloseButton::leaveEvent(QEvent* e) {
    Q_UNUSED(e);
    isEnter_ = false;
    isPressed_ = false;
    update();
}

void StateCloseButton::mousePressEvent(QMouseEvent* e) {
    isPressed_ = true;
    update();
    QToolButton::mousePressEvent(e);
}

void StateCloseButton::mouseReleaseEvent(QMouseEvent* e) {
    isPressed_ = false;
    update();
    QToolButton::mouseReleaseEvent(e);
}

void StateCloseButton::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    if (isPressed_) {
        painter.setOpacity(0.6);
    } else if (isEnter_) {
        painter.setOpacity(0.8);
    }

    // Use opposite theme for visibility
    Theme theme = isDarkTheme() ? Theme::Light : Theme::Dark;
    const FluentIcon closeIcon(FluentIconEnum::Close);
    closeIcon.render(&painter, rect(), theme);
}

// ============================================================================
// StateToolTip
// ============================================================================

StateToolTip::StateToolTip(const QString& title, const QString& content, QWidget* parent)
    : QWidget(parent), title_(title), content_(content) {
    initUi();
}

QString StateToolTip::title() const { return title_; }

void StateToolTip::setTitle(const QString& title) {
    title_ = title;
    if (titleLabel_) {
        titleLabel_->setText(title);
        titleLabel_->adjustSize();
    }
}

QString StateToolTip::content() const { return content_; }

void StateToolTip::setContent(const QString& content) {
    content_ = content;
    if (contentLabel_) {
        contentLabel_->setText(content);
        contentLabel_->adjustSize();
    }
}

void StateToolTip::setState(bool isDone) {
    isDone_ = isDone;
    update();

    if (isDone) {
        QTimer::singleShot(1000, this, &StateToolTip::fadeOut);
    }
}

bool StateToolTip::isDone() const { return isDone_; }

QPoint StateToolTip::getSuitablePos() {
    QWidget* p = parentWidget();
    if (!p) {
        return QPoint();
    }

    for (int i = 0; i < 10; ++i) {
        int dy = i * (height() + 16);
        QPoint pos(p->width() - width() - 24, 50 + dy);
        QWidget* widget = p->childAt(pos + QPoint(2, 2));
        if (auto* tip = qobject_cast<StateToolTip*>(widget)) {
            Q_UNUSED(tip);
            pos += QPoint(0, height() + 16);
        } else {
            return pos;
        }
    }

    return QPoint(parentWidget()->width() - width() - 24, 50);
}

void StateToolTip::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    // Use opposite theme for visibility
    Theme theme = isDarkTheme() ? Theme::Light : Theme::Dark;

    if (!isDone_) {
        // Draw rotating sync icon
        painter.translate(19, 18);
        painter.rotate(rotateAngle_);
        const FluentIcon syncIcon(FluentIconEnum::Sync);
        syncIcon.render(&painter, QRect(-8, -8, 16, 16), theme);
    } else {
        // Draw completed icon (Accept = checkmark)
        const FluentIcon acceptIcon(FluentIconEnum::Accept);
        acceptIcon.render(&painter, QRect(11, 10, 16, 16), theme);
    }
}

void StateToolTip::initUi() {
    setAttribute(Qt::WA_StyledBackground);
    setProperty("qssClass", QStringLiteral("StateToolTip"));

    titleLabel_ = new QLabel(title_, this);
    contentLabel_ = new QLabel(content_, this);
    rotateTimer_ = new QTimer(this);
    opacityEffect_ = new QGraphicsOpacityEffect(this);
    opacityAnimation_ = new QPropertyAnimation(opacityEffect_, QByteArrayLiteral("opacity"), this);
    closeButton_ = new StateCloseButton(this);

    setGraphicsEffect(opacityEffect_);
    opacityEffect_->setOpacity(1);
    rotateTimer_->setInterval(50);
    contentLabel_->setMinimumWidth(200);

    connect(closeButton_, &QToolButton::clicked, this, &StateToolTip::onCloseButtonClicked);
    connect(rotateTimer_, &QTimer::timeout, this, &StateToolTip::onRotateTimerTimeout);

    setQss();
    initLayout();

    rotateTimer_->start();
}

void StateToolTip::initLayout() {
    int w = qMax(titleLabel_->sizeHint().width(), contentLabel_->sizeHint().width()) + 56;
    setFixedSize(w, 51);

    titleLabel_->move(32, 9);
    contentLabel_->move(12, 27);
    closeButton_->move(width() - 24, 19);
}

void StateToolTip::setQss() {
    titleLabel_->setObjectName(QStringLiteral("titleLabel"));
    contentLabel_->setObjectName(QStringLiteral("contentLabel"));

    qfw::setStyleSheet(this, FluentStyleSheet::StateToolTip);

    titleLabel_->adjustSize();
    contentLabel_->adjustSize();
}

void StateToolTip::fadeOut() {
    rotateTimer_->stop();

    opacityAnimation_->setDuration(200);
    opacityAnimation_->setStartValue(1.0);
    opacityAnimation_->setEndValue(0.0);

    connect(opacityAnimation_, &QPropertyAnimation::finished, this, &QObject::deleteLater);
    opacityAnimation_->start();
}

void StateToolTip::onRotateTimerTimeout() {
    rotateAngle_ = (rotateAngle_ + deltaAngle_) % 360;
    update();
}

void StateToolTip::onCloseButtonClicked() {
    emit closedSignal();
    hide();
}

}  // namespace qfw
