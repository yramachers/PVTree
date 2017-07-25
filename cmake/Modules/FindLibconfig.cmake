find_path(Libconfig_INCLUDE_DIR libconfig.h++)
find_library(Libconfig_LIBRARY NAMES config++)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Libconfig
  REQUIRED_VARS Libconfig_INCLUDE_DIR Libconfig_LIBRARY
  )

if(Libconfig_FOUND)
  set(Libconfig_INCLUDE_DIRS ${Libconfig_INCLUDE_DIR})

  if(NOT Libconfig_LIBRARIES)
    set(Libconfig_LIBRARIES ${Libconfig_LIBRARY})
  endif()

  if(NOT TARGET Libconfig::Libconfig)
    add_library(Libconfig::Libconfig UNKNOWN IMPORTED)
    set_target_properties(Libconfig::Libconfig PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${Libconfig_INCLUDE_DIRS}"
      IMPORTED_LOCATION "${Libconfig_LIBRARY}"
      )
  endif()
endif()

