#include "setting_interface.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QStandardPaths>

#include "common/app_config.h"
#include "common/gallery_style_sheet.h"
#include "common/signal_bus.h"
#include "components/widgets/info_bar.h"
#include "components/widgets/label.h"

namespace qfw {

SettingInterface::SettingInterface(QWidget* parent) : ScrollArea(parent) {
    scrollWidget_ = new QWidget(this);
    expandLayout_ = new ExpandLayout(scrollWidget_);

    // setting label
    settingLabel_ = new QLabel(tr("Settings"), this);

    // music folders
    musicInThisPCGroup_ = new SettingCardGroup(tr("Music on this PC"), scrollWidget_);
    musicFolderCard_ = new FolderListSettingCard(
        appConfig().musicFolders(), tr("Local music library"), QString(),
        QStandardPaths::writableLocation(QStandardPaths::MusicLocation), musicInThisPCGroup_);

    downloadFolderCard_ =
        new PushSettingCard(tr("Choose folder"), FluentIconEnum::Download, tr("Download directory"),
                            appConfig().getDownloadFolder(), musicInThisPCGroup_);

    // personalization
    personalGroup_ = new SettingCardGroup(tr("Personalization"), scrollWidget_);

    micaCard_ = new SwitchSettingCard(FluentIconEnum::Transparent, tr("Mica effect"),
                                      tr("Apply semi transparent to windows and surfaces"),
                                      appConfig().micaEnabled(), personalGroup_);

    themeCard_ = new OptionsSettingCard(
        &QConfig::instance().themeModeItem(), FluentIconEnum::Brush, tr("Application theme"),
        tr("Change the appearance of your application"),
        {tr("Light"), tr("Dark"), tr("Use system setting")}, personalGroup_);

    themeColorCard_ = new CustomColorSettingCard(
        &QConfig::instance().themeColorItem(), FluentIconEnum::Palette, tr("Theme color"),
        tr("Change the theme color of you application"), personalGroup_);

    zoomCard_ = new OptionsSettingCard(
        static_cast<OptionsConfigItem*>(appConfig().dpiScale()), FluentIconEnum::Zoom,
        tr("Interface zoom"), tr("Change the size of widgets and fonts"),
        {QStringLiteral("100%"), QStringLiteral("125%"), QStringLiteral("150%"),
         QStringLiteral("175%"), QStringLiteral("200%"), tr("Use system setting")},
        personalGroup_);

    languageCard_ = new ComboBoxSettingCard(
        static_cast<OptionsConfigItem*>(appConfig().language()), FluentIconEnum::Language,
        tr("Language"), tr("Set your preferred language for UI"),
        {tr("Use system setting"), QString::fromUtf8("简体中文"), QStringLiteral("English")},
        personalGroup_);

    // material
    materialGroup_ = new SettingCardGroup(tr("Material"), scrollWidget_);
    blurRadiusCard_ = new RangeSettingCard(static_cast<RangeConfigItem*>(appConfig().blurRadius()),
                                           FluentIconEnum::Album, tr("Acrylic blur radius"),
                                           tr("The greater the radius, the more blurred the image"),
                                           materialGroup_);

    // update software
    updateSoftwareGroup_ = new SettingCardGroup(tr("Software update"), scrollWidget_);
    updateOnStartUpCard_ = new SwitchSettingCard(
        FluentIconEnum::Update, tr("Check for updates when the application starts"),
        tr("The new version will be more stable and have more features"),
        appConfig().checkUpdateAtStartUp(), updateSoftwareGroup_);

    // application
    aboutGroup_ = new SettingCardGroup(tr("About"), scrollWidget_);
    helpCard_ = new HyperlinkCard(
        QString::fromUtf8(AppConfig::HELP_URL), tr("Open help page"), FluentIconEnum::Help,
        tr("Help"), tr("Discover new features and learn useful tips about QtFluentWidgets"),
        aboutGroup_);

    feedbackCard_ = new PrimaryPushSettingCard(
        tr("Provide feedback"), FluentIconEnum::Feedback, tr("Provide feedback"),
        tr("Help us improve QtFluentWidgets by providing feedback"), aboutGroup_);

    const QString aboutContent = QStringLiteral("© %1 %2, %3. %4 %5")
                                     .arg(tr("Copyright"))
                                     .arg(AppConfig::YEAR)
                                     .arg(QString::fromUtf8(AppConfig::AUTHOR))
                                     .arg(tr("Version"))
                                     .arg(QString::fromUtf8(AppConfig::VERSION));

    aboutCard_ = new PrimaryPushSettingCard(tr("Check update"), FluentIconEnum::Info, tr("About"),
                                            aboutContent, aboutGroup_);

    initWidget();
    initLayout();
    connectSignalToSlot();
}

void SettingInterface::initWidget() {
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setViewportMargins(0, 80, 0, 20);
    setWidget(scrollWidget_);
    setWidgetResizable(true);
    setObjectName(QStringLiteral("settingInterface"));
    setProperty("qssClass", QStringLiteral("SettingInterface"));

    // initialize style sheet
    scrollWidget_->setObjectName(QStringLiteral("scrollWidget"));
    settingLabel_->setObjectName(QStringLiteral("settingLabel"));
    applyGalleryStyleSheet(this, GalleryStyleSheet::SettingInterface);

    micaCard_->setEnabled(isWin11());
}

void SettingInterface::initLayout() {
    settingLabel_->move(36, 30);

    // add cards to group
    musicInThisPCGroup_->addSettingCard(musicFolderCard_);
    musicInThisPCGroup_->addSettingCard(downloadFolderCard_);

    personalGroup_->addSettingCard(micaCard_);
    personalGroup_->addSettingCard(themeCard_);
    personalGroup_->addSettingCard(themeColorCard_);
    personalGroup_->addSettingCard(zoomCard_);
    personalGroup_->addSettingCard(languageCard_);

    materialGroup_->addSettingCard(blurRadiusCard_);

    updateSoftwareGroup_->addSettingCard(updateOnStartUpCard_);

    aboutGroup_->addSettingCard(helpCard_);
    aboutGroup_->addSettingCard(feedbackCard_);
    aboutGroup_->addSettingCard(aboutCard_);

    // add setting card group to layout
    expandLayout_->setSpacing(28);
    expandLayout_->setContentsMargins(36, 10, 36, 0);
    expandLayout_->addWidget(musicInThisPCGroup_);
    expandLayout_->addWidget(personalGroup_);
    expandLayout_->addWidget(materialGroup_);
    expandLayout_->addWidget(updateSoftwareGroup_);
    expandLayout_->addWidget(aboutGroup_);
}

void SettingInterface::showRestartTooltip() {
    InfoBar::success(tr("Updated successfully"), tr("Configuration takes effect after restart"),
                     Qt::Horizontal, true, 1500, InfoBarPosition::TopRight, this);
}

void SettingInterface::onDownloadFolderCardClicked() {
    QString folder =
        QFileDialog::getExistingDirectory(this, tr("Choose folder"), QStringLiteral("./"));
    if (folder.isEmpty() || appConfig().getDownloadFolder() == folder) {
        return;
    }

    appConfig().setDownloadFolder(folder);
    downloadFolderCard_->setContent(folder);
}

void SettingInterface::connectSignalToSlot() {
    connect(&QConfig::instance(), &QConfig::appRestartSig, this,
            &SettingInterface::showRestartTooltip);

    // music in the pc
    connect(downloadFolderCard_, &PushSettingCard::clicked, this,
            &SettingInterface::onDownloadFolderCardClicked);

    // personalization
    connect(micaCard_, &SwitchSettingCard::checkedChanged, &signalBus(),
            &SignalBus::micaEnableChanged);

    // about
    connect(feedbackCard_, &PrimaryPushSettingCard::clicked, this,
            []() { QDesktopServices::openUrl(QUrl(QString::fromUtf8(AppConfig::FEEDBACK_URL))); });
}

}  // namespace qfw
