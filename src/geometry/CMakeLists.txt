add_library(geometry SHARED vertex.cpp polygon.cpp turtle.cpp)
add_dependencies(geometry ${ROOT_LIBRARIES})

add_cppcheck(geometry)

install(TARGETS geometry
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib/static)