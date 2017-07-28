# Create a list of all the programs
set(PVTREE_PROGRAMS
  initialTest
  basicVisualize
  leafVisualize
  leafCheck
  basicSimulate
  randomVisualize
  treeScan
  benchmark
  convergence
  persistenceCheck
  dailyEnergyPlotter
  yearlyEnergyPlotter
  bestyearlyEnergyPlotter
  smartsIntegrationTesting
  configurationTesting
  leafSimulate
  yearlyTreeScan
  converter
  convertYearly
  bestTreeVisualizer
  treeVisualizer
  efficiencyCorrelationPlotter
  ntupleCombiner
  convergenceCombiner
  materialConfigurationTest
  #particleSwarmTesting Needs eo, not used yet
  climateDataTesting
  smartsClimateAffect
  lightfieldTest
  yearlyLightfieldScan
  lightfieldCombiner
  lightfieldTreeScan
  basicLightfieldSimulate
  visualiseBestYearly
  yearlyDiffuseLightfieldScan
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

  # Configure install-time RPATH to builtin libraries
  if(UNIX AND NOT APPLE)
    set_target_properties(pvtree-${_pv_program}
      PROPERTIES INSTALL_RPATH "\$ORIGIN/../${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}"
      )
  elseif(APPLE)
    set_target_properties(pvtree-${_pv_program}
      PROPERTIES INSTALL_RPATH "@executable_path/../${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}"
      )
  endif()

  # Run static check of source code
  add_cppcheck(pvtree-${_pv_program})
  install(TARGETS pvtree-${_pv_program} DESTINATION ${CMAKE_INSTALL_BINDIR})
endforeach()

