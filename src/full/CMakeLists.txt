add_library(fullsim SHARED leafTrackerHit.cpp leafTrackerSD.cpp runAction.cpp eventAction.cpp 
  weightedParticleGun.cpp primaryGeneratorAction.cpp actionInitialization.cpp leafConstruction.cpp 
  detectorConstruction.cpp visualizationAction.cpp steppingAction.cpp opticalPhysicsList.cpp
  layeredLeafConstruction.cpp lightfieldGeneratorAction.cpp)
add_dependencies(fullsim ${Geant4_LIBRARIES} geometry treeSystems)

add_cppcheck(fullsim)

#Install results into respective directories
install(TARGETS fullsim
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib/static)