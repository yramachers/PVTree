add_library(material SHARED materialFactory.cpp)
add_dependencies(material ${Geant4_LIBRARIES} ${LIBCONFIG_LIBRARY} )

add_cppcheck(material)

#Install results into respective directories
install(TARGETS material
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib/static)