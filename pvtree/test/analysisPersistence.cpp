#include "pvtree/test/catch.hpp"
#include "pvtree/treeSystem/treeFactory.hpp"
#include "pvtree/leafSystem/leafFactory.hpp"
#include "pvtree/analysis/yearlyResult.hpp"
#include "pvtree/utils/equality.hpp"

#include <stdexcept>
#include <cstdio>
#include <string>

#include "TFile.h"
#include "TList.h"

TEST_CASE( "analysis/yearlyResult", "[analysis]" ) {

  auto tree = TreeFactory::instance()->getTree("sympodial");
  auto leaf = LeafFactory::instance()->getLeaf("cordate");

  // Create some random analysis 'results'
  int seed        = 1024;
  int trialNumber = 100;
  int equalityPrecisionFactor = 10;

  // Use a tree parameter to generate random energy values
  // and day numbers!
  tree->setParameter("totalEnergy", 500.0);
  tree->setRandomParameterRange("totalEnergy", 0.0, 1000.0);
  tree->setParameter("day", 1);
  tree->setRandomParameterRange("day", 1, 365);

  TList exportList;

  // Create varied lists
  std::vector<double> energies;
  std::vector<time_t> dayTimes;

  bool cloningSuccessful = true;
  for (int t=0; t<trialNumber; t++){

    tree->randomizeParameters(seed+t);
    leaf->randomizeParameters(seed+t);

    if (t % 10 == 0){
      energies.clear();
      dayTimes.clear();
    }

    energies.push_back(tree->getDoubleParameter("totalEnergy"));
    dayTimes.push_back(tree->getIntegerParameter("day"));

    // ... Simulation would go here ...

    std::string treeName = "tree" + std::to_string(t) + "_Seed" + std::to_string(seed);
    TreeConstructionInterface* clonedTree = static_cast<TreeConstructionInterface*>(tree->Clone(treeName.c_str()));

    std::string leafName = "leaf" + std::to_string(t) + "_Seed" + std::to_string(seed);
    LeafConstructionInterface* clonedLeaf = static_cast<LeafConstructionInterface*>(leaf->Clone(leafName.c_str()));

    if ( *clonedTree != *tree || *clonedLeaf != *leaf ){
      cloningSuccessful = false;
    }


    // Add to the list that will be exported
    YearlyResult* result = new YearlyResult();
    result->setTree(clonedTree);
    result->setLeaf(clonedLeaf);
    result->setDayTimes(dayTimes);
    result->setEnergyDeposited(energies);

    exportList.Add(result);
  }

  // Check that there wasn't any problem in cloning the structures.
  CHECK( cloningSuccessful );

  // Test that the analysis results can be stored correctly
  std::string persistFileName = "/tmp/unit-analysisPersistence-temp.root";
  TFile exportFile(persistFileName.c_str(), "RECREATE");

  exportList.Write("testedStructures", TObject::kSingleKey);

  exportFile.Close();


  TFile importFile(persistFileName.c_str(), "READ");
  TList* importList = (TList*)importFile.FindObjectAny("testedStructures");

  // Compare imported and exported lists
  REQUIRE( importList->GetSize() == exportList.GetSize() );

  TIter importIterator(importList);
  TIter exportIterator(&exportList);

  bool treesIdentical = true;
  bool leavesIdentical = true;
  bool daysIdentical = true;
  bool energiesIdentical = true;
  for (int i=0; i<importList->GetSize(); i++) {

    YearlyResult* importedResult = static_cast<YearlyResult*>(importIterator.Next());
    YearlyResult* exportedResult = static_cast<YearlyResult*>(exportIterator.Next());

    if ( *(importedResult->getTree()) != *(exportedResult->getTree()) ){
      treesIdentical = false;
    }

    if ( *(importedResult->getLeaf()) != *(exportedResult->getLeaf()) ){
      leavesIdentical = false;
    }

    if ( importedResult->getDayTimes() != exportedResult->getDayTimes() ){
      daysIdentical = false;
    }

    std::vector<double> importedEnergies = importedResult->getEnergyDeposited();
    std::vector<double> exportedEnergies = exportedResult->getEnergyDeposited();

    if (importedEnergies.size() != exportedEnergies.size()) {
      energiesIdentical = false;
      continue;
    }

    // For floating point value comparison use special equality check.
    for (unsigned int j=0; j<importedEnergies.size(); j++){
      if ( !almost_equal(importedEnergies[j], exportedEnergies[j], equalityPrecisionFactor) ){
	energiesIdentical = false;
      }
    }

  }

  CHECK( treesIdentical );
  CHECK( leavesIdentical );
  CHECK( daysIdentical );
  CHECK( energiesIdentical );

  importFile.Close();

  // Test that the temporary file can be deleted.
  int removeFileFlag = std::remove(persistFileName.c_str());
  REQUIRE( removeFileFlag == 0 );

}
