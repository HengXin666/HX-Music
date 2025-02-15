# * 查找 KF6WindowSystem 库
# 本模块用于查找 KF6WindowSystem 库及其头文件所在目录。
#
# KF6WINDOWSYSTEM_INCLUDE_DIR - 存放 KF6WindowSystem 头文件的目录
# KF6WINDOWSYSTEM_LIBRARIES   - KF6WindowSystem 库文件
# KF6WINDOWSYSTEM_FOUND       - 若找到 KF6WindowSystem 则为 TRUE

# 查找 KF6WindowSystem 头文件
find_path(KF6WINDOWSYSTEM_INCLUDE_DIR
  NAMES KF6/KWindowSystem/KWindowSystem
  PATHS /usr/include/KF6/ /usr/local/include/KF6 ${Lib_ROOT}/include/KF6
)

# 查找 KF6WindowSystem 库
find_library(KF6WINDOWSYSTEM_LIBRARIES
  NAMES KF6WindowSystem
  PATHS /usr/lib /usr/local/lib ${Lib_ROOT}/lib
)

# 标准化查找结果
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(KF6WindowSystem DEFAULT_MSG 
  KF6WINDOWSYSTEM_LIBRARIES KF6WINDOWSYSTEM_INCLUDE_DIR
)

# 如果找到库，则创建 CMake 目标
if(KF6WINDOWSYSTEM_FOUND)
  if(NOT TARGET KF6::WindowSystem)
    add_library(KF6::WindowSystem UNKNOWN IMPORTED)
  endif()
  set_target_properties(
    KF6::WindowSystem
    PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${KF6WINDOWSYSTEM_INCLUDE_DIR}/KF6/KWindowSystem"
      IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
      IMPORTED_LOCATION "${KF6WINDOWSYSTEM_LIBRARIES}"
  )
  mark_as_advanced(KF6WINDOWSYSTEM_LIBRARIES)
endif()
