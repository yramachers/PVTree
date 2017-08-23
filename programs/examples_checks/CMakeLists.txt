# Create a list of all the programs
set(PVTREE_PROGRAMS
  initialTest
  leafVisualize
  leafCheck
  benchmark
  convergence
  persistenceCheck
  dailyEnergyPlotter
  smartsIntegrationTesting
  configurationTesting
  leafSimulate
  efficiencyCorrelationPlotter
  ntupleCombiner
  convergenceCombiner
  materialConfigurationTest
  #particleSwarmTesting Needs eo, not used yet
  climateDataTesting
  smartsClimateAffect
  lightfieldTest
  lightfieldCombiner
  )

foreach(_pv_program ${PVTREE_PROGRAMS})
  # Create the exe from each target
  add_executable(pvtree-${_pv_program} ${_pv_program}.cpp)

  # Inc dirs until we add target include dirs
  target_include_directories(pvtree-${_pv_program}
    PUBLIC
    ${PROJECT_SOURCE_DIR}
    )

  # Link to all until we can better modularize
  # So will overlink, but makes things work...
  target_link_libraries(pvtree-${_pv_program}
    pvtree-analysis
    pvtree-fullsim
    pvtree-treeSystems
    pvtree-geometry
    pvtree-location
    pvtree-leafSystems
    pvtree-climate
    pvtree-utils
    pvtree-solpos
    # And all ROOT libs until we can define better...
    ${ROOT_MathCore_LIBRARY}
    ${ROOT_Gpad_LIBRARY}
    ${ROOT_Graf_LIBRARY}
    )

  # Don't install - use as source code examples to look at.
  #  install(TARGETS pvtree-${_pv_program} DESTINATION ${CMAKE_INSTALL_BINDIR})
endforeach()
