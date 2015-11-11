#include "testing/catch.hpp"
#include "climate/climateFactory.hpp"
#include "climate/climate.hpp"
#include "treeSystem/treeFactory.hpp"
#include "leafSystem/leafFactory.hpp"
#include "utils/equality.hpp"
#include "full/detectorConstruction.hpp"
#include "full/layeredLeafConstruction.hpp"
#include "full/leafConstruction.hpp"
#include "full/actionInitialization.hpp"
#include "full/primaryGeneratorAction.hpp"
#include "full/opticalPhysicsList.hpp"
#include "recorders/convergenceRecorder.hpp"
#include "recorders/dummyRecorder.hpp"
#include "solarSimulation/sun.hpp"
#include "material/materialFactory.hpp"
#include <time.h>

#include <iostream>
#include <iomanip>

#include "G4RunManager.hh"
#include "Randomize.hh"

time_t getTestTime();

TEST_CASE( "simulation/geant", "[simulation]" ) {

  // Define the test case
  std::string treeType = "sympodial";
  std::string leafType = "simple";
  unsigned int photonNumberPerEvent = 1000u;
  int geant4Seed = 12345;
  int lSystemSeed = 5432;

  // Get the device location details
  LocationDetails deviceLocation("location.cfg");

  // Prepare the climate factory with the default configuration
  ClimateFactory::instance()->setDeviceLocation(deviceLocation);
  ClimateFactory::instance()->setConfigurationFile("uk-2013to2015.cfg");

  // Get the LSystems to be used
  std::shared_ptr<TreeConstructionInterface> tree = TreeFactory::instance()->getTree(treeType);
  std::shared_ptr<LeafConstructionInterface> leaf = LeafFactory::instance()->getLeaf(leafType);

  //Define the sun setting, just an arbitrary time and date for now
  Sun sun(deviceLocation);

  // Set the default materials to be used
  MaterialFactory::instance()->addConfigurationFile("defaults-tree.cfg");

  // Setup Geant4
  G4Random::setTheEngine(new CLHEP::RanecuEngine);
  G4Random::setTheSeed(geant4Seed);

  G4RunManager* runManager = new G4RunManager;

  // Set mandatory initialization classes  
  DetectorConstruction* detector = new DetectorConstruction(tree, leaf);
  runManager->SetUserInitialization(detector);

  //Construct a recorder to obtain results
  ConvergenceRecorder recorder;

  OpticalPhysicsList* physicsList = new OpticalPhysicsList;
  runManager->SetUserInitialization(physicsList);

  // Setup primary generator to initialize for the simulation
  runManager->SetUserInitialization(new ActionInitialization(&recorder, 
   [&photonNumberPerEvent, &sun] () -> G4VUserPrimaryGeneratorAction* { 
							       return new PrimaryGeneratorAction(photonNumberPerEvent, &sun); }) );

  // Initialize G4 kernel
  runManager->Initialize();

  // Change parameters
  tree->randomizeParameters(lSystemSeed);
  leaf->randomizeParameters(lSystemSeed);
  detector->resetGeometry(tree, leaf);

  // Re-initialize the detector geometry
  G4bool destroyFirst;
  runManager->ReinitializeGeometry(destroyFirst = true);
  
  // Set the time
  sun.setDate( getTestTime() );
  sun.setTime(12, 0, 0);

  //Run simulation with a single event per time point
  G4int eventNumber = 1;
  runManager->BeamOn(eventNumber);

  // Get the total surface area which is "sensitive" from current tested detector.
  double sensitiveArea = detector->getSensitiveSurfaceArea();

  int checkPrecision = 10;
  CHECK( almost_equal( (float)sensitiveArea, 0.1322582999f, checkPrecision) );

  // Get the number of leaves
  int    numberOfLeaves = detector->getNumberOfLeaves();
  int    numberOfRejectedLeaves = detector->getNumberOfRejectedLeaves();

  CHECK( numberOfLeaves == 10 );
  CHECK( numberOfRejectedLeaves == 22 );

  // Get size of the axially alligned bounding box structure along the axis
  double structureXSize = detector->getXSize();
  double structureYSize = detector->getYSize();
  double structureZSize = detector->getZSize();
  
  CHECK( almost_equal( (float)structureXSize, 0.5467519042f, checkPrecision) );
  CHECK( almost_equal( (float)structureYSize, 0.7683677904f, checkPrecision) );
  CHECK( almost_equal( (float)structureZSize, 1.9940984162f, checkPrecision) );

  double totalEnergyDeposited = 0.0;
  long totalPhotonCounts = 0;
  long totalHitCounts = 0;
  std::vector<std::vector<double > > hitEnergies = recorder.getSummedHitEnergies();
  std::vector<std::vector<long > > photonCounts = recorder.getPhotonCounts();
  std::vector<std::vector<long > > hitCounts = recorder.getHitCounts();

  // Check only one result present
  CHECK( hitEnergies.size() == 1u );
  CHECK( photonCounts.size() == 1u );
  CHECK( hitCounts.size() == 1u );

  for (double eventHitEnergy : hitEnergies[0]) {
    totalEnergyDeposited += (eventHitEnergy/1000.0);
  }
  for (long photonCount : photonCounts[0]) {
    totalPhotonCounts += photonCount;
  }
  for (long hitCount : hitCounts[0]) {
    totalHitCounts += hitCount;
  }

  CHECK( almost_equal( (float)totalEnergyDeposited, 0.0040563401f, checkPrecision) );
  CHECK( totalPhotonCounts == photonNumberPerEvent );
  CHECK( totalHitCounts == 5 );

  // Clear results
  recorder.reset();

  CHECK( recorder.getSummedHitEnergies().size() == 0u );
  CHECK( recorder.getPhotonCounts().size() == 0u );
  CHECK( recorder.getHitCounts().size() == 0u );

  // Repeat detector construction but this time apply constraints
  unsigned int treeTrialNumber = 0u;
  unsigned int maximumTrialNumber = 50u;
  unsigned int passingTrees = 0u;
  while(treeTrialNumber < maximumTrialNumber) {
    treeTrialNumber++;

    // Allow the geometry to be rebuilt with new settings
    tree->randomizeParameters(lSystemSeed+treeTrialNumber);
    leaf->randomizeParameters(lSystemSeed+treeTrialNumber);

    detector->resetGeometry(tree, leaf);
    
    // Re-initialize the detector geometry
    runManager->ReinitializeGeometry(destroyFirst = true);

    // Apply pre-selection to the tree after manual construction.
    detector->Construct();

    double minimumArea = 0.5;//m
    if (detector->getSensitiveSurfaceArea() < minimumArea){
      continue;
    }
    if ( detector->getNumberOfRejectedLeaves() > detector->getNumberOfLeaves() ) {
      continue;
    }
    
    passingTrees++;
  }

  // Check that the same number of structures passed
  CHECK( passingTrees == 2 );
  recorder.reset();

  // Run simulation using the different tree types
  std::vector<std::string > availableTreeTypes = {"helical", "monopodial", "stochastic", "stump", "sympodial", "ternary"};

  for (auto currentTreeType : availableTreeTypes) {

      tree = TreeFactory::instance()->getTree(currentTreeType);
      detector->resetGeometry(tree, leaf);

      // Re-initialize the detector geometry
      runManager->ReinitializeGeometry(destroyFirst = true);

      // Run the simulation
      runManager->BeamOn(eventNumber);

      // Clear up any results
      recorder.reset();
  }

  // Check that single leaves can be simulated
  std::vector<std::string > availableLeafTypes = {"simple", "cordate", "rose", "planar"};
  
  //Default turtle at origin
  Turtle* initialTurtle = new Turtle();

  for (auto currentLeafType : availableLeafTypes) {

      // Re-initialize the detector geometry
      runManager->ReinitializeGeometry(destroyFirst = true);

      leaf = LeafFactory::instance()->getLeaf(currentLeafType);
      LayeredLeafConstruction* leafDetector = new LayeredLeafConstruction(leaf, initialTurtle);

      // Switch to the new detector
      runManager->SetUserInitialization(leafDetector);

      // Run the simulation
      runManager->BeamOn(eventNumber);

      // Clear up any results
      recorder.reset();
  }

  // Repeat for alternative leaf constructor
  for (auto currentLeafType : availableLeafTypes) {

      // Re-initialize the detector geometry
      runManager->ReinitializeGeometry(destroyFirst = true);

      leaf = LeafFactory::instance()->getLeaf(currentLeafType);
      LeafConstruction* leafDetector = new LeafConstruction(leaf, initialTurtle);

      // Switch to the new detector
      runManager->SetUserInitialization(leafDetector);

      // Run the simulation
      runManager->BeamOn(eventNumber);

      // Clear up any results
      recorder.reset();
  }

  // Running again but now with a dummy recorder.
  DummyRecorder dummyRecorder;

  runManager->SetUserInitialization(new ActionInitialization(&dummyRecorder, 
   [&photonNumberPerEvent, &sun] () -> G4VUserPrimaryGeneratorAction* { 
							       return new PrimaryGeneratorAction(photonNumberPerEvent, &sun); }) );

  // Run with a single event
  runManager->BeamOn(eventNumber);

  // Clean up
  delete runManager;
}
