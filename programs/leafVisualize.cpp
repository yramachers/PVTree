#include "pvtree/leafSystem/leafFactory.hpp"
#include "pvtree/geometry/turtle.hpp"
#include "pvtree/full/layeredLeafConstruction.hpp"
#include "pvtree/full/visualizationAction.hpp"
#include "pvtree/full/material/materialFactory.hpp"
#include "pvtree/utils/getopt_pp.h"
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
  std::cout << "leafVisualize help" << std::endl;
  std::cout << "\t -l, --leaf <LEAF TYPE NAME>" << std::endl;
}

int main(int argc, char** argv) {

  std::string leafType;

  GetOpt::GetOpt_pp ops(argc, argv);


  // Check for help request
  if (ops >> GetOpt::OptionPresent('h', "help")){
    showHelp();
    return 0;
  }

  ops >> GetOpt::Option('l', "leaf", leafType, "rose");

  // Also do not run if other arguments are present
  if (ops.options_remain()) {
    std::cerr << "Oops! Unexpected options." << std::endl;
    showHelp();
    return -1;
  }

  // Set the default materials to be used
  MaterialFactory::instance()->addConfigurationFile("defaults-tree.cfg");

  //Default turtle at origin
  Turtle* initialTurtle = new Turtle();

  auto leafConstructor = LeafFactory::instance()->getLeaf(leafType);
  leafConstructor->print();

  LayeredLeafConstruction* leaf = new LayeredLeafConstruction(leafConstructor, initialTurtle);
  
  leaf->Construct();
  G4LogicalVolume* logicalWorldVolume = leaf->getLogicalVolume();

  //Setting the Geant visualization
  G4VisManager* visManager = new G4VisExecutive;
  visManager->RegisterRunDurationUserVisAction
    ("Testing visualization",
     new VisualizationAction(logicalWorldVolume),
     G4VisExtent(-0.5*m,0.5*m,-0.5*m,0.5*m,-0.5*m,0.5*m));
  visManager->Initialize ();

  G4UIExecutive* ui = new G4UIExecutive(argc, argv);
  G4UImanager* UImanager = G4UImanager::GetUIpointer();

  UImanager->ApplyCommand ("/control/verbose 2");
  UImanager->ApplyCommand ("/vis/verbose parameters");
  UImanager->ApplyCommand ("/vis/open OGLSQt");
  UImanager->ApplyCommand ("/vis/scene/create");
  UImanager->ApplyCommand ("/vis/scene/add/userAction");
  UImanager->ApplyCommand ("/vis/scene/add/axes");
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
  delete initialTurtle;
}






