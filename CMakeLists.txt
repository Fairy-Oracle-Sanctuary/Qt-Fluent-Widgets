cmake_minimum_required(VERSION 3.16)

project(QtFluentWidgets LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
