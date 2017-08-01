# Create Catch main library
add_library(pvtree-catchmain SHARED catch-main.cpp catch.hpp)

# Prep Geant4 datasets for test environment
#set(PVTREE_GEANT4_DATASET_PATHS)
#foreach(_g4ds ${Geant4_DATASETS})
#  list(APPEND PVTREE_GEANT4_DATASET_PATHS "${Geant4_DATASET_${_g4ds}_ENVVAR}=${Geant4_DATASET_${_g4ds}_PATH}")
#endforeach()

# add_test wrapper so data paths can be set in test environment
function(pvtree_add_test _name)
  add_test(NAME ${_name} COMMAND ${_name})
  set_tests_properties(${_name}
    PROPERTIES
      ENVIRONMENT "PVTREE_CLIMATE_DATA_PATH=$ENV{PVTREE_CLIMATE_DATA_PATH};ROOT_INCLUDE_PATH=${PROJECT_SOURCE_DIR}"
  )
endfunction()


add_executable(analysisPersistence analysisPersistence.cpp)
target_link_libraries(analysisPersistence
  pvtree-catchmain
  pvtree-treeSystems
  pvtree-leafSystems
  pvtree-analysis
  ${ROOT_RIO_LIBRARY}
  )
pvtree_add_test(analysisPersistence)

add_executable(climateVariableAccess climateVariableAccess.cpp)
target_link_libraries(climateVariableAccess
  pvtree-catchmain
  pvtree-climate
  pvtree-location
  )
pvtree_add_test(climateVariableAccess)

add_executable(configurationReading configurationReading.cpp)
target_link_libraries(configurationReading
  pvtree-catchmain
  pvtree-utils
  Libconfig::Libconfig
  )
pvtree_add_test(configurationReading)

add_executable(geantSimulation geantSimulation.cpp)
target_link_libraries(geantSimulation
  pvtree-catchmain
  pvtree-fullsim
  pvtree-utils
  )
pvtree_add_test(geantSimulation)

add_executable(leafConstruction leafConstruction.cpp)
target_link_libraries(leafConstruction
  pvtree-catchmain
  pvtree-leafSystems
  ${ROOT_RIO_LIBRARY}
  )
pvtree_add_test(leafConstruction)

add_executable(locationAccess locationAccess.cpp)
target_link_libraries(locationAccess
  pvtree-catchmain
  pvtree-location
  )
pvtree_add_test(locationAccess)

add_executable(smartsRunning smartsRunning.cpp)
target_link_libraries(smartsRunning
  pvtree-catchmain
  pvtree-fullsim
  pvtree-location
  )
pvtree_add_test(smartsRunning)

add_executable(treeConstruction treeConstruction.cpp)
target_link_libraries(treeConstruction
  pvtree-catchmain
  pvtree-treeSystems
  ${ROOT_RIO_LIBRARY}
  )
pvtree_add_test(treeConstruction)

add_executable(selfLocation selfLocation.cpp)
target_link_libraries(selfLocation
  pvtree-catchmain
  pvtree-utils
  )
# - Should not need environment for this!!
add_test(NAME selfLocation COMMAND selfLocation)

#add_cppcheck(unittest
#  SUPPRESS
#  "*:${CMAKE_SOURCE_DIR}/include/testing/catch.hpp"
#  FORCE)

