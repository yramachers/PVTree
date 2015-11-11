#For persistence with ROOT need to create dictionaries for solar simulation objects.
root_generate_dictionary(solarDictionary 
  solarSimulation/plenoptic1D.hpp
  solarSimulation/plenoptic3D.hpp
  LINKDEF solarSimulation/Linkdef.h
  OPTIONS -rmf solarDictionary.rootmap -rml libsolarSimulation.so)

add_library(solarSimulation SHARED spectrum.cpp sun.cpp smarts295.f spectrumFactory.cpp 
solarDictionary.cxx plenoptic1D.cpp plenoptic3D.cpp)

add_dependencies(solarSimulation ${ROOT_LIBRARIES} solpos)

add_cppcheck(solarSimulation)

#Install results into respective directories
install(TARGETS solarSimulation
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib/static)

install(FILES 
  ${CMAKE_CURRENT_BINARY_DIR}/solarDictionary_rdict.pcm
  ${CMAKE_CURRENT_BINARY_DIR}/solarDictionary.rootmap
  DESTINATION lib/)