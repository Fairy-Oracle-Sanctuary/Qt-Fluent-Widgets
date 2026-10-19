#include "chart_interface.h"
#include "components/widgets/chart_widget.h"
#include "components/widgets/button.h"
#include "components/widgets/label.h"
#include "common/config.h"
#include "window/fluent_window.h"
#include "components/navigation/navigation_interface.h"
#include <QFile>
#include <QHideEvent>
#include <QJsonArray>
#include <QJsonDocument>
#include <QRandomGenerator>

namespace qfw {
static const QStringList& keys() {
    static const QStringList names={"line","bar","pie","scatter","kline","radar","boxplot","heat","relation","funnel","gauge","calendar"};
    return names;
}
static QStringList titles() {
    return {ChartInterface::tr("Line chart"),ChartInterface::tr("Bar chart"),ChartInterface::tr("Pie chart"),
        ChartInterface::tr("Scatter chart"),ChartInterface::tr("Candlestick chart"),ChartInterface::tr("Radar chart"),
        ChartInterface::tr("Boxplot chart"),ChartInterface::tr("Heatmap"),ChartInterface::tr("Relation graph"),
        ChartInterface::tr("Funnel chart"),ChartInterface::tr("Gauge"),ChartInterface::tr("Calendar chart")};
}
static void setExample(ChartWidget* chart, const QString& key) {
    QFile file(":/gallery/chart/examples.json"); file.open(QIODevice::ReadOnly);
    const auto value=QJsonDocument::fromJson(file.readAll()).object().value(key);
    if(value.isString()) chart->setOptionJS(value.toString()); else chart->setOption(value.toObject());
}
class ChartExamplePage : public GalleryInterface {
public:
    ChartExamplePage(const QString& title,const QString& key,QWidget* parent)
        :GalleryInterface(title,"qfw::ChartWidget",parent) {
        setObjectName(key+"ChartInterface");
        auto* chart=new ChartWidget(widget());chart->setFixedHeight(550);setExample(chart,key);
        contentLayout()->addWidget(chart);enableTransparentBackground();
    }
};
class ChartRealtimePage : public GalleryInterface {
public:
    explicit ChartRealtimePage(QWidget* parent)
        :GalleryInterface(ChartInterface::tr("Real-time data"),"qfw::ChartWidget",parent) {
        setObjectName("realtimeChartInterface");
        contentLayout()->addWidget(new ChartRealtimeDemo(widget()));enableTransparentBackground();
    }
};
class ChartWindow : public FluentWindow {
public:
    ChartWindow() {
        setAttribute(Qt::WA_DeleteOnClose); resize(960,780); setMinimumWidth(760);
        setWindowTitle(ChartInterface::tr("QtFluentWidgets — Charts"));
        setWindowIcon(QIcon(":/gallery/images/logo.png"));
        const auto labels=titles();
        const QList<FluentIconEnum> icons={FluentIconEnum::ClearSelection,FluentIconEnum::Alignment,
            FluentIconEnum::Palette,FluentIconEnum::More,FluentIconEnum::FullScreen,FluentIconEnum::Globe,
            FluentIconEnum::Tiles,FluentIconEnum::Brightness,FluentIconEnum::IOT,FluentIconEnum::PieSingle,
            FluentIconEnum::SpeedMedium,FluentIconEnum::Calendar};
        for(int i=0;i<keys().size();++i) {
            auto* page=new ChartExamplePage(labels[i],keys()[i],this);
            addSubInterface(page,icons[i],labels[i],NavigationItemPosition::Scroll);
        }
        auto* realtime=new ChartRealtimePage(this);
        addSubInterface(realtime,FluentIconEnum::Update,ChartInterface::tr("Real-time data"),NavigationItemPosition::Scroll);
        navigationInterface_->addItem("chartTheme",QVariant::fromValue(FluentIconEnum::Constract),ChartInterface::tr("Toggle theme"),
            []{QConfig::instance().setTheme(isDarkTheme()?Theme::Light:Theme::Dark);},false,
            NavigationItemPosition::Bottom,ChartInterface::tr("Toggle theme"));
    }
};
ChartRealtimeDemo::ChartRealtimeDemo(QWidget* parent):QWidget(parent) {
    auto* layout=new QVBoxLayout(this); layout->setContentsMargins(0,0,0,0);layout->setSpacing(30);
    chart_=new ChartWidget(this); chart_->setFixedSize(400,400); layout->addWidget(chart_,0,Qt::AlignLeft);
    auto* animation=new PushButton(tr("Disable animation"),this); animation->setFixedWidth(120);
    start_=new PushButton(tr("Start real-time updates"),this); start_->setFixedWidth(120);
    stop_=new PushButton(tr("Stop updates"),this); stop_->setFixedWidth(120); stop_->setEnabled(false);
    layout->addSpacing(10);layout->addWidget(animation,0,Qt::AlignLeft);layout->addWidget(start_,0,Qt::AlignLeft);layout->addWidget(stop_,0,Qt::AlignLeft);
    for(int i=0;i<7;++i)values_.append(QRandomGenerator::global()->bounded(100,501));
    timer_.setInterval(100); connect(&timer_,&QTimer::timeout,this,[this]{values_.removeFirst();values_.append(QRandomGenerator::global()->bounded(100,501));updateOption();});
    connect(animation,&QAbstractButton::clicked,this,[this,animation]{animation_=!animation_;chart_->setAnimationEnabled(animation_);animation->setText(animation_?tr("Disable animation"):tr("Enable animation"));});
    connect(start_,&QAbstractButton::clicked,this,[this]{timer_.start();start_->setEnabled(false);stop_->setEnabled(true);});
    connect(stop_,&QAbstractButton::clicked,this,&ChartRealtimeDemo::stop); updateOption();
}
void ChartRealtimeDemo::stop(){timer_.stop();start_->setEnabled(true);stop_->setEnabled(false);}
void ChartRealtimeDemo::hideEvent(QHideEvent* e){stop();QWidget::hideEvent(e);}
void ChartRealtimeDemo::updateOption() {
    QJsonArray values;for(auto value:values_)values.append(value);
    chart_->setOption({{"title",QJsonObject{{"text",tr("Real-time data demo")},{"subtext",tr("Simulated live data stream")}}},
        {"tooltip",QJsonObject{{"trigger","axis"}}},{"xAxis",QJsonObject{{"type","category"},{"data",QJsonArray{"Mon","Tue","Wed","Thu","Fri","Sat","Sun"}}}},
        {"yAxis",QJsonObject{{"type","value"}}},{"series",QJsonArray{QJsonObject{{"name",tr("Data")},{"type","bar"},{"data",values}}}}});
}
ChartInterface::ChartInterface(QWidget* parent):GalleryInterface(tr("Charts"),"qfw::ChartWidget",parent) {
    setObjectName("chartInterface");
    auto* hub=new QWidget(this);auto* layout=new QVBoxLayout(hub);layout->setContentsMargins(0,0,0,0);
    auto* open=new PushButton(tr("Open all chart examples"),hub);layout->addWidget(open,0,Qt::AlignLeft);
    connect(open,&QAbstractButton::clicked,this,[]{
        auto* window=new ChartWindow;window->show();
        window->setMicaEffectEnabled(window->isMicaEffectEnabled());
    });
    auto* preview=new ChartWidget(hub);preview->setFixedHeight(550);setExample(preview,"line");layout->addWidget(preview);
    addExampleCard(tr("ECharts examples"),hub,"components/widgets/chart_widget.h",1);
    addExampleCard(tr("Real-time data"),new ChartRealtimeDemo(this),"components/widgets/chart_widget.h");
}
}
