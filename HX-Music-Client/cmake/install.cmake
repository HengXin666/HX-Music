file(GLOB_RECURSE src_files CONFIGURE_DEPENDS 
    src/*.cpp
    include/*.h
    include/*.hpp
)

file(GLOB_RECURSE qrc_files CONFIGURE_DEPENDS 
    resources/*.qrc
)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Qml Quick)

qt_add_executable(HX-Music-Client
    ${src_files}
    ${qrc_files}
)

target_compile_features(HX-Music-Client PUBLIC cxx_std_20)

# 本头文件
target_include_directories(HX-Music-Client PRIVATE include)

# 公共头文件
target_include_directories(HX-Music-Client PRIVATE ../include)

# 导入 HXLibs
target_link_libraries(HX-Music-Client
    PRIVATE HXLibs
    PRIVATE Qt::Core
    PRIVATE Qt::Widgets
    PRIVATE Qt::Gui
    PRIVATE Qt::Qml
    PRIVATE Qt::Quick
)

# 添加TBB依赖
find_package(TBB REQUIRED)
target_link_libraries(HX-Music-Client PRIVATE TBB::tbb)

set(QT_QML_GENERATE_QMLLS_INI ON)

# 添加 QML 文件所在目录作为资源路径
qt_add_qml_module(HX-Music-Client
    URI HX.Music # QML 中 import 的名字
    VERSION 1.0
    QML_FILES
        # 主界面
        resources/qml/Main.qml
        # 窗口
        resources/qml/window/LyricsWindow.qml
        resources/qml/window/BorderlessWindow.qml
        resources/qml/window/FullScreenWindow.qml
        # 控件
        resources/qml/widget/PlaybackBar.qml
        resources/qml/widget/MainSideBar.qml
        resources/qml/widget/MusicProgressBar.qml
        resources/qml/widget/VolumeButton.qml
        resources/qml/widget/PlayModeButton.qml
        resources/qml/widget/LoopingScrollingText.qml
        resources/qml/widget/MusicListView.qml
        resources/qml/widget/PlayStatusButton.qml
        resources/qml/widget/ProgressBarRect.qml
        resources/qml/widget/PlaylistView.qml
        resources/qml/widget/MessageManager.qml
        resources/qml/widget/UploadListView.qml
        # 内部控件组件
        resources/qml/widget/internal/MusicActionButton.qml
        resources/qml/widget/internal/AudioVisualizerBars.qml
        resources/qml/widget/internal/ColorPicker.qml
        resources/qml/widget/internal/IconTextField.qml
        resources/qml/widget/internal/SideNavItem.qml
        resources/qml/widget/internal/GlowButton.qml
        resources/qml/widget/internal/TextButton.qml
        # 页面
        resources/qml/view/SettingView.qml
        resources/qml/view/LoginView.qml
        resources/qml/view/AllMusicListView.qml
        resources/qml/view/UserView.qml
        resources/qml/view/BackendView.qml
        # 全局状态数据
        resources/qml/data/LyricsState.qml
)

# Qt拓展 (音频播放)
find_package(Qt6 REQUIRED COMPONENTS Multimedia)
target_link_libraries(HX-Music-Client PRIVATE Qt::Multimedia)

# Qt拓展 (SVG)
find_package(Qt6 REQUIRED COMPONENTS Svg)
target_link_libraries(HX-Music-Client PRIVATE Qt::Svg)

# Qt拓展 (XML)
find_package(Qt6 REQUIRED COMPONENTS Xml)
target_link_libraries(HX-Music-Client PRIVATE Qt::Xml)

# Qt拓展 (编码)
# find_package(Qt6 REQUIRED COMPONENTS Core5Compat)
# target_link_libraries(HX-Music-Client PRIVATE Qt::Core5Compat)

# Qt拓展 (并行库)
# find_package(Qt6 REQUIRED COMPONENTS Concurrent)
# target_link_libraries(HX-Music-Client PRIVATE Qt6::Concurrent)

# 主题
find_package(Qt6 REQUIRED COMPONENTS QuickControls2)
target_link_libraries(HX-Music-Client PRIVATE Qt6::QuickControls2)

if (WIN32)
    # 解决路径问题, 确保 windeployqt.exe 存在
    set(QT_BIN_DIR "${QT_COMPILER_PATH}/bin")
    if(NOT EXISTS "${QT_BIN_DIR}/windeployqt.exe")
        message(FATAL_ERROR "Error: windeployqt.exe not found in ${QT_BIN_DIR}")
    endif()

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/Debug")
        add_custom_command(TARGET HX-Music-Client POST_BUILD
            COMMAND "${QT_BIN_DIR}/windeployqt.exe" --debug "$<TARGET_FILE:HX-Music-Client>"
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/Debug"
        )
    elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
        file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/Release")
        add_custom_command(TARGET HX-Music-Client POST_BUILD
            COMMAND "${QT_BIN_DIR}/windeployqt.exe" --release "$<TARGET_FILE:HX-Music-Client>"
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
    target_link_libraries(HX-Music-Client PRIVATE
        "${LIB_ROOT}/debug/lib/tag.lib"
        "${LIB_ROOT}/debug/lib/tag_c.lib"
    )
else()
    find_package(taglib CONFIG REQUIRED)  # 必须小写"taglib"
    target_link_libraries(HX-Music-Client
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
    target_link_libraries(HX-Music-Client PRIVATE
        "${LIB_ROOT}/debug/lib/ass.lib"
    )
else()
    find_package(LibAss REQUIRED)
    target_link_libraries(HX-Music-Client PRIVATE LibAss)
endif()

# find_package(Qt6 REQUIRED COMPONENTS WaylandCompositor WaylandClient)
# target_link_libraries(HX-Music-Client PRIVATE Qt6::WaylandCompositor Qt6::WaylandClient)

# find_package(Qt6 REQUIRED COMPONENTS DBus)
# target_link_libraries(HX-Music-Client PRIVATE Qt6::DBus)

# 查找 X11
# find_package(X11 REQUIRED)
# target_link_libraries(HX-Music-Client PRIVATE ${X11_LIBRARIES})

set_target_properties(HX-Music-Client PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE OFF
    WIN32_EXECUTABLE OFF # 这里需要为 OFF 才可以让vscode在控制台中输出...
)

include(GNUInstallDirs)
install(TARGETS HX-Music-Client
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
