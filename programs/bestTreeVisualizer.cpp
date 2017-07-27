/*!
 * @file 
 * \brief Application to visualize the structure found in a yearly 
 *        tree scan, which was found to have the highest efficiency.
 *
 * Considers all the trees in a TList contained within a file. It
 * currently just considers the surface energy density as the
 * variable of interest.
 */
#include "pvtree/treeSystem/treeConstructionInterface.hpp"
#include "pvtree/leafSystem/leafConstructionInterface.hpp"
#include "pvtree/analysis/yearlyResult.hpp"
#include "pvtree/full/detectorConstruction.hpp"
#include "pvtree/full/visualizationAction.hpp"
#include "pvtree/full/material/materialFactory.hpp"
#include "pvtree/utils/getopt_pp.h"
#include <iostream>
#include <string>

#include "globals.hh"
#include "G4VisExecutive.hh"
#include "G4VisExtent.hh"
#include "G4UImanager.hh"
#include "G4UIExecutive.hh"
#include "G4SystemOfUnits.hh"

// save diagnostic state
#pragma GCC diagnostic push 

// turn off the specific warning.
#pragma GCC diagnostic ignored "-Wshadow"

// ROOT includes
#include "TFile.h"
#include "TList.h"

// turn the warnings back on
#pragma GCC diagnostic pop

void showHelp() {
  std::cout << "bestTreeVisualizer help" << std::endl;
  std::cout << "\t -f, --inputRootFile <ROOT FILE NAME>" << std::endl;
}

int main(int argc, char** argv) {
  std::string filename;

  GetOpt::GetOpt_pp ops(argc, argv);

  // Check for help request
  if (ops >> GetOpt::OptionPresent('h', "help")){
    showHelp();
    return 0;
  }

  ops >> GetOpt::Option('f', "inputRootFile",  filename,           "");
  if (filename == ""){
    std::cerr << "Empty filename" << std::endl;
    showHelp();
    return -1;
  }

  // Also do not run if other arguments are present
  if (ops.options_remain()) {
    std::cerr << "Oops! Unexpected options." << std::endl;
    showHelp();
    return -1;
  }
  
  TFile ff(filename.c_str(),"READ");
  TList* structureList = (TList*)ff.Get("testedStructures");
  TIter structureListIterator(structureList);

//   TList* treeList = (TList*)ff.Get("testedTrees");
//   TList* leafList = (TList*)ff.Get("testedLeaves");

//   TIter iterTrees(treeList);
//   TIter iterLeaves(leafList);

  if ( structureList->GetSize() == 0 ){
    std::cout << "There are no trees to consider." << std::endl;
    return 1;
  }


  // Identify the optimal tree
  double highestEfficiency   = 0.0;
  double eff;
  double area, energy, sx, sy, lai;
  int id = 0;

  std::shared_ptr<TreeConstructionInterface> bestTree;
  std::shared_ptr<LeafConstructionInterface> bestLeaf;
  bool foundATree = false;

//   TreeConstructionInterface* bestTree;
//   LeafConstructionInterface* bestLeaf;
  while (YearlyResult* currentStructure = (YearlyResult*)structureListIterator()) {
    TreeConstructionInterface* clonedT = currentStructure->getTree();
    LeafConstructionInterface* clonedL = currentStructure->getLeaf();;

    area = clonedT->getDoubleParameter("sensitiveArea");
    energy = clonedT->getDoubleParameter("totalEnergy");
    sx = clonedT->getDoubleParameter("structureXSize");
    sy = clonedT->getDoubleParameter("structureYSize");
    lai = area / (sx * sy);

    eff = energy * lai;
    if (eff > highestEfficiency) { // book best tree
      bestTree          = std::shared_ptr<TreeConstructionInterface>( clonedT);
      bestLeaf          = std::shared_ptr<LeafConstructionInterface>( clonedL);
      highestEfficiency = eff;
      foundATree = true;
    }
    id++;
  }

  if (foundATree == false) {
    std::cout << "Unable to find a structure that meets requirements. Nothing to visualize." << std::endl; 
    return 1;
  }

  // Print out the best parameters
  bestTree->print();
  bestLeaf->print();
  std::cout << "Tree ID: "<< id << "; Best efficiency = " << highestEfficiency << std::endl;
  // If an output file is specified save the chosen tree there
//   if (outputRootFile != "") {
//     TFile outputFile(outputRootFile.c_str(), "RECREATE");
//     bestTree->Write("selectedTree");
//     bestLeaf->Write("selectedLeaf");
//     outputFile.Close();
//   }

  // Set the default materials to be used
  MaterialFactory::instance()->addConfigurationFile("defaults-tree.cfg");

  // Visualize the tree in the standard way
  DetectorConstruction* detector = new DetectorConstruction(bestTree, bestLeaf);

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
