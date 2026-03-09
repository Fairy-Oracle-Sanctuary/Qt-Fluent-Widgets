cmake_minimum_required(VERSION 3.16)

project(QtFluentWidgets LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Svg)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Svg)

# Qt version check: only allow Qt 5.15+ or Qt 6.7+
# if(${QT_VERSION_MAJOR} EQUAL 6)
#     if(${QT_VERSION_MINOR} GREATER_EQUAL 3 AND ${QT_VERSION_MINOR} LESS_EQUAL 6)
#         message(FATAL_ERROR 
#             "Qt ${Qt${QT_VERSION_MAJOR}_VERSION} is NOT supported. "
#             "This project requires Qt 5.15+ or Qt 6.7+. "
#             "Qt 6.3-6.6 have incompatible API changes."
#         )
#     endif()
# endif()

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
