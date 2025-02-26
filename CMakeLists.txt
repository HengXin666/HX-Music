cmake_minimum_required(VERSION 3.16)

# 启用自动查找 vcpkg 依赖
set(CMAKE_TOOLCHAIN_FILE "D:/MyApp/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

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
    # 请定义 vcpkg 的路径
    set(LIB_ROOT "D:/MyApp/vcpkg/installed/x64-windows")
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
