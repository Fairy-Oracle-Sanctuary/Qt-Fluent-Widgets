#include "components/widgets/frameless_window.h"

#include <QFrame>
#include <QPalette>
#include <QTimer>
#include <QVBoxLayout>

#include "common/config.h"
#include "components/window/title_bar.h"

#ifdef Q_OS_WIN
#include "components/window/windows_window_effect.h"
#endif

#ifdef Q_OS_MAC
#include "components/window/mac_window_effect.h"
#endif

namespace qfw {

FluentMainWindow::FluentMainWindow(QWidget* parent) : FramelessMainWindow(parent) {
    setWindowTitle(QStringLiteral("qfluentwidgets frameless window"));

    auto* root = new QWidget(this);
    auto* rootLayout = new QVBoxLayout(root);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    titleBar_ = new qfw::StandardTitleBar(root);
    rootLayout->addWidget(titleBar_);

    contentFrame_ = new QFrame(root);
    contentFrame_->setObjectName(QStringLiteral("contentFrame"));
    contentFrame_->setFrameShape(QFrame::NoFrame);
    rootLayout->addWidget(contentFrame_, 1);

    setCentralWidget(root);

    // Align with main.cpp Mica setup
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAutoFillBackground(false);
    setStyleSheet(QStringLiteral("background: transparent;"));

    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(0, 0, 0, 0));
    setPalette(pal);
}

void FluentMainWindow::setContentWidget(QWidget* widget) {
    if (!contentFrame_ || !widget) {
        return;
    }

    if (!contentFrame_->layout()) {
        auto* layout = new QVBoxLayout(contentFrame_);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
    }

    if (auto* layout = qobject_cast<QLayout*>(contentFrame_->layout())) {
        while (QLayoutItem* item = layout->takeAt(0)) {
            if (QWidget* w = item->widget()) {
                w->setParent(nullptr);
            }
            delete item;
        }
        layout->addWidget(widget);
    }
}

void FluentMainWindow::clearDefaultTitleBar() {
    // Clear FluentMainWindow's titleBar_
    if (titleBar_) {
        titleBar_->hide();
        titleBar_->setParent(nullptr);
        delete titleBar_;
        titleBar_ = nullptr;
    }

    // Also clear MacFramelessWindowBase's titleBar_ on macOS
#ifdef Q_OS_MAC
    MacFramelessWindowBase::clearTitleBar();
#endif
}

void FluentMainWindow::applyMica() {
#ifdef Q_OS_WIN
    const HWND hWnd = reinterpret_cast<HWND>(winId());
    if (!hWnd) {
        return;
    }

    WindowsWindowEffect eff;
    eff.setMicaEffect(hWnd, qfw::isDarkTheme(), false);
#elif defined(Q_OS_MAC)
    MacWindowEffect eff;
    eff.setMicaEffect(this, qfw::isDarkTheme(), false);
#endif
}

void FluentMainWindow::showEvent(QShowEvent* e) {
    FramelessMainWindow::showEvent(e);

    if (!micaApplied_) {
        micaApplied_ = true;

        // Delay applyMica to let Qt finish setting up the Cocoa view hierarchy.
        // On macOS, the NSView may not be added to contentView yet when showEvent fires.
        QTimer::singleShot(0, this, [this]() {
            applyMica();
        });

        QObject::connect(&qfw::QConfig::instance(), &qfw::QConfig::themeChanged, this,
                         [this]() { applyMica(); });
    }
}

}  // namespace qfw
