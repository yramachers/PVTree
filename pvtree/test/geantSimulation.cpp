#include "pvtree/test/catch.hpp"
#include "pvtree/climate/climateFactory.hpp"
#include "pvtree/climate/climate.hpp"
#include "pvtree/full/solarSimulation/spectrumFactory.hpp"
#include "pvtree/treeSystem/treeFactory.hpp"
#include "pvtree/leafSystem/leafFactory.hpp"
#include "pvtree/utils/equality.hpp"
#include "pvtree/utils/resource.hpp"
#include "pvtree/full/detectorConstruction.hpp"
#include "pvtree/full/layeredLeafConstruction.hpp"
#include "pvtree/full/leafConstruction.hpp"
#include "pvtree/full/actionInitialization.hpp"
#include "pvtree/full/primaryGeneratorAction.hpp"
#include "pvtree/full/opticalPhysicsList.hpp"
#include "pvtree/full/recorders/convergenceRecorder.hpp"
#include "pvtree/full/recorders/dummyRecorder.hpp"
#include "pvtree/full/solarSimulation/sun.hpp"
#include "pvtree/full/material/materialFactory.hpp"
#include <time.h>

#include <iostream>
#include <iomanip>

#include "G4RunManager.hh"
#include "Randomize.hh"

time_t getTestTime() {
  struct tm calendarTime;
  calendarTime.tm_sec = 0;
  calendarTime.tm_min = 0;
  calendarTime.tm_hour = 12;
  calendarTime.tm_mday = 12;
  calendarTime.tm_mon = 3;
  calendarTime.tm_year = 114;
  calendarTime.tm_isdst = 1;
  return mktime(&calendarTime);
}

TEST_CASE("simulation/geant", "[simulation]") {
  // Note: current hard-coded test case values are possibly inappropriate,
  //       as the G4 random generator in primaryGeneratorAction appears to be 
  //       affected by the state of G4 logical volumes (eg changing the 
  //       treeSpacingFactor in detectorConstruction).
  // Define the test case
  std::string treeType = "sympodial";
  std::string leafType = "simple";
  unsigned int photonNumberPerEvent = 1000u; // was 1000u
  int geant4Seed = 12345;
  int lSystemSeed = 5432;

  // Load PVTree's data environment
  pvtree::loadEnvironment();

  // Get the device location details
  LocationDetails deviceLocation("location.cfg");
  //  std::cout << "Got device location." << std::endl;

  // Set the altitude of the spectrum factory using location details
  SpectrumFactory::instance()->setAltitude(deviceLocation.getAltitude());

  // Prepare the climate factory with the default configuration
  ClimateFactory::instance()->setDeviceLocation(deviceLocation);
  ClimateFactory::instance()->setConfigurationFile("default.cfg");
  //  std::cout << "Got climate configuration." << std::endl;

  // Get the LSystems to be used
  std::shared_ptr<TreeConstructionInterface> tree;
  tree = TreeFactory::instance()->getTree(treeType);

  std::shared_ptr<LeafConstructionInterface> leaf;
  leaf =LeafFactory::instance()->getLeaf(leafType);

  //  std::cout << "Got tree and leaf constructed." << std::endl;
  
  // Define the sun setting, just an arbitrary time and date for now
  Sun sun(deviceLocation);

  // Set the time
  sun.setDate(getTestTime());
  // sun.setDate(19, 2014); // fixed day like in treeScan
  sun.setTime(12, 0, 0);
  //  std::cout << "Got Sun set up." << std::endl;

  // Set the default materials to be used
  MaterialFactory::instance()->addConfigurationFile("defaults-tree.cfg");
  //  std::cout << "Got materials." << std::endl;

  // Setup Geant4
  G4Random::setTheEngine(new CLHEP::RanecuEngine);
  G4Random::setTheSeed(geant4Seed);

  G4RunManager* runManager = new G4RunManager;

  // Set mandatory initialization classes
  DetectorConstruction* detector = new DetectorConstruction(tree, leaf);
  runManager->SetUserInitialization(detector);

  // Construct a recorder to obtain results
  ConvergenceRecorder recorder;

  OpticalPhysicsList* physicsList = new OpticalPhysicsList;
  runManager->SetUserInitialization(physicsList);

  // Setup primary generator to initialize for the simulation
  runManager->SetUserInitialization(new ActionInitialization(
      &recorder,
      [&photonNumberPerEvent, &sun ]() -> G4VUserPrimaryGeneratorAction *
      { return new PrimaryGeneratorAction(photonNumberPerEvent, &sun); }));

  // Initialize G4 kernel
  runManager->Initialize();
  runManager->SetVerboseLevel(0);
  //  std::cout << "Run Manager setup and init." << std::endl;
  
  // Change parameters
  tree->randomizeParameters(lSystemSeed);
  leaf->randomizeParameters(lSystemSeed);
  detector->resetGeometry(tree, leaf);
//   tree->print();
//   leaf->print();
//   std::cout << "Done detector reset geometry method." << std::endl;
  
  // Re-initialize the detector geometry
  //  G4bool destroyFirst;
  runManager->ReinitializeGeometry(true, false);
  runManager->BeamOn(0); // fake start to build geometry

  //  runManager->DefineWorldVolume(detector->Construct());  // reconstruction
  //  runManager->GeometryHasBeenModified();
//   std::cout << "Got run manager reset and re-initialized." << std::endl;
  
  // Run simulation with a single event per time point
  G4int eventNumber = 1;
  runManager->BeamOn(eventNumber);
  //  std::cout << "Got first beam on done." << std::endl;
  
  // Get the total surface area which is "sensitive" from current tested
  // detector.
  double sensitiveArea = detector->getSensitiveSurfaceArea();
  //  std::cout << "Got sensitive area: " << sensitiveArea << std::endl;

  int checkPrecision = 10;
  CHECK(almost_equal((float)sensitiveArea, 0.107676f, checkPrecision));

  // Get the number of leaves
  int numberOfLeaves = detector->getNumberOfLeaves();
  int numberOfRejectedLeaves = detector->getNumberOfRejectedLeaves();

  CHECK(numberOfLeaves == 8);
  CHECK(numberOfRejectedLeaves == 308);

  // Get size of the axially alligned bounding box structure along the axis
  double structureXSize = detector->getXSize();
  double structureYSize = detector->getYSize();
  double structureZSize = detector->getZSize();
  //  std::cout << "structure size X: " << structureXSize << std::endl;
  //  std::cout << "structure size Y: " << structureYSize << std::endl;
  //  std::cout << "structure size Z: " << structureZSize << std::endl;

  CHECK(almost_equal((float)structureXSize, 0.592576f, checkPrecision));
  CHECK(almost_equal((float)structureYSize, 0.820069f, checkPrecision));
  CHECK(almost_equal((float)structureZSize, 2.03961f, checkPrecision));

  double totalEnergyDeposited = 0.0;
  long totalPhotonCounts = 0;
  long totalHitCounts = 0;
  std::vector<std::vector<double> > hitEnergies =
      recorder.getSummedHitEnergies();
  std::vector<std::vector<long> > photonCounts = recorder.getPhotonCounts();
  std::vector<std::vector<long> > hitCounts = recorder.getHitCounts();

  // Check only one result present
  CHECK(hitEnergies.size() == 1u);
  CHECK(photonCounts.size() == 1u);
  CHECK(hitCounts.size() == 1u);

  for (double eventHitEnergy : hitEnergies[0]) {
    totalEnergyDeposited += (eventHitEnergy / 1000.0);
  }
  for (long photonCount : photonCounts[0]) {
    totalPhotonCounts += photonCount;
  }
  for (long hitCount : hitCounts[0]) {
    totalHitCounts += hitCount;
  }

  CHECK(
      almost_equal((float)totalEnergyDeposited, 0.0f, checkPrecision));
  CHECK(totalPhotonCounts == photonNumberPerEvent);
  CHECK(totalHitCounts == 0);
  //  std::cout << "Energy Deposited: " << totalEnergyDeposited << std::endl;

  // Clear results
  recorder.reset();

  CHECK(recorder.getSummedHitEnergies().size() == 0u);
  CHECK(recorder.getPhotonCounts().size() == 0u);
  CHECK(recorder.getHitCounts().size() == 0u);

  // Repeat detector construction but this time apply constraints
  unsigned int treeTrialNumber = 0u;
  unsigned int maximumTrialNumber = 50u;
  unsigned int passingTrees = 0u;
  while (treeTrialNumber < maximumTrialNumber) {
    treeTrialNumber++;

    // Allow the geometry to be rebuilt with new settings
    tree->randomizeParameters(lSystemSeed + treeTrialNumber);
    leaf->randomizeParameters(lSystemSeed + treeTrialNumber);

    detector->resetGeometry(tree, leaf);
    runManager->ReinitializeGeometry(true, false);         // clean up
    runManager->BeamOn(0); // fake start to build geometry

    // Apply pre-selection to the tree after manual construction.
    //    detector->Construct();

    double minimumArea = 0.5;  // m
    if (detector->getSensitiveSurfaceArea() < minimumArea) {
      continue;
    }
    if (detector->getNumberOfRejectedLeaves() > detector->getNumberOfLeaves()) {
      continue;
    }

    passingTrees++;
  }

  // Check that the same number of structures passed
  CHECK(passingTrees == 0);
  recorder.reset();

  // Run simulation using the different tree types
  std::vector<std::string> availableTreeTypes = {
      "helical", "monopodial", "stump", "sympodial"};

  std::vector<float> received_energy = {
    0.607917f, 0.826579f, 1.10628f, 0.0f};

  int counter = 0;
  checkPrecision = 100;
  for (auto currentTreeType : availableTreeTypes) {
    tree = TreeFactory::instance()->getTree(currentTreeType);

    // Allow the geometry to be rebuilt with new settings
    tree->randomizeParameters(lSystemSeed + counter);
    leaf->randomizeParameters(lSystemSeed + counter);
    detector->resetGeometry(tree, leaf);
    runManager->ReinitializeGeometry(true, false);         // clean up
    runManager->BeamOn(0); // fake start to build geometry

    // Re-initialize the detector geometry
    // runManager->ReinitializeGeometry(true, false);         // clean up
    // runManager->DefineWorldVolume(detector->Construct());  // reconstruction
    //    runManager->GeometryHasBeenModified();
    //    runManager->ReinitializeGeometry(destroyFirst = true);

    // Run the simulation
    runManager->BeamOn(eventNumber);

    // check for total energy deposited
    totalEnergyDeposited = 0.0;
    totalPhotonCounts = 0;
    totalHitCounts = 0;
    hitEnergies = recorder.getSummedHitEnergies();
    hitCounts = recorder.getHitCounts();
    photonCounts = recorder.getPhotonCounts();
    for (double eventHitEnergy : hitEnergies[0]) {
      totalEnergyDeposited += eventHitEnergy;
    }
    for (long photonCount : photonCounts[0]) {
    totalPhotonCounts += photonCount;
    }
    for (long hitCount : hitCounts[0]) {
      totalHitCounts += hitCount;
    }
    CHECK(
	almost_equal((float)totalEnergyDeposited, received_energy[counter], checkPrecision));
    //    std::cout << "Energy: " << totalEnergyDeposited << " Expected: " << received_energy[counter] << std::endl;
    //std::cout << "Hits: " << totalHitCounts << " / " << totalPhotonCounts << std::endl;
    // Clear up any results
    recorder.reset();
    counter++;
  }


  // Clean up
  delete runManager;
}
