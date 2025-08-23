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