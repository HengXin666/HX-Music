cmake_minimum_required(VERSION 3.16)

project(HX-Music
    VERSION 0.0.1
    DESCRIPTION "HX-Music"
    HOMEPAGE_URL "https://github.com/HengXin666/HX-Music"
    LANGUAGES CXX
)

# QT Start
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(WIN32)
  set(LIB_ROOT "${PROJECT_SOURCE_DIR}/lib")
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