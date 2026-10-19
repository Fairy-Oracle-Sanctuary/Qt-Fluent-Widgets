#include "frameless_web_engine_view.h"
#include "components/window/frameless_window.h"

namespace qfw {
static QWidget* prepareWebEngineHost(QWidget* parent) {
#ifdef Q_OS_WIN
    if (parent && dynamic_cast<WindowsFramelessWindowBase*>(parent->window()))
        parent->window()->setAttribute(Qt::WA_TranslucentBackground);
#endif
    return parent;
}
FramelessWebEngineView::FramelessWebEngineView(QWidget* parent)
    : QWebEngineView(prepareWebEngineHost(parent)) {
#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
    setHtml(QString());
#endif
#ifdef Q_OS_WIN
    if (auto* host = qobject_cast<WindowsFramelessMainWindow*>(window())) host->updateFrameless();
    else if (auto* host = qobject_cast<WindowsFramelessWindow*>(window())) host->updateFrameless();
    else if (auto* host = qobject_cast<WindowsFramelessDialog*>(window())) host->updateFrameless();
#endif
}
}
