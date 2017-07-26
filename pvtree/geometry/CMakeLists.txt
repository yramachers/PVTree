add_library(pvtree-geometry SHARED
  polygon.cpp
  polygon.hpp
  turtle.cpp
  turtle.hpp
  vertex.cpp
  vertex.hpp
  )
target_include_directories(pvtree-geometry PUBLIC
  ${ROOT_INCLUDE_DIRS}
  )
target_link_libraries(pvtree-geometry PUBLIC
  ${ROOT_Physics_LIBRARY}
  )

add_cppcheck(pvtree-geometry)

install(TARGETS pvtree-geometry
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib/static)
