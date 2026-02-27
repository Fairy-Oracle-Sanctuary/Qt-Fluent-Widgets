#include "main_window.h"

#include <QVBoxLayout>

#include "common/gallery_icon.h"
#include "common/icon.h"
#include "common/signal_bus.h"
#include "common/translator.h"
#include "components/navigation/navigation_interface.h"
#include "view/basic_input_interface.h"
#include "view/date_time_interface.h"
#include "view/dialog_interface.h"
#include "view/gallery_interface.h"
#include "view/home_interface.h"
#include "view/icon_interface.h"
#include "view/layout_interface.h"
#include "view/material_interface.h"
#include "view/menu_interface.h"
#include "view/navigation_view_interface.h"
#include "view/scroll_interface.h"
#include "view/setting_interface.h"
#include "view/status_info_interface.h"
#include "view/text_interface.h"
#include "view/view_interface.h"

namespace qfw {

MainWindow::MainWindow(QWidget* parent) : FluentWindow(parent) {
    initWindow();
    initNavigation();
    connectSignalToSlot();
}

void MainWindow::initWindow() {
    resize(960, 780);
    setMinimumWidth(760);
    setWindowIcon(QIcon(QStringLiteral(":/gallery/images/logo.png")));
    setWindowTitle(QStringLiteral("QtFluentWidgets Gallery"));

    // Center window on screen
    QRect desktop = QApplication::primaryScreen()->availableGeometry();
    move(desktop.width() / 2 - width() / 2, desktop.height() / 2 - height() / 2);
}

void MainWindow::initNavigation() {
    Translator t;

    static GalleryIcon emojiTabSymbolsIcon(GalleryIconEnum::EmojiTabSymbols);
    static GalleryIcon menuIcon(GalleryIconEnum::Menu);
    static GalleryIcon textIcon(GalleryIconEnum::Text);
    static GalleryIcon gridIcon(GalleryIconEnum::Grid);

    auto* homePage = new HomeInterface(this);
    homePage->setObjectName(QStringLiteral("homeInterface"));
    addSubInterface(homePage, FluentIconEnum::Home, tr("Home"));

    auto* iconPage = new IconInterface(this);
    iconPage->setObjectName(QStringLiteral("iconInterface"));
    addSubInterface(iconPage, QVariant::fromValue<const FluentIconBase*>(&emojiTabSymbolsIcon),
                    t.icons());

    navigationInterface_->addSeparator();

    auto* basicInputPage = new BasicInputInterface(this);
    basicInputPage->setObjectName(QStringLiteral("basicInputInterface"));
    addSubInterface(basicInputPage, FluentIconEnum::CheckBox, t.basicInput(),
                    NavigationItemPosition::Scroll);

    auto* dateTimePage = new DateTimeInterface(this);
    dateTimePage->setObjectName(QStringLiteral("dateTimeInterface"));
    addSubInterface(dateTimePage, FluentIconEnum::DateTime, t.dateTime(),
                    NavigationItemPosition::Scroll);

    auto* dialogPage = new DialogInterface(this);
    dialogPage->setObjectName(QStringLiteral("dialogInterface"));
    addSubInterface(dialogPage, FluentIconEnum::Message, t.dialogs(),
                    NavigationItemPosition::Scroll);

    auto* layoutPage = new LayoutInterface(this);
    layoutPage->setObjectName(QStringLiteral("layoutInterface"));
    addSubInterface(layoutPage, FluentIconEnum::Layout, t.layout(), NavigationItemPosition::Scroll);

    auto* materialPage = new MaterialInterface(this);
    materialPage->setObjectName(QStringLiteral("materialInterface"));
    addSubInterface(materialPage, FluentIconEnum::Palette, t.material(),
                    NavigationItemPosition::Scroll);

    auto* menuPage = new MenuInterface(this);
    menuPage->setObjectName(QStringLiteral("menuInterface"));
    addSubInterface(menuPage, QVariant::fromValue<const FluentIconBase*>(&menuIcon), t.menus(),
                    NavigationItemPosition::Scroll);

    auto* navViewPage = new NavigationViewInterface(this);
    navViewPage->setObjectName(QStringLiteral("navViewInterface"));
    addSubInterface(navViewPage, FluentIconEnum::Menu, t.navigation(),
                    NavigationItemPosition::Scroll);

    auto* scrollPage = new ScrollInterface(this);
    scrollPage->setObjectName(QStringLiteral("scrollInterface"));
    addSubInterface(scrollPage, FluentIconEnum::Scroll, t.scroll(), NavigationItemPosition::Scroll);

    auto* statusInfoPage = new StatusInfoInterface(this);
    statusInfoPage->setObjectName(QStringLiteral("statusInfoInterface"));
    addSubInterface(statusInfoPage, FluentIconEnum::Chat, t.statusInfo(),
                    NavigationItemPosition::Scroll);

    auto* textPage = new TextInterface(this);
    textPage->setObjectName(QStringLiteral("textInterface"));
    addSubInterface(textPage, QVariant::fromValue<const FluentIconBase*>(&textIcon), t.text(),
                    NavigationItemPosition::Scroll);

    auto* viewPage = new ViewInterface(this);
    viewPage->setObjectName(QStringLiteral("viewInterface"));
    addSubInterface(viewPage, QVariant::fromValue<const FluentIconBase*>(&gridIcon), t.view(),
                    NavigationItemPosition::Scroll);

    // Add settings interface at bottom
    auto* settingsPage = new SettingInterface(this);
    settingsPage->setObjectName(QStringLiteral("settingsPage"));
    addSubInterface(settingsPage, FluentIconEnum::Setting, tr("Settings"),
                    NavigationItemPosition::Bottom);
}

void MainWindow::connectSignalToSlot() {
    connect(&signalBus(), &SignalBus::switchToSampleCard, this, &MainWindow::onSwitchToSample);
    connect(&signalBus(), &SignalBus::micaEnableChanged, this, &MainWindow::onMicaEnableChanged);
}

void MainWindow::onSwitchToSample(const QString& routeKey, int index) {
    // Find the interface with matching objectName and switch to it
    QList<GalleryInterface*> interfaces = findChildren<GalleryInterface*>();
    for (auto* w : interfaces) {
        if (w->objectName() == routeKey) {
            switchTo(w);
            w->scrollToCard(index);
            break;
        }
    }
}

void MainWindow::onMicaEnableChanged(bool enabled) { setMicaEffectEnabled(enabled); }

}  // namespace qfw
