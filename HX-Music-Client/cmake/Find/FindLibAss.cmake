# * 查找 LibAss 库
# 本模块用于查找 libass 库及其头文件所在目录。
#
# LIBASS_INCLUDE_DIR - 存放 libass 头文件的目录（例如：ass/ass.h）
# LIBASS_LIBRARIES   - libass 库文件
# LIBASS_FOUND       - 若找到 libass 则为 TRUE

find_path(LIBASS_INCLUDE_DIR
  NAMES ass/ass.h
  PATHS /usr/include /usr/local/include ${Lib_ROOT}/lib
)

find_library(LIBASS_LIBRARIES
  NAMES ass libass
  PATHS /usr/lib /usr/local/lib ${Lib_ROOT}/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibAss DEFAULT_MSG LIBASS_LIBRARIES LIBASS_INCLUDE_DIR)

if(LIBASS_FOUND)
  if(NOT TARGET LibAss)
    add_library(LibAss UNKNOWN IMPORTED)
  endif()
  set_target_properties(
    LibAss
    PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${LIBASS_INCLUDE_DIR}"
      IMPORTED_LINK_INTERFACE_LANGUAGES "C"
      IMPORTED_LOCATION "${LIBASS_LIBRARIES}"
  )
  mark_as_advanced(LIBASS_LIBRARIES)
endif()