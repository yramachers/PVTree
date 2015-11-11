#For persistence with ROOT need to create dictionaries for leaf constructors.
root_generate_dictionary(leafSystemDictionary 
  leafSystem/leafConstructionInterface.hpp 
  leafSystem/cordateConstruction.hpp
  leafSystem/roseConstruction.hpp
  leafSystem/simpleConstruction.hpp
  leafSystem/planarConstruction.hpp
  LINKDEF leafSystem/Linkdef.h
  OPTIONS -rmf leafSystemDictionary.rootmap -rml libleafSystems.so)

add_library(leafSystems SHARED leafConstructionInterface.cpp leafFactory.cpp cordate.cpp rose.cpp simple.cpp 
  planar.cpp cordateConstruction.cpp roseConstruction.cpp simpleConstruction.cpp planarConstruction.cpp
  leafSystemDictionary.cxx)

add_dependencies(leafSystems geometry ${ROOT_LIBRARIES})

add_cppcheck(leafSystems)

#Install results into respective directories
install(TARGETS leafSystems
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib/static)

install(FILES 
  ${CMAKE_CURRENT_BINARY_DIR}/leafSystemDictionary_rdict.pcm
  ${CMAKE_CURRENT_BINARY_DIR}/leafSystemDictionary.rootmap
  DESTINATION lib/)