add_library(pvtree-utils SHARED
  equality.hpp
  getRSS.cpp
  getopt_pp.cpp
  getopt_pp.h
  signalReceiver.cpp
  signalReceiver.hpp
  )

add_cppcheck(pvtree-utils)

#Install results into respective directories
install(TARGETS pvtree-utils
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib/static)
