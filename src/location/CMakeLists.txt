add_library(location SHARED locationDetails.cpp)
add_dependencies(location ${LIBCONFIG_LIBRARY})

add_cppcheck(location)

#Install results into respective directories
install(TARGETS location
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib/static)