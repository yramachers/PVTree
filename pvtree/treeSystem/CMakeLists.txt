#For persistence with ROOT need to create dictionaries for tree constructors.
root_generate_dictionary(pvtree-treeSystemDictionary
  treeConstructionInterface.hpp
  helicalConstruction.hpp
  monopodialConstruction.hpp
  stochasticConstruction.hpp
  sympodialConstruction.hpp
  ternaryConstruction.hpp
  stumpConstruction.hpp
  LINKDEF Linkdef.h
  OPTIONS -rmf pvtree-treeSystemDictionary.rootmap -rml pvtree-libtreeSystems.so
  -I${CMAKE_CURRENT_SOURCE_DIR}
  -I${Libconfig_INCLUDE_DIRS}
  )

add_library(pvtree-treeSystems SHARED
  helical.cpp
  helical.hpp
  helicalConstruction.cpp
  helicalConstruction.hpp
  monopodial.cpp
  monopodial.hpp
  monopodialConstruction.cpp
  monopodialConstruction.hpp
  stochastic.cpp
  stochastic.hpp
  stochasticConstruction.cpp
  stochasticConstruction.hpp
  stump.cpp
  stump.hpp
  stumpConstruction.cpp
  stumpConstruction.hpp
  sympodial.cpp
  sympodial.hpp
  sympodialConstruction.cpp
  sympodialConstruction.hpp
  ternary.cpp
  ternary.hpp
  ternaryConstruction.cpp
  ternaryConstruction.hpp
  treeConstructionInterface.cpp
  treeConstructionInterface.hpp
  treeFactory.cpp
  treeFactory.hpp
  treeSystemInterface.hpp
  pvtree-treeSystemDictionary.cxx
  )

target_include_directories(pvtree-treeSystems
  PUBLIC ${ROOT_INCLUDE_DIRS}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} # For dictionary file
  )
target_link_libraries(pvtree-treeSystems PUBLIC
  pvtree-geometry
  Libconfig::Libconfig
  ${ROOT_Core_LIBRARY}
  )

add_cppcheck(pvtree-treeSystems)

#Install results into respective directories
install(TARGETS pvtree-treeSystems
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib/static)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/pvtree-treeSystemDictionary_rdict.pcm
  ${CMAKE_CURRENT_BINARY_DIR}/pvtree-treeSystemDictionary.rootmap
  DESTINATION lib/)
