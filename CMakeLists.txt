cmake_minimum_required(VERSION 3.16)
project(HX-Music)

# 公共依赖: HXLibs
add_subdirectory(libs/HXLibs)

# 控制是否构建客户端和服务端
option(BUILD_CLIENT "Build HX-Music client" ON)
option(BUILD_SERVER "Build HX-Music server" ON)

if(BUILD_CLIENT)
    add_subdirectory(HX-Music-Client)
endif()

if(BUILD_SERVER AND LINUX)
    add_subdirectory(HX-Music-Server)
endif()
