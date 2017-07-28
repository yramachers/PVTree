add_library(pvtree-utils SHARED
  equality.hpp
  getRSS.cpp
  getopt_pp.cpp
  getopt_pp.h
  signalReceiver.cpp
  signalReceiver.hpp
  )

add_cppcheck(pvtree-utils)

install(TARGETS pvtree-utils
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}
  )
