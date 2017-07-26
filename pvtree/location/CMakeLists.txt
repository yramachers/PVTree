add_library(pvtree-location SHARED
  locationDetails.hpp
  locationDetails.cpp
  )
target_link_libraries(pvtree-location PRIVATE Libconfig::Libconfig)

add_cppcheck(location)

#Install results into respective directories
install(TARGETS pvtree-location
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib/static)
