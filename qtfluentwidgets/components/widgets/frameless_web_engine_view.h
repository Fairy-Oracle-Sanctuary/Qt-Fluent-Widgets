#pragma once
#include <QWebEngineView>

namespace qfw {
// Optional WebEngine integration, matching qframelesswindow.webengine.
class FramelessWebEngineView : public QWebEngineView {
    Q_OBJECT
public:
    explicit FramelessWebEngineView(QWidget* parent = nullptr);
};
}
