# Create unit tests
add_executable(unittest unittest.cpp treeConstruction.cpp leafConstruction.cpp smartsRunning.cpp 
  configurationReading.cpp analysisPersistence.cpp climateVariableAccess.cpp geantSimulation.cpp 
  locationAccess.cpp)
target_link_libraries(unittest ${LIBS})

add_cppcheck(unittest
  SUPPRESS
  "*:${CMAKE_SOURCE_DIR}/include/testing/catch.hpp"
  FORCE)

add_test(NAME AllTests COMMAND unittest)