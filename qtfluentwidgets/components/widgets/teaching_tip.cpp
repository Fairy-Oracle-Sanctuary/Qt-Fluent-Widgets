#include "components/widgets/teaching_tip.h"

#include <QCloseEvent>
#include <QEvent>
#include <QShowEvent>
#include <QTimer>

#include "common/screen.h"
#include "common/style_sheet.h"
#include "components/widgets/flyout.h"

namespace qfw {

// TeachingTipView implementation

TeachingTipView::TeachingTipView(const QString& title, const QString& content, QWidget* parent)
    : FlyoutView(title, content, parent) {
    init(TeachingTipTailPosition::Bottom);
}

TeachingTipView::TeachingTipView(const QString& title, const QString& content, const QIcon& icon,
                                 QWidget* parent)
    : FlyoutView(title, content, icon, parent) {
    init(TeachingTipTailPosition::Bottom);
}

TeachingTipView::TeachingTipView(const QString& title, const QString& content,
                                 const FluentIconBase& icon, QWidget* parent)
    : FlyoutView(title, content, icon, parent) {
    init(TeachingTipTailPosition::Bottom);
}

TeachingTipView::TeachingTipView(const QString& title, const QString& content, const QIcon& icon,
                                 const QVariant& image, bool isClosable,
                                 TeachingTipTailPosition tailPosition, QWidget* parent)
    : FlyoutView(title, content, icon, parent) {
    setClosable(isClosable);
    if (image.canConvert<QPixmap>()) {
        setImage(image.value<QPixmap>());
    } else if (image.canConvert<QImage>()) {
        setImage(image.value<QImage>());
    } else if (image.canConvert<QString>()) {
        setImage(image.toString());
    }
    init(tailPosition);
}

TeachingTipView::TeachingTipView(const QString& title, const QString& content,
                                 const FluentIconBase& icon, const QVariant& image, bool isClosable,
                                 TeachingTipTailPosition tailPosition, QWidget* parent)
    : FlyoutView(title, content, icon, parent) {
    setClosable(isClosable);
    if (image.canConvert<QPixmap>()) {
        setImage(image.value<QPixmap>());
    } else if (image.canConvert<QImage>()) {
        setImage(image.value<QImage>());
    } else if (image.canConvert<QString>()) {
        setImage(image.toString());
    }
    init(tailPosition);
}

void TeachingTipView::init(TeachingTipTailPosition tailPosition) {
    manager_ = TeachingTipManager::make(tailPosition);
    hBoxLayout_ = new QHBoxLayout();
    hBoxLayout_->setContentsMargins(0, 0, 0, 0);
}

TeachingTipManager* TeachingTipView::manager() const { return manager_; }

void TeachingTipView::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);
    // TeachingTipView does not paint itself - the bubble handles painting
}

// TeachTipBubble implementation

TeachTipBubble::TeachTipBubble(FlyoutViewBase* view, TeachingTipTailPosition tailPosition,
                               QWidget* parent)
    : QWidget(parent), view_(view) {
    manager_ = TeachingTipManager::make(tailPosition);
    hBoxLayout_ = new QHBoxLayout(this);

    manager_->doLayout(this);
    if (view_) {
        hBoxLayout_->addWidget(view_);
    }
}

void TeachTipBubble::setView(QWidget* view) {
    if (view_) {
        hBoxLayout_->removeWidget(qobject_cast<QWidget*>(view_));
        qobject_cast<QWidget*>(view_)->deleteLater();
    }
    view_ = qobject_cast<FlyoutViewBase*>(view);
    if (view_) {
        hBoxLayout_->addWidget(view_);
    }
}

FlyoutViewBase* TeachTipBubble::view() const { return view_; }

TeachingTipManager* TeachTipBubble::manager() const { return manager_; }

QHBoxLayout* TeachTipBubble::hBoxLayout() const { return hBoxLayout_; }

void TeachTipBubble::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    painter.setBrush(isDarkTheme() ? QColor(40, 40, 40) : QColor(248, 248, 248));
    painter.setPen(isDarkTheme() ? QColor(23, 23, 23) : QColor(0, 0, 0, 17));

    manager_->draw(this, painter);
}

// TeachingTipManager implementation

TeachingTipManager::TeachingTipManager(QObject* parent) : QObject(parent) {}

void TeachingTipManager::doLayout(TeachTipBubble* tip) {
    if (tip && tip->hBoxLayout()) {
        tip->hBoxLayout()->setContentsMargins(0, 0, 0, 0);
    }
}

ImagePosition TeachingTipManager::imagePosition() const { return ImagePosition::Top; }

QPoint TeachingTipManager::position(TeachingTip* tip) const {
    QPoint pos = calculatePosition(tip);
    int x = pos.x();
    int y = pos.y();

    QRect rect = getCurrentScreenGeometry(true);
    x = qMax(rect.left(), qMin(pos.x(), rect.right() - tip->width() - 4));
    y = qMax(rect.top(), qMin(pos.y(), rect.bottom() - tip->height() - 4));

    return QPoint(x, y);
}

void TeachingTipManager::draw(TeachTipBubble* tip, QPainter& painter) {
    QRect rect = tip->rect().adjusted(1, 1, -1, -1);
    painter.drawRoundedRect(rect, 8, 8);
}

QPoint TeachingTipManager::calculatePosition(TeachingTip* tip) const { return tip->pos(); }

TeachingTipManager* TeachingTipManager::make(TeachingTipTailPosition position) {
    switch (position) {
        case TeachingTipTailPosition::Top:
            return new TopTailTeachingTipManager();
        case TeachingTipTailPosition::Bottom:
            return new BottomTailTeachingTipManager();
        case TeachingTipTailPosition::Left:
            return new LeftTailTeachingTipManager();
        case TeachingTipTailPosition::Right:
            return new RightTailTeachingTipManager();
        case TeachingTipTailPosition::TopLeft:
            return new TopLeftTailTeachingTipManager();
        case TeachingTipTailPosition::TopRight:
            return new TopRightTailTeachingTipManager();
        case TeachingTipTailPosition::BottomLeft:
            return new BottomLeftTailTeachingTipManager();
        case TeachingTipTailPosition::BottomRight:
            return new BottomRightTailTeachingTipManager();
        case TeachingTipTailPosition::LeftTop:
            return new LeftTopTailTeachingTipManager();
        case TeachingTipTailPosition::LeftBottom:
            return new LeftBottomTailTeachingTipManager();
        case TeachingTipTailPosition::RightTop:
            return new RightTopTailTeachingTipManager();
        case TeachingTipTailPosition::RightBottom:
            return new RightBottomTailTeachingTipManager();
        case TeachingTipTailPosition::None:
        default:
            return new TeachingTipManager();
    }
}

// TopTailTeachingTipManager implementation

TopTailTeachingTipManager::TopTailTeachingTipManager(QObject* parent)
    : TeachingTipManager(parent) {}

void TopTailTeachingTipManager::doLayout(TeachTipBubble* tip) {
    if (tip && tip->hBoxLayout()) {
        tip->hBoxLayout()->setContentsMargins(0, 8, 0, 0);
    }
}

ImagePosition TopTailTeachingTipManager::imagePosition() const { return ImagePosition::Bottom; }

void TopTailTeachingTipManager::draw(TeachTipBubble* tip, QPainter& painter) {
    int w = tip->width();
    int h = tip->height();
    int pt = tip->hBoxLayout()->contentsMargins().top();

    QPainterPath path;
    path.addRoundedRect(1, pt, w - 2, h - pt - 1, 8, 8);
    path.addPolygon(
        QPolygonF({QPointF(w / 2.0 - 7, pt), QPointF(w / 2.0, 1), QPointF(w / 2.0 + 7, pt)}));

    painter.drawPath(path.simplified());
}

QPoint TopTailTeachingTipManager::calculatePosition(TeachingTip* tip) const {
    QWidget* target = tip->target();
    if (!target) return tip->pos();

    QPoint pos = target->mapToGlobal(QPoint(0, target->height()));
    int x = pos.x() + target->width() / 2 - tip->sizeHint().width() / 2;
    int y = pos.y() - tip->layout()->contentsMargins().top();
    return QPoint(x, y);
}

// BottomTailTeachingTipManager implementation

BottomTailTeachingTipManager::BottomTailTeachingTipManager(QObject* parent)
    : TeachingTipManager(parent) {}

void BottomTailTeachingTipManager::doLayout(TeachTipBubble* tip) {
    if (tip && tip->hBoxLayout()) {
        tip->hBoxLayout()->setContentsMargins(0, 0, 0, 8);
    }
}

void BottomTailTeachingTipManager::draw(TeachTipBubble* tip, QPainter& painter) {
    int w = tip->width();
    int h = tip->height();
    int pb = tip->hBoxLayout()->contentsMargins().bottom();

    QPainterPath path;
    path.addRoundedRect(1, 1, w - 2, h - pb - 1, 8, 8);
    path.addPolygon(QPolygonF(
        {QPointF(w / 2.0 - 7, h - pb), QPointF(w / 2.0, h - 1), QPointF(w / 2.0 + 7, h - pb)}));

    painter.drawPath(path.simplified());
}

QPoint BottomTailTeachingTipManager::calculatePosition(TeachingTip* tip) const {
    QWidget* target = tip->target();
    if (!target) return tip->pos();

    QPoint pos = target->mapToGlobal(QPoint());
    int x = pos.x() + target->width() / 2 - tip->sizeHint().width() / 2;
    int y = pos.y() - tip->sizeHint().height() + tip->layout()->contentsMargins().bottom();
    return QPoint(x, y);
}

// LeftTailTeachingTipManager implementation

LeftTailTeachingTipManager::LeftTailTeachingTipManager(QObject* parent)
    : TeachingTipManager(parent) {}

void LeftTailTeachingTipManager::doLayout(TeachTipBubble* tip) {
    if (tip && tip->hBoxLayout()) {
        tip->hBoxLayout()->setContentsMargins(8, 0, 0, 0);
    }
}

ImagePosition LeftTailTeachingTipManager::imagePosition() const { return ImagePosition::Right; }

void LeftTailTeachingTipManager::draw(TeachTipBubble* tip, QPainter& painter) {
    int w = tip->width();
    int h = tip->height();
    int pl = 8;

    QPainterPath path;
    path.addRoundedRect(pl, 1, w - pl - 2, h - 2, 8, 8);
    path.addPolygon(
        QPolygonF({QPointF(pl, h / 2.0 - 7), QPointF(1, h / 2.0), QPointF(pl, h / 2.0 + 7)}));

    painter.drawPath(path.simplified());
}

QPoint LeftTailTeachingTipManager::calculatePosition(TeachingTip* tip) const {
    QWidget* target = tip->target();
    if (!target) return tip->pos();

    QMargins m = tip->layout()->contentsMargins();
    QPoint pos = target->mapToGlobal(QPoint(target->width(), 0));
    int x = pos.x() - m.left();
    int y = pos.y() - tip->view()->sizeHint().height() / 2 + target->height() / 2 - m.top();
    return QPoint(x, y);
}

// RightTailTeachingTipManager implementation

RightTailTeachingTipManager::RightTailTeachingTipManager(QObject* parent)
    : TeachingTipManager(parent) {}

void RightTailTeachingTipManager::doLayout(TeachTipBubble* tip) {
    if (tip && tip->hBoxLayout()) {
        tip->hBoxLayout()->setContentsMargins(0, 0, 8, 0);
    }
}

ImagePosition RightTailTeachingTipManager::imagePosition() const { return ImagePosition::Left; }

void RightTailTeachingTipManager::draw(TeachTipBubble* tip, QPainter& painter) {
    int w = tip->width();
    int h = tip->height();
    int pr = 8;

    QPainterPath path;
    path.addRoundedRect(1, 1, w - pr - 1, h - 2, 8, 8);
    path.addPolygon(QPolygonF(
        {QPointF(w - pr, h / 2.0 - 7), QPointF(w - 1, h / 2.0), QPointF(w - pr, h / 2.0 + 7)}));

    painter.drawPath(path.simplified());
}

QPoint RightTailTeachingTipManager::calculatePosition(TeachingTip* tip) const {
    QWidget* target = tip->target();
    if (!target) return tip->pos();

    QMargins m = tip->layout()->contentsMargins();
    QPoint pos = target->mapToGlobal(QPoint(0, 0));
    int x = pos.x() - tip->sizeHint().width() + m.right();
    int y = pos.y() - tip->view()->sizeHint().height() / 2 + target->height() / 2 - m.top();
    return QPoint(x, y);
}

// TopLeftTailTeachingTipManager implementation

TopLeftTailTeachingTipManager::TopLeftTailTeachingTipManager(QObject* parent)
    : TopTailTeachingTipManager(parent) {}

void TopLeftTailTeachingTipManager::draw(TeachTipBubble* tip, QPainter& painter) {
    int w = tip->width();
    int h = tip->height();
    int pt = tip->hBoxLayout()->contentsMargins().top();

    QPainterPath path;
    path.addRoundedRect(1, pt, w - 2, h - pt - 1, 8, 8);
    path.addPolygon(QPolygonF({QPointF(20, pt), QPointF(27, 1), QPointF(34, pt)}));

    painter.drawPath(path.simplified());
}

QPoint TopLeftTailTeachingTipManager::calculatePosition(TeachingTip* tip) const {
    QWidget* target = tip->target();
    if (!target) return tip->pos();

    QPoint pos = target->mapToGlobal(QPoint(0, target->height()));
    int x = pos.x() - tip->layout()->contentsMargins().left();
    int y = pos.y() - tip->layout()->contentsMargins().top();
    return QPoint(x, y);
}

// TopRightTailTeachingTipManager implementation

TopRightTailTeachingTipManager::TopRightTailTeachingTipManager(QObject* parent)
    : TopTailTeachingTipManager(parent) {}

void TopRightTailTeachingTipManager::draw(TeachTipBubble* tip, QPainter& painter) {
    int w = tip->width();
    int h = tip->height();
    int pt = tip->hBoxLayout()->contentsMargins().top();

    QPainterPath path;
    path.addRoundedRect(1, pt, w - 2, h - pt - 1, 8, 8);
    path.addPolygon(QPolygonF({QPointF(w - 20, pt), QPointF(w - 27, 1), QPointF(w - 34, pt)}));

    painter.drawPath(path.simplified());
}

QPoint TopRightTailTeachingTipManager::calculatePosition(TeachingTip* tip) const {
    QWidget* target = tip->target();
    if (!target) return tip->pos();

    QPoint pos = target->mapToGlobal(QPoint(target->width(), target->height()));
    int x = pos.x() - tip->sizeHint().width() + tip->layout()->contentsMargins().left();
    int y = pos.y() - tip->layout()->contentsMargins().top();
    return QPoint(x, y);
}

// BottomLeftTailTeachingTipManager implementation

BottomLeftTailTeachingTipManager::BottomLeftTailTeachingTipManager(QObject* parent)
    : BottomTailTeachingTipManager(parent) {}

void BottomLeftTailTeachingTipManager::draw(TeachTipBubble* tip, QPainter& painter) {
    int w = tip->width();
    int h = tip->height();
    int pb = tip->hBoxLayout()->contentsMargins().bottom();

    QPainterPath path;
    path.addRoundedRect(1, 1, w - 2, h - pb - 1, 8, 8);
    path.addPolygon(QPolygonF({QPointF(20, h - pb), QPointF(27, h - 1), QPointF(34, h - pb)}));

    painter.drawPath(path.simplified());
}

QPoint BottomLeftTailTeachingTipManager::calculatePosition(TeachingTip* tip) const {
    QWidget* target = tip->target();
    if (!target) return tip->pos();

    QPoint pos = target->mapToGlobal(QPoint());
    int x = pos.x() - tip->layout()->contentsMargins().left();
    int y = pos.y() - tip->sizeHint().height() + tip->layout()->contentsMargins().bottom();
    return QPoint(x, y);
}

// BottomRightTailTeachingTipManager implementation

BottomRightTailTeachingTipManager::BottomRightTailTeachingTipManager(QObject* parent)
    : BottomTailTeachingTipManager(parent) {}

void BottomRightTailTeachingTipManager::draw(TeachTipBubble* tip, QPainter& painter) {
    int w = tip->width();
    int h = tip->height();
    int pb = tip->hBoxLayout()->contentsMargins().bottom();

    QPainterPath path;
    path.addRoundedRect(1, 1, w - 2, h - pb - 1, 8, 8);
    path.addPolygon(
        QPolygonF({QPointF(w - 20, h - pb), QPointF(w - 27, h - 1), QPointF(w - 34, h - pb)}));

    painter.drawPath(path.simplified());
}

QPoint BottomRightTailTeachingTipManager::calculatePosition(TeachingTip* tip) const {
    QWidget* target = tip->target();
    if (!target) return tip->pos();

    QPoint pos = target->mapToGlobal(QPoint(target->width(), 0));
    int x = pos.x() - tip->sizeHint().width() + tip->layout()->contentsMargins().left();
    int y = pos.y() - tip->sizeHint().height() + tip->layout()->contentsMargins().bottom();
    return QPoint(x, y);
}

// LeftTopTailTeachingTipManager implementation

LeftTopTailTeachingTipManager::LeftTopTailTeachingTipManager(QObject* parent)
    : LeftTailTeachingTipManager(parent) {}

ImagePosition LeftTopTailTeachingTipManager::imagePosition() const { return ImagePosition::Bottom; }

void LeftTopTailTeachingTipManager::draw(TeachTipBubble* tip, QPainter& painter) {
    int w = tip->width();
    int h = tip->height();
    int pl = 8;

    QPainterPath path;
    path.addRoundedRect(pl, 1, w - pl - 2, h - 2, 8, 8);
    path.addPolygon(QPolygonF({QPointF(pl, 10), QPointF(1, 17), QPointF(pl, 24)}));

    painter.drawPath(path.simplified());
}

QPoint LeftTopTailTeachingTipManager::calculatePosition(TeachingTip* tip) const {
    QWidget* target = tip->target();
    if (!target) return tip->pos();

    QMargins m = tip->layout()->contentsMargins();
    QPoint pos = target->mapToGlobal(QPoint(target->width(), 0));
    int x = pos.x() - m.left();
    int y = pos.y() - m.top();
    return QPoint(x, y);
}

// LeftBottomTailTeachingTipManager implementation

LeftBottomTailTeachingTipManager::LeftBottomTailTeachingTipManager(QObject* parent)
    : LeftTailTeachingTipManager(parent) {}

ImagePosition LeftBottomTailTeachingTipManager::imagePosition() const { return ImagePosition::Top; }

void LeftBottomTailTeachingTipManager::draw(TeachTipBubble* tip, QPainter& painter) {
    int w = tip->width();
    int h = tip->height();
    int pl = 9;

    QPainterPath path;
    path.addRoundedRect(pl, 1, w - pl - 1, h - 2, 8, 8);
    path.addPolygon(QPolygonF({QPointF(pl, h - 10), QPointF(1, h - 17), QPointF(pl, h - 24)}));

    painter.drawPath(path.simplified());
}

QPoint LeftBottomTailTeachingTipManager::calculatePosition(TeachingTip* tip) const {
    QWidget* target = tip->target();
    if (!target) return tip->pos();

    QMargins m = tip->layout()->contentsMargins();
    QPoint pos = target->mapToGlobal(QPoint(target->width(), target->height()));
    int x = pos.x() - m.left();
    int y = pos.y() - tip->sizeHint().height() + m.bottom();
    return QPoint(x, y);
}

// RightTopTailTeachingTipManager implementation

RightTopTailTeachingTipManager::RightTopTailTeachingTipManager(QObject* parent)
    : RightTailTeachingTipManager(parent) {}

ImagePosition RightTopTailTeachingTipManager::imagePosition() const {
    return ImagePosition::Bottom;
}

void RightTopTailTeachingTipManager::draw(TeachTipBubble* tip, QPainter& painter) {
    int w = tip->width();
    int h = tip->height();
    int pr = 8;

    QPainterPath path;
    path.addRoundedRect(1, 1, w - pr - 1, h - 2, 8, 8);
    path.addPolygon(QPolygonF({QPointF(w - pr, 10), QPointF(w - 1, 17), QPointF(w - pr, 24)}));

    painter.drawPath(path.simplified());
}

QPoint RightTopTailTeachingTipManager::calculatePosition(TeachingTip* tip) const {
    QWidget* target = tip->target();
    if (!target) return tip->pos();

    QMargins m = tip->layout()->contentsMargins();
    QPoint pos = target->mapToGlobal(QPoint(0, 0));
    int x = pos.x() - tip->sizeHint().width() + m.right();
    int y = pos.y() - m.top();
    return QPoint(x, y);
}

// RightBottomTailTeachingTipManager implementation

RightBottomTailTeachingTipManager::RightBottomTailTeachingTipManager(QObject* parent)
    : RightTailTeachingTipManager(parent) {}

ImagePosition RightBottomTailTeachingTipManager::imagePosition() const {
    return ImagePosition::Top;
}

void RightBottomTailTeachingTipManager::draw(TeachTipBubble* tip, QPainter& painter) {
    int w = tip->width();
    int h = tip->height();
    int pr = 8;

    QPainterPath path;
    path.addRoundedRect(1, 1, w - pr - 1, h - 2, 8, 8);
    path.addPolygon(
        QPolygonF({QPointF(w - pr, h - 10), QPointF(w - 1, h - 17), QPointF(w - pr, h - 24)}));

    painter.drawPath(path.simplified());
}

QPoint RightBottomTailTeachingTipManager::calculatePosition(TeachingTip* tip) const {
    QWidget* target = tip->target();
    if (!target) return tip->pos();

    QMargins m = tip->layout()->contentsMargins();
    QPoint pos = target->mapToGlobal(QPoint(0, target->height()));
    int x = pos.x() - tip->sizeHint().width() + m.right();
    int y = pos.y() - tip->sizeHint().height() + m.bottom();
    return QPoint(x, y);
}

// TeachingTip implementation

TeachingTip::TeachingTip(FlyoutViewBase* view, QWidget* target, int duration,
                         TeachingTipTailPosition tailPosition, QWidget* parent,
                         bool isDeleteOnClose)
    : QWidget(parent), target_(target), duration_(duration), isDeleteOnClose_(isDeleteOnClose) {
    manager_ = TeachingTipManager::make(tailPosition);

    hBoxLayout_ = new QHBoxLayout(this);
    opacityAni_ = new QPropertyAnimation(this, QByteArrayLiteral("windowOpacity"), this);

    bubble_ = new TeachTipBubble(view, tailPosition, this);

    hBoxLayout_->setContentsMargins(15, 8, 15, 20);
    hBoxLayout_->addWidget(bubble_);
    setShadowEffect();

    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);

    if (parent && parent->window()) {
        parent->window()->installEventFilter(this);
    }
}

void TeachingTip::setShadowEffect(int blurRadius, const QPoint& offset) {
    QColor color(0, 0, 0, isDarkTheme() ? 80 : 30);
    shadowEffect_ = new QGraphicsDropShadowEffect(bubble_);
    shadowEffect_->setBlurRadius(blurRadius);
    shadowEffect_->setOffset(offset);
    shadowEffect_->setColor(color);
    bubble_->setGraphicsEffect(nullptr);
    bubble_->setGraphicsEffect(shadowEffect_);
}

void TeachingTip::fadeOut() {
    opacityAni_->setDuration(167);
    opacityAni_->setStartValue(1.0);
    opacityAni_->setEndValue(0.0);
    connect(opacityAni_, &QPropertyAnimation::finished, this, &TeachingTip::close);
    opacityAni_->start();
}

void TeachingTip::showEvent(QShowEvent* e) {
    if (duration_ >= 0) {
        QTimer::singleShot(duration_, this, &TeachingTip::fadeOut);
    }

    move(manager_->position(this));
    adjustSize();

    opacityAni_->setDuration(167);
    opacityAni_->setStartValue(0.0);
    opacityAni_->setEndValue(1.0);
    opacityAni_->start();

    QWidget::showEvent(e);
}

void TeachingTip::closeEvent(QCloseEvent* e) {
    if (isDeleteOnClose_) {
        deleteLater();
    }
    QWidget::closeEvent(e);
}

bool TeachingTip::eventFilter(QObject* obj, QEvent* e) {
    if (parent() && obj == qobject_cast<QWidget*>(parent())->window()) {
        if (e->type() == QEvent::Resize || e->type() == QEvent::WindowStateChange ||
            e->type() == QEvent::Move) {
            move(manager_->position(this));
        }
    }
    return QWidget::eventFilter(obj, e);
}

void TeachingTip::addWidget(QWidget* widget, int stretch, Qt::Alignment align) {
    if (auto* v = qobject_cast<FlyoutView*>(view())) {
        v->addWidget(widget, stretch, align);
    }
}

void TeachingTip::setView(FlyoutViewBase* view) {
    if (bubble_) {
        bubble_->setView(qobject_cast<QWidget*>(view));
    }
}

FlyoutViewBase* TeachingTip::view() const { return bubble_ ? bubble_->view() : nullptr; }

QWidget* TeachingTip::target() const { return target_; }

TeachingTip* TeachingTip::make(FlyoutViewBase* view, QWidget* target, int duration,
                               TeachingTipTailPosition tailPosition, QWidget* parent,
                               bool isDeleteOnClose) {
    auto* w = new TeachingTip(view, target, duration, tailPosition, parent, isDeleteOnClose);
    w->show();
    return w;
}

TeachingTip* TeachingTip::create(QWidget* target, const QString& title, const QString& content,
                                 const QVariant& icon, const QVariant& image, bool isClosable,
                                 int duration, TeachingTipTailPosition tailPosition,
                                 QWidget* parent, bool isDeleteOnClose) {
    auto* view = new TeachingTipView(title, content, QIcon(), image, isClosable, tailPosition);

    if (icon.canConvert<QIcon>()) {
        view->setIcon(icon.value<QIcon>());
    }

    auto* tip = TeachingTip::make(view, target, duration, tailPosition, parent, isDeleteOnClose);
    connect(view, &FlyoutView::closed, tip, &TeachingTip::close);
    return tip;
}

// PopupTeachingTip implementation

PopupTeachingTip::PopupTeachingTip(FlyoutViewBase* view, QWidget* target, int duration,
                                   TeachingTipTailPosition tailPosition, QWidget* parent,
                                   bool isDeleteOnClose)
    : TeachingTip(view, target, duration, tailPosition, parent, isDeleteOnClose) {
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
}

}  // namespace qfw
