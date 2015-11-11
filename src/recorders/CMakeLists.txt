add_library(recorders SHARED dummyRecorder.cpp convergenceRecorder.cpp)
add_dependencies(recorders ${Geant4_LIBRARIES} ${ROOT_LIBRARIES})

add_cppcheck(recorders)

#Install results into respective directories
install(TARGETS recorders
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib/static)