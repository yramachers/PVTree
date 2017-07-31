/*!
 * @file
 * \brief Application to visualize randomly chosen trees. Useful for
 *        checking the random parameter ranges are not causing issues.
 *
 */

#include "pvtree/treeSystem/treeFactory.hpp"
#include "pvtree/leafSystem/leafFactory.hpp"
#include "pvtree/geometry/turtle.hpp"
#include "pvtree/full/detectorConstruction.hpp"
#include "pvtree/full/visualizationAction.hpp"
#include "pvtree/full/material/materialFactory.hpp"
#include "pvtree/utils/getopt_pp.h"
#include "pvtree/climate/climateFactory.hpp"

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

void showHelp() {
  std::cout << "randomVisualize help" << std::endl;
  std::cout << "\t -t, --tree <TREE TYPE NAME>" << std::endl;
  std::cout << "\t -l, --leaf <LEAF TYPE NAME>" << std::endl;
  std::cout << "\t -s, --seed <INTEGER>" << std::endl;
}

/*! \brief Random tree visualizer.
 *
 * Example how
 *
 * @param[in] argc Number of command line arguments.
 * @param[in] argv Accepts two command line arguments. The first is the
 *                 name of the tree system and the second is the name of
 *                 the leaf system.
 *
 */
int main(int argc, char** argv) {
  std::string treeType, leafType;
  unsigned int seed;

  GetOpt::GetOpt_pp ops(argc, argv);

  // Check for help request
  if (ops >> GetOpt::OptionPresent('h', "help")) {
    showHelp();
    return 0;
  }

  ops >> GetOpt::Option('t', "tree", treeType, "helical");
  ops >> GetOpt::Option('l', "leaf", leafType, "cordate");
  ops >> GetOpt::Option('s', "seed", seed, 1234u);

  // Also do not run if other arguments are present
  if (ops.options_remain()) {
    std::cerr << "Oops! Unexpected options." << std::endl;
    showHelp();
    return -1;
  }

  // double updatePeriod = 3.0; //Seconds
  // int randomSeedStart = 1234; //Set a default random number seed starting
  // point

  // Set the default materials to be used
  MaterialFactory::instance()->addConfigurationFile("defaults-tree.cfg");

  // Obtain the constructors
  auto tree = TreeFactory::instance()->getTree(treeType);
  auto leaf = LeafFactory::instance()->getLeaf(leafType);

  // Randomize the parameters
  tree->randomizeParameters(seed);
  leaf->randomizeParameters(seed);

  // Prepare the climate factory with the default configuration
  ClimateFactory::instance()->setConfigurationFile("default.cfg");

  DetectorConstruction* detector = new DetectorConstruction(tree, leaf);

  detector->Construct();
  G4LogicalVolume* logicalWorldVolume = detector->getLogicalVolume();

  // Set the scale of the visualization extent
  G4double visualScale = 3.0 * m;

  // Setting the Geant visualization
  G4VisManager* visManager = new G4VisExecutive;
  visManager->RegisterRunDurationUserVisAction(
      "Testing visualization", new VisualizationAction(logicalWorldVolume),
      G4VisExtent(-visualScale, visualScale, -visualScale, visualScale,
                  -visualScale, visualScale));
  visManager->Initialize();

  G4UIExecutive* ui = new G4UIExecutive(argc, argv);
  G4UImanager* UImanager = G4UImanager::GetUIpointer();

  UImanager->ApplyCommand("/control/verbose 2");
  UImanager->ApplyCommand("/vis/verbose parameters");
  UImanager->ApplyCommand("/vis/open OGLSQt");
  UImanager->ApplyCommand("/vis/scene/create");
  UImanager->ApplyCommand("/vis/scene/add/userAction");
  UImanager->ApplyCommand("/vis/sceneHandler/attach");

  UImanager->ApplyCommand("/vis/scene/add/scale");
  UImanager->ApplyCommand("/vis/scene/add/axes");

  UImanager->ApplyCommand("/vis/viewer/set/upVector 0 0 1");
  UImanager->ApplyCommand("/vis/viewer/set/projection p 45 deg");
  UImanager->ApplyCommand("/vis/viewer/set/viewpointThetaPhi 70.0 20.0 deg");
  UImanager->ApplyCommand("/vis/viewer/set/rotationStyle freeRotation");
  UImanager->ApplyCommand(
      "/vis/viewer/set/style s");  // solid (display faces of geometry)
  UImanager->ApplyCommand("/vis/viewer/set/background 1 1 1 1");
  UImanager->ApplyCommand("/vis/viewer/flush");

  ui->SessionStart();

  delete ui;
  delete visManager;

  /*
  std::chrono::time_point<std::chrono::system_clock> start, end;
  std::chrono::duration<double> elapsed_seconds;

  int currentSeed = randomSeedStart;

  //Start the infinite loop
  start = std::chrono::system_clock::now();
  while(true) {
    //Check if sufficient time passed, then make a new tree to draw
    end = std::chrono::system_clock::now();
    elapsed_seconds = end - start;

    if (elapsed_seconds.count() > updatePeriod){
      if (tree != 0){
        delete tree;
      }

      currentSeed++;
      tree = createRandomTree(currentSeed);
      myCanvas.cd(1);
      tree->draw();

      myCanvas.Update();

      end = std::chrono::system_clock::now();
      start = end;
    }

    gSystem->ProcessEvents();
  }
  */
}
