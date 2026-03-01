cmake_minimum_required(VERSION 3.16)

project(qtfluentwidgets LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Svg)

qt_standard_project_setup()

qt_add_library(qtfluentwidgets STATIC
    common/config.cpp
    common/config.h
    common/font.cpp
    common/font.h
    common/style_sheet.cpp
    common/style_sheet.h
    common/icon.cpp
    common/icon.h
    common/theme_listener.cpp
    common/theme_listener.h
    common/translator.cpp
    common/translator.h
    common/router.cpp
    common/router.h
    common/smooth_scroll.cpp
    common/smooth_scroll.h
    common/screen.cpp
    common/screen.h
    common/color.cpp
    common/color.h
    common/auto_wrap.cpp
    common/auto_wrap.h
    common/animation.cpp
    common/animation.h

    components/widgets/button.cpp
    components/widgets/button.h
    components/widgets/tool_tip.cpp
    components/widgets/tool_tip.h
    components/widgets/flyout.cpp
    components/widgets/flyout.h
    components/widgets/acrylic_label.cpp
    components/widgets/acrylic_label.h
    components/navigation/breadcrumb.cpp
    components/navigation/breadcrumb.h
    components/navigation/navigation_widget.cpp
    components/navigation/navigation_widget.h
    components/navigation/navigation_panel.cpp
    components/navigation/navigation_panel.h
    components/navigation/navigation_bar.cpp
    components/navigation/navigation_bar.h
    components/navigation/navigation_interface.cpp
    components/navigation/navigation_interface.h
    components/widgets/scroll_bar.cpp
    components/widgets/scroll_bar.h
    components/widgets/scroll_area.cpp
    components/widgets/scroll_area.h
    components/widgets/line_edit.cpp
    components/widgets/line_edit.h
    components/widgets/label.cpp
    components/widgets/label.h
    components/widgets/info_badge.cpp
    components/widgets/info_badge.h
    components/widgets/check_box.cpp
    components/widgets/check_box.h
    components/widgets/frameless_window.cpp
    components/widgets/frameless_window.h
    components/widgets/card_widget.cpp
    components/widgets/card_widget.h
    components/widgets/menu.cpp
    components/widgets/menu.h

    components/widgets/stacked_widget.cpp
    components/widgets/stacked_widget.h
    components/widgets/separator.cpp
    components/widgets/separator.h
    components/widgets/icon_widget.cpp
    components/widgets/icon_widget.h
    components/widgets/table_view.cpp
    components/widgets/table_view.h
    components/widgets/list_view.cpp
    components/widgets/list_view.h
    components/widgets/progress_bar.cpp
    components/widgets/progress_bar.h
    components/widgets/progress_ring.cpp
    components/widgets/progress_ring.h
    components/widgets/slider.cpp
    components/widgets/slider.h
    components/widgets/switch_button.cpp
    components/widgets/switch_button.h
    components/widgets/combo_box.cpp
    components/widgets/combo_box.h
    components/widgets/state_tool_tip.cpp
    components/widgets/state_tool_tip.h
    components/widgets/tree_view.cpp
    components/widgets/tree_view.h
    components/widgets/cycle_list_widget.cpp
    components/widgets/cycle_list_widget.h
    components/widgets/teaching_tip.cpp
    components/widgets/teaching_tip.h
    components/widgets/pips_pager.cpp
    components/widgets/pips_pager.h
    components/widgets/spin_box.cpp
    components/widgets/spin_box.h
    components/widgets/flip_view.cpp
    components/widgets/flip_view.h
    components/widgets/tab_view.cpp
    components/widgets/tab_view.h
    components/widgets/model_combo_box.cpp
    components/widgets/model_combo_box.h
    components/widgets/info_bar.cpp
    components/widgets/info_bar.h
    components/widgets/command_bar.cpp
    components/widgets/command_bar.h

    components/layout/v_box_layout.cpp
    components/layout/v_box_layout.h
    components/layout/expand_layout.cpp
    components/layout/expand_layout.h
    components/layout/flow_layout.cpp
    components/layout/flow_layout.h

    components/date_time/picker_base.cpp
    components/date_time/picker_base.h
    components/date_time/time_picker.cpp
    components/date_time/time_picker.h
    components/date_time/date_picker.cpp
    components/date_time/date_picker.h
    components/date_time/calendar_view.cpp
    components/date_time/calendar_view.h
    components/date_time/calendar_picker.cpp
    components/date_time/calendar_picker.h
    components/date_time/fast_calendar_view.cpp
    components/date_time/fast_calendar_view.h
    components/date_time/fast_calendar_picker.cpp
    components/date_time/fast_calendar_picker.h

    components/settings/setting_card_group.cpp
    components/settings/setting_card_group.h
    components/settings/setting_card.cpp
    components/settings/setting_card.h
    components/settings/expand_setting_card.cpp
    components/settings/expand_setting_card.h
    components/settings/custom_color_setting_card.cpp
    components/settings/custom_color_setting_card.h
    components/settings/options_setting_card.cpp
    components/settings/options_setting_card.h
    components/settings/folder_list_setting_card.cpp
    components/settings/folder_list_setting_card.h

    components/dialog_box/mask_dialog_base.cpp
    components/dialog_box/mask_dialog_base.h
    components/dialog_box/message_box_base.cpp
    components/dialog_box/message_box_base.h
    components/dialog_box/message_dialog.cpp
    components/dialog_box/message_dialog.h
    components/dialog_box/folder_list_dialog.cpp
    components/dialog_box/folder_list_dialog.h
    components/dialog_box/color_dialog.cpp
    components/dialog_box/color_dialog.h
    components/dialog_box/dialog.cpp
    components/dialog_box/dialog.h

    components/navigation/pivot.cpp
    components/navigation/pivot.h
    components/navigation/segmented_widget.cpp
    components/navigation/segmented_widget.h

    window/stacked_widget.cpp
    window/stacked_widget.h

    window/fluent_window.cpp
    window/fluent_window.h

    components/window/title_bar.cpp
    components/window/title_bar.h
    components/window/frameless_window.h
    components/window/mac_frameless_window.h
    components/window/linux_frameless_window.h

    resources/resource.qrc
)

if(WIN32)
    target_sources(qtfluentwidgets PRIVATE
        components/window/windows_frameless_window.cpp
        components/window/windows_frameless_window.h
        components/window/windows_window_effect.cpp
        components/window/windows_window_effect.h
    )
endif()

if(APPLE)
    # Enable Objective-C++ for macOS
    enable_language(OBJCXX)
    
    target_sources(qtfluentwidgets PRIVATE
        components/window/mac_frameless_window.mm
        components/window/mac_frameless_window.h
        components/window/mac_window_effect.mm
        components/window/mac_window_effect.h
    )
    
    # Link macOS frameworks
    find_library(COCOA_FRAMEWORK Cocoa REQUIRED)
    target_link_libraries(qtfluentwidgets PUBLIC ${COCOA_FRAMEWORK})
endif()

target_link_libraries(qtfluentwidgets PUBLIC Qt6::Widgets Qt6::Svg)

# Link Windows libraries that may be missing
if(WIN32)
    find_library(MPR_LIB mpr PATHS "C:/Program Files (x86)/Windows Kits/10/Lib/*/um/x64")
    if(MPR_LIB)
        target_link_libraries(qtfluentwidgets PUBLIC ${MPR_LIB})
    endif()
endif()

target_include_directories(qtfluentwidgets PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

