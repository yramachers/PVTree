/*!
 * @file
 * \brief Application to test the visualization of the simulation,
 *        where a small number of optical photons are generated and
 *        fired at a default Ternary+Cordate leaf structure.
 *
 * The visualization shows the world bounding box, photon tracks, hits
 * and the complete detector geometry.
 */

#include "pvtree/treeSystem/treeFactory.hpp"
#include "pvtree/leafSystem/leafFactory.hpp"
#include "pvtree/geometry/turtle.hpp"
#include "pvtree/full/detectorConstruction.hpp"
#include "pvtree/full/actionInitialization.hpp"
#include "pvtree/full/opticalPhysicsList.hpp"
#include "pvtree/full/primaryGeneratorAction.hpp"
#include "pvtree/full/recorders/dummyRecorder.hpp"
#include "pvtree/full/material/materialFactory.hpp"
#include "pvtree/utils/getopt_pp.h"
#include "pvtree/utils/resource.hpp"
#include "pvtree/location/locationDetails.hpp"

#include <iostream>
#include <vector>
#include <memory>

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

// save diagnostic state
#pragma GCC diagnostic push

// turn off the specific warning.
#pragma GCC diagnostic ignored "-Wshadow"

#include "TFile.h"

// turn the warnings back on
#pragma GCC diagnostic pop

void showHelp() {
  std::cout << "basicSimulate help" << std::endl;
  std::cout << "\t -t, --tree <TREE TYPE NAME>" << std::endl;
  std::cout << "\t -l, --leaf <LEAF TYPE NAME>" << std::endl;
  std::cout << "\t --inputTreeFile <ROOT FILENAME> :\t default ''" << std::endl;
  std::cout << "\t --photonNumber <INTEGER>" << std::endl;
}

/*! Test program for the simulation step. */
int main(int argc, char** argv) {
  std::string treeType, leafType;
  unsigned int photonNumberPerEvent;
  std::string inputTreeFileName;
  bool singleTreeRunning = false;

  GetOpt::GetOpt_pp ops(argc, argv);

  // Check for help request
  if (ops >> GetOpt::OptionPresent('h', "help")) {
    showHelp();
    return 0;
  }

  ops >> GetOpt::Option('t', "tree", treeType, "sympodial");
  ops >> GetOpt::Option('l', "leaf", leafType, "simple");
  ops >> GetOpt::Option("inputTreeFile", inputTreeFileName, "");
  ops >> GetOpt::Option("photonNumber", photonNumberPerEvent, 10u);

  // Also do not run if other arguments are present
  if (ops.options_remain()) {
    std::cerr << "Oops! Unexpected options." << std::endl;
    showHelp();
    return -1;
  }

  // Report input parameters
  if (inputTreeFileName != "") {
    std::cout << "Just using selected tree from " << inputTreeFileName
              << std::endl;
    singleTreeRunning = true;
  } else {
    std::cout << "Tree type = " << treeType << std::endl;
    std::cout << "Leaf type = " << leafType << std::endl;
    singleTreeRunning = false;
  }

  // Load PVTree environment
  pvtree::loadEnvironment();

  // Prepare initial conditions for test trunk and leaves
  std::shared_ptr<TreeConstructionInterface> tree;
  std::shared_ptr<LeafConstructionInterface> leaf;

  if (!singleTreeRunning) {
    tree = TreeFactory::instance()->getTree(treeType);
    leaf = LeafFactory::instance()->getLeaf(leafType);
  } else {
    TFile inputTreeFile(inputTreeFileName.c_str(), "READ");
    tree = std::shared_ptr<TreeConstructionInterface>(
        (TreeConstructionInterface*)inputTreeFile.FindObjectAny(
            "selectedTree"));
    leaf = std::shared_ptr<LeafConstructionInterface>(
        (LeafConstructionInterface*)inputTreeFile.FindObjectAny(
            "selectedLeaf"));
    inputTreeFile.Close();
  }

  // Get the device location details
  LocationDetails deviceLocation("location.cfg");

  // Set the altitude of the spectrum factory using location details
  SpectrumFactory::instance()->setAltitude(deviceLocation.getAltitude());

  // Prepare the climate factory with the default configuration
  ClimateFactory::instance()->setConfigurationFile("default.cfg");
  ClimateFactory::instance()->setDeviceLocation(deviceLocation);

  // Define the sun setting, just an arbitrary time and date for now
  Sun sun(deviceLocation);
  sun.setDate(190, 2014);
  sun.setTime(12, 30, 30);

  // Set the default materials to be used
  MaterialFactory::instance()->addConfigurationFile("defaults-tree.cfg");

  // Detect interactive mode (if no arguments) and define UI session
  //
  G4UIExecutive* ui = new G4UIExecutive(argc, argv);

  // Choose the Random engine
  G4Random::setTheEngine(new CLHEP::RanecuEngine);
  G4Random::setTheSeed(1234);

  G4RunManager* runManager = new G4RunManager;

  // Set mandatory initialization classes
  //
  runManager->SetUserInitialization(new DetectorConstruction(tree, leaf));
  OpticalPhysicsList* physicsList = new OpticalPhysicsList;
  runManager->SetUserInitialization(physicsList);

  // Set user action classes
  DummyRecorder dummyRecorder;

  // Setup primary generator to initialize for the simulation
  runManager->SetUserInitialization(new ActionInitialization(
      &dummyRecorder,
      [&photonNumberPerEvent, &sun ]() -> G4VUserPrimaryGeneratorAction *
      { return new PrimaryGeneratorAction(photonNumberPerEvent, &sun); }));

  // Initialize visualization
  //
  G4VisManager* visManager = new G4VisExecutive;
  visManager->Initialize();

  // Get the pointer to the User Interface manager
  G4UImanager* UImanager = G4UImanager::GetUIpointer();

  // Process macro or start UI session
  //
  if (!ui) {
    // Only implementing interactive mode!
  } else {
    // interactive mode
    UImanager->ApplyCommand("/run/verbose 2");
    UImanager->ApplyCommand("/run/initialize");
    UImanager->ApplyCommand("/control/verbose 2");

    UImanager->ApplyCommand("/vis/open OGL");
    UImanager->ApplyCommand("/vis/scene/create");
    UImanager->ApplyCommand("/vis/scene/add/userAction");

    // Draw geometry
    UImanager->ApplyCommand("/vis/drawVolume");

    UImanager->ApplyCommand("/vis/scene/add/axes");
    UImanager->ApplyCommand("/vis/scene/add/scale");
    UImanager->ApplyCommand("/vis/viewer/set/upVector 0 0 1");
    UImanager->ApplyCommand("/vis/viewer/set/projection p 45 deg");
    UImanager->ApplyCommand("/vis/viewer/set/viewpointThetaPhi 90.0 90.0 deg");
    UImanager->ApplyCommand("/vis/viewer/set/rotationStyle freeRotation");
    UImanager->ApplyCommand(
        "/vis/viewer/set/style s");  // solid (display faces of geometry)
    UImanager->ApplyCommand("/vis/viewer/set/background 1 1 1 1");

    // Disable auto refresh and quieten vis messages whilst scene and
    // trajectories are established
    UImanager->ApplyCommand("/vis/viewer/set/autoRefresh false");

    // Draw the trajectories
    UImanager->ApplyCommand("/vis/scene/add/trajectories smooth");
    UImanager->ApplyCommand("/vis/modeling/trajectories/create/drawByCharge");
    UImanager->ApplyCommand(
        "/vis/modeling/trajectories/drawByCharge-0/default/setDrawStepPts "
        "true");
    UImanager->ApplyCommand(
        "/vis/modeling/trajectories/drawByCharge-0/default/setStepPtsSize 2");

    // Draw the hits
    UImanager->ApplyCommand("/vis/scene/add/hits");

    // Superimpose all the events
    UImanager->ApplyCommand("/vis/scene/endOfEventAction accumulate");

    UImanager->ApplyCommand("/vis/viewer/set/autoRefresh true");
    UImanager->ApplyCommand("/vis/viewer/flush");

    // Generate 1 event by default
    UImanager->ApplyCommand("/run/beamOn 1");

    ui->SessionStart();
    delete ui;
  }

  // Job termination
  // Free the store: user actions, physics_list and detector_description are
  // owned and deleted by the run manager, so they should not be deleted
  // in the main() program !
  delete visManager;
  delete runManager;
}
