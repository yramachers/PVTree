
# Create a list of all the examples
set(exampleTargets initialTest basicVisualize leafVisualize leafCheck basicSimulate 
  randomVisualize treeScan benchmark convergence persistenceCheck dailyEnergyPlotter
  yearlyEnergyPlotter bestyearlyEnergyPlotter smartsIntegrationTesting configurationTesting leafSimulate 
  yearlyTreeScan converter convertYearly bestTreeVisualizer efficiencyCorrelationPlotter ntupleCombiner 
  convergenceCombiner materialConfigurationTest particleSwarmTesting climateDataTesting 
  smartsClimateAffect lightfieldTest yearlyLightfieldScan lightfieldCombiner lightfieldTreeScan 
  basicLightfieldSimulate visualiseBestYearly yearlyDiffuseLightfieldScan)

foreach(_exampleTarget ${exampleTargets})

  # Create the exe from each target
  add_executable(${_exampleTarget} ${_exampleTarget}.cpp)
  target_link_libraries(${_exampleTarget} ${LIBS})

  # Run static check of source code
  add_cppcheck(${_exampleTarget})

endforeach(_exampleTarget ${exampleTargets})

#Install results into respective directories
install(TARGETS ${exampleTargets}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib/static)

