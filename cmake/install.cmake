file(GLOB_RECURSE src_files CONFIGURE_DEPENDS 
    src/*.cpp
    include/*.h
    include/*.hpp
)

file(GLOB_RECURSE qrc_files CONFIGURE_DEPENDS 
    resources/*.qrc
)

include_directories(include)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

qt_add_executable(HX-Music
    ${src_files}
    ${qrc_files}
)

target_link_libraries(HX-Music
    PRIVATE Qt::Widgets
    PRIVATE Qt::Gui
)

# Qt拓展 (音频播放)
find_package(Qt6 REQUIRED COMPONENTS Multimedia)
target_link_libraries(HX-Music PRIVATE Qt::Multimedia)

# Qt拓展 (SVG)
find_package(Qt6 REQUIRED COMPONENTS Svg)
target_link_libraries(HX-Music PRIVATE Qt::Svg)

# Qt拓展 (XML)
find_package(Qt6 REQUIRED COMPONENTS Xml)
target_link_libraries(HX-Music PRIVATE Qt::Xml)

# Qt拓展 (编码)
find_package(Qt6 REQUIRED COMPONENTS Core5Compat)
target_link_libraries(HX-Music PRIVATE Qt::Core5Compat)

# Qt拓展 (并行库)
# find_package(Qt6 REQUIRED COMPONENTS Concurrent)
# target_link_libraries(HX-Music PRIVATE Qt6::Concurrent)

if (WIN32)
    # 解决路径问题, 确保 windeployqt.exe 存在
    set(QT_BIN_DIR "${CMAKE_PREFIX_PATH}/bin")
    if(NOT EXISTS "${QT_BIN_DIR}/windeployqt.exe")
        message(FATAL_ERROR "Error: windeployqt.exe not found in ${QT_BIN_DIR}")
    endif()

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/Debug")
        add_custom_command(TARGET HX-Music POST_BUILD
            COMMAND "${QT_BIN_DIR}/windeployqt.exe" --debug "$<TARGET_FILE:HX-Music>"
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/Debug"
        )
    elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
        file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/Release")
        add_custom_command(TARGET HX-Music POST_BUILD
            COMMAND "${QT_BIN_DIR}/windeployqt.exe" --release "$<TARGET_FILE:HX-Music>"
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/Release"
        )
    endif()
endif()

# 第三方依赖 (音频信息解析)
if (WIN32)
    # 添加头文件路径
    include_directories(
        "${LIB_ROOT}/include"
    )

    # 链接库文件
    target_link_libraries(HX-Music PRIVATE
        "${LIB_ROOT}/debug/lib/tag.lib"
        "${LIB_ROOT}/debug/lib/tag_c.lib"
    )
else()
    find_package(taglib CONFIG REQUIRED)  # 必须小写"taglib"
    target_link_libraries(HX-Music
        PRIVATE
        TagLib::TagLib
    )
endif()

# 第三方依赖 (Ass字幕渲染)
if (WIN32)
    # 添加头文件路径
    include_directories(
        "${LIB_ROOT}/include"
    )

    # 链接库文件
    target_link_libraries(HX-Music PRIVATE
        "${LIB_ROOT}/debug/lib/ass.lib"
    )
else()
    find_package(libass CONFIG REQUIRED)  # 使用CONFIG模式
    target_link_libraries(HX-Music PRIVATE
        LibAss::LibAss
    )
endif()

# find_package(Qt6 REQUIRED COMPONENTS WaylandCompositor WaylandClient)
# target_link_libraries(HX-Music PRIVATE Qt6::WaylandCompositor Qt6::WaylandClient)

# find_package(Qt6 REQUIRED COMPONENTS DBus)
# target_link_libraries(HX-Music PRIVATE Qt6::DBus)

# 第三方依赖 (在Wayland下实现透明、顶置窗口 | @tip: 使用QWindow即可!)
if(FALSE AND NOT WIN32)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK REQUIRED gtk+-3.0)
    pkg_check_modules(GTK_LAYER_SHELL REQUIRED gtk-layer-shell-0)
    
    target_include_directories(HX-Music PRIVATE 
        ${GTK_INCLUDE_DIRS} 
        ${GTK_LAYER_SHELL_INCLUDE_DIRS}
    )
    target_link_libraries(HX-Music PRIVATE 
        ${GTK_LIBRARIES} 
        ${GTK_LAYER_SHELL_LIBRARIES}
    )

    # 查找 KF6WindowSystem
    find_package(KF6WindowSystem REQUIRED)
    target_link_libraries(HX-Music PUBLIC KF6::WindowSystem)
endif()

set_target_properties(HX-Music PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE OFF
    WIN32_EXECUTABLE OFF # 这里需要为 OFF 才可以让vscode在控制台中输出...
)

include(GNUInstallDirs)
install(TARGETS HX-Music
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
