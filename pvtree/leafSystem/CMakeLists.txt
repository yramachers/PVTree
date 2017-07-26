#For persistence with ROOT need to create dictionaries for leaf constructors.
root_generate_dictionary(pvtree-leafSystemDictionary
  leafConstructionInterface.hpp
  cordateConstruction.hpp
  roseConstruction.hpp
  simpleConstruction.hpp
  planarConstruction.hpp
  LINKDEF Linkdef.h
  OPTIONS -rmf pvtree-leafSystemDictionary.rootmap -rml pvtree-libleafSystems.so
  -I${CMAKE_CURRENT_SOURCE_DIR}
  -I${Libconfig_INCLUDE_DIR}
  )

add_library(pvtree-leafSystems SHARED
  cordate.cpp
  cordate.hpp
  cordateConstruction.cpp
  cordateConstruction.hpp
  leafConstructionInterface.cpp
  leafConstructionInterface.hpp
  leafFactory.cpp
  leafFactory.hpp
  leafSystemInterface.hpp
  planar.cpp
  planar.hpp
  planarConstruction.cpp
  planarConstruction.hpp
  rose.cpp
  rose.hpp
  roseConstruction.cpp
  roseConstruction.hpp
  simple.cpp
  simple.hpp
  simpleConstruction.cpp
  simpleConstruction.hpp
  pvtree-leafSystemDictionary.cxx
  )

target_include_directories(pvtree-leafSystems
  PUBLIC ${ROOT_INCLUDE_DIRS}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} # For dictionary
  )
target_link_libraries(pvtree-leafSystems
  PUBLIC ${ROOT_Core_LIBRARY} pvtree-geometry Libconfig::Libconfig
  )

add_cppcheck(pvtree-leafSystems)

#Install results into respective directories
install(TARGETS pvtree-leafSystems
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib/static)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/pvtree-leafSystemDictionary_rdict.pcm
  ${CMAKE_CURRENT_BINARY_DIR}/pvtree-leafSystemDictionary.rootmap
  DESTINATION lib/)
