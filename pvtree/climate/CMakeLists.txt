add_library(pvtree-climate SHARED
  climate.cpp
  climate.hpp
  climateData.cpp
  climateData.hpp
  climateFactory.cpp
  climateFactory.hpp
  )
target_include_directories(pvtree-climate
  PUBLIC ${ROOT_INCLUDE_DIRS}
  )
target_link_libraries(pvtree-climate
  PUBLIC pvtree-location eccodes ${ROOT_MathMore_LIBRARY}
  PRIVATE Libconfig::Libconfig pvtree-utils
  )

add_cppcheck(pvtree-climate)

#Install results into respective directories
install(TARGETS pvtree-climate
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )
