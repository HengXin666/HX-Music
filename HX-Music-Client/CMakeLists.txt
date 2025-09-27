cmake_minimum_required(VERSION 3.16)
project(HX-Music-Client
    VERSION 1.0.0
    DESCRIPTION "HX-Music-Client"
    HOMEPAGE_URL "https://github.com/HengXin666/HX-Music/HX-Music-Client"
    LANGUAGES C CXX
)

# QT Start
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 加载依赖
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Find/)

include(cmake/install.cmake)

if(CMAKE_PROJECT_NAME STREQUAL "HX-Music-Client")
    include(cmake/utils.cmake)
    include(cmake/develop.cmake)
    message("Building: ${CMAKE_PROJECT_NAME}")
    # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wuninitialized -Winit-self")
    if (CMAKE_BUILD_TYPE STREQUAL "Release")
        message("=-=-=-=-=-=-=当前是 [Release] 模式=-=-=-=-=-=-=")
    else()
        message("=-=-=-=-=-=-=当前是 [DEBUG] 模式=-=-=-=-=-=-=")
    endif()
else()
    # todo
endif()
