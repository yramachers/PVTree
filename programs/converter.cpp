#include <iostream>
#include <string>

// ROOT includes
#include "TFile.h"
#include "TList.h"
#include "TNtupleD.h"

// PVTree includes
#include "pvtree/treeSystem/treeConstructionInterface.hpp"
#include "pvtree/leafSystem/leafConstructionInterface.hpp"
#include "pvtree/analysis/yearlyResult.hpp"
#include "pvtree/utils/getopt_pp.h"
#include "pvtree/utils/resource.hpp"

void showHelp() {
  std::cout << "converter help" << std::endl;
  std::cout << "\t -f, --inputRootFile <ROOT FILE NAME>" << std::endl;
}

int main(int argc, char** argv) {
  std::string filename;
  std::string filename_out;

  GetOpt::GetOpt_pp ops(argc, argv);

  // Check for help request
  if (ops >> GetOpt::OptionPresent('h', "help")) {
    showHelp();
    return 0;
  }

  ops >> GetOpt::Option('f', "inputRootFile", filename, "");

  if (filename == "") {
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

  pvtree::loadEnvironment();

  filename_out = "converted_out.root";
  TFile ff(filename.c_str(), "READ");
  TList* structureList = (TList*)ff.Get("testedStructures");
  TIter structureListIterator(structureList);
  //   TList* treeList = (TList*)ff.Get("testedTrees");
  //   TList* leafList = (TList*)ff.Get("testedLeaves");

  //   TIter iterTrees(treeList);
  //   TIter iterLeaves(leafList);

  if (structureList->GetSize() == 0) {
    std::cout << "There are no trees to consider." << std::endl;
    return 1;
  }

  TFile ffout(filename_out.c_str(), "RECREATE");
  ffout.cd();
  //  TNtupleD* results = new TNtupleD("treeoutput","Tree data
  //  output","id:area:nleaves:initialE:energy:structureX:structureY:structureZ:eff");
  TNtupleD* results = new TNtupleD(
      "treeoutput", "Tree data output",
      "id:area:nleaves:energy:structureX:structureY:structureZ:eff");
  double id = 0.;
  double area;
  double nleaves;
  double energy;
  //  double ien;
  //  double density;
  double lai;
  double eff;
  double sx, sy, sz;

  ff.cd();
  double besteff = 0.0;
  while (YearlyResult* currentStructure =
             (YearlyResult*)structureListIterator()) {
    TreeConstructionInterface* clonedT = currentStructure->getTree();

    area = clonedT->getDoubleParameter("sensitiveArea");
    nleaves = clonedT->getIntegerParameter("leafNumber");
    energy = clonedT->getDoubleParameter("totalEnergy");
    //    ien = clonedT->getDoubleParameter("totalInitial");
    //    density = energy / area;
    sx = clonedT->getDoubleParameter("structureXSize");
    sy = clonedT->getDoubleParameter("structureYSize");
    sz = clonedT->getDoubleParameter("structureZSize");
    lai = area / (sx * sy);

    eff = energy * lai;  // harvest ratio times leaf area index
    //    eff = density; // harvest ratio times leaf area index

    if (eff > besteff) {  // book best tree
      TreeConstructionInterface* bestT = clonedT;
      besteff = eff;
      bestT->print();
      std::cout << "Tree ID: " << id << "; Best efficiency = " << besteff
                << std::endl;
    }
    ffout.cd();
    results->Fill(id, area, nleaves, energy, sx, sy, sz, eff);
    //    results->Fill(id,area, nleaves, energy, eff);
    id++;
    ff.cd();
  }
  ffout.cd();
  results->Write();
  ffout.Close();

  ff.Close();
}
