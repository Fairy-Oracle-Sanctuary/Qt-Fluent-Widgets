#include "components/widgets/frameless_window.h"

#include <QFrame>
#include <QPalette>
#include <QVBoxLayout>

#include "common/config.h"
#include "components/window/title_bar.h"
#include "components/window/frameless_window.h"

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

void FluentMainWindow::applyMica() {
#ifdef Q_OS_WIN
    const HWND hWnd = reinterpret_cast<HWND>(winId());
    if (!hWnd) {
        return;
    }

    WindowsWindowEffect eff;
    eff.setMicaEffect(hWnd, qfw::isDarkTheme(), false);
#endif
}

void FluentMainWindow::showEvent(QShowEvent* e) {
    FramelessMainWindow::showEvent(e);

    if (!micaApplied_) {
        micaApplied_ = true;

        // Apply after show, consistent with main.cpp
        applyMica();

        QObject::connect(&qfw::QConfig::instance(), &qfw::QConfig::themeChanged, this,
                         [this]() { applyMica(); });
    }
}

}  // namespace qfw
