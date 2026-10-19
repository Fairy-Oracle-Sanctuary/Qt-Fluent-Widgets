#include "chart_widget.h"
#include "frameless_web_engine_view.h"
#include "common/config.h"
#include <QApplication>
#include <QHideEvent>
#include <QJsonArray>
#include <QJsonDocument>
#include <QPainter>
#include <QQuickWidget>
#include <QResizeEvent>
#include <QShowEvent>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineView>
#include <mutex>
#include <functional>

static void initChartResources() { Q_INIT_RESOURCE(chart); }

namespace qfw {
static QString json(const QJsonArray& value) {
    return QString::fromUtf8(QJsonDocument(value).toJson(QJsonDocument::Compact));
}

class ChartPage : public QWebEnginePage {
public:
    ChartPage(QWebEngineProfile* profile, ChartWidget* owner, QObject* parent)
        : QWebEnginePage(profile, parent), owner_(owner) {}
protected:
    void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString& message,
                                  int, const QString&) override {
        if (level == ErrorMessageLevel && owner_) {
            const auto owner = owner_;
            QMetaObject::invokeMethod(owner_, [owner,message] {
                if (owner) emit owner->renderWarning(message);
            }, Qt::QueuedConnection);
        }
    }
private:
    QPointer<ChartWidget> owner_;
};

// Do not reveal based on a JS acknowledgement or an elapsed timeout: Chromium
// and Qt present independently. Confirm marker, then blank background pixels.
class ChartFrameGate : public QWidget {
public:
    ChartFrameGate(QWebEngineView* browser, QWidget* parent, std::function<void()> ready)
        : QWidget(parent), browser_(browser), ready_(std::move(ready)) {
        setObjectName(QStringLiteral("chartFrameGate"));
        setAttribute(Qt::WA_OpaquePaintEvent);
        timer_.setInterval(16);
        QObject::connect(&timer_, &QTimer::timeout, this, [this] { checkFrame(); });
        hide();
    }
    void arm(const QColor& background) {
        cancel(); pending = true; background_ = background;
        setGeometry(browser_->geometry()); update(); show(); raise();
    }
    void prepare() {
        if (!pending || phase_) return;
        if (QApplication::platformName() == QStringLiteral("offscreen")) { finish(); return; }
        ++serial_; marker_ = QColor(17 + serial_ % 200, 31 + (serial_ / 200) % 190, 197);
        phase_ = 1;
        browser_->page()->runJavaScript(QStringLiteral("prepareChartFrame.apply(null,%1);")
            .arg(json({marker_.name(), background_.name()})));
        timer_.start();
    }
    void cancel() { timer_.stop(); pending = false; phase_ = 0; hide(); }
    bool pending = false;
protected:
    void paintEvent(QPaintEvent*) override { QPainter p(this); p.fillRect(rect(), background_); }
private:
    void checkFrame() {
        if (!pending || !browser_->isVisible()) return;
        QImage frame;
        if (auto* surface = browser_->findChild<QQuickWidget*>()) {
            if (!surface->isVisible()) return;
            frame = surface->grabFramebuffer();
        } else {
            // Qt 5's QWidget delegate presents into the widget backing store.
            frame = browser_->grab().toImage();
        }
        if (frame.isNull()) return;
        const auto expected = phase_ == 1 ? marker_ : background_;
        for (int y : {0, frame.height()/2, frame.height()-1})
            for (int x : {0, frame.width()/2, frame.width()-1})
                if (frame.pixelColor(x, y).rgb() != expected.rgb()) return;
        if (phase_ == 1) {
            phase_ = 2; browser_->page()->runJavaScript(QStringLiteral("releaseChartFrame();"));
        } else finish();
    }
    void finish() { cancel(); const auto ready = ready_; ready(); }
    QWebEngineView* browser_;
    std::function<void()> ready_;
    QTimer timer_;
    QColor background_, marker_;
    int serial_ = 0, phase_ = 0;
};

ChartWidget::ChartWidget(QWidget* parent) : SimpleCardWidget(parent) {
    static std::once_flag resources;
    std::call_once(resources, initChartResources);
    setProperty("qssClass", "SimpleCardWidget");
    setBackgroundColor(normalBackgroundColor());
    layout_ = new QVBoxLayout(this); layout_->setContentsMargins(15,15,15,15);
    static std::weak_ptr<QWebEngineProfile> sharedProfile;
    profile_ = sharedProfile.lock();
    if (!profile_) { profile_ = std::make_shared<QWebEngineProfile>(); sharedProfile = profile_; }
    browser_ = new FramelessWebEngineView(this);
    browser_->setObjectName(QStringLiteral("chartBrowser"));
    auto* page = new ChartPage(profile_.get(), this, browser_); browser_->setPage(page);
    page->setBackgroundColor(normalBackgroundColor());
    layout_->addWidget(browser_);
    // Initialize the native surface before Mica is applied to the host.
#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
    browser_->setHtml(QString());
#endif
    gate_ = new ChartFrameGate(browser_, this, [this] { updateChart(); });
    renderTimer_.setSingleShot(true);
    connect(&renderTimer_, &QTimer::timeout, this, &ChartWidget::updateChart);
    connect(browser_, &QWebEngineView::loadFinished, this, [this](bool success) {
        if (!pageRequested_ || browser_->url() != QUrl("qrc:/qfluentwidgets/js/chart.html")) return;
        initialized_ = success;
        if (success) { if (gate_->pending) gate_->prepare(); else scheduleUpdate(); }
        else gate_->cancel();
        emit loadFinished(success);
    });
    connect(&QConfig::instance(), &QConfig::themeChanged, this, &ChartWidget::themeChanged);
}
ChartWidget::~ChartWidget() {
    renderTimer_.stop(); gate_->cancel();
    // A page must be destroyed before its profile (QObject child order differs).
    delete browser_; browser_ = nullptr;
    profile_.reset();
}
bool ChartWidget::isFramePending() const { return gate_->pending; }
QColor ChartWidget::normalBackgroundColor() const { return QColor(isDarkTheme() ? "#202020" : "#f3f3f3"); }
void ChartWidget::setOption(const QJsonObject& option) { option_ = option; jsOption_.clear(); pendingOption_ = true; scheduleUpdate(); }
void ChartWidget::setOptionJS(const QString& source) { jsOption_ = source; option_ = {}; pendingOption_ = true; scheduleUpdate(); }
void ChartWidget::setAnimationEnabled(bool enabled) { animationEnabled_ = enabled; scheduleUpdate(); }
void ChartWidget::clear() {
    option_ = {}; jsOption_.clear(); pendingOption_ = replayPending_ = false; renderTimer_.stop();
    if (initialized_) browser_->page()->runJavaScript(QStringLiteral("clearChart();"));
}
void ChartWidget::resize() { if (initialized_) browser_->page()->runJavaScript(QStringLiteral("scheduleResize();")); }
void ChartWidget::scheduleUpdate() { if (initialized_ && isVisible()) renderTimer_.start(0); }
void ChartWidget::updateChart() {
    if (!initialized_ || !isVisible() || gate_->pending) return;
    const auto bg = normalBackgroundColor();
    browser_->page()->setBackgroundColor(bg);
    QJsonValue source(QJsonValue::Null);
    if (pendingOption_) source = jsOption_.isEmpty()
        ? QStringLiteral("option = %1;").arg(QString::fromUtf8(QJsonDocument(option_).toJson(QJsonDocument::Compact))) : jsOption_;
    const QJsonValue theme = isDarkTheme() ? QJsonValue("dark") : QJsonValue(QJsonValue::Null);
    const auto arguments = json({source,theme,bg.name(),animationEnabled_,replayPending_});
    pendingOption_ = replayPending_ = false;
    browser_->page()->runJavaScript(QStringLiteral("updateChart.apply(null,%1);").arg(arguments));
}
void ChartWidget::themeChanged() {
    setBackgroundColor(normalBackgroundColor()); update();
    if (isVisible()) {
        browser_->page()->setBackgroundColor(normalBackgroundColor());
        if (gate_->pending) { gate_->arm(normalBackgroundColor()); if (initialized_) gate_->prepare(); }
    }
    scheduleUpdate();
}
void ChartWidget::showEvent(QShowEvent* event) {
    SimpleCardWidget::showEvent(event); layout_->activate();
    gate_->arm(normalBackgroundColor()); replayPending_ = true;
    if (!pageRequested_) { pageRequested_ = true; browser_->setUrl(QUrl("qrc:/qfluentwidgets/js/chart.html")); }
    renderTimer_.stop(); if (initialized_) gate_->prepare();
}
void ChartWidget::hideEvent(QHideEvent* event) {
    renderTimer_.stop(); gate_->cancel();
    if (initialized_) browser_->page()->runJavaScript(QStringLiteral("hideChart();"));
    SimpleCardWidget::hideEvent(event);
}
void ChartWidget::resizeEvent(QResizeEvent* event) {
    SimpleCardWidget::resizeEvent(event);
    if (gate_) { layout_->activate(); gate_->setGeometry(browser_->geometry()); }
}
}
