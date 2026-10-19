#include "components/widgets/drawer.h"

#include <QApplication>
#include <QCloseEvent>
#include <QGraphicsDropShadowEffect>
#include <QHideEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QShortcut>
#include <QShowEvent>
#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "common/config.h"
#include "components/widgets/button.h"
#include "components/widgets/label.h"

namespace qfw {
namespace {
class DrawerPanel : public QWidget {
public:
    using QWidget::QWidget;

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.fillRect(rect(), isDarkTheme() ? QColor("#202020") : QColor(Qt::white));
    }
};
void validatePosition(DrawerPosition position) {
    if (position != DrawerPosition::LEFT && position != DrawerPosition::RIGHT &&
        position != DrawerPosition::TOP && position != DrawerPosition::BOTTOM) {
        throw std::invalid_argument("Invalid drawer position");
    }
}
}  // namespace
Drawer::Drawer(QWidget* parent, DrawerPosition position) : Drawer(QString(), parent, position) {}
Drawer::Drawer(const QString& title, QWidget* parent, DrawerPosition position)
    : QWidget(parent), animation_(this), raiseTimer_(this), position_(position) {
    if (!parent) {
        throw std::invalid_argument("Drawer requires a parent widget");
    }
    validatePosition(position);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_NoSystemBackground);
    panel_ = new DrawerPanel(this);
    panel_->setObjectName(QStringLiteral("drawerPanel"));
    auto* shadow = new QGraphicsDropShadowEffect(panel_);
    shadow->setBlurRadius(48);
    shadow->setOffset(0, 0);
    shadow->setColor(QColor(0, 0, 0, 110));
    panel_->setGraphicsEffect(shadow);
    auto* layout = new QVBoxLayout(panel_);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->setSizeConstraint(QLayout::SetNoConstraint);
    auto* header = new QWidget(panel_);
    auto* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(20, 16, 12, 8);
    titleLabel_ = new SubtitleLabel(header);
    titleLabel_->setTextFormat(Qt::PlainText);
    closeButton_ = new TransparentToolButton(FluentIcon(FluentIconEnum::Close), header);
    closeButton_->setFixedSize(32, 32);
    closeButton_->setIconSize(QSize(16, 16));
    closeButton_->setToolTip(tr("Close"));
    closeButton_->setAccessibleName(tr("Close"));
    headerLayout->addWidget(titleLabel_, 1);
    headerLayout->addWidget(closeButton_);
    contentWidget = new QWidget(panel_);
    viewLayout = new QVBoxLayout(contentWidget);
    viewLayout->setContentsMargins(20, 12, 20, 20);
    viewLayout->setSpacing(12);
    layout->addWidget(header);
    layout->addWidget(contentWidget, 1);
    setTitle(title);
    connect(closeButton_, &QAbstractButton::clicked, this, [this] { close(); });
    auto* escape = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    escape->setContext(Qt::WidgetWithChildrenShortcut);
    connect(escape, &QShortcut::activated, this, [this] {
        if (escClosable_) {
            close();
        }
    });
    animation_.setEasingCurve(QEasingCurve::OutCubic);
    connect(&animation_, &QVariantAnimation::valueChanged, this, [this](const QVariant& value) {
        progress_ = value.toReal();
        updatePanelGeometry();
        update();
    });
    connect(&animation_, &QVariantAnimation::finished, this, [this] {
        if (open_) {
            progress_ = 1;
            updatePanelGeometry();
            emit opened();
        } else {
            finishClosed();
        }
    });
    raiseTimer_.setSingleShot(true);
    connect(&raiseTimer_, &QTimer::timeout, this, [this] {
        if (isVisible()) {
            raise();
        }
    });
    connect(&QConfig::instance(), &QConfig::themeChanged, panel_, [this] { panel_->update(); });
    parent->installEventFilter(this);
    setGeometry(parent->rect());
    QWidget::hide();
    updatePanelGeometry();
}
QString Drawer::title() const { return titleLabel_->text(); }
void Drawer::setTitle(const QString& title) {
    titleLabel_->setText(title);
    titleLabel_->setVisible(!title.isEmpty());
    setAccessibleName(title);
}
void Drawer::addWidget(QWidget* widget, int stretch, Qt::Alignment alignment) {
    if (!widget || widget == this || widget == panel_ || widget == contentWidget ||
        widget->isAncestorOf(this)) {
        throw std::invalid_argument("Invalid drawer content");
    }
    viewLayout->addWidget(widget, stretch, alignment);
}
DrawerPosition Drawer::position() const { return position_; }
void Drawer::setPosition(DrawerPosition position) {
    validatePosition(position);
    position_ = position;
    updatePanelGeometry();
}
int Drawer::drawerSize() const { return size_; }
void Drawer::setDrawerSize(int size) {
    if (size <= 0) {
        throw std::invalid_argument("Drawer size must be positive");
    }
    size_ = size;
    updatePanelGeometry();
}
int Drawer::animationDuration() const { return duration_; }
void Drawer::setAnimationDuration(int duration) {
    if (duration < 0) {
        throw std::invalid_argument("Invalid drawer duration");
    }
    duration_ = duration;
    if (!duration && animation_.state() == QAbstractAnimation::Running) {
        animateTo(open_ ? 1 : 0);
    }
}
QColor Drawer::maskColor() const { return mask_; }
void Drawer::setMaskColor(const QColor& color) {
    if (!color.isValid()) {
        throw std::invalid_argument("Invalid mask color");
    }
    mask_ = color;
    update();
}
bool Drawer::isClosableOnMaskClicked() const { return maskClosable_; }
void Drawer::setClosableOnMaskClicked(bool enabled) { maskClosable_ = enabled; }
bool Drawer::isEscClosable() const { return escClosable_; }
void Drawer::setEscClosable(bool enabled) { escClosable_ = enabled; }
bool Drawer::isOpen() const { return open_; }
QWidget* Drawer::panelWidget() const { return panel_; }
void Drawer::open(DrawerPosition position) {
    setPosition(position);
    open();
}
void Drawer::open() {
    if (open_ && !isHidden()) {
        raise();
        return;
    }
    if (isHidden()) {
        previousFocus_ = QApplication::focusWidget();
    }
    open_ = true;
    setGeometry(parentWidget()->rect());
    QWidget::show();
    raise();
    setFocus(Qt::PopupFocusReason);
    animateTo(1);
}
void Drawer::show() { open(); }
bool Drawer::close() {
    if (!open_) {
        return true;
    }
    open_ = false;
    animateTo(0);
    return true;
}
void Drawer::hide() { finishClosed(); }
void Drawer::animateTo(qreal value) {
    animation_.stop();
    const int duration = qRound(duration_ * std::abs(value - progress_));
    if (!duration) {
        progress_ = value;
        updatePanelGeometry();
        update();
        if (open_) {
            emit opened();
        } else {
            finishClosed();
        }
        return;
    }
    animation_.setStartValue(progress_);
    animation_.setEndValue(value);
    animation_.setDuration(duration);
    animation_.start();
}
void Drawer::finishClosed() {
    if (finishing_) {
        return;
    }
    const bool active = open_ || progress_ > 0 || !isHidden();
    finishing_ = true;
    animation_.stop();
    raiseTimer_.stop();
    open_ = false;
    progress_ = 0;
    QWidget::hide();
    updatePanelGeometry();
    if (active && previousFocus_ && previousFocus_->isVisible() && parentWidget()->isVisible()) {
        previousFocus_->setFocus(Qt::OtherFocusReason);
    }
    previousFocus_.clear();
    finishing_ = false;
    if (active) {
        emit closed();
    }
}
void Drawer::updatePanelGeometry() {
    const bool vertical = position_ == DrawerPosition::LEFT || position_ == DrawerPosition::RIGHT;
    const int extent = std::min(size_, vertical ? width() : height());
    if (position_ == DrawerPosition::LEFT) {
        panel_->setGeometry(qRound(-extent * (1 - progress_)), 0, extent, height());
    } else if (position_ == DrawerPosition::RIGHT) {
        panel_->setGeometry(qRound(width() - extent * progress_), 0, extent, height());
    } else if (position_ == DrawerPosition::TOP) {
        panel_->setGeometry(0, qRound(-extent * (1 - progress_)), width(), extent);
    } else {
        panel_->setGeometry(0, qRound(height() - extent * progress_), width(), extent);
    }
}
bool Drawer::eventFilter(QObject* watched, QEvent* event) {
    if (watched == parentWidget()) {
        if (event->type() == QEvent::Resize) {
            setGeometry(parentWidget()->rect());
            updatePanelGeometry();
        } else if (event->type() == QEvent::Hide) {
            finishClosed();
        } else if ((event->type() == QEvent::ChildAdded ||
                    event->type() == QEvent::ChildPolished) &&
                   isVisible()) {
            raiseTimer_.start(0);
        }
    }
    return QWidget::eventFilter(watched, event);
}
void Drawer::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    QColor color = mask_;
    color.setAlpha(qRound(color.alpha() * progress_));
    painter.fillRect(rect(), color);
}
void Drawer::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && !panel_->geometry().contains(event->pos()) &&
        maskClosable_) {
        close();
    }
    event->accept();
}
void Drawer::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    if (!open_) {
        previousFocus_ = QApplication::focusWidget();
        open_ = true;
        raise();
        animateTo(1);
    }
}
void Drawer::hideEvent(QHideEvent* event) {
    QWidget::hideEvent(event);
    if (!finishing_ && (open_ || progress_ > 0)) {
        finishClosed();
    }
}
void Drawer::closeEvent(QCloseEvent* event) {
    event->ignore();
    close();
}
bool Drawer::focusNextPrevChild(bool next) {
    auto widgets = panel_->findChildren<QWidget*>();
    QList<QWidget*> focusable;
    for (auto* widget : widgets) {
        if (widget->isVisible() && widget->isEnabled() && (widget->focusPolicy() & Qt::TabFocus)) {
            focusable.append(widget);
        }
    }
    if (!focusable.isEmpty()) {
        int index = focusable.indexOf(QApplication::focusWidget());
        index = (index + (next ? 1 : -1) + focusable.size()) % focusable.size();
        focusable.at(index)->setFocus(next ? Qt::TabFocusReason : Qt::BacktabFocusReason);
    }
    return true;
}
}  // namespace qfw
