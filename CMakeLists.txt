cmake_minimum_required(VERSION 3.16)

project(QtFluentWidgets LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Svg)
qt_standard_project_setup()

# Add the library
add_subdirectory(qtfluentwidgets)

add_subdirectory(app)
