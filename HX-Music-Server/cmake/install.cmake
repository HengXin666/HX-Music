file(GLOB_RECURSE src_files CONFIGURE_DEPENDS 
    src/*.cpp
    include/*.h
    include/*.hpp
)

add_executable(HX-Music-Server
    ${src_files}
    ${qrc_files}
)

target_compile_features(HX-Music-Server PUBLIC cxx_std_20)

# 本头文件
target_include_directories(HX-Music-Server PRIVATE include)

# 公共头文件
target_include_directories(HX-Music-Server PRIVATE ../include)

# 导入 HXLibs
target_link_libraries(HX-Music-Server
    PRIVATE HXLibs
)

# 查找 Python
find_package(Python COMPONENTS Interpreter Development REQUIRED)

# 查找 pybind11
find_package(pybind11 REQUIRED)

# 链接 pybind11
target_link_libraries(HX-Music-Server PRIVATE pybind11::embed)

# 查找 SQLite3
find_package(SQLite3 REQUIRED)

# 链接 SQLite3 库
target_link_libraries(HX-Music-Server PRIVATE SQLite::SQLite3)

# 第三方依赖 (音频信息解析)
if (WIN32)
    # 添加头文件路径
    include_directories(
        "${LIB_ROOT}/include"
    )

    # 链接库文件
    target_link_libraries(HX-Music-Server PRIVATE
        "${LIB_ROOT}/debug/lib/tag.lib"
        "${LIB_ROOT}/debug/lib/tag_c.lib"
    )
else()
    find_package(taglib CONFIG REQUIRED)  # 必须小写"taglib"
    target_link_libraries(HX-Music-Server
        PRIVATE
        TagLib::TagLib
    )
endif()