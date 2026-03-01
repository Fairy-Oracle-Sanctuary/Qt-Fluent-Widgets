#include "window/fluent_window.h"

#include <QApplication>
#include <QLabel>
#include <QLayout>
#include <QPaintEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QStackedWidget>

#include "common/config.h"
#include "common/icon.h"
#include "common/router.h"
#include "common/style_sheet.h"
#include "components/navigation/navigation_bar.h"
#include "components/navigation/navigation_interface.h"
#include "components/window/title_bar.h"
#include "window/stacked_widget.h"

#ifdef Q_OS_WIN
#include <windows.h>

#include "components/window/windows_window_effect.h"

#endif

namespace qfw {

// ============================================================================
// FluentWidget
// ============================================================================

FluentWidget::FluentWidget(QWidget* parent) : FluentMainWindow(parent) {
    backgroundColor_ = normalBackgroundColor();

    // FramelessWindow has its own StandardTitleBar. FluentWindow uses its own title bar,
    // so hide the default one to avoid double title bars.
    if (auto* standardTb = findChild<qfw::StandardTitleBar*>()) {
        standardTb->hide();
        standardTb->setFixedHeight(0);
    }

    bgAni_ = new QPropertyAnimation(this, QByteArrayLiteral("backgroundColor"), this);
    bgAni_->setDuration(150);

    // enable mica by default on win11 like python
    setMicaEffectEnabled(true);

    // set up title bar (python: FluentWidgetTitleBar)
    setTitleBar(new qfw::FluentWidgetTitleBar(this));

    connect(&QConfig::instance(), &QConfig::themeChangedFinished, this,
            &FluentWidget::onThemeChangedFinished);
}

void FluentWidget::setContentWidget(QWidget* widget) { FluentMainWindow::setContentWidget(widget); }

void FluentWidget::setCustomBackgroundColor(const QColor& light, const QColor& dark) {
    lightBackgroundColor_ = QColor(light);
    darkBackgroundColor_ = QColor(dark);
    setBackgroundColor(normalBackgroundColor());
}

QColor FluentWidget::normalBackgroundColor() const {
    if (!isMicaEffectEnabled()) {
        return isDarkTheme() ? darkBackgroundColor_ : lightBackgroundColor_;
    }
    return QColor(0, 0, 0, 0);
}

void FluentWidget::applyMica() {
#ifdef Q_OS_WIN
    const HWND hWnd = reinterpret_cast<HWND>(winId());
    if (!hWnd) {
        return;
    }

    WindowsWindowEffect eff;
    eff.setMicaEffect(hWnd, isDarkTheme(), false);
#endif
}

void FluentWidget::setMicaEffectEnabled(bool enabled) {
#ifdef Q_OS_WIN
    // match python: win11 build >= 22000
    if (enabled) {
        isMicaEnabled_ = true;
        applyMica();
        setBackgroundColor(normalBackgroundColor());
    } else {
        isMicaEnabled_ = false;
        // No explicit remove API; fallback to transparent background.
        setBackgroundColor(normalBackgroundColor());
    }
#else
    Q_UNUSED(enabled);
#endif
}

void FluentWidget::setBackgroundColor(const QColor& c) {
    backgroundColor_ = c;
    update();
}

QRect FluentWidget::systemTitleBarRect(const QSize& size) const {
    Q_UNUSED(size);
    // only meaningful on macOS in python; keep a reasonable default.
    return QRect(0, isFullScreen() ? 0 : 2, 75, size.height());
}

void FluentWidget::setTitleBar(TitleBarBase* titleBar) {
    if (!titleBar) {
        return;
    }

    if (titleBar_ == titleBar) {
        return;
    }

    if (titleBar_) {
        titleBar_->setParent(nullptr);
        titleBar_->deleteLater();
    }

    titleBar_ = titleBar;
    titleBar_->setParent(this);
    titleBar_->raise();
}

void FluentWidget::onThemeChangedFinished() {
    if (isMicaEffectEnabled()) {
        applyMica();
    }
}

void FluentWidget::paintEvent(QPaintEvent* e) {
    FluentMainWindow::paintEvent(e);

    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setBrush(backgroundColor_);
    painter.drawRect(rect());
}

void FluentWidget::showEvent(QShowEvent* e) {
    FluentMainWindow::showEvent(e);

    if (!micaApplied_) {
        micaApplied_ = true;
        if (isMicaEffectEnabled()) {
            applyMica();
        }
    }
}

// ============================================================================
// FluentWindowBase
// ============================================================================

FluentWindowBase::FluentWindowBase(QWidget* parent) : FluentWidget(parent) {
    hBoxLayout_ = new QHBoxLayout();
    hBoxLayout_->setSpacing(0);
    hBoxLayout_->setContentsMargins(0, 0, 0, 0);

    stackedWidget_ = new qfw::StackedWidget(this);

    // Apply FluentWindow stylesheet to stackedWidget like python
    qfw::setStyleSheet(stackedWidget_, qfw::FluentStyleSheet::FluentWindow);

    auto* host = new QWidget(this);
    host->setLayout(hBoxLayout_);
    setContentWidget(host);
}

void FluentWindowBase::switchTo(QWidget* subInterface) {
    if (!stackedWidget_) {
        return;
    }
    stackedWidget_->setCurrentWidget(subInterface, false);
}

void FluentWindowBase::onCurrentInterfaceChanged(int index) {
    if (!stackedWidget_ || !navigationInterface_) {
        return;
    }

    QWidget* w = stackedWidget_->widget(index);
    if (!w) {
        return;
    }

    navigationInterface_->setCurrentItem(w->objectName());

    if (Router* history = navigationInterface_->history(); history && stackedWidget_->view()) {
        history->push(stackedWidget_->view(), w->objectName());
    }

    updateStackedBackground();
}

void FluentWindowBase::updateStackedBackground() {
    if (!stackedWidget_ || !stackedWidget_->currentWidget()) {
        return;
    }

    const bool isTransparent =
        stackedWidget_->currentWidget()->property("isStackedTransparent").toBool();
    if (stackedWidget_->property("isTransparent").toBool() == isTransparent) {
        return;
    }

    stackedWidget_->setProperty("isTransparent", isTransparent);
    stackedWidget_->setStyle(QApplication::style());
}

QRect FluentWindowBase::systemTitleBarRect(const QSize& size) const {
    return QRect(size.width() - 75, isFullScreen() ? 0 : 8, 75, size.height());
}

// ============================================================================
// FluentTitleBar
// ============================================================================

FluentTitleBar::FluentTitleBar(QWidget* parent) : TitleBar(parent) {
    setFixedHeight(48);
    setProperty("qssClass", "FluentTitleBar");

    // Rebuild the default TitleBar layout to match python:
    // remove window buttons from the main hBox, then add them into a top-aligned vBox on the right.
    if (auto* layout = hBoxLayout()) {
        layout->removeWidget(minimizeButton());
        layout->removeWidget(maximizeButton());
        layout->removeWidget(closeButton());
    }

    iconLabel_ = new QLabel(this);
    iconLabel_->setFixedSize(18, 18);

    titleLabel_ = new QLabel(this);
    titleLabel_->setObjectName(QStringLiteral("titleLabel"));

    // Insert icon + title at the start
    hBoxLayout()->insertWidget(0, iconLabel_, 0, Qt::AlignLeft | Qt::AlignVCenter);
    hBoxLayout()->insertSpacing(1, 6);
    hBoxLayout()->insertWidget(2, titleLabel_, 0, Qt::AlignLeft | Qt::AlignVCenter);

    auto* vBox = new QVBoxLayout();
    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(0);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setAlignment(Qt::AlignTop);
    buttonLayout->addWidget(minimizeButton());
    buttonLayout->addWidget(maximizeButton());
    buttonLayout->addWidget(closeButton());

    vBox->addLayout(buttonLayout);
    vBox->addStretch(1);
    hBoxLayout()->addLayout(vBox, 0);

    if (auto* w = window()) {
        connect(w, &QWidget::windowTitleChanged, this, &FluentTitleBar::setTitle);
        connect(w, &QWidget::windowIconChanged, this, &FluentTitleBar::setIcon);
    }

    qfw::setStyleSheet(this, qfw::FluentStyleSheet::FluentWindow);

    updateWindowIcon();
    updateWindowTitle();
}

// FluentWidgetTitleBar
// ============================================================================

FluentWidgetTitleBar::FluentWidgetTitleBar(QWidget* parent) : FluentTitleBar(parent) {
    hBoxLayout()->setContentsMargins(16, 0, 0, 0);

    // Height follows button row like python
    const int h = closeButton() ? closeButton()->height() : 0;
    setFixedHeight(h > 0 ? h : 32);

    for (auto* button : findChildren<qfw::TitleBarButton*>()) {
        qfw::setStyleSheet(button, qfw::FluentStyleSheet::FluentWindow);
    }
}

void FluentTitleBar::updateWindowTitle() {
    if (auto* w = window()) {
        setTitle(w->windowTitle());
    }
}

void FluentTitleBar::updateWindowIcon() {
    if (auto* w = window()) {
        setIcon(w->windowIcon());
    }
}

void FluentTitleBar::setTitle(const QString& title) {
    if (titleLabel_) {
        titleLabel_->setText(title);
        titleLabel_->adjustSize();
    }
}

void FluentTitleBar::setIcon(const QIcon& icon) {
    if (iconLabel_) {
        iconLabel_->setPixmap(icon.pixmap(18, 18));
    }
}

// ============================================================================
// FluentWindow
// ============================================================================

FluentWindow::FluentWindow(QWidget* parent) : FluentWindowBase(parent) {
    setTitleBar(new qfw::FluentTitleBar(this));

    navigationInterface_ = new qfw::NavigationInterface(this, true, true, true);
    navigationInterface_->setAcrylicEnabled(true);
    widgetLayout_ = new QHBoxLayout();

    // initialize layout
    hBoxLayout_->addWidget(navigationInterface_);
    hBoxLayout_->addLayout(widgetLayout_);

    hBoxLayout_->setStretchFactor(widgetLayout_, 1);

    widgetLayout_->addWidget(stackedWidget_);
    widgetLayout_->setContentsMargins(0, 48, 0, 0);

    connect(navigationInterface_, &qfw::NavigationInterface::displayModeChanged, this, [this]() {
        if (titleBar()) {
            titleBar()->raise();
        }
    });

    if (titleBar()) {
        titleBar()->raise();
    }
}

NavigationWidget* FluentWindow::addSubInterface(QWidget* subInterface, const QVariant& icon,
                                                const QString& text) {
    return addSubInterface(subInterface, icon, text, qfw::NavigationItemPosition::Top, nullptr,
                           false);
}

NavigationWidget* FluentWindow::addSubInterface(QWidget* subInterface, FluentIconEnum icon,
                                                const QString& text,
                                                NavigationItemPosition position) {
    return addSubInterface(subInterface, FluentIcon(icon).qicon(), text, position);
}

NavigationWidget* FluentWindow::addSubInterface(QWidget* subInterface, const FluentIconBase& icon,
                                                const QString& text,
                                                NavigationItemPosition position) {
    return addSubInterface(subInterface, icon.qicon(), text, position);
}

NavigationWidget* FluentWindow::addSubInterface(QWidget* subInterface, const QVariant& icon,
                                                const QString& text,
                                                qfw::NavigationItemPosition position,
                                                QWidget* parent, bool isTransparent) {
    if (!subInterface) {
        return nullptr;
    }

    if (subInterface->objectName().isEmpty()) {
        return nullptr;
    }

    subInterface->setProperty("isStackedTransparent", isTransparent);

    if (stackedWidget_) {
        stackedWidget_->addWidget(subInterface);
    }

    const QString routeKey = subInterface->objectName();

    NavigationWidget* item = nullptr;
    const QString parentRouteKey = parent ? parent->objectName() : QString();
    item = navigationInterface_->addItem(
        routeKey, icon, text, [this, subInterface]() { switchTo(subInterface); }, true, position,
        QString(), parentRouteKey);

    if (stackedWidget_ && stackedWidget_->count() == 1) {
        connect(stackedWidget_, &qfw::StackedWidget::currentChanged, this,
                [this](int index) { onCurrentInterfaceChanged(index); });
        if (navigationInterface_) {
            navigationInterface_->setCurrentItem(routeKey);
        }

        if (Router* history = navigationInterface_->history(); history && stackedWidget_->view()) {
            history->setDefaultRouteKey(stackedWidget_->view(), routeKey);
        }
    }

    updateStackedBackground();
    return item;
}

NavigationWidget* FluentWindow::addSubInterface(QWidget* subInterface, const QVariant& icon,
                                                const QString& text,
                                                qfw::NavigationItemPosition position) {
    return addSubInterface(subInterface, icon, text, position, nullptr, false);
}

NavigationWidget* FluentWindow::addSubInterface(QWidget* subInterface, FluentIconEnum icon,
                                                const QString& text) {
    return addSubInterface(subInterface, FluentIcon(icon).qicon(), text);
}

NavigationWidget* FluentWindow::addSubInterface(QWidget* subInterface, const FluentIconBase& icon,
                                                const QString& text) {
    return addSubInterface(subInterface, icon.qicon(), text);
}

// ==========================================================================
// MSFluentTitleBar
// ==========================================================================

MSFluentTitleBar::MSFluentTitleBar(QWidget* parent) : FluentTitleBar(parent) {
    // Python:
    //   self.hBoxLayout.insertSpacing(0, 20)
    //   self.hBoxLayout.insertSpacing(2, 2)
    // Python's FluentTitleBar does NOT insert spacing between icon and title.
    // Our C++ FluentTitleBar inserts spacing(1, 6) between icon and title.
    // Remove it first, then apply MS style spacing.
    if (auto* layout = hBoxLayout()) {
        // Remove the icon-title spacing inserted by FluentTitleBar (at index 1)
        if (layout->count() > 1) {
            QLayoutItem* item = layout->itemAt(1);
            if (item && item->spacerItem()) {
                layout->removeItem(item);
                delete item;
            }
        }

        // Insert MS style spacing:
        // insertSpacing(0, 0) - left padding before icon (tuned to be closer to the left)
        // insertSpacing(2, 10) - spacing between icon and title (tuned to be slightly larger)
        layout->insertSpacing(0, 0);
        layout->insertSpacing(2, 10);
    }

    // Increase title font size (use pixel size for reliability)
    if (titleLabel_) {
        QFont f = titleLabel_->font();
        f.setPixelSize(12);  // Slightly larger than default
        titleLabel_->setFont(f);
    }
}

// ==========================================================================
// MSFluentWindow
// ==========================================================================

MSFluentWindow::MSFluentWindow(QWidget* parent) : FluentWindowBase(parent) {
    setTitleBar(new qfw::MSFluentTitleBar(this));

    navigationBar_ = new qfw::NavigationBar(this);

    // initialize layout (python: margins (0,48,0,0))
    if (hBoxLayout_) {
        hBoxLayout_->setContentsMargins(0, 48, 0, 0);
        hBoxLayout_->addWidget(navigationBar_);
        hBoxLayout_->addWidget(stackedWidget_, 1);
    }

    if (titleBar()) {
        titleBar()->raise();
        titleBar()->setAttribute(Qt::WA_StyledBackground);
    }
}

void MSFluentWindow::resizeEvent(QResizeEvent* event) {
    // For MSFluentWindow we keep the title bar aligned to the window's left edge,
    // so icon/title can be closer to the left.
    if (titleBar()) {
        titleBar()->move(24, 0);
        titleBar()->resize(width() - 24, titleBar()->height());
    }
    FluentWindowBase::resizeEvent(event);
}

NavigationWidget* MSFluentWindow::addSubInterface(QWidget* subInterface, const QVariant& icon,
                                                  const QString& text,
                                                  NavigationItemPosition position) {
    return addSubInterface(subInterface, icon, text, QVariant(), position, false);
}

NavigationWidget* MSFluentWindow::addSubInterface(QWidget* subInterface, FluentIconEnum icon,
                                                  const QString& text,
                                                  NavigationItemPosition position) {
    return addSubInterface(subInterface, FluentIcon(icon).qicon(), text, position);
}

NavigationWidget* MSFluentWindow::addSubInterface(QWidget* subInterface, const FluentIconBase& icon,
                                                  const QString& text,
                                                  NavigationItemPosition position) {
    return addSubInterface(subInterface, icon.qicon(), text, position);
}

NavigationWidget* MSFluentWindow::addSubInterface(QWidget* subInterface, const QVariant& icon,
                                                  const QString& text, const QVariant& selectedIcon,
                                                  NavigationItemPosition position,
                                                  bool isTransparent) {
    if (!subInterface || subInterface->objectName().isEmpty() || !navigationBar_ ||
        !stackedWidget_) {
        return nullptr;
    }

    subInterface->setProperty("isStackedTransparent", isTransparent);
    stackedWidget_->addWidget(subInterface);

    const QString routeKey = subInterface->objectName();
    auto* item = navigationBar_->addItem(
        routeKey, icon, text, [this, subInterface]() { switchTo(subInterface); }, true,
        selectedIcon, position);

    if (stackedWidget_->count() == 1) {
        connect(stackedWidget_, &qfw::StackedWidget::currentChanged, this,
                &MSFluentWindow::onCurrentInterfaceChangedMs);
        navigationBar_->setCurrentItem(routeKey);

        if (Router* history = navigationBar_->history(); history && stackedWidget_->view()) {
            history->setDefaultRouteKey(stackedWidget_->view(), routeKey);
        }
    }

    updateStackedBackground();
    return item;
}

void MSFluentWindow::removeInterface(QWidget* subInterface, bool isDelete) {
    if (!subInterface) {
        return;
    }

    if (navigationBar_) {
        navigationBar_->removeWidget(subInterface->objectName());
    }
    if (stackedWidget_) {
        stackedWidget_->removeWidget(subInterface);
    }

    subInterface->hide();
    if (isDelete) {
        subInterface->deleteLater();
    }
}

void MSFluentWindow::onCurrentInterfaceChangedMs(int index) {
    if (!stackedWidget_ || !navigationBar_) {
        return;
    }

    QWidget* w = stackedWidget_->widget(index);
    if (!w) {
        return;
    }

    navigationBar_->setCurrentItem(w->objectName());

    if (Router* history = navigationBar_->history(); history && stackedWidget_->view()) {
        history->push(stackedWidget_->view(), w->objectName());
    }

    updateStackedBackground();
}

void FluentWindow::removeInterface(QWidget* subInterface, bool isDelete) {
    if (!subInterface) {
        return;
    }

    if (navigationInterface_) {
        navigationInterface_->removeWidget(subInterface->objectName());
    }
    if (stackedWidget_) {
        stackedWidget_->removeWidget(subInterface);
    }

    subInterface->hide();
    if (isDelete) {
        subInterface->deleteLater();
    }
}

void FluentWindow::resizeEvent(QResizeEvent* e) {
    FluentWindowBase::resizeEvent(e);

    // python: self.titleBar.move(46, 0); self.titleBar.resize(self.width()-46,
    // self.titleBar.height())
    if (titleBar()) {
        titleBar()->move(46, 0);
        titleBar()->resize(width() - 46, titleBar()->height());
    }
}

// ============================================================================
// SplitTitleBar
// ============================================================================

SplitTitleBar::SplitTitleBar(QWidget* parent) : TitleBar(parent) {
    // Add window icon
    iconLabel_ = new QLabel(this);
    iconLabel_->setFixedSize(18, 18);
    hBoxLayout()->insertSpacing(0, 12);
    hBoxLayout()->insertWidget(1, iconLabel_, 0, Qt::AlignLeft | Qt::AlignBottom);

    if (auto* w = window()) {
        connect(w, &QWidget::windowIconChanged, this, [this](const QIcon& icon) { setIcon(icon); });
    }

    // Add spacing between icon and title
    hBoxLayout()->insertSpacing(2, 8);

    // Add title label
    titleLabel_ = new QLabel(this);
    hBoxLayout()->insertWidget(3, titleLabel_, 0, Qt::AlignLeft | Qt::AlignBottom);
    titleLabel_->setObjectName(QStringLiteral("titleLabel"));

    // Apply style
    qfw::setStyleSheet(this, qfw::FluentStyleSheet::FluentWindow);

    if (auto* w = window()) {
        connect(w, &QWidget::windowTitleChanged, this,
                [this](const QString& title) { setTitle(title); });
    }
}

void SplitTitleBar::setTitle(const QString& title) {
    if (titleLabel_) {
        titleLabel_->setText(title);
        titleLabel_->adjustSize();
    }
}

void SplitTitleBar::setIcon(const QIcon& icon) {
    if (iconLabel_) {
        iconLabel_->setPixmap(icon.pixmap(18, 18));
    }
}

// ============================================================================
// SplitFluentWindow
// ============================================================================

SplitFluentWindow::SplitFluentWindow(QWidget* parent) : FluentWindow(parent) {
    setTitleBar(new SplitTitleBar(this));

#ifdef Q_OS_MACOS
    titleBar()->setFixedHeight(48);
#endif

    // python: self.widgetLayout.setContentsMargins(0, 0, 0, 0)
    if (widgetLayout_) {
        widgetLayout_->setContentsMargins(0, 0, 0, 0);
    }

    if (titleBar()) {
        titleBar()->raise();
    }

    // Connect displayModeChanged to raise title bar
    if (navigationInterface_) {
        connect(navigationInterface_, &NavigationInterface::displayModeChanged, this, [this]() {
            if (titleBar()) titleBar()->raise();
        });
    }
}

}  // namespace qfw
