cmake_minimum_required(VERSION 3.16)

project(qtfluentwidgets_app LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Svg LinguistTools)
qt_standard_project_setup()

qt_add_executable(qtfluentwidgets_app
    main.cpp
    common/app_config.cpp
    common/app_config.h
    common/gallery_icon.cpp
    common/gallery_icon.h
    common/gallery_style_sheet.cpp
    common/gallery_style_sheet.h
    common/signal_bus.cpp
    common/signal_bus.h
    common/translator.cpp
    common/translator.h
    common/trie.cpp
    common/trie.h
    components/link_card.cpp
    components/link_card.h
    components/sample_card.cpp
    components/sample_card.h
    view/main_window.cpp
    view/main_window.h
    view/gallery_interface.cpp
    view/gallery_interface.h
    view/icon_interface.cpp
    view/icon_interface.h
    view/basic_input_interface.cpp
    view/basic_input_interface.h
    view/date_time_interface.cpp
    view/date_time_interface.h
    view/dialog_interface.cpp
    view/dialog_interface.h
    view/home_interface.cpp
    view/layout_interface.cpp
    view/layout_interface.h
    view/material_interface.cpp
    view/material_interface.h
    view/menu_interface.cpp
    view/menu_interface.h
    view/navigation_view_interface.cpp
    view/navigation_view_interface.h
    view/scroll_interface.cpp
    view/scroll_interface.h
    view/status_info_interface.cpp
    view/status_info_interface.h
    view/text_interface.cpp
    view/text_interface.h
    view/view_interface.cpp
    view/view_interface.h
    view/home_interface.h
    view/setting_interface.cpp
    view/setting_interface.h
    resources/gallery.qrc
)

# Windows exe icon and subsystem
if(WIN32)
    target_sources(qtfluentwidgets_app PRIVATE resources/app.rc)
    if(MSVC)
        set_target_properties(qtfluentwidgets_app PROPERTIES
            WIN32_EXECUTABLE TRUE
            LINK_FLAGS "/SUBSYSTEM:WINDOWS"
        )
    elseif(MINGW)
        set_target_properties(qtfluentwidgets_app PROPERTIES
            WIN32_EXECUTABLE TRUE
            LINK_FLAGS "-mwindows"
        )
    endif()
endif()

target_link_libraries(qtfluentwidgets_app PRIVATE
    qtfluentwidgets
    Qt6::Widgets
    Qt6::Svg
)

target_include_directories(qtfluentwidgets_app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Generate translation files
set(TS_FILES
    resources/i18n/gallery.zh_CN.ts
)

# Get all source files for lupdate
set(LUPDATE_SOURCES
    main.cpp
    common/app_config.cpp
    common/app_config.h
    common/gallery_icon.cpp
    common/gallery_icon.h
    common/gallery_style_sheet.cpp
    common/gallery_style_sheet.h
    common/signal_bus.cpp
    common/signal_bus.h
    common/translator.cpp
    common/translator.h
    common/trie.cpp
    common/trie.h
    components/link_card.cpp
    components/link_card.h
    components/sample_card.cpp
    components/sample_card.h
    view/main_window.cpp
    view/main_window.h
    view/gallery_interface.cpp
    view/gallery_interface.h
    view/icon_interface.cpp
    view/icon_interface.h
    view/basic_input_interface.cpp
    view/basic_input_interface.h
    view/date_time_interface.cpp
    view/date_time_interface.h
    view/dialog_interface.cpp
    view/dialog_interface.h
    view/home_interface.cpp
    view/home_interface.h
    view/layout_interface.cpp
    view/layout_interface.h
    view/material_interface.cpp
    view/material_interface.h
    view/menu_interface.cpp
    view/menu_interface.h
    view/navigation_view_interface.cpp
    view/navigation_view_interface.h
    view/scroll_interface.cpp
    view/scroll_interface.h
    view/status_info_interface.cpp
    view/status_info_interface.h
    view/text_interface.cpp
    view/text_interface.h
    view/view_interface.cpp
    view/view_interface.h
    view/setting_interface.cpp
    view/setting_interface.h
)

qt_add_lupdate(qtfluentwidgets_app
    TS_FILES ${TS_FILES}
    SOURCES ${LUPDATE_SOURCES}
)

qt_add_lrelease(qtfluentwidgets_app
    TS_FILES ${TS_FILES}
)
