#include "components/window/title_bar.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QStyleOption>
#include <QWindow>

#include "common/config.h"

#ifdef Q_OS_WIN
// Disable Windows min/max macros before including Windows headers
#define NOMINMAX
#include <windows.h>
#endif

namespace qfw {

// ============================================================================
// TitleBarButton
// ============================================================================

TitleBarButton::TitleBarButton(QWidget* parent) : QAbstractButton(parent) {
    setCursor(Qt::ArrowCursor);
    setFixedSize(46, 32);
    setState(TitleBarButtonState::Normal);

    // Default icon colors
    normalColor_ = QColor(0, 0, 0);
    hoverColor_ = QColor(0, 0, 0);
    pressedColor_ = QColor(0, 0, 0);

    // Default background colors
    normalBgColor_ = QColor(0, 0, 0, 0);
    hoverBgColor_ = QColor(0, 0, 0, 26);
    pressedBgColor_ = QColor(0, 0, 0, 51);
}

void TitleBarButton::setState(TitleBarButtonState state) {
    state_ = state;
    update();
}

TitleBarButtonState TitleBarButton::state() const { return state_; }

bool TitleBarButton::isPressed() const { return state_ == TitleBarButtonState::Pressed; }

QColor TitleBarButton::normalColor() const { return normalColor_; }

QColor TitleBarButton::hoverColor() const { return hoverColor_; }

QColor TitleBarButton::pressedColor() const { return pressedColor_; }

QColor TitleBarButton::normalBackgroundColor() const { return normalBgColor_; }

QColor TitleBarButton::hoverBackgroundColor() const { return hoverBgColor_; }

QColor TitleBarButton::pressedBackgroundColor() const { return pressedBgColor_; }

void TitleBarButton::setNormalColor(const QColor& color) {
    normalColor_ = color;
    update();
}

void TitleBarButton::setHoverColor(const QColor& color) {
    hoverColor_ = color;
    update();
}

void TitleBarButton::setPressedColor(const QColor& color) {
    pressedColor_ = color;
    update();
}

void TitleBarButton::setNormalBackgroundColor(const QColor& color) {
    normalBgColor_ = color;
    update();
}

void TitleBarButton::setHoverBackgroundColor(const QColor& color) {
    hoverBgColor_ = color;
    update();
}

void TitleBarButton::setPressedBackgroundColor(const QColor& color) {
    pressedBgColor_ = color;
    update();
}

void TitleBarButton::enterEvent(QEnterEvent* event) {
    setState(TitleBarButtonState::Hover);
    QAbstractButton::enterEvent(event);
}

void TitleBarButton::leaveEvent(QEvent* event) {
    setState(TitleBarButtonState::Normal);
    QAbstractButton::leaveEvent(event);
}

void TitleBarButton::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        setState(TitleBarButtonState::Pressed);
    }
    QAbstractButton::mousePressEvent(event);
}

std::pair<QColor, QColor> TitleBarButton::getColors() const {
    switch (state_) {
        case TitleBarButtonState::Hover:
            return {hoverColor_, hoverBgColor_};
        case TitleBarButtonState::Pressed:
            return {pressedColor_, pressedBgColor_};
        case TitleBarButtonState::Normal:
        default:
            return {normalColor_, normalBgColor_};
    }
}

// ============================================================================
// MinimizeButton
// ============================================================================

MinimizeButton::MinimizeButton(QWidget* parent) : TitleBarButton(parent) {}

void MinimizeButton::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    auto [color, bgColor] = getColors();

    // Draw background
    painter.setBrush(bgColor);
    painter.setPen(Qt::NoPen);
    painter.drawRect(rect());

    // Draw minimize icon (horizontal line)
    painter.setBrush(Qt::NoBrush);
    QPen pen(color, 1);
    pen.setCosmetic(true);
    painter.setPen(pen);
    painter.drawLine(18, 16, 28, 16);
}

// ============================================================================
// MaximizeButton
// ============================================================================

MaximizeButton::MaximizeButton(QWidget* parent) : TitleBarButton(parent) {}

void MaximizeButton::setMaximized(bool maximized) {
    if (maximized_ == maximized) {
        return;
    }
    maximized_ = maximized;
    setState(TitleBarButtonState::Normal);
}

bool MaximizeButton::isMaximized() const { return maximized_; }

void MaximizeButton::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    auto [color, bgColor] = getColors();

    // Draw background
    painter.setBrush(bgColor);
    painter.setPen(Qt::NoPen);
    painter.drawRect(rect());

    // Draw maximize/restore icon
    painter.setBrush(Qt::NoBrush);
    QPen pen(color, 1);
    pen.setCosmetic(true);
    painter.setPen(pen);

    qreal r = devicePixelRatioF();
    painter.scale(1.0 / r, 1.0 / r);

    if (!maximized_) {
        // Maximize icon: square
        painter.drawRect(static_cast<int>(18 * r), static_cast<int>(11 * r),
                         static_cast<int>(10 * r), static_cast<int>(10 * r));
    } else {
        // Restore icon: overlapping squares
        painter.drawRect(static_cast<int>(18 * r), static_cast<int>(13 * r),
                         static_cast<int>(8 * r), static_cast<int>(8 * r));

        int x0 = static_cast<int>(18 * r) + static_cast<int>(2 * r);
        int y0 = static_cast<int>(13 * r);
        int dw = static_cast<int>(2 * r);

        QPainterPath path;
        path.moveTo(x0, y0);
        path.lineTo(x0, y0 - dw);
        path.lineTo(x0 + static_cast<int>(8 * r), y0 - dw);
        path.lineTo(x0 + static_cast<int>(8 * r), y0 - dw + static_cast<int>(8 * r));
        path.lineTo(x0 + static_cast<int>(8 * r) - dw, y0 - dw + static_cast<int>(8 * r));
        painter.drawPath(path);
    }
}

// ============================================================================
// CloseButton
// ============================================================================

CloseButton::CloseButton(QWidget* parent) : TitleBarButton(parent) {
    // Close button has special red colors like Windows
    setHoverColor(Qt::white);
    setPressedColor(Qt::white);
    setHoverBackgroundColor(QColor(232, 17, 35));
    setPressedBackgroundColor(QColor(241, 112, 122));
}

void CloseButton::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    auto [color, bgColor] = getColors();

    // Draw background
    painter.setBrush(bgColor);
    painter.setPen(Qt::NoPen);
    painter.drawRect(rect());

    // Draw X icon
    painter.setBrush(Qt::NoBrush);
    QPen pen(color, 1);
    pen.setCosmetic(true);
    painter.setPen(pen);

    qreal r = devicePixelRatioF();
    painter.scale(1.0 / r, 1.0 / r);

    int x0 = static_cast<int>(18 * r);
    int y0 = static_cast<int>(11 * r);
    int size = static_cast<int>(10 * r);

    // Two diagonal lines forming an X
    painter.drawLine(x0, y0, x0 + size, y0 + size);
    painter.drawLine(x0 + size, y0, x0, y0 + size);
}
// ============================================================================
// TitleBarBase
// ============================================================================

TitleBarBase::TitleBarBase(QWidget* parent) : QWidget(parent) {
    minBtn_ = new MinimizeButton(this);
    closeBtn_ = new CloseButton(this);
    maxBtn_ = new MaximizeButton(this);

    setFixedHeight(32);

    updateButtonColors();

    connect(&QConfig::instance(), &QConfig::themeChanged, this,
            [this](qfw::Theme) { updateButtonColors(); });

    // Connect signals
    connect(minBtn_, &QAbstractButton::clicked, this, [this]() {
        if (auto* w = window()) {
            w->showMinimized();
        }
    });

    connect(maxBtn_, &QAbstractButton::clicked, this, &TitleBarBase::toggleMaximized);

    connect(closeBtn_, &QAbstractButton::clicked, this, [this]() {
        if (auto* w = window()) {
            w->close();
        }
    });

    // Install event filter on parent window
    if (parent) {
        // Install on the actual window (not just parent) to receive WindowStateChange
        if (auto* w = parent->window()) {
            w->installEventFilter(this);
        }
    }
}

TitleBarBase::~TitleBarBase() = default;

MinimizeButton* TitleBarBase::minimizeButton() const { return minBtn_; }

MaximizeButton* TitleBarBase::maximizeButton() const { return maxBtn_; }

CloseButton* TitleBarBase::closeButton() const { return closeBtn_; }

bool TitleBarBase::isDoubleClickEnabled() const { return doubleClickEnabled_; }

void TitleBarBase::setDoubleClickEnabled(bool enabled) { doubleClickEnabled_ = enabled; }

bool TitleBarBase::canDrag(const QPoint& pos) const {
    return isDragRegion(pos) && !hasButtonPressed();
}

void TitleBarBase::mouseDoubleClickEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton || !doubleClickEnabled_) {
        QWidget::mouseDoubleClickEvent(event);
        return;
    }
    toggleMaximized();
}

void TitleBarBase::mouseMoveEvent(QMouseEvent* event) {
#ifdef Q_OS_WIN
    if (!canDrag(event->pos())) {
        return;
    }

    // Windows: use system move via WM_SYSCOMMAND SC_MOVE
    if (auto* w = window()) {
        if (auto* nativeWindow = w->windowHandle()) {
            HWND hWnd = reinterpret_cast<HWND>(nativeWindow->winId());
            ReleaseCapture();
            SendMessageW(hWnd, WM_SYSCOMMAND, SC_MOVE + HTCAPTION, 0);
        }
    }
#else
    QWidget::mouseMoveEvent(event);
#endif
}

void TitleBarBase::mousePressEvent(QMouseEvent* event) {
#ifndef Q_OS_WIN
    if (canDrag(event->pos())) {
        // Non-Windows: start drag manually
        if (auto* w = window()) {
            w->windowHandle()->startSystemMove();
        }
    }
#else
    QWidget::mousePressEvent(event);
#endif
}

bool TitleBarBase::eventFilter(QObject* watched, QEvent* event) {
    if (watched == window() && event->type() == QEvent::WindowStateChange) {
        maxBtn_->setMaximized(window()->isMaximized());
    }
    return QWidget::eventFilter(watched, event);
}

bool TitleBarBase::isDragRegion(const QPoint& pos) const {
    int width = 0;
    for (auto* btn : findChildren<TitleBarButton*>()) {
        if (btn->isVisible()) {
            width += btn->width();
        }
    }
    return pos.x() > 0 && pos.x() < this->width() - width;
}

bool TitleBarBase::hasButtonPressed() const {
    for (auto* btn : findChildren<TitleBarButton*>()) {
        if (btn->isPressed()) {
            return true;
        }
    }
    return false;
}

void TitleBarBase::updateButtonColors() {
    const QColor c = isDarkTheme() ? QColor(255, 255, 255) : QColor(0, 0, 0);

    if (minBtn_) {
        minBtn_->setNormalColor(c);
        minBtn_->setHoverColor(c);
        minBtn_->setPressedColor(c);
    }
    if (maxBtn_) {
        maxBtn_->setNormalColor(c);
        maxBtn_->setHoverColor(c);
        maxBtn_->setPressedColor(c);
    }

    if (closeBtn_) {
        closeBtn_->setNormalColor(c);
    }
}

void TitleBarBase::toggleMaximized() {
    if (auto* w = window()) {
        if (w->isMaximized()) {
            w->showNormal();
        } else {
            w->showMaximized();
        }
    }
}

// ============================================================================
// TitleBar
// ============================================================================

TitleBar::TitleBar(QWidget* parent) : TitleBarBase(parent) {
    hBoxLayout_ = new QHBoxLayout(this);
    hBoxLayout_->setSpacing(0);
    hBoxLayout_->setContentsMargins(0, 0, 0, 0);
    hBoxLayout_->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

    hBoxLayout_->addStretch(1);
    hBoxLayout_->addWidget(minBtn_, 0, Qt::AlignRight);
    hBoxLayout_->addWidget(maxBtn_, 0, Qt::AlignRight);
    hBoxLayout_->addWidget(closeBtn_, 0, Qt::AlignRight);
}

QHBoxLayout* TitleBar::hBoxLayout() const { return hBoxLayout_; }

// ============================================================================
// StandardTitleBar
// ============================================================================

StandardTitleBar::StandardTitleBar(QWidget* parent) : TitleBar(parent) {
    // Add window icon
    iconLabel_ = new QLabel(this);
    iconLabel_->setFixedSize(20, 20);
    hBoxLayout()->insertSpacing(0, 10);
    hBoxLayout()->insertWidget(1, iconLabel_, 0, Qt::AlignLeft);

    if (auto* w = window()) {
        connect(w, &QWidget::windowIconChanged, this, &StandardTitleBar::setIcon);
    }

    // Add title label
    titleLabel_ = new QLabel(this);
    hBoxLayout()->insertWidget(2, titleLabel_, 0, Qt::AlignLeft);
    titleLabel_->setStyleSheet(QStringLiteral(
        "QLabel{ background: transparent; font: 13px 'Segoe UI'; padding: 0 4px; }"));

    // Update title color on theme change
    auto updateTitleColor = [this]() {
        const QColor c = isDarkTheme() ? QColor(255, 255, 255) : QColor(0, 0, 0);
        titleLabel_->setStyleSheet(QStringLiteral("QLabel{ background: transparent; font: 13px "
                                                  "'Segoe UI'; padding: 0 4px; color: %1; }")
                                       .arg(c.name()));
    };
    connect(&QConfig::instance(), &QConfig::themeChanged, this, updateTitleColor);
    updateTitleColor();  // Set initial color

    if (auto* w = window()) {
        connect(w, &QWidget::windowTitleChanged, this, &StandardTitleBar::setTitle);
    }
}

void StandardTitleBar::setTitle(const QString& title) {
    titleLabel_->setText(title);
    titleLabel_->adjustSize();
}

void StandardTitleBar::setIcon(const QIcon& icon) { iconLabel_->setPixmap(icon.pixmap(20, 20)); }

}  // namespace qfw
