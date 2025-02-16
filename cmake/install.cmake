file(GLOB_RECURSE src_files CONFIGURE_DEPENDS 
    src/*.cpp
    include/*.h
    include/*.hpp
)

file(GLOB_RECURSE qrc_files CONFIGURE_DEPENDS 
    resources/*.qrc
)

include_directories(include)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Gui)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(HX-Music
        MANUAL_FINALIZATION
        ${src_files}
        ${qrc_files}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET HX-Music APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(HX-Music SHARED
            ${src_files}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(HX-Music
            ${src_files}
        )
    endif()
endif()

target_link_libraries(HX-Music
    PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
    PRIVATE Qt${QT_VERSION_MAJOR}::Gui
)

# Qt拓展 (音频播放)
find_package(Qt6 REQUIRED COMPONENTS Multimedia)
target_link_libraries(HX-Music PRIVATE Qt6::Multimedia)

# Qt拓展 (SVG)
find_package(Qt6 REQUIRED COMPONENTS Svg)
target_link_libraries(HX-Music PRIVATE Qt6::Svg)

# Qt拓展 (XML)
find_package(Qt6 REQUIRED COMPONENTS Xml)
target_link_libraries(HX-Music PRIVATE Qt6::Xml)

# Qt拓展 (并行库)
# find_package(Qt6 REQUIRED COMPONENTS Concurrent)
# target_link_libraries(HX-Music PRIVATE Qt6::Concurrent)

find_package(Qt6Gui REQUIRED)
target_include_directories(HX-Music PRIVATE ${Qt6Gui_PRIVATE_INCLUDE_DIRS})

# 第三方依赖 (音频信息解析)
find_package(TagLib REQUIRED)
target_link_libraries(HX-Music PRIVATE TagLib)

# 第三方依赖 (Ass字幕渲染)
find_package(LibAss REQUIRED)
target_link_libraries(HX-Music PRIVATE LibAss)

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

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.hx.music)
endif()
set_target_properties(HX-Music PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS HX-Music
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(HX-Music)
endif()