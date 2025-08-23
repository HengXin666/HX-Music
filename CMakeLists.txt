cmake_minimum_required(VERSION 3.16)
project(HX-Music)

# 公共依赖: HXLibs
add_subdirectory(libs/HXLibs)

# 子模块: 客户端
add_subdirectory(HX-Music-Client)

# 子模块: 服务端
add_subdirectory(HX-Music-Server)