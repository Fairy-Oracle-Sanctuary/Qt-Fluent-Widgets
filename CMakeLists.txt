cmake_minimum_required(VERSION 3.16)

project(QtFluentWidgets LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(UNIX AND NOT APPLE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Svg)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Svg)

if(${QT_VERSION_MAJOR} EQUAL 6)
    qt_standard_project_setup()
elseif(${QT_VERSION_MAJOR} EQUAL 5)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTORCC ON)
endif()

# Add the library
add_subdirectory(qtfluentwidgets)

add_subdirectory(app)

# Enable whole-program optimization for optimized builds when the selected
# compiler and generator support it. Unsupported toolchains keep the default
# behavior instead of failing configuration.
option(QFW_ENABLE_IPO "Enable interprocedural optimization in Release builds" ON)
if(QFW_ENABLE_IPO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT QFW_IPO_SUPPORTED OUTPUT QFW_IPO_ERROR)
    if(QFW_IPO_SUPPORTED)
        set_property(TARGET qtfluentwidgets PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
        set_property(TARGET qtfluentwidgets_app PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
    else()
        message(STATUS "IPO/LTO is unavailable: ${QFW_IPO_ERROR}")
    endif()
endif()
