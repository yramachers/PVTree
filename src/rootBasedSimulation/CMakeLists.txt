add_library(rootBasedSimulation SHARED leaf.cpp rootGeometry.cpp)
add_dependencies(rootBasedSimulation ${ROOT_LIBRARIES} geometry)

add_cppcheck(rootBasedSimulation)

#Install results into respective directories
install(TARGETS rootBasedSimulation
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib/static)