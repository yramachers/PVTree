#include <iostream>
#include <fstream>
#include <string>

// ROOT includes
#include "TFile.h"
#include "TList.h"

// PVTree includes
#include "pvtree/treeSystem/treeConstructionInterface.hpp"
#include "pvtree/leafSystem/leafConstructionInterface.hpp"
#include "pvtree/analysis/yearlyResult.hpp"
#include "pvtree/utils/getopt_pp.h"
#include "pvtree/utils/resource.hpp"

void showHelp() {
  std::cout << "batch converter help" << std::endl;
  std::cout << "\t -f, --inputListFile <LIST FILE NAME>: default 'filenames.txt'" << std::endl;
}

int main(int argc, char** argv) {
  std::string filelist;
  std::string rootfilename;

  GetOpt::GetOpt_pp ops(argc, argv);

  // Check for help request
  if (ops >> GetOpt::OptionPresent('h', "help")) {
    showHelp();
    return 0;
  }

  ops >> GetOpt::Option('f', "inputListFile", filelist, "filenames.txt");

  if (filelist == "") {
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

  // read file with list of root file names to consider
  std::ifstream flist;
  std::vector<std::string> filevector;
  flist.open(filelist.c_str());
  if (flist.is_open()) {
    while (std::getline(flist,rootfilename))
      filevector.push_back(rootfilename);
  }
  else {
    std::cout << "Unable to open file containing root file list." << std::endl;
    return 1;
  }
  flist.close();

  int fid = 1;
  int id = 0;
  int bestfid;
  double globalbest = 0.0;
  TreeConstructionInterface* bestT;
  TreeConstructionInterface* globalbestT;
  
  double area;
  double energy;
  double eff;
  double lai;
  double sx, sy;
  
  double besteff = 0.0;
  for (std::string rfn : filevector) { // check efficiency for each file
    TFile ff(rfn.c_str(), "READ");
    TList* structureList = (TList*)ff.Get("testedStructures");
    TIter structureListIterator(structureList);
    
    if (structureList->GetSize() == 0) {
      std::cout << "There are no trees to consider." << std::endl;
      ff.Close();
      continue;
    }

    while (YearlyResult* currentStructure =
	   (YearlyResult*)structureListIterator()) {
      TreeConstructionInterface* clonedT = currentStructure->getTree();

      area = clonedT->getDoubleParameter("sensitiveArea");
      energy = clonedT->getDoubleParameter("totalIntegratedEnergyDeposit");
      sx = clonedT->getDoubleParameter("structureXSize");
      sy = clonedT->getDoubleParameter("structureYSize");
      lai = area / (sx * sy);

      eff = energy * lai;
      if (eff > besteff) {  // book best tree
	bestT = clonedT;
	besteff = eff;
	std::cout << "File ID: " << fid << "; Tree ID: " << id << "; Best efficiency = " 
		  << besteff << std::endl;
      }
      id++;
    }
    ff.Close();
    
    if (besteff > globalbest) {  // book best tree
      bestfid = fid;
      globalbest = besteff;
      globalbestT = bestT;
    }
    fid++;
    id = 0;
    besteff = 0.0;
  }
  std::cout << "Summary: " << std::endl;
  globalbestT->print();
  std::cout << "File ID: " << bestfid << "; Best global efficiency = " 
	    << globalbest << std::endl;
}
