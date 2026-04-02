#include "top_main_window.h"

#include <QVBoxLayout>

#include "common/gallery_icon.h"
#include "common/icon.h"
#include "common/signal_bus.h"
#include "common/translator.h"
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

TopMainWindow::TopMainWindow(QWidget* parent) : TopFluentWindow(parent) {
    initWindow();
    initNavigation();
    connectSignalToSlot();
}

void TopMainWindow::initWindow() {
    resize(960, 780);
    setMinimumWidth(760);
    setWindowIcon(QIcon(QStringLiteral(":/gallery/images/logo.png")));
    setWindowTitle(QStringLiteral("QtFluentWidgets Gallery - Top Navigation"));

    // Center window on screen
    QRect desktop = QApplication::primaryScreen()->availableGeometry();
    move(desktop.width() / 2 - width() / 2, desktop.height() / 2 - height() / 2);
}

void TopMainWindow::initNavigation() {
    Translator t;

    static GalleryIcon emojiTabSymbolsIcon(GalleryIconEnum::EmojiTabSymbols);
    static GalleryIcon menuIcon(GalleryIconEnum::Menu);
    static GalleryIcon textIcon(GalleryIconEnum::Text);
    static GalleryIcon gridIcon(GalleryIconEnum::Grid);

    auto* homePage = new HomeInterface(this);
    homePage->setObjectName(QStringLiteral("homeInterface"));
    addSubInterface(homePage, FluentIconEnum::Home, tr("Home"), TopNavigationItemPosition::Left);

    auto* iconPage = new IconInterface(this);
    iconPage->setObjectName(QStringLiteral("iconInterface"));
    addSubInterface(iconPage, QVariant::fromValue<const FluentIconBase*>(&emojiTabSymbolsIcon),
                    t.icons(), TopNavigationItemPosition::Left);

    auto* basicInputPage = new BasicInputInterface(this);
    basicInputPage->setObjectName(QStringLiteral("basicInputInterface"));
    addSubInterface(basicInputPage, FluentIconEnum::CheckBox, t.basicInput(),
                    TopNavigationItemPosition::Left);

    auto* dateTimePage = new DateTimeInterface(this);
    dateTimePage->setObjectName(QStringLiteral("dateTimeInterface"));
    addSubInterface(dateTimePage, FluentIconEnum::DateTime, t.dateTime(),
                    TopNavigationItemPosition::Left);

    auto* dialogPage = new DialogInterface(this);
    dialogPage->setObjectName(QStringLiteral("dialogInterface"));
    addSubInterface(dialogPage, FluentIconEnum::Message, t.dialogs(),
                    TopNavigationItemPosition::Left);

    auto* layoutPage = new LayoutInterface(this);
    layoutPage->setObjectName(QStringLiteral("layoutInterface"));
    addSubInterface(layoutPage, FluentIconEnum::Layout, t.layout(), TopNavigationItemPosition::Left);

    auto* materialPage = new MaterialInterface(this);
    materialPage->setObjectName(QStringLiteral("materialInterface"));
    addSubInterface(materialPage, FluentIconEnum::Palette, t.material(),
                    TopNavigationItemPosition::Left);

    auto* menuPage = new MenuInterface(this);
    menuPage->setObjectName(QStringLiteral("menuInterface"));
    addSubInterface(menuPage, QVariant::fromValue<const FluentIconBase*>(&menuIcon), t.menus(),
                    TopNavigationItemPosition::Left);

    auto* navViewPage = new NavigationViewInterface(this);
    navViewPage->setObjectName(QStringLiteral("navViewInterface"));
    addSubInterface(navViewPage, FluentIconEnum::Menu, t.navigation(),
                    TopNavigationItemPosition::Left);

    auto* scrollPage = new ScrollInterface(this);
    scrollPage->setObjectName(QStringLiteral("scrollInterface"));
    addSubInterface(scrollPage, FluentIconEnum::Scroll, t.scroll(), TopNavigationItemPosition::Left);

    auto* statusInfoPage = new StatusInfoInterface(this);
    statusInfoPage->setObjectName(QStringLiteral("statusInfoInterface"));
    addSubInterface(statusInfoPage, FluentIconEnum::Chat, t.statusInfo(),
                    TopNavigationItemPosition::Left);

    auto* textPage = new TextInterface(this);
    textPage->setObjectName(QStringLiteral("textInterface"));
    addSubInterface(textPage, QVariant::fromValue<const FluentIconBase*>(&textIcon), t.text(),
                    TopNavigationItemPosition::Left);

    auto* viewPage = new ViewInterface(this);
    viewPage->setObjectName(QStringLiteral("viewInterface"));
    addSubInterface(viewPage, QVariant::fromValue<const FluentIconBase*>(&gridIcon), t.view(),
                    TopNavigationItemPosition::Left);

    // Add settings interface at right
    auto* settingsPage = new SettingInterface(this);
    settingsPage->setObjectName(QStringLiteral("settingsPage"));
    addSubInterface(settingsPage, FluentIconEnum::Setting, tr("Settings"),
                    TopNavigationItemPosition::Right);
}

void TopMainWindow::connectSignalToSlot() {
    connect(&signalBus(), &SignalBus::switchToSampleCard, this, &TopMainWindow::onSwitchToSample);
    connect(&signalBus(), &SignalBus::micaEnableChanged, this, &TopMainWindow::onMicaEnableChanged);
}

void TopMainWindow::onSwitchToSample(const QString& routeKey, int index) {
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

void TopMainWindow::onMicaEnableChanged(bool enabled) { setMicaEffectEnabled(enabled); }

}  // namespace qfw
