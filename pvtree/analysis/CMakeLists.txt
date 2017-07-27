#For persistence with ROOT need to create dictionaries for analysis objects.
root_generate_dictionary(pvtree-analysisDictionary
  yearlyResult.hpp
  LINKDEF Linkdef.h
  OPTIONS -rmf pvtree-analysisDictionary.rootmap -rml libpvtree-analysis.so
  -I${CMAKE_CURRENT_SOURCE_DIR}
  -I${Libconfig_INCLUDE_DIR} # Needed because we depend on pvtree-treeSystems, which depends on...
  )

add_library(pvtree-analysis SHARED
  rootStyles.cpp
  rootStyles.hpp
  yearlyResult.cpp
  yearlyResult.hpp
  pvtree-analysisDictionary.cxx
  )
target_include_directories(pvtree-analysis
  PUBLIC ${ROOT_INCLUDE_DIRS}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} # For dictionary
  )
target_link_libraries(pvtree-analysis
  PUBLIC pvtree-treeSystems ${ROOT_MathMore_LIBRARY}
  )

add_cppcheck(pvtree-analysis)

#Install results into respective directories
install(TARGETS pvtree-analysis
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib/static)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/pvtree-analysisDictionary_rdict.pcm
  ${CMAKE_CURRENT_BINARY_DIR}/pvtree-analysisDictionary.rootmap
  DESTINATION lib/)

