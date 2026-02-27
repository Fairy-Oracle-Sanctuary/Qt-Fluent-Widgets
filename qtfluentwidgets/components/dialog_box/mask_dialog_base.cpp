#include "components/dialog_box/mask_dialog_base.h"
#include "common/config.h"

#include <QEasingCurve>
#include <QApplication>

namespace qfw {

MaskDialogBase::MaskDialogBase(QWidget* parent) : QDialog(parent) {
    hBoxLayout = new QHBoxLayout(this);
    windowMask = new QWidget(this);
    widget = new QFrame(this);
    widget->setObjectName(QStringLiteral("centerWidget"));

    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    if (parent) {
        setGeometry(0, 0, parent->width(), parent->height());
    }

    int c = isDarkTheme() ? 0 : 255;
    windowMask->resize(size());
    windowMask->setStyleSheet(QStringLiteral("background: rgba(%1, %1, %1, 0.6)").arg(c));
    
    hBoxLayout->addWidget(widget);
    setShadowEffect();

    if (window()) {
        window()->installEventFilter(this);
    }
    windowMask->installEventFilter(this);
    widget->installEventFilter(this);
}

MaskDialogBase::~MaskDialogBase() {
}

void MaskDialogBase::setShadowEffect(int blurRadius, const QPoint& offset, const QColor& color) {
    auto* shadowEffect = new QGraphicsDropShadowEffect(widget);
    shadowEffect->setBlurRadius(blurRadius);
    shadowEffect->setOffset(offset.x(), offset.y());
    shadowEffect->setColor(color);
    widget->setGraphicsEffect(nullptr);
    widget->setGraphicsEffect(shadowEffect);
}

void MaskDialogBase::setMaskColor(const QColor& color) {
    windowMask->setStyleSheet(QStringLiteral("background: rgba(%1, %2, %3, %4)")
        .arg(color.red()).arg(color.green()).arg(color.blue()).arg(color.alpha()));
}

void MaskDialogBase::showEvent(QShowEvent* e) {
    auto* opacityEffect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(opacityEffect);
    
    auto* opacityAni = new QPropertyAnimation(opacityEffect, "opacity", this);
    opacityAni->setStartValue(0.0);
    opacityAni->setEndValue(1.0);
    opacityAni->setDuration(200);
    opacityAni->setEasingCurve(QEasingCurve::InSine);
    connect(opacityAni, &QPropertyAnimation::finished, this, [this]() {
        setGraphicsEffect(nullptr);
    });
    opacityAni->start(QAbstractAnimation::DeleteWhenStopped);
    
    QDialog::showEvent(e);
}

void MaskDialogBase::done(int code) {
    widget->setGraphicsEffect(nullptr);
    auto* opacityEffect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(opacityEffect);
    
    auto* opacityAni = new QPropertyAnimation(opacityEffect, "opacity", this);
    opacityAni->setStartValue(1.0);
    opacityAni->setEndValue(0.0);
    opacityAni->setDuration(100);
    connect(opacityAni, &QPropertyAnimation::finished, this, [this, code]() {
        onDone(code);
    });
    opacityAni->start(QAbstractAnimation::DeleteWhenStopped);
}

void MaskDialogBase::onDone(int code) {
    setGraphicsEffect(nullptr);
    QDialog::done(code);
}

bool MaskDialogBase::isClosableOnMaskClicked() const {
    return isClosableOnMaskClicked_;
}

void MaskDialogBase::setClosableOnMaskClicked(bool isClosable) {
    isClosableOnMaskClicked_ = isClosable;
}

void MaskDialogBase::setDraggable(bool draggable) {
    isDraggable_ = draggable;
}

bool MaskDialogBase::isDraggable() const {
    return isDraggable_;
}

void MaskDialogBase::resizeEvent(QResizeEvent* e) {
    windowMask->resize(size());
    QDialog::resizeEvent(e);
}

bool MaskDialogBase::eventFilter(QObject* obj, QEvent* e) {
    if (obj == window()) {
        if (e->type() == QEvent::Resize) {
            auto* re = static_cast<QResizeEvent*>(e);
            resize(re->size());
        }
    } else if (obj == windowMask) {
        if (e->type() == QEvent::MouseButtonRelease) {
            auto* me = static_cast<QMouseEvent*>(e);
            if (me->button() == Qt::LeftButton && isClosableOnMaskClicked()) {
                reject();
            }
        }
    } else if (obj == widget && isDraggable()) {
        if (e->type() == QEvent::MouseButtonPress) {
            auto* me = static_cast<QMouseEvent*>(e);
            if (me->button() == Qt::LeftButton) {
                if (!widget->contentsRect().contains(me->pos())) {
                    dragPos_ = me->pos();
                    return true;
                }
            }
        } else if (e->type() == QEvent::MouseMove && !dragPos_.isNull()) {
            auto* me = static_cast<QMouseEvent*>(e);
            QPoint pos = widget->pos() + me->pos() - dragPos_;
            pos.setX(qMax(0, qMin(pos.x(), width() - widget->width())));
            pos.setY(qMax(0, qMin(pos.y(), height() - widget->height())));
            widget->move(pos);
            return true;
        } else if (e->type() == QEvent::MouseButtonRelease) {
            dragPos_ = QPoint();
        }
    }
    
    return QDialog::eventFilter(obj, e);
}

} // namespace qfw
