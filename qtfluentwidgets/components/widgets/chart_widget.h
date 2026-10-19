#pragma once

#include "card_widget.h"
#include <QJsonObject>
#include <QTimer>
#include <memory>

class QWebEngineView;
class QWebEngineProfile;

namespace qfw {
class ChartFrameGate;

// Explicitly link qtfluentwidgets_chart; the base library has no WebEngine dependency.
// Construct before showing a frameless host. All methods run on the GUI thread.
// setOptionJS executes trusted application code, not untrusted user input.
class ChartWidget : public SimpleCardWidget {
    Q_OBJECT
public:
    explicit ChartWidget(QWidget* parent = nullptr);
    ~ChartWidget() override;
    void setOption(const QJsonObject& option);
    void setOptionJS(const QString& source);
    void setAnimationEnabled(bool enabled);
    void clear();
    using QWidget::resize;
    void resize();
    QSize sizeHint() const override { return QSize(400, 300); }
    QWebEngineView* browser() const { return browser_; }
    bool isInitialized() const { return initialized_; }
    bool isPageRequested() const { return pageRequested_; }
    bool isFramePending() const;
signals:
    void loadFinished(bool success);
    void renderWarning(const QString& message);
protected:
    QColor normalBackgroundColor() const override;
    QColor hoverBackgroundColor() const override { return normalBackgroundColor(); }
    QColor pressedBackgroundColor() const override { return normalBackgroundColor(); }
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
private:
    void scheduleUpdate();
    void updateChart();
    void themeChanged();
    QJsonObject option_;
    QString jsOption_;
    std::shared_ptr<QWebEngineProfile> profile_;
    QWebEngineView* browser_ = nullptr;
    ChartFrameGate* gate_ = nullptr;
    QVBoxLayout* layout_ = nullptr;
    QTimer renderTimer_;
    bool initialized_ = false, pageRequested_ = false;
    bool pendingOption_ = false, replayPending_ = false, animationEnabled_ = true;
};
}
