#pragma once

#include <QLabel>
#include <QUrl>
#include <QWidget>

#include "components/layout/expand_layout.h"
#include "components/settings/custom_color_setting_card.h"
#include "components/settings/folder_list_setting_card.h"
#include "components/settings/options_setting_card.h"
#include "components/settings/setting_card.h"
#include "components/settings/setting_card_group.h"
#include "components/widgets/scroll_area.h"

namespace qfw {

// SettingInterface - Settings page with various setting cards
class SettingInterface : public ScrollArea {
    Q_OBJECT

public:
    explicit SettingInterface(QWidget* parent = nullptr);

private:
    void initWidget();
    void initLayout();
    void connectSignalToSlot();

    void showRestartTooltip();
    void onDownloadFolderCardClicked();

    QWidget* scrollWidget_ = nullptr;
    ExpandLayout* expandLayout_ = nullptr;
    QLabel* settingLabel_ = nullptr;

    // Setting groups
    SettingCardGroup* musicInThisPCGroup_ = nullptr;
    SettingCardGroup* personalGroup_ = nullptr;
    SettingCardGroup* materialGroup_ = nullptr;
    SettingCardGroup* updateSoftwareGroup_ = nullptr;
    SettingCardGroup* aboutGroup_ = nullptr;

    FolderListSettingCard* musicFolderCard_ = nullptr;
    PushSettingCard* downloadFolderCard_ = nullptr;

    // Personalization cards
    SwitchSettingCard* micaCard_ = nullptr;
    OptionsSettingCard* themeCard_ = nullptr;
    CustomColorSettingCard* themeColorCard_ = nullptr;
    OptionsSettingCard* zoomCard_ = nullptr;
    ComboBoxSettingCard* languageCard_ = nullptr;

    RangeSettingCard* blurRadiusCard_ = nullptr;

    SwitchSettingCard* updateOnStartUpCard_ = nullptr;

    // About cards
    HyperlinkCard* helpCard_ = nullptr;
    PrimaryPushSettingCard* feedbackCard_ = nullptr;
    PrimaryPushSettingCard* aboutCard_ = nullptr;
};

}  // namespace qfw
