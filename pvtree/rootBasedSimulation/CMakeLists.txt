add_library(pvtree-rootBasedSimulation SHARED
  leaf.cpp
  leaf.hpp
  rootGeometry.cpp
  rootGeometry.hpp
  )
target_include_directories(pvtree-rootBasedSimulation
  PUBLIC ${ROOT_INCLUDE_DIRS}
  )
target_link_libraries(pvtree-rootBasedSimulation
  PUBLIC ${ROOT_Physics_LIBRARY}
  PRIVATE pvtree-geometry
  )


add_cppcheck(pvtree-rootBasedSimulation)

#Install results into respective directories
install(TARGETS pvtree-rootBasedSimulation
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib/static)
