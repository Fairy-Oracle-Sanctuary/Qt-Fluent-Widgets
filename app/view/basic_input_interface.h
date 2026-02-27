#pragma once

#include "view/gallery_interface.h"

class QButtonGroup;

namespace qfw {

class Action;
class CheckBox;
class ComboBox;
class DropDownPushButton;
class DropDownToolButton;
class EditableComboBox;
class HyperlinkButton;
class PillPushButton;
class PillToolButton;
class PrimaryDropDownPushButton;
class PrimaryPushButton;
class PrimarySplitPushButton;
class PrimarySplitToolButton;
class PrimaryToolButton;
class PushButton;
class RadioButton;
class RoundMenu;
class Slider;
class SplitPushButton;
class SplitToolButton;
class SwitchButton;
class ToggleButton;
class ToggleToolButton;
class ToolButton;
class TransparentDropDownPushButton;
class TransparentDropDownToolButton;
class TransparentPushButton;
class TransparentToolButton;
class TransparentTogglePushButton;
class TransparentToggleToolButton;

class BasicInputInterface : public GalleryInterface {
    Q_OBJECT

public:
    explicit BasicInputInterface(QWidget* parent = nullptr);

private slots:
    void onSwitchCheckedChanged(bool isChecked);

private:
    RoundMenu* createStandMenu(QWidget* button);
    RoundMenu* createEmailMenu();
    RoundMenu* createIkunMenu();

    SwitchButton* switchButton_ = nullptr;
};

}  // namespace qfw
