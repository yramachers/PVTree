# - Generate binreloc header/source with mangled names, ensuring first
# character(s) are a valid for a C identifier
string(RANDOM LENGTH 18 MANGLE_BINRELOC_MAIN)
set(MANGLE_BINRELOC "MB${MANGLE_BINRELOC_MAIN}")
configure_file(pvtree_binreloc.h.in pvtree_binreloc.h @ONLY)

# Code relative paths
file(RELATIVE_PATH PVTREE_LIBDIR_TO_SHAREDIR "${CMAKE_INSTALL_FULL_LIBDIR}/Pvtree" "${CMAKE_INSTALL_FULL_DATADIR}/${PROJECT_NAME}")
configure_file(resource.cpp.in resource.cpp @ONLY)

add_library(pvtree-utils SHARED
  equality.hpp
  getRSS.cpp
  getopt_pp.cpp
  getopt_pp.h
  pvtree_binreloc.h
  pvtree_binreloc.c
  ${CMAKE_CURRENT_BINARY_DIR}/resource.cpp
  resource.hpp
  signalReceiver.cpp
  signalReceiver.hpp
  )
target_include_directories(pvtree-utils
  PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  )

add_cppcheck(pvtree-utils)

install(TARGETS pvtree-utils
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}
  )
