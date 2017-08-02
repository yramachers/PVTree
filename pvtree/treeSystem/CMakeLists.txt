#For persistence with ROOT need to create dictionaries for tree constructors.
root_generate_dictionary(pvtree-treeSystem_dict
  pvtree/treeSystem/treeConstructionInterface.hpp
  pvtree/treeSystem/helicalConstruction.hpp
  pvtree/treeSystem/monopodialConstruction.hpp
  pvtree/treeSystem/stochasticConstruction.hpp
  pvtree/treeSystem/sympodialConstruction.hpp
  pvtree/treeSystem/ternaryConstruction.hpp
  pvtree/treeSystem/stumpConstruction.hpp
  MODULE pvtree-treeSystems
  LINKDEF Linkdef.h
  OPTIONS
    -noIncludePaths
    -I${Libconfig_INCLUDE_DIRS}
  )

foreach(_clingHdr
  treeConstructionInterface.hpp
  helicalConstruction.hpp
  monopodialConstruction.hpp
  stochasticConstruction.hpp
  sympodialConstruction.hpp
  ternaryConstruction.hpp
  stumpConstruction.hpp)
  configure_file(${_clingHdr} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/pvtree/treeSystem/${_clingHdr} COPYONLY)
  install(FILES ${_clingHdr} DESTINATION ${CMAKE_INSTALL_LIBDIR}/pvtree/treeSystem/)
endforeach()

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
  pvtree-treeSystem_dict.cxx
  )

target_include_directories(pvtree-treeSystems
  PUBLIC ${ROOT_INCLUDE_DIRS}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} # For dictionary file
  )
target_link_libraries(pvtree-treeSystems
  PUBLIC
    pvtree-geometry
    Libconfig::Libconfig
    ${ROOT_Core_LIBRARY}
  PRIVATE
    pvtree-utils
  )

add_cppcheck(pvtree-treeSystems)

#Install results into respective directories
install(TARGETS pvtree-treeSystems
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )

