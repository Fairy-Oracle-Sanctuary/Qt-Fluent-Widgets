#include "tool_tip.h"

#include <QApplication>
#include <QEvent>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QScreen>
#include <QScrollBar>
#include <QTableView>

#include "common/style_sheet.h"

namespace qfw {

// ToolTip implementation

ToolTip::ToolTip(const QString& text, QWidget* parent) : QFrame(parent), text_(text) {
    initUi();
    setQss();
}

QString ToolTip::text() const { return text_; }

void ToolTip::setText(const QString& text) {
    text_ = text;
    if (label_) {
        label_->setText(text);
    }
    if (container_) {
        container_->adjustSize();
    }
    adjustSize();
}

int ToolTip::duration() const { return duration_; }

void ToolTip::setDuration(int duration) { duration_ = duration; }

void ToolTip::adjustPos(QWidget* widget, ToolTipPosition position) {
    ToolTipPositionManager* manager = ToolTipPositionManager::make(position);
    move(manager->position(this, widget));
}

void ToolTip::showEvent(QShowEvent* e) {
    if (opacityAni_) {
        opacityAni_->setStartValue(0.0);
        opacityAni_->setEndValue(1.0);
        opacityAni_->start();
    }

    if (timer_) {
        timer_->stop();
        if (duration_ > 0) {
            timer_->start(duration_ + (opacityAni_ ? opacityAni_->duration() : 0));
        }
    }

    QFrame::showEvent(e);
}

void ToolTip::hideEvent(QHideEvent* e) {
    if (timer_) {
        timer_->stop();
    }
    QFrame::hideEvent(e);
}

QFrame* ToolTip::createContainer() { return new QFrame(this); }

void ToolTip::initUi() {
    setProperty("qssClass", "ToolTip");

    container_ = createContainer();
    timer_ = new QTimer(this);

    layout_ = new QHBoxLayout(this);
    containerLayout_ = new QHBoxLayout(container_);
    label_ = new QLabel(text_, this);

    // set layout
    layout_->setContentsMargins(12, 8, 12, 12);
    layout_->addWidget(container_);
    containerLayout_->addWidget(label_);
    containerLayout_->setContentsMargins(8, 6, 8, 6);

    // add opacity effect
    opacityAni_ = new QPropertyAnimation(this, QByteArrayLiteral("windowOpacity"), this);
    opacityAni_->setDuration(150);

    // add shadow
    shadowEffect_ = new QGraphicsDropShadowEffect(this);
    shadowEffect_->setBlurRadius(25);
    shadowEffect_->setColor(QColor(0, 0, 0, 50));
    shadowEffect_->setOffset(0, 5);
    container_->setGraphicsEffect(shadowEffect_);

    timer_->setSingleShot(true);
    connect(timer_, &QTimer::timeout, this, &ToolTip::hide);

    // set window attributes
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
}

void ToolTip::setQss() {
    if (container_) {
        container_->setObjectName(QStringLiteral("container"));
    }
    if (label_) {
        label_->setObjectName(QStringLiteral("contentLabel"));
    }
    qfw::setStyleSheet(this, qfw::FluentStyleSheet::ToolTip);
    if (label_) {
        label_->adjustSize();
    }
    adjustSize();
}

// ToolTipPositionManager implementation

QPoint ToolTipPositionManager::position(ToolTip* tooltip, QWidget* parent) {
    QPoint pos = _pos(tooltip, parent);
    int x = pos.x();
    int y = pos.y();

    // Get current screen geometry
    QScreen* screen = QApplication::screenAt(pos);
    if (!screen) {
        screen = QApplication::primaryScreen();
    }
    QRect rect = screen ? screen->availableGeometry() : QRect();

    x = qMax(rect.left(), qMin(pos.x(), rect.right() - tooltip->width() - 4));
    y = qMax(rect.top(), qMin(pos.y(), rect.bottom() - tooltip->height() - 4));

    return QPoint(x, y);
}

ToolTipPositionManager* ToolTipPositionManager::make(ToolTipPosition position) {
    switch (position) {
        case ToolTipPosition::Top:
            return new TopToolTipManager();
        case ToolTipPosition::Bottom:
            return new BottomToolTipManager();
        case ToolTipPosition::Left:
            return new LeftToolTipManager();
        case ToolTipPosition::Right:
            return new RightToolTipManager();
        case ToolTipPosition::TopRight:
            return new TopRightToolTipManager();
        case ToolTipPosition::TopLeft:
            return new TopLeftToolTipManager();
        case ToolTipPosition::BottomRight:
            return new BottomRightToolTipManager();
        case ToolTipPosition::BottomLeft:
            return new BottomLeftToolTipManager();
        default:
            return new TopToolTipManager();
    }
}

// TopToolTipManager

QPoint TopToolTipManager::_pos(ToolTip* tooltip, QWidget* parent) {
    QPoint pos = parent->mapToGlobal(QPoint());
    int x = pos.x() + parent->width() / 2 - tooltip->width() / 2;
    int y = pos.y() - tooltip->height();
    return QPoint(x, y);
}

// BottomToolTipManager

QPoint BottomToolTipManager::_pos(ToolTip* tooltip, QWidget* parent) {
    QPoint pos = parent->mapToGlobal(QPoint());
    int x = pos.x() + parent->width() / 2 - tooltip->width() / 2;
    int y = pos.y() + parent->height();
    return QPoint(x, y);
}

// LeftToolTipManager

QPoint LeftToolTipManager::_pos(ToolTip* tooltip, QWidget* parent) {
    QPoint pos = parent->mapToGlobal(QPoint());
    int x = pos.x() - tooltip->width();
    int y = pos.y() + (parent->height() - tooltip->height()) / 2;
    return QPoint(x, y);
}

// RightToolTipManager

QPoint RightToolTipManager::_pos(ToolTip* tooltip, QWidget* parent) {
    QPoint pos = parent->mapToGlobal(QPoint());
    int x = pos.x() + parent->width();
    int y = pos.y() + (parent->height() - tooltip->height()) / 2;
    return QPoint(x, y);
}

// TopRightToolTipManager

QPoint TopRightToolTipManager::_pos(ToolTip* tooltip, QWidget* parent) {
    QPoint pos = parent->mapToGlobal(QPoint());
    QMargins margins = tooltip->contentsMargins();
    int x = pos.x() + parent->width() - tooltip->width() + margins.right();
    int y = pos.y() - tooltip->height();
    return QPoint(x, y);
}

// TopLeftToolTipManager

QPoint TopLeftToolTipManager::_pos(ToolTip* tooltip, QWidget* parent) {
    QPoint pos = parent->mapToGlobal(QPoint());
    QMargins margins = tooltip->contentsMargins();
    int x = pos.x() - margins.left();
    int y = pos.y() - tooltip->height();
    return QPoint(x, y);
}

// BottomRightToolTipManager

QPoint BottomRightToolTipManager::_pos(ToolTip* tooltip, QWidget* parent) {
    QPoint pos = parent->mapToGlobal(QPoint());
    QMargins margins = tooltip->contentsMargins();
    int x = pos.x() + parent->width() - tooltip->width() + margins.right();
    int y = pos.y() + parent->height();
    return QPoint(x, y);
}

// BottomLeftToolTipManager

QPoint BottomLeftToolTipManager::_pos(ToolTip* tooltip, QWidget* parent) {
    QPoint pos = parent->mapToGlobal(QPoint());
    QMargins margins = tooltip->contentsMargins();
    int x = pos.x() - margins.left();
    int y = pos.y() + parent->height();
    return QPoint(x, y);
}

// ItemViewToolTipManager

ItemViewToolTipManager::ItemViewToolTipManager(const QRect& itemRect) : itemRect_(itemRect) {}

QPoint ItemViewToolTipManager::_pos(ToolTip* tooltip, QWidget* view) {
    QPoint pos = view->mapToGlobal(itemRect_.topLeft());
    int x = pos.x();
    int y = pos.y() - tooltip->height() + 10;
    return QPoint(x, y);
}

ItemViewToolTipManager* ItemViewToolTipManager::make(ItemViewToolTipType tipType,
                                                     const QRect& itemRect) {
    if (tipType == ItemViewToolTipType::Table) {
        return new TableItemToolTipManager(itemRect);
    }
    return new ItemViewToolTipManager(itemRect);
}

// TableItemToolTipManager

TableItemToolTipManager::TableItemToolTipManager(const QRect& itemRect)
    : ItemViewToolTipManager(itemRect) {}

QPoint TableItemToolTipManager::_pos(ToolTip* tooltip, QWidget* view) {
    QTableView* tableView = qobject_cast<QTableView*>(view);
    QPoint pos = view->mapToGlobal(itemRect_.topLeft());
    int x = pos.x();
    int y = pos.y() - tooltip->height() + 10;

    if (tableView) {
        if (tableView->verticalHeader()->isVisible()) {
            x += tableView->verticalHeader()->width();
        }
        if (tableView->horizontalHeader()->isVisible()) {
            y += tableView->horizontalHeader()->height();
        }
    }

    return QPoint(x, y);
}

// ToolTipFilter implementation

ToolTipFilter::ToolTipFilter(QWidget* parent, int showDelay, ToolTipPosition position)
    : QObject(parent), tooltipDelay_(showDelay), position_(position) {
    timer_ = new QTimer(this);
    timer_->setSingleShot(true);
    connect(timer_, &QTimer::timeout, this, &ToolTipFilter::showToolTip);
}

bool ToolTipFilter::eventFilter(QObject* obj, QEvent* e) {
    QWidget* parentWidget = qobject_cast<QWidget*>(this->parent());
    if (!parentWidget) {
        return QObject::eventFilter(obj, e);
    }

    switch (e->type()) {
        case QEvent::ToolTip:
            return true;  // consume native tooltip event

        case QEvent::Hide:
        case QEvent::Leave:
            hideToolTip();
            break;

        case QEvent::Enter:
            isEnter_ = true;
            if (canShowToolTip()) {
                if (!tooltip_) {
                    tooltip_ = createToolTip();
                }

                int t = parentWidget->toolTipDuration();
                if (t > 0) {
                    tooltip_->setDuration(t);
                } else {
                    tooltip_->setDuration(-1);  // won't disappear automatically
                }

                // show the tool tip after delay
                timer_->start(tooltipDelay_);
            }
            break;

        case QEvent::MouseButtonPress:
            hideToolTip();
            break;

        default:
            break;
    }

    return QObject::eventFilter(obj, e);
}

void ToolTipFilter::setToolTipDelay(int delay) { tooltipDelay_ = delay; }

int ToolTipFilter::toolTipDelay() const { return tooltipDelay_; }

void ToolTipFilter::showToolTip() {
    if (!isEnter_) {
        return;
    }

    QWidget* parentWidget = qobject_cast<QWidget*>(parent());
    if (!parentWidget || !tooltip_) {
        return;
    }

    tooltip_->setText(parentWidget->toolTip());
    tooltip_->adjustPos(parentWidget, position_);
    tooltip_->show();
}

void ToolTipFilter::hideToolTip() {
    isEnter_ = false;
    if (timer_) {
        timer_->stop();
    }
    if (tooltip_) {
        tooltip_->hide();
    }
}

ToolTip* ToolTipFilter::createToolTip() {
    QWidget* parentWidget = qobject_cast<QWidget*>(parent());
    if (!parentWidget) {
        return nullptr;
    }
    return new ToolTip(parentWidget->toolTip(), parentWidget->window());
}

bool ToolTipFilter::canShowToolTip() {
    QWidget* parentWidget = qobject_cast<QWidget*>(parent());
    if (!parentWidget) {
        return false;
    }
    return parentWidget->isWidgetType() && !parentWidget->toolTip().isEmpty() &&
           parentWidget->isEnabled();
}

// ItemViewToolTip implementation

void ItemViewToolTip::adjustPos(QAbstractItemView* view, const QRect& itemRect,
                                ItemViewToolTipType tooltipType) {
    ItemViewToolTipManager* manager = ItemViewToolTipManager::make(tooltipType, itemRect);
    move(manager->position(this, view));
    delete manager;
}

// ItemViewToolTipDelegate implementation

ItemViewToolTipDelegate::ItemViewToolTipDelegate(QAbstractItemView* parent, int showDelay,
                                                 ItemViewToolTipType tooltipType)
    : ToolTipFilter(parent, showDelay, ToolTipPosition::Top),
      tooltipType_(tooltipType),
      viewport_(parent->viewport()) {
    parent->installEventFilter(this);
    parent->viewport()->installEventFilter(this);
    connect(parent->horizontalScrollBar(), &QScrollBar::valueChanged, this,
            &ItemViewToolTipDelegate::hideToolTip);
    connect(parent->verticalScrollBar(), &QScrollBar::valueChanged, this,
            &ItemViewToolTipDelegate::hideToolTip);
}

bool ItemViewToolTipDelegate::eventFilter(QObject* obj, QEvent* e) {
    QAbstractItemView* view = qobject_cast<QAbstractItemView*>(parent());

    if (obj == parent()) {
        switch (e->type()) {
            case QEvent::Hide:
            case QEvent::Leave:
                hideToolTip();
                break;
            case QEvent::Enter:
                isEnter_ = true;
                break;
            default:
                break;
        }
    } else if (obj == viewport_) {
        if (e->type() == QEvent::MouseButtonPress) {
            hideToolTip();
        }
    }

    return QObject::eventFilter(obj, e);
}

void ItemViewToolTipDelegate::setText(const QString& text) {
    text_ = text;
    if (tooltip_) {
        tooltip_->setText(text);
    }
}

QString ItemViewToolTipDelegate::text() const { return text_; }

void ItemViewToolTipDelegate::setToolTipDuration(int duration) {
    tooltipDuration_ = duration;
    if (tooltip_) {
        tooltip_->setDuration(duration);
    }
}

int ItemViewToolTipDelegate::toolTipDuration() const { return tooltipDuration_; }

bool ItemViewToolTipDelegate::helpEvent(QHelpEvent* event, QAbstractItemView* view,
                                        const QStyleOptionViewItem& option,
                                        const QModelIndex& index) {
    if (!event || !view) {
        return false;
    }

    if (event->type() == QEvent::ToolTip) {
        QString text = index.data(Qt::ToolTipRole).toString();
        if (text.isEmpty()) {
            hideToolTip();
            return false;
        }

        text_ = text;
        currentIndex_ = index;

        if (!tooltip_) {
            tooltip_ = createToolTip();
            tooltip_->setDuration(tooltipDuration_);
        }

        // show the tool tip after delay
        timer_->start(tooltipDelay_);
    }

    return true;
}

ToolTip* ItemViewToolTipDelegate::createToolTip() {
    QAbstractItemView* view = qobject_cast<QAbstractItemView*>(parent());
    if (!view) {
        return nullptr;
    }
    return new ItemViewToolTip(text_, view->window());
}

bool ItemViewToolTipDelegate::canShowToolTip() { return true; }

}  // namespace qfw
