cmake_minimum_required(VERSION 3.16)

project(HX-Music
    VERSION 0.0.1
    DESCRIPTION "HX-Music"
    HOMEPAGE_URL "https://github.com/HengXin666/HX-Music"
    LANGUAGES CXX
)

# QT Start
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Gui)

include(cmake/install.cmake)

if(CMAKE_PROJECT_NAME STREQUAL "HX-Music")
    include(cmake/utils.cmake)
    include(cmake/develop.cmake)
else()
    # todo
endif()