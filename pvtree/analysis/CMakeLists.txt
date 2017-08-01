#For persistence with ROOT need to create dictionaries for analysis objects.
root_generate_dictionary(pvtree-analysis_dict
  pvtree/analysis/yearlyResult.hpp
  MODULE pvtree-analysis
  LINKDEF Linkdef.h
  OPTIONS
    -noIncludePaths
    -I${Libconfig_INCLUDE_DIR} # Needed because we depend on pvtree-treeSystems, which depends on...
  )
configure_file(yearlyResult.hpp ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/pvtree/analysis/yearlyResult.hpp COPYONLY)

add_library(pvtree-analysis SHARED
  rootStyles.cpp
  rootStyles.hpp
  yearlyResult.cpp
  yearlyResult.hpp
  pvtree-analysis_dict.cxx
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
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}
  )
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/libpvtree-analysis_rdict.pcm
  ${CMAKE_CURRENT_BINARY_DIR}/libpvtree-analysis.rootmap
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}
  )

