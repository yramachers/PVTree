#include "pvtree/treeSystem/treeFactory.hpp"
#include "pvtree/leafSystem/leafFactory.hpp"
#include "pvtree/geometry/turtle.hpp"
#include "pvtree/full/detectorConstruction.hpp"
#include "pvtree/full/actionInitialization.hpp"
#include "pvtree/full/opticalPhysicsList.hpp"
#include "pvtree/full/primaryGeneratorAction.hpp"
#include "pvtree/full/recorders/dummyRecorder.hpp"
#include "pvtree/full/material/materialFactory.hpp"
#include "pvtree/location/locationDetails.hpp"
#include "pvtree/utils/getopt_pp.h"
#include <iostream>
#include <vector>
#include <memory>
#include <chrono>

#include "globals.hh"
#include "G4VisExecutive.hh"
#include "G4VisExtent.hh"
#include "G4UImanager.hh"
#include "G4UIExecutive.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include "G4RunManager.hh"
#include "G4StepLimiterPhysics.hh"

#include "pvtree/full/solarSimulation/sun.hpp"
#include "pvtree/climate/climateFactory.hpp"
#include "pvtree/full/solarSimulation/spectrumFactory.hpp"

// For memory monitoring
size_t getPeakRSS();
size_t getCurrentRSS();

void showHelp() {
  std::cout << "benchmark help" << std::endl;
  std::cout << "\t -t, --tree <TREE TYPE NAME>" << std::endl;
  std::cout << "\t -l, --leaf <LEAF TYPE NAME>" << std::endl;
  std::cout << "\t --treeNumber <INTEGER>" << std::endl;
  std::cout << "\t --photonNumber <INTEGER>" << std::endl;
}

int main(int argc, char** argv) {
  std::string treeType, leafType;
  unsigned int treeNumber;
  unsigned int photonNumberPerEvent;

  GetOpt::GetOpt_pp ops(argc, argv);

  // Check for help request
  if (ops >> GetOpt::OptionPresent('h', "help")) {
    showHelp();
    return 0;
  }

  ops >> GetOpt::Option('t', "tree", treeType, "ternary");
  ops >> GetOpt::Option('l', "leaf", leafType, "cordate");
  ops >> GetOpt::Option("photonNumber", photonNumberPerEvent, 50000u);
  ops >> GetOpt::Option("treeNumber", treeNumber, 100u);

  // Also do not run if other arguments are present
  if (ops.options_remain()) {
    std::cerr << "Oops! Unexpected options." << std::endl;
    showHelp();
    return -1;
  }

  // keep track of the times for each of the trees
  std::chrono::time_point<std::chrono::system_clock> start, end, stepStart,
      stepEnd;
  std::chrono::duration<double> elapsed_seconds, elapsed_initialize,
      elapsed_wrapUp, elapsed_simulation;

  // Initialize Geant4
  stepStart = std::chrono::system_clock::now();
  // Reduce the verbosity
  G4UImanager* UImanager = G4UImanager::GetUIpointer();
  UImanager->ApplyCommand("/run/verbose 0");
  UImanager->ApplyCommand("/event/verbose 0");
  UImanager->ApplyCommand("/process/verbose 0");
  UImanager->ApplyCommand("/control/verbose 0");
  UImanager->ApplyCommand("/units/verbose 0");
  UImanager->ApplyCommand("/geometry/verbose 0");
  UImanager->ApplyCommand("/tracking/verbose 0");
  UImanager->ApplyCommand("/particle/verbose 0");
  UImanager->ApplyCommand("/material/verbose 0");
  UImanager->ApplyCommand("/hits/verbose 0");
  UImanager->ApplyCommand("/random/verbose 0");

  // Choose the Random engine
  G4Random::setTheEngine(new CLHEP::RanecuEngine);
  G4int initialSeed = 1234;
  G4Random::setTheSeed(initialSeed);

  // Get the device location details
  LocationDetails deviceLocation("location.cfg");

  // Set the altitude of the spectrum factory using location details
  SpectrumFactory::instance()->setAltitude(deviceLocation.getAltitude());

  // Prepare the climate factory with the default configuration
  ClimateFactory::instance()->setConfigurationFile("default.cfg");
  ClimateFactory::instance()->setDeviceLocation(deviceLocation);

  // Prepare initial conditions for test trunk and leaves
  auto tree = TreeFactory::instance()->getTree(treeType);
  auto leaf = LeafFactory::instance()->getLeaf(leafType);

  // Define the sun setting, just an arbitrary time and date for now
  Sun sun(deviceLocation);
  sun.setDate(190, 2014);
  sun.setTime(12, 30, 30);

  // Set the default materials to be used
  MaterialFactory::instance()->addConfigurationFile("defaults-tree.cfg");

  // Construct the default run manager
  G4RunManager* runManager = new G4RunManager;

  // Set mandatory initialization classes
  OpticalPhysicsList* physicsList = new OpticalPhysicsList;
  runManager->SetUserInitialization(physicsList);

  DummyRecorder dummyRecorder;

  // Setup primary generator to initialize for the simulation
  runManager->SetUserInitialization(new ActionInitialization(
      &dummyRecorder,
      [&photonNumberPerEvent, &sun ]() -> G4VUserPrimaryGeneratorAction *
      { return new PrimaryGeneratorAction(photonNumberPerEvent, &sun); }));

  DetectorConstruction* detector = new DetectorConstruction(tree, leaf);

  runManager->SetUserInitialization(detector);

  // Initialize G4 kernel
  runManager->Initialize();
  stepEnd = std::chrono::system_clock::now();
  elapsed_initialize += stepEnd - stepStart;

  // Start the clock on the many-tree sim loop
  start = std::chrono::system_clock::now();
  for (unsigned int x = 0; x < treeNumber; x++) {
    // Allow the geometry to be rebuilt with new settings
    tree->randomizeParameters(x);
    detector->resetGeometry(tree, leaf);

    // Re-initialize the detector geometry
    G4bool destroyFirst;
    runManager->ReinitializeGeometry(destroyFirst = true);

    // Start a simulation run
    stepStart = std::chrono::system_clock::now();
    int numberOfEvent = 1;
    runManager->BeamOn(numberOfEvent);
    stepEnd = std::chrono::system_clock::now();
    elapsed_simulation += stepEnd - stepStart;

    if (x % 10 == 0) {
      std::cout << "Attempted tree " << x << std::endl;
      std::cout << "Current memory usage = "
                << getCurrentRSS() / (1024.0 * 1024.0) << " MB"
                << "   Peak memory usage = " << getPeakRSS() / (1024.0 * 1024.0)
                << " MB" << std::endl;
    }
  }
  end = std::chrono::system_clock::now();
  elapsed_seconds = end - start;

  // Job termination
  stepStart = std::chrono::system_clock::now();
  delete runManager;
  stepEnd = std::chrono::system_clock::now();
  elapsed_wrapUp += stepEnd - stepStart;

  // Report benchmark results to screen.
  std::cout << "Total time taken for " << treeNumber
            << " trees = " << elapsed_seconds.count() << " sec" << std::endl;
  std::cout << "Initialize time = " << elapsed_initialize.count() << " sec"
            << std::endl;
  std::cout << "Wrap up time = " << elapsed_wrapUp.count() << " sec"
            << std::endl;
  std::cout << "Simulation time = " << elapsed_simulation.count() << " sec"
            << std::endl;

  std::cout << "Average time per tree = "
            << elapsed_seconds.count() / treeNumber << " sec" << std::endl;
  std::cout << "Estimated trees per hour = "
            << treeNumber*((60.0 * 60.0) / elapsed_seconds.count())
            << std::endl;

  // Report memory usage
  std::cout << "Peak memory usage = " << getPeakRSS() / (1024.0 * 1024.0)
            << " MB" << std::endl;

  return 0;
}
