#include "treeSystem/treeFactory.hpp"
#include "leafSystem/leafFactory.hpp"
#include "full/detectorConstruction.hpp"
#include "full/visualizationAction.hpp"
#include "material/materialFactory.hpp"
#include "utils/getopt_pp.h"
#include <iostream>
#include <vector>
#include <memory>

#include "globals.hh"
#include "G4VisExecutive.hh"
#include "G4VisExtent.hh"
#include "G4UImanager.hh"
#include "G4UIExecutive.hh"
#include "G4SystemOfUnits.hh"

void showHelp() {
  std::cout << "basicVisualize help" << std::endl;
  std::cout << "\t -t, --tree <TREE TYPE NAME>" << std::endl;
  std::cout << "\t -l, --leaf <LEAF TYPE NAME>" << std::endl;
}

int main(int argc, char** argv) {


  std::string treeType, leafType;

  GetOpt::GetOpt_pp ops(argc, argv);

  // Check for help request
  if (ops >> GetOpt::OptionPresent('h', "help")){
    showHelp();
    return 0;
  }

  ops >> GetOpt::Option('t', "tree", treeType, "helical");
  ops >> GetOpt::Option('l', "leaf", leafType, "cordate");

  // Also do not run if other arguments are present
  if (ops.options_remain()) {
    std::cerr << "Oops! Unexpected options." << std::endl;
    showHelp();
    return -1;
  }

  auto tree = TreeFactory::instance()->getTree(treeType);
  tree->print();
  auto leaf = LeafFactory::instance()->getLeaf(leafType);
  leaf->print();

  // Set the default materials to be used
  MaterialFactory::instance()->addConfigurationFile("defaults-tree.cfg");

  DetectorConstruction* detector = new DetectorConstruction(tree, leaf);

  detector->Construct();
  G4LogicalVolume* logicalWorldVolume = detector->getLogicalVolume();

  //Setting the Geant visualization
  G4VisManager* visManager = new G4VisExecutive;
  visManager->RegisterRunDurationUserVisAction
    ("Testing visualization", new VisualizationAction(logicalWorldVolume), 
     G4VisExtent(-10*m,10*m,-10*m,10*m,-10*m,10*m));
  visManager->Initialize();

  G4UIExecutive* ui = new G4UIExecutive(argc, argv);
  G4UImanager* UImanager = G4UImanager::GetUIpointer();

  UImanager->ApplyCommand ("/control/verbose 2");
  UImanager->ApplyCommand ("/vis/verbose parameters");
  UImanager->ApplyCommand ("/vis/open OGLSQt");
  UImanager->ApplyCommand ("/vis/scene/create");
  UImanager->ApplyCommand ("/vis/scene/add/userAction");
  UImanager->ApplyCommand ("/vis/scene/add/scale");
  UImanager->ApplyCommand ("/vis/sceneHandler/attach");
  
  UImanager->ApplyCommand ("/vis/viewer/set/upVector 0 0 1");
  UImanager->ApplyCommand ("/vis/viewer/set/projection p 45 deg");
  UImanager->ApplyCommand ("/vis/viewer/set/viewpointThetaPhi 90.0 90.0 deg");
  UImanager->ApplyCommand ("/vis/viewer/set/rotationStyle freeRotation");
  UImanager->ApplyCommand ("/vis/viewer/set/style s"); // solid (display faces of geometry)
  UImanager->ApplyCommand ("/vis/viewer/set/background 1 1 1 1");
  UImanager->ApplyCommand ("/vis/viewer/flush");
  
  ui->SessionStart();

  delete ui;
  delete visManager;
}






