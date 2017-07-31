#include "pvtree/treeSystem/treeFactory.hpp"
#include "pvtree/leafSystem/leafFactory.hpp"
#include "pvtree/analysis/yearlyResult.hpp"
#include "pvtree/geometry/turtle.hpp"
#include "pvtree/utils/getopt_pp.h"
#include <iostream>
#include <vector>
#include <memory>

#include "TFile.h"

/*! @file
 * \brief Example program to check that tree and leaf constructors can
 *        be saved to a root file and then retrieved.
 */

void showHelp() {
  std::cout << "persistenceCheck help" << std::endl;
  std::cout << "\t -t, --tree <TREE TYPE NAME>" << std::endl;
  std::cout << "\t -l, --leaf <LEAF TYPE NAME>" << std::endl;
}

void create(std::string treeType, std::string leafType) {
  auto initialTree = TreeFactory::instance()->getTree(treeType);
  auto initialLeaf = LeafFactory::instance()->getLeaf(leafType);

  // Change some of the parameters
  initialTree->setParameter("elongationRate", 1.309);
  initialTree->setParameter("widthIncreaseRate", 1.832);
  initialTree->setParameter("branchingAngle", 2.0);
  initialTree->setParameter("divergenceAngle1", 280.0);
  initialTree->setParameter("divergenceAngle2", 12.0);
  initialTree->setParameter("lengthScale", 65.0);

  // Show current parameters
  std::cout << "Initial Tree Parameters: -" << std::endl;
  initialTree->print();

  std::cout << "Initial Leaf Parameters: -" << std::endl;
  initialLeaf->print();

  // Open a root file
  TFile exportFile("persistenceCheck.root", "RECREATE");

  // Make a yearly result
  std::vector<time_t> dayTimes;
  std::vector<double> dayEnergySums;

  // Need to clone the tree and leaf because YearlyResult requires ownership.
  TreeConstructionInterface* clonedTree =
      (TreeConstructionInterface*)initialTree->Clone();
  LeafConstructionInterface* clonedLeaf =
      (LeafConstructionInterface*)initialLeaf->Clone();

  YearlyResult initialResult;
  initialResult.setTree(clonedTree);
  initialResult.setLeaf(clonedLeaf);
  initialResult.setDayTimes(dayTimes);
  initialResult.setEnergyDeposited(dayEnergySums);
  initialResult.Write("initialResult");

  // Close the root file
  exportFile.Close();
}

void check() {
  // Open a root file
  TFile importFile("persistenceCheck.root", "READ");

  // Read in yearly result
  YearlyResult* importedResult =
      (YearlyResult*)importFile.FindObjectAny("initialResult");

  // Show loaded parameters
  std::cout << "Loaded Tree Parameters: -" << std::endl;
  importedResult->getTree()->print();

  std::cout << "Loaded Leaf Parameters: -" << std::endl;
  importedResult->getLeaf()->print();

  // Close the root file
  importFile.Close();
}

int main(int argc, char** argv) {
  std::string treeType, leafType;

  GetOpt::GetOpt_pp ops(argc, argv);

  // Check for help request
  if (ops >> GetOpt::OptionPresent('h', "help")) {
    showHelp();
    return 0;
  }

  ops >> GetOpt::Option('t', "tree", treeType, "ternary");
  ops >> GetOpt::Option('l', "leaf", leafType, "cordate");

  // Also do not run if other arguments are present
  if (ops.options_remain()) {
    std::cerr << "Oops! Unexpected options." << std::endl;
    showHelp();
    return -1;
  }

  // Write a file containing usual results
  create(treeType, leafType);

  // ... Some time later ...
  std::cout << "\n...Close and re-open the file some time later...\n"
            << std::endl;

  // Check that the file can be read back in normally
  check();
}
