cmake_minimum_required(VERSION 3.16)
project(HX-Music
    VERSION 1.0.0
    DESCRIPTION "HX-Music"
    HOMEPAGE_URL "https://github.com/HengXin666/HX-Music"
    LANGUAGES C CXX
)

# QT Start
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_BUILD_TYPE "Debug")
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    message("=-=-=-=-=-=-=当前是 [Release] 模式=-=-=-=-=-=-=")
else()
    message("=-=-=-=-=-=-=当前是 [DEBUG] 模式=-=-=-=-=-=-=")
endif()

# 加载依赖
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Find/)

include(cmake/install.cmake)

if(CMAKE_PROJECT_NAME STREQUAL "HX-Music")
    include(cmake/utils.cmake)
    include(cmake/develop.cmake)
else()
    # todo
endif()
