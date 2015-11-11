#For persistence with ROOT need to create dictionaries for analysis objects.
root_generate_dictionary(analysisDictionary 
  analysis/yearlyResult.hpp
  LINKDEF analysis/Linkdef.h
  OPTIONS -rmf analysisDictionary.rootmap -rml libanalysis.so)

add_library(analysis SHARED yearlyResult.cpp analysisDictionary.cxx rootStyles.cpp)

add_cppcheck(analysis)

#Install results into respective directories
install(TARGETS analysis
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib/static)

install(FILES 
  ${CMAKE_CURRENT_BINARY_DIR}/analysisDictionary_rdict.pcm
  ${CMAKE_CURRENT_BINARY_DIR}/analysisDictionary.rootmap
  DESTINATION lib/)

