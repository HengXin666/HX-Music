# * Find TagLib
# Find the TagLib library and include directories.
#
# TAGLIB_INCLUDE_DIR - Directory containing TagLib headers (e.g. taglib/fileref.h)
# TAGLIB_LIBRARIES   - The TagLib library file.
# TAGLIB_FOUND       - TRUE if TagLib is found.

find_path(TAGLIB_INCLUDE_DIR
  NAMES taglib/fileref.h taglib/tag.h
  PATHS /usr/include /usr/local/include ${Lib_ROOT}/lib
)

find_library(TAGLIB_LIBRARIES
  NAMES tag taglib
  PATHS /usr/lib /usr/local/lib ${Lib_ROOT}/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(TagLib DEFAULT_MSG TAGLIB_LIBRARIES TAGLIB_INCLUDE_DIR)

if(TAGLIB_FOUND)
  if(NOT TARGET TagLib)
    add_library(TagLib UNKNOWN IMPORTED)
  endif()
  set_target_properties(
    TagLib
    PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${TAGLIB_INCLUDE_DIR}"
      IMPORTED_LINK_INTERFACE_LANGUAGES "C++"
      IMPORTED_LOCATION "${TAGLIB_LIBRARIES}"
  )
  mark_as_advanced(TAGLIB_LIBRARIES)
endif()
