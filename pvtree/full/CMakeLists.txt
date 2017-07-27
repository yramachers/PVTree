include(${Geant4_USE_FILE})

root_generate_dictionary(pvtree-fullsimDictionary
  pvtree/full/solarSimulation/plenoptic1D.hpp
  pvtree/full/solarSimulation/plenoptic3D.hpp
  LINKDEF solarSimulation/Linkdef.h
  OPTIONS -rmf pvtree-fullsimDictionary.rootmap -rml libpvtree-fullsim.so
  )

add_library(pvtree-fullsim SHARED
  actionInitialization.cpp
  actionInitialization.hpp
  commandRandomizeDetector.hpp
  detectorConstruction.cpp
  detectorConstruction.hpp
  eventAction.cpp
  eventAction.hpp
  layeredLeafConstruction.cpp
  layeredLeafConstruction.hpp
  leafConstruction.cpp
  leafConstruction.hpp
  leafTrackerHit.cpp
  leafTrackerHit.hpp
  leafTrackerSD.cpp
  leafTrackerSD.hpp
  lightfieldGeneratorAction.cpp
  lightfieldGeneratorAction.hpp
  opticalPhysicsList.cpp
  opticalPhysicsList.hpp
  primaryGeneratorAction.cpp
  primaryGeneratorAction.hpp
  runAction.cpp
  runAction.hpp
  steppingAction.cpp
  steppingAction.hpp
  visualizationAction.cpp
  visualizationAction.hpp
  weightedParticleGun.cpp
  weightedParticleGun.hpp
  material/materialFactory.cpp
  material/materialFactory.hpp
  recorders/convergenceRecorder.cpp
  recorders/convergenceRecorder.hpp
  recorders/dummyRecorder.cpp
  recorders/dummyRecorder.hpp
  recorders/recorderBase.hpp
  solarSimulation/HosekSkyModel.cpp
  solarSimulation/HosekSkyModel.hpp
  solarSimulation/plenoptic1D.cpp
  solarSimulation/plenoptic1D.hpp
  solarSimulation/plenoptic3D.cpp
  solarSimulation/plenoptic3D.hpp
  solarSimulation/smarts295.f
  solarSimulation/smartsWrap.hpp
  solarSimulation/spectrum.cpp
  solarSimulation/spectrum.hpp
  solarSimulation/spectrumFactory.cpp
  solarSimulation/spectrumFactory.hpp
  solarSimulation/sun.cpp
  solarSimulation/sun.hpp
  pvtree-fullsimDictionary.cxx
  )
target_include_directories(pvtree-fullsim
  PUBLIC
    ${Geant4_INCLUDE_DIRS}
    ${ROOT_INCLUDE_DIRS}
  )
target_link_libraries(pvtree-fullsim
  PUBLIC
    ${Geant4_LIBRARIES}
    ${ROOT_Hist_LIBRARY}
    ${ROOT_Tree_LIBRARY}
    ${ROOT_RIO_LIBRARY}
    pvtree-geometry pvtree-treeSystems pvtree-leafSystems pvtree-climate pvtree-solpos
  )

add_cppcheck(pvtree-fullsim)

#Install results into respective directories
install(TARGETS pvtree-fullsim
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib/static)
