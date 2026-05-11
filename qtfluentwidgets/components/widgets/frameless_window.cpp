#include "components/widgets/frameless_window.h"

#include <QApplication>
#include <QFrame>
#include <QPainter>
#include <QPalette>
#include <QVBoxLayout>

#include "common/config.h"
#include "components/window/frameless_window.h"
#include "components/window/title_bar.h"

#ifdef Q_OS_WIN
#include "components/window/windows_window_effect.h"
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

void FluentMainWindow::applyMica() {
#ifdef Q_OS_WIN
    const HWND hWnd = reinterpret_cast<HWND>(winId());
    if (!hWnd) {
        return;
    }

    setAttribute(Qt::WA_TranslucentBackground, true);

    WindowsWindowEffect eff;
    eff.setMicaEffect(hWnd, qfw::isDarkTheme(), false);
#endif
}

void FluentMainWindow::showEvent(QShowEvent* e) {
    FramelessMainWindow::showEvent(e);

    if (!micaApplied_) {
        micaApplied_ = true;
        applyMica();

        QObject::connect(&qfw::QConfig::instance(), &qfw::QConfig::themeChanged, this,
                         [this]() { applyMica(); });
    }
}

void FluentMainWindow::paintEvent(QPaintEvent* e) {
#ifdef Q_OS_WIN
    // Fix click-through on Qt 5.15.2 with Mica/Acrylic:
    // WA_TranslucentBackground may cause Windows to set WS_EX_TRANSPARENT,
    // which makes the window click-through. Remove it on every paint.
    const HWND hWnd = reinterpret_cast<HWND>(winId());
    if (hWnd) {
        LONG_PTR exStyle = ::GetWindowLongPtrW(hWnd, GWL_EXSTYLE);
        if (exStyle & WS_EX_TRANSPARENT) {
            exStyle &= ~WS_EX_TRANSPARENT;
            ::SetWindowLongPtrW(hWnd, GWL_EXSTYLE, exStyle);
        }
    }
#endif

    if (micaApplied_) {
        QPainter painter(this);
        painter.setCompositionMode(QPainter::CompositionMode_Clear);
        painter.eraseRect(rect());
        return;
    }
    FramelessMainWindow::paintEvent(e);
}

}  // namespace qfw
