#For persistence with ROOT need to create dictionaries for tree constructors.
root_generate_dictionary(treeSystemDictionary 
  treeSystem/treeConstructionInterface.hpp 
  treeSystem/helicalConstruction.hpp
  treeSystem/monopodialConstruction.hpp
  treeSystem/stochasticConstruction.hpp
  treeSystem/sympodialConstruction.hpp
  treeSystem/ternaryConstruction.hpp 
  treeSystem/stumpConstruction.hpp
  LINKDEF treeSystem/Linkdef.h
  OPTIONS -rmf treeSystemDictionary.rootmap -rml libtreeSystems.so)

add_library(treeSystems SHARED treeConstructionInterface.cpp treeFactory.cpp monopodial.cpp sympodial.cpp 
  ternary.cpp helical.cpp stochastic.cpp stump.cpp monopodialConstruction.cpp sympodialConstruction.cpp 
  ternaryConstruction.cpp helicalConstruction.cpp stochasticConstruction.cpp stumpConstruction.cpp
  treeSystemDictionary.cxx)

add_dependencies(treeSystems ${ROOT_LIBRARIES})

add_cppcheck(treeSystems)

#Install results into respective directories
install(TARGETS treeSystems 
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib/static)

install(FILES 
  ${CMAKE_CURRENT_BINARY_DIR}/treeSystemDictionary_rdict.pcm
  ${CMAKE_CURRENT_BINARY_DIR}/treeSystemDictionary.rootmap
  DESTINATION lib/)