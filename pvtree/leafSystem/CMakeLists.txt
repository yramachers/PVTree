#For persistence with ROOT need to create dictionaries for leaf constructors.
root_generate_dictionary(pvtree-leafSystem_dict
  pvtree/leafSystem/leafConstructionInterface.hpp
  pvtree/leafSystem/cordateConstruction.hpp
  pvtree/leafSystem/roseConstruction.hpp
  pvtree/leafSystem/simpleConstruction.hpp
  pvtree/leafSystem/planarConstruction.hpp
  MODULE pvtree-leafSystems
  LINKDEF Linkdef.h
  OPTIONS
    -noIncludePaths
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
  pvtree-leafSystem_dict.cxx
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
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}
  )

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/libpvtree-leafSystems_rdict.pcm
  ${CMAKE_CURRENT_BINARY_DIR}/libpvtree-leafSystems.rootmap
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}
  )
