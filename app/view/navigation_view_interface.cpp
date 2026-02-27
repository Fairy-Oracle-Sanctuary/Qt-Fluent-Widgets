#include "view/navigation_view_interface.h"

#include <QCheckBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QStackedWidget>
#include <QVBoxLayout>

#include "common/icon.h"
#include "common/router.h"
#include "components/navigation/breadcrumb.h"
#include "components/navigation/pivot.h"
#include "components/navigation/segmented_widget.h"
#include "components/widgets/check_box.h"
#include "components/widgets/combo_box.h"
#include "components/widgets/label.h"
#include "components/widgets/spin_box.h"
#include "components/widgets/tab_view.h"

namespace qfw {

// ==========================================================================
// PivotInterface
// ==========================================================================

PivotInterface::PivotInterface(QWidget* parent) : QWidget(parent) {
    setFixedSize(500, 140);

    pivot_ = new Pivot(this);
    stackedWidget_ = new QStackedWidget(this);
    vBoxLayout_ = new QVBoxLayout(this);

    auto* songInterface = new QLabel(tr("Song Interface"), this);
    auto* albumInterface = new QLabel(tr("Album Interface"), this);
    auto* artistInterface = new QLabel(tr("Artist Interface"), this);

    addSubInterface(songInterface, "songInterface", tr("Song"));
    addSubInterface(albumInterface, "albumInterface", tr("Album"));
    addSubInterface(artistInterface, "artistInterface", tr("Artist"));

    vBoxLayout_->addWidget(pivot_, 0, Qt::AlignLeft);
    vBoxLayout_->addWidget(stackedWidget_);
    vBoxLayout_->setContentsMargins(0, 0, 0, 0);

    connect(stackedWidget_, &QStackedWidget::currentChanged, this,
            &PivotInterface::onCurrentIndexChanged);
    stackedWidget_->setCurrentWidget(songInterface);
    pivot_->setCurrentItem(songInterface->objectName());

    Router::instance()->setDefaultRouteKey(stackedWidget_, songInterface->objectName());
}

void PivotInterface::addSubInterface(QLabel* widget, const QString& objectName,
                                     const QString& text) {
    widget->setObjectName(objectName);
    widget->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    stackedWidget_->addWidget(widget);
    pivot_->addItem(objectName, text,
                    [this, widget](bool) { stackedWidget_->setCurrentWidget(widget); });
}

void PivotInterface::onCurrentIndexChanged(int index) {
    auto* widget = qobject_cast<QLabel*>(stackedWidget_->widget(index));
    if (widget) {
        pivot_->setCurrentItem(widget->objectName());
        Router::instance()->push(stackedWidget_, widget->objectName());
    }
}

// ==========================================================================
// SegmentedInterface
// ==========================================================================

SegmentedInterface::SegmentedInterface(QWidget* parent) : PivotInterface(parent) {
    // Replace pivot with SegmentedWidget
    vBoxLayout_->removeWidget(pivot_);
    pivot_->deleteLater();

    pivot_ = new SegmentedWidget(this);
    qobject_cast<SegmentedWidget*>(pivot_)->setItemPadding(36);
    vBoxLayout_->insertWidget(0, pivot_, 0, Qt::AlignLeft);

    // Re-add items
    auto* songInterface = new QLabel(tr("Song Interface"), this);
    auto* albumInterface = new QLabel(tr("Album Interface"), this);
    auto* artistInterface = new QLabel(tr("Artist Interface"), this);

    addSubInterface(songInterface, "segSongInterface", tr("Song"));
    addSubInterface(albumInterface, "segAlbumInterface", tr("Album"));
    addSubInterface(artistInterface, "segArtistInterface", tr("Artist"));

    stackedWidget_->setCurrentWidget(songInterface);
    pivot_->setCurrentItem(songInterface->objectName());
    Router::instance()->setDefaultRouteKey(stackedWidget_, songInterface->objectName());
}

// ==========================================================================
// TabInterface
// ==========================================================================

TabInterface::TabInterface(QWidget* parent) : QWidget(parent) {
    tabBar_ = new TabBar(this);
    stackedWidget_ = new QStackedWidget(this);
    tabView_ = new QWidget(this);
    controlPanel_ = new QFrame(this);

    movableCheckBox_ = new CheckBox(tr("IsTabMovable"), this);
    scrollableCheckBox_ = new CheckBox(tr("IsTabScrollable"), this);
    shadowEnabledCheckBox_ = new CheckBox(tr("IsTabShadowEnabled"), this);
    tabMaxWidthLabel_ = new BodyLabel(tr("TabMaximumWidth"), this);
    tabMaxWidthSpinBox_ = new SpinBox(this);
    closeDisplayModeLabel_ = new BodyLabel(tr("TabCloseButtonDisplayMode"), this);
    closeDisplayModeComboBox_ = new ComboBox(this);

    hBoxLayout_ = new QHBoxLayout(this);
    vBoxLayout_ = new QVBoxLayout(tabView_);
    panelLayout_ = new QVBoxLayout(controlPanel_);

    initWidget();
}

void TabInterface::initWidget() {
    initLayout();

    shadowEnabledCheckBox_->setChecked(true);

    tabMaxWidthSpinBox_->setRange(60, 400);
    tabMaxWidthSpinBox_->setValue(tabBar_->tabMaximumWidth());

    closeDisplayModeComboBox_->addItem(tr("Always"),
                                       static_cast<int>(TabCloseButtonDisplayMode::Always));
    closeDisplayModeComboBox_->addItem(tr("OnHover"),
                                       static_cast<int>(TabCloseButtonDisplayMode::OnHover));
    closeDisplayModeComboBox_->addItem(tr("Never"),
                                       static_cast<int>(TabCloseButtonDisplayMode::Never));
    connect(closeDisplayModeComboBox_, &ComboBox::currentIndexChanged, this,
            &TabInterface::onDisplayModeChanged);

    addSubInterface(new QLabel(tr("Song Interface"), this), "tabSongInterface", tr("Song"),
                    ":/gallery/images/MusicNote.png");
    addSubInterface(new QLabel(tr("Album Interface"), this), "tabAlbumInterface", tr("Album"),
                    ":/gallery/images/Dvd.png");
    addSubInterface(new QLabel(tr("Artist Interface"), this), "tabArtistInterface", tr("Artist"),
                    ":/gallery/images/Singer.png");

    controlPanel_->setObjectName("controlPanel");

    connectSignalToSlot();

    Router::instance()->setDefaultRouteKey(stackedWidget_, "tabSongInterface");
}

void TabInterface::initLayout() {
    tabBar_->setTabMaximumWidth(200);

    setFixedHeight(280);
    controlPanel_->setFixedWidth(220);
    hBoxLayout_->addWidget(tabView_, 1);
    hBoxLayout_->addWidget(controlPanel_, 0, Qt::AlignRight);
    hBoxLayout_->setContentsMargins(0, 0, 0, 0);

    vBoxLayout_->addWidget(tabBar_);
    vBoxLayout_->addWidget(stackedWidget_);
    vBoxLayout_->setContentsMargins(0, 0, 0, 0);

    panelLayout_->setSpacing(8);
    panelLayout_->setContentsMargins(14, 16, 14, 14);
    panelLayout_->setAlignment(Qt::AlignTop);

    panelLayout_->addWidget(movableCheckBox_);
    panelLayout_->addWidget(scrollableCheckBox_);
    panelLayout_->addWidget(shadowEnabledCheckBox_);

    panelLayout_->addSpacing(4);
    panelLayout_->addWidget(tabMaxWidthLabel_);
    panelLayout_->addWidget(tabMaxWidthSpinBox_);

    panelLayout_->addSpacing(4);
    panelLayout_->addWidget(closeDisplayModeLabel_);
    panelLayout_->addWidget(closeDisplayModeComboBox_);
}

void TabInterface::connectSignalToSlot() {
    connect(movableCheckBox_, &CheckBox::stateChanged, this,
            [this]() { tabBar_->setMovable(movableCheckBox_->isChecked()); });
    connect(scrollableCheckBox_, &CheckBox::stateChanged, this,
            [this]() { tabBar_->setScrollable(scrollableCheckBox_->isChecked()); });
    connect(shadowEnabledCheckBox_, &CheckBox::stateChanged, this,
            [this]() { tabBar_->setTabShadowEnabled(shadowEnabledCheckBox_->isChecked()); });

    connect(tabMaxWidthSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged), tabBar_,
            &TabBar::setTabMaximumWidth);

    connect(tabBar_, &TabBar::tabAddRequested, this, &TabInterface::addTab);
    connect(tabBar_, &TabBar::tabCloseRequested, this, &TabInterface::removeTab);

    // Connect tab bar currentChanged to switch stacked widget
    connect(tabBar_, &TabBar::currentChanged, this, [this](int index) {
        auto* item = tabBar_->tabItem(index);
        if (item) {
            auto* widget = findChild<QLabel*>(item->routeKey());
            if (widget) {
                stackedWidget_->setCurrentWidget(widget);
            }
        }
    });

    connect(stackedWidget_, &QStackedWidget::currentChanged, this,
            &TabInterface::onCurrentIndexChanged);
}

void TabInterface::addSubInterface(QLabel* widget, const QString& objectName, const QString& text,
                                   const QString& icon) {
    widget->setObjectName(objectName);
    widget->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    stackedWidget_->addWidget(widget);

    FluentIconEnum iconEnum = FluentIconEnum::Up;
    if (icon == ":/gallery/images/MusicNote.png") {
        iconEnum = FluentIconEnum::Volume;
    } else if (icon == ":/gallery/images/Dvd.png") {
        iconEnum = FluentIconEnum::Folder;
    } else if (icon == ":/gallery/images/Singer.png") {
        iconEnum = FluentIconEnum::Chat;
    } else if (icon == ":/gallery/images/Smiling_with_heart.png") {
        iconEnum = FluentIconEnum::Heart;
    }

    tabBar_->addTab(objectName, text, iconEnum);
}

void TabInterface::onDisplayModeChanged(int index) {
    auto mode =
        static_cast<TabCloseButtonDisplayMode>(closeDisplayModeComboBox_->itemData(index).toInt());
    tabBar_->setCloseButtonDisplayMode(mode);
}

void TabInterface::onCurrentIndexChanged(int index) {
    auto* widget = qobject_cast<QLabel*>(stackedWidget_->widget(index));
    if (!widget) {
        return;
    }

    tabBar_->setCurrentTab(widget->objectName());
    Router::instance()->push(stackedWidget_, widget->objectName());
}

void TabInterface::addTab() {
    QString text = tr("硝子酱一级棒卡哇伊×%1").arg(tabCount_);
    auto* label = new QLabel("🥰 " + text, this);
    addSubInterface(label, text, text, ":/gallery/images/Smiling_with_heart.png");
    tabCount_++;
}

void TabInterface::removeTab(int index) {
    auto* item = tabBar_->tabItem(index);
    if (!item) {
        return;
    }

    auto* widget = findChild<QLabel*>(item->routeKey());
    if (widget) {
        stackedWidget_->removeWidget(widget);
        tabBar_->removeTab(index);
        widget->deleteLater();
    }
}

// ==========================================================================
// NavigationViewInterface
// ==========================================================================

NavigationViewInterface::NavigationViewInterface(QWidget* parent)
    : GalleryInterface(tr("Navigation"), "qtfluentwidgets.components.navigation", parent) {
    setObjectName("navigationViewInterface");

    // Breadcrumb bar
    auto* breadcrumb = new BreadcrumbBar(this);
    QStringList items = {
        tr("Home"),  tr("Documents"),  tr("Study"),   tr("Japanese Sensei"), tr("Action Film"),
        tr("G Cup"), tr("Mikami Yua"), tr("Folder1"), tr("Folder2")};
    for (const auto& item : items) {
        breadcrumb->addItem(item, item);
    }

    addExampleCard(tr("Breadcrumb bar"), breadcrumb,
                   "https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/examples/"
                   "navigation/breadcrumb_bar/demo.py",
                   1);

    addExampleCard(tr("A basic pivot"), new PivotInterface(this),
                   "https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/examples/"
                   "navigation/pivot/demo.py");

    addExampleCard(tr("A segmented control"), new SegmentedInterface(this),
                   "https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/examples/"
                   "navigation/segmented_widget/demo.py");

    addExampleCard(tr("Another segmented control"), createToggleToolWidget(),
                   "https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/master/examples/navigation/"
                   "segmented_tool_widget/demo.py");

    auto* card = addExampleCard(tr("A tab bar"), new TabInterface(this),
                                "https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                "examples/navigation/tab_view/demo.py",
                                1);
    // Note: card->topLayout()->setContentsMargins(12, 0, 0, 0) would require access to internal
    // layout
}

QWidget* NavigationViewInterface::createToggleToolWidget() {
    static FluentIcon folderIcon(FluentIconEnum::Folder);
    static FluentIcon checkBoxIcon(FluentIconEnum::CheckBox);
    static FluentIcon calendarIcon(FluentIconEnum::Calendar);

    auto* w = new SegmentedToggleToolWidget(this);
    w->addItem("k1", QVariant::fromValue<const FluentIconBase*>(&folderIcon));
    w->addItem("k2", QVariant::fromValue<const FluentIconBase*>(&checkBoxIcon));
    w->addItem("k3", QVariant::fromValue<const FluentIconBase*>(&calendarIcon));
    w->setCurrentItem("k1");
    return w;
}

}  // namespace qfw
