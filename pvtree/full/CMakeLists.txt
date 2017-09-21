include(${Geant4_USE_FILE})

root_generate_dictionary(pvtree-fullsim_dict
  pvtree/full/solarSimulation/plenoptic1D.hpp
  pvtree/full/solarSimulation/plenoptic3D.hpp
  MODULE pvtree-fullsim
  LINKDEF solarSimulation/Linkdef.h
  OPTIONS -noIncludePaths
  )
foreach(_clingHdr solarSimulation/plenoptic1D.hpp solarSimulation/plenoptic3D.hpp)
  configure_file(${_clingHdr} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/pvtree/full/${_clingHdr} COPYONLY)
  install(FILES ${_clingHdr} DESTINATION ${CMAKE_INSTALL_LIBDIR}/pvtree/full/)
endforeach()


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
  recorders/forestRecorder.cpp
  recorders/forestRecorder.hpp
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
  pvtree-fullsim_dict.cxx
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
  PRIVATE
    pvtree-utils
  )

add_cppcheck(pvtree-fullsim)

#Install results into respective directories
install(TARGETS pvtree-fullsim
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )

# - Recurse into solarSimulation to generate HosekSkyModel data
add_subdirectory(solarSimulation)

