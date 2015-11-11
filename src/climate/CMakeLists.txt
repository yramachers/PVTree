add_library(climate SHARED climateData.cpp climateFactory.cpp climate.cpp)
add_dependencies(climate ${LIBCONFIG_LIBRARY} ${ECCODES_LIBRARY})

add_cppcheck(climate)

#Install results into respective directories
install(TARGETS climate
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib/static)