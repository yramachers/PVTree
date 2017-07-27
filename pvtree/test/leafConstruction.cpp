#include "pvtree/test/catch.hpp"
#include "pvtree/leafSystem/leafFactory.hpp"
#include <stdexcept>
#include <cstdio>
#include <sstream>

#include "TFile.h"

TEST_CASE( "leafSystem/leafFactory", "[leaf]" ) {

  auto cordateLeaf = LeafFactory::instance()->getLeaf("cordate");

  // Test that current parameters can be set and retrieved
  cordateLeaf->setParameter("initialAngle", 4.6);
  REQUIRE( cordateLeaf->getDoubleParameter("initialAngle") == 4.6);


  // Test that the random generator seeding produces consistant results
  cordateLeaf->setRandomParameterRange("initialAngle", 4.0, 10.5);
  int seed = 1234;
  cordateLeaf->randomizeParameter(seed, "initialAngle");
  double result1 = cordateLeaf->getDoubleParameter("initialAngle");
  cordateLeaf->randomizeParameter(seed, "initialAngle");
  double result2 = cordateLeaf->getDoubleParameter("initialAngle");
  REQUIRE( result1 == result2 );


  // Test that the random generator seeding produces consistant results when randomizing all parameters
  seed = 4321;
  cordateLeaf->randomizeParameters(seed);
  result1 = cordateLeaf->getDoubleParameter("initialAngle");
  cordateLeaf->randomizeParameters(seed);
  result2 = cordateLeaf->getDoubleParameter("initialAngle");
  REQUIRE( result1 == result2 );


  // Test that a non-existant parameter can not be retrieved
  bool nonExistantParameterRetrieved = true;
  try{
    cordateLeaf->getDoubleParameter("noParameterWithName");
  } catch(const std::invalid_argument& err) {
    nonExistantParameterRetrieved = false;
  }
  REQUIRE( nonExistantParameterRetrieved == false );


  // Test that a non-existant parameter (for a given type) can not be retrieved.
  nonExistantParameterRetrieved = true;
  try{
    cordateLeaf->getIntegerParameter("initialAngle");
  } catch(const std::invalid_argument& err) {
    nonExistantParameterRetrieved = false;
  }
  REQUIRE( nonExistantParameterRetrieved == false );


  // Test that a non-existant parameter can not be randomized
  bool nonExistantParameterRandomized = true;
  try{
    cordateLeaf->randomizeParameter(seed, "noParameterWithName");
  } catch(const std::invalid_argument& err) {
    nonExistantParameterRandomized = false;
  }
  REQUIRE( nonExistantParameterRandomized == false );


  // Test that you can have a double and integer parameter with the same name (maybe shouldn't)
  cordateLeaf->setParameter("divergenceAngle", 7.5);
  cordateLeaf->setParameter("divergenceAngle", 4);

  bool bothParametersRetrievedCorrectly = cordateLeaf->getDoubleParameter("divergenceAngle") == 7.5 &&
    cordateLeaf->getIntegerParameter("divergenceAngle") == 4;
  REQUIRE( bothParametersRetrievedCorrectly == true  );

  // Test that the parameter ranges can be set and retrieved from file successfully.
  std::string persistFileName = "/tmp/unit-leafConstuction-temp.root";

  TFile exportFile(persistFileName.c_str(), "RECREATE");
  cordateLeaf->Write("testLeaf");
  exportFile.Close();

  TFile importFile(persistFileName.c_str(), "READ");

  LeafConstructionInterface* importLeaf = static_cast<LeafConstructionInterface*>(importFile.FindObjectAny("testLeaf"));

  REQUIRE( *importLeaf == *cordateLeaf );

  // If I randomize parameters it should no longer be equal
  seed++;
  cordateLeaf->randomizeParameters(seed);

  REQUIRE( *importLeaf != *cordateLeaf );

  // Randomizing the loaded leaf should return the equality
  importLeaf->randomizeParameters(seed);
  REQUIRE( *importLeaf == *cordateLeaf );

  // It should also not match a completely different leaf
  auto simpleLeaf = LeafFactory::instance()->getLeaf("simple");

  REQUIRE( *importLeaf != *simpleLeaf );

  importFile.Close();

  // Test that the temporary file can be deleted.
  int removeFileFlag = std::remove(persistFileName.c_str());
  REQUIRE( removeFileFlag == 0 );

  // Check that the L-Systems evolve in the same manner
  std::stringstream actualLeafState(std::string(""));

  // Check cordate initial state
  cordateLeaf = LeafFactory::instance()->getLeaf("cordate");
  cordateLeaf->print( actualLeafState );

  std::string refLeafState = R"( ------------------------------------------------------------
 |  Double Parameter :      Value |    Minimum |    Maximum |
 ------------------------------------------------------------
 |      initialAngle :         90 |         70 |        120 |
 |   divergenceAngle :         15 |         10 |         20 |
 |         curlAngle :          8 |          3 |         15 |
 |        growthRate :        0.1 |       0.05 |        0.3 |
 |        stemLength :          0 |          0 |          0 |
 |         thickness :       0.01 |      0.002 |       0.06 |
 ------------------------------------------------------------
 | Integer Parameter :      Value |    Minimum |    Maximum |
 ------------------------------------------------------------
 |   iterationNumber :          5 |          3 |          5 |
 ------------------------------------------------------------
Produced Cordate Rules = /(90)G(0)[A(1)][B(1)]
)";

  CHECK( refLeafState == actualLeafState.str() );
  actualLeafState.str("");

  // Check planar initial state
  auto planarLeaf = LeafFactory::instance()->getLeaf("planar");
  planarLeaf->print( actualLeafState );

  refLeafState = R"( ------------------------------------------------------------
 |  Double Parameter :      Value |    Minimum |    Maximum |
 ------------------------------------------------------------
 |      initialAngle :          0 |          0 |        360 |
 | initialEdgeLength :          1 |          1 |          1 |
 |    mainGrowthRate :          1 |          1 |          1 |
 |      offsetLength :          0 |          0 |          0 |
 |         thickness :       0.01 |       0.01 |       0.01 |
 ------------------------------------------------------------
 | Integer Parameter :      Value |    Minimum |    Maximum |
 ------------------------------------------------------------
 |   iterationNumber :          0 |          0 |          0 |
 ------------------------------------------------------------
Produced Planar Rules = G(0,1)/(0)[{&(90)G(0.5,1)/(90)&(90)G(0.5,1).&(90)G(1,1).&(90)G(1,1).}][/(180){&(90)G(0.5,1)/(90)&(90)G(0.5,1).&(90)G(1,1).&(90)G(1,1).}]
)";

  CHECK( refLeafState == actualLeafState.str() );
  actualLeafState.str("");

  // Check rose initial state
  auto roseLeaf = LeafFactory::instance()->getLeaf("rose");
  roseLeaf->print( actualLeafState );

  refLeafState = R"( ------------------------------------------------------------------
 |        Double Parameter :      Value |    Minimum |    Maximum |
 ------------------------------------------------------------------
 |            initialAngle :         90 |          0 |        360 |
 |       mainInitialLength :        0.2 |       0.03 |        0.2 |
 |          mainGrowthRate :        0.9 |        0.5 |        0.9 |
 |    lateralInitialLength :       0.05 |       0.03 |       0.08 |
 |       lateralGrowthRate :        1.2 |        0.8 |        1.3 |
 | growthPotentialDecrease :          1 |          1 |          1 |
 |         divergenceAngle :         71 |         45 |        140 |
 |               thickness :       0.01 |      0.002 |       0.06 |
 ------------------------------------------------------------------
 |       Integer Parameter :      Value |    Minimum |    Maximum |
 ------------------------------------------------------------------
 |         iterationNumber :          5 |          4 |          8 |
 ------------------------------------------------------------------
Produced Rose Rules = /(90)[A(0)]
)";

  CHECK( refLeafState == actualLeafState.str() );
  actualLeafState.str("");

  // Check simple initial state
  simpleLeaf = LeafFactory::instance()->getLeaf("simple");
  simpleLeaf->print( actualLeafState );

  refLeafState = R"( ------------------------------------------------------------------
 |        Double Parameter :      Value |    Minimum |    Maximum |
 ------------------------------------------------------------------
 |            initialAngle :         90 |          0 |        360 |
 |       mainInitialLength :       0.05 |       0.01 |       0.09 |
 |          mainGrowthRate :       0.75 |        0.5 |        1.1 |
 |    lateralInitialLength :       0.02 |      0.005 |       0.04 |
 |       lateralGrowthRate :        0.8 |        0.8 |        1.3 |
 | growthPotentialDecrease :          1 |          1 |          1 |
 |         divergenceAngle :         75 |         45 |        140 |
 |               thickness :       0.01 |      0.002 |       0.06 |
 ------------------------------------------------------------------
 |       Integer Parameter :      Value |    Minimum |    Maximum |
 ------------------------------------------------------------------
 |         iterationNumber :          4 |          4 |          8 |
 ------------------------------------------------------------------
Produced Simple Rules = /(90)[A(0)]
)";

  CHECK( refLeafState == actualLeafState.str() );
  actualLeafState.str("");
}
