cmake_minimum_required(VERSION 3.16)
project(HX-Music-Server
    VERSION 1.0.0
    DESCRIPTION "HX-Music-Server"
    HOMEPAGE_URL "https://github.com/HengXin666/HX-Music/HX-Music-Server"
    LANGUAGES C CXX
)

include(cmake/install.cmake)

if(CMAKE_PROJECT_NAME STREQUAL "HX-Music-Server")
    include(cmake/utils.cmake)
    include(cmake/develop.cmake)
    message("Building: ${CMAKE_PROJECT_NAME}")

    if (CMAKE_BUILD_TYPE STREQUAL "Release")
        message("=-=-=-=-=-=-=当前是 [Release] 模式=-=-=-=-=-=-=")
    else()
        message("=-=-=-=-=-=-=当前是 [DEBUG] 模式=-=-=-=-=-=-=")
    endif()
else()
    # todo
endif()
