#include "pvtree/test/catch.hpp"
#include "pvtree/treeSystem/treeFactory.hpp"
#include <stdexcept>
#include <cstdio>
#include <sstream>

#include "TFile.h"

TEST_CASE("treeSystem/treeFactory", "[tree]") {
  auto helicalTree = TreeFactory::instance()->getTree("helical");

  // Test that current parameters can be set and retrieved
  helicalTree->setParameter("initialLength", 4.6);
  REQUIRE(helicalTree->getDoubleParameter("initialLength") == 4.6);

  // Test that the random generator seeding produces consistant results
  helicalTree->setRandomParameterRange("initialLength", 4.0, 10.5);
  int seed = 1234;
  helicalTree->randomizeParameter(seed, "initialLength");
  double result1 = helicalTree->getDoubleParameter("initialLength");
  helicalTree->randomizeParameter(seed, "initialLength");
  double result2 = helicalTree->getDoubleParameter("initialLength");
  REQUIRE(result1 == result2);

  // Test that the random generator seeding produces consistant results when
  // randomizing all parameters
  seed = 4321;
  helicalTree->randomizeParameters(seed);
  result1 = helicalTree->getDoubleParameter("initialLength");
  helicalTree->randomizeParameters(seed);
  result2 = helicalTree->getDoubleParameter("initialLength");
  REQUIRE(result1 == result2);

  // Test that a non-existant parameter can not be retrieved
  bool nonExistantParameterRetrieved = true;
  try {
    helicalTree->getDoubleParameter("noParameterWithName");
  } catch (const std::invalid_argument& err) {
    nonExistantParameterRetrieved = false;
  }
  REQUIRE(nonExistantParameterRetrieved == false);

  // Test that a non-existant parameter (for a given type) can not be retrieved.
  nonExistantParameterRetrieved = true;
  try {
    helicalTree->getIntegerParameter("initialLength");
  } catch (const std::invalid_argument& err) {
    nonExistantParameterRetrieved = false;
  }
  REQUIRE(nonExistantParameterRetrieved == false);

  // Test that a non-existant parameter can not be randomized
  bool nonExistantParameterRandomized = true;
  try {
    helicalTree->randomizeParameter(seed, "noParameterWithName");
  } catch (const std::invalid_argument& err) {
    nonExistantParameterRandomized = false;
  }
  REQUIRE(nonExistantParameterRandomized == false);

  // Test that you can have a double and integer parameter with the same name
  // (maybe shouldn't)
  helicalTree->setParameter("branchingAngle", 7.5);
  helicalTree->setParameter("branchingAngle", 4);

  bool bothParametersRetrievedCorrectly =
      helicalTree->getDoubleParameter("branchingAngle") == 7.5 &&
      helicalTree->getIntegerParameter("branchingAngle") == 4;
  REQUIRE(bothParametersRetrievedCorrectly == true);

  // Test that the parameter ranges can be set and retrieved from file
  // successfully.
  std::string persistFileName = "/tmp/unit-treeConstuction-temp.root";

  TFile exportFile(persistFileName.c_str(), "RECREATE");
  helicalTree->Write("testTree");
  exportFile.Close();

  TFile importFile(persistFileName.c_str(), "READ");

  TreeConstructionInterface* importTree =
      static_cast<TreeConstructionInterface*>(
          importFile.FindObjectAny("testTree"));

  REQUIRE(*importTree == *helicalTree);

  // If I randomize parameters it should no longer be equal
  seed++;
  helicalTree->randomizeParameters(seed);

  REQUIRE(*importTree != *helicalTree);

  // Randomizing the loaded tree should return the equality
  importTree->randomizeParameters(seed);
  REQUIRE(*importTree == *helicalTree);

  // It should also not match a completely different tree
  auto sympodialTree = TreeFactory::instance()->getTree("sympodial");

  REQUIRE(*importTree != *sympodialTree);

  importFile.Close();

  // Test that the temporary file can be deleted.
  int removeFileFlag = std::remove(persistFileName.c_str());
  REQUIRE(removeFileFlag == 0);

  // Check that the L-Systems evolve in the same manner
  std::stringstream actualTreeState(std::string(""));

  // Check helical initial state
  helicalTree = TreeFactory::instance()->getTree("helical");
  helicalTree->print(actualTreeState);

  std::string refTreeState =
      R"( -------------------------------------------------------------
 |   Double Parameter :      Value |    Minimum |    Maximum |
 -------------------------------------------------------------
 |      initialLength :       0.42 |        0.3 |        0.7 |
 |   initialWidthEven :        0.2 |       0.15 |       0.25 |
 |    initialWidthOdd :       0.23 |       0.15 |       0.25 |
 |      initialRadius :          0 |          0 |        0.6 |
 | initialOrientation :          0 |          0 |        360 |
 |     elongationRate :       0.93 |       0.85 |       0.93 |
 |   branchElongation :        0.9 |        0.9 |       0.95 |
 |    contractionRate :       0.85 |        0.7 |       0.85 |
 |       minimumWidth :      0.015 |      0.015 |      0.035 |
 |       turningAngle :       15.2 |       12.2 |       18.2 |
 |   inclinationAngle :         90 |         70 |         90 |
 |         incDecRate :          5 |          4 |          6 |
 |     branchingAngle :        2.8 |        1.4 |        3.8 |
 -------------------------------------------------------------
 |  Integer Parameter :      Value |    Minimum |    Maximum |
 -------------------------------------------------------------
 |  stepsBetweenSplit :          4 |          4 |          4 |
 |        stalkPoints :          8 |          2 |          8 |
 |   branchlessPoints :          0 |          0 |          0 |
 |    iterationNumber :         17 |         15 |         18 |
 |       simpleBranch :          0 |          0 |          0 |
 -------------------------------------------------------------
Produced Helical Rules = /(0)[/(0)&(90)f(0)+(90)&(-90)A(0.42,0.2,0,0)][/(45)&(90)f(0)+(90)&(-90)A(0.42,0.23,0,0)][/(90)&(90)f(0)+(90)&(-90)A(0.42,0.2,0,0)][/(135)&(90)f(0)+(90)&(-90)A(0.42,0.23,0,0)][/(180)&(90)f(0)+(90)&(-90)A(0.42,0.2,0,0)][/(225)&(90)f(0)+(90)&(-90)A(0.42,0.23,0,0)][/(270)&(90)f(0)+(90)&(-90)A(0.42,0.2,0,0)][/(315)&(90)f(0)+(90)&(-90)A(0.42,0.23,0,0)]
)";

  CHECK(refTreeState == actualTreeState.str());
  actualTreeState.str("");

  // Check monopodial initial state
  auto monopodialTree = TreeFactory::instance()->getTree("monopodial");
  monopodialTree->print(actualTreeState);

  refTreeState =
      R"( -------------------------------------------------------------
 |   Double Parameter :      Value |    Minimum |    Maximum |
 -------------------------------------------------------------
 |      initialHeight :          1 |          0 |          2 |
 |       initialWidth :        0.2 |        0.1 |        0.4 |
 | initialOrientation :         67 |          0 |        360 |
 |  contractionRatio1 :        0.9 |       0.55 |        0.9 |
 |  contractionRatio2 :        0.7 |        0.3 |        0.8 |
 |    branchingAngle1 :         45 |         10 |        110 |
 |    branchingAngle2 :         60 |         10 |        120 |
 |    divergenceAngle :      137.5 |          0 |        180 |
 |  widthDecreaseRate :      0.707 |        0.7 |        0.8 |
 -------------------------------------------------------------
 |  Integer Parameter :      Value |    Minimum |    Maximum |
 -------------------------------------------------------------
 |    iterationNumber :          6 |          4 |          8 |
 -------------------------------------------------------------
Produced Monopodial Rules = /(67)A(1,0.2)
)";

  CHECK(refTreeState == actualTreeState.str());
  actualTreeState.str("");

  // Check stochastic initial state
  auto stochasticTree = TreeFactory::instance()->getTree("stochastic");
  stochasticTree->print(actualTreeState);

  refTreeState =
      R"( --------------------------------------------------------------
 |    Double Parameter :      Value |    Minimum |    Maximum |
 --------------------------------------------------------------
 |       initialHeight :       0.37 |        0.3 |       0.57 |
 |        initialWidth :       0.05 |       0.04 |       0.07 |
 |  initialOrientation :        314 |          0 |        360 |
 |      elongationRate :       1.15 |        1.1 |       1.25 |
 |   widthIncreaseRate :        1.6 |        1.4 |        1.8 |
 |      branchingAngle :         21 |         11 |         31 |
 |     branchingAngle2 :         31 |         11 |         41 |
 |    divergenceAngle1 :         99 |         70 |        120 |
 |    divergenceAngle2 :        167 |        147 |        177 |
 |     angleToVertical :         12 |         10 |         15 |
 |         lengthScale :       0.18 |       0.15 |       0.24 |
 |        lengthScale2 :      0.098 |       0.09 |       0.12 |
 | branchProbReduction :       0.67 |       0.57 |       0.77 |
 |   initialBranchProb :        0.9 |        0.8 |       0.95 |
 --------------------------------------------------------------
 |   Integer Parameter :      Value |    Minimum |    Maximum |
 --------------------------------------------------------------
 | leafIterationNumber :          2 |          2 |          4 |
 | totalLeafIterations :          3 |          2 |          4 |
 |     iterationNumber :          7 |          5 |          8 |
 |                seed :       1234 |       1234 |       1234 |
 --------------------------------------------------------------
Produced Stochastic Rules = RandomSeed(1234)!(0.05,1.6)F(0.37)/(314)A
)";

  CHECK(refTreeState == actualTreeState.str());
  actualTreeState.str("");

  // Check the stump initial state
  auto stumpTree = TreeFactory::instance()->getTree("stump");
  stumpTree->print(actualTreeState);

  refTreeState =
      R"( ------------------------------------------------------------
 |  Double Parameter :      Value |    Minimum |    Maximum |
 ------------------------------------------------------------
 |     initialHeight :        0.5 |        0.1 |          1 |
 |      initialWidth :       0.05 |       0.01 |        0.1 |
 |    initialAzimuth :        180 |          0 |        360 |
 |  initialElevation :         45 |          0 |         90 |
 ------------------------------------------------------------
 | Integer Parameter :      Value |    Minimum |    Maximum |
 ------------------------------------------------------------
 |   iterationNumber :          0 |          0 |          0 |
 ------------------------------------------------------------
Produced Stump Rules = !(0.05)F(0.5)/(180)&(45)F(0.5)
)";

  CHECK(refTreeState == actualTreeState.str());
  actualTreeState.str("");

  // Check the sympodial initial state
  sympodialTree = TreeFactory::instance()->getTree("sympodial");
  sympodialTree->print(actualTreeState);

  refTreeState =
      R"( -------------------------------------------------------------
 |   Double Parameter :      Value |    Minimum |    Maximum |
 -------------------------------------------------------------
 |      initialHeight :          1 |      0.001 |          2 |
 |       initialWidth :        0.2 |       0.01 |        0.4 |
 | initialOrientation :         67 |          0 |        360 |
 |  contractionRatio1 :       0.75 |        0.1 |        0.9 |
 |  contractionRatio2 :       0.68 |        0.1 |        0.8 |
 |    branchingAngle1 :         28 |          5 |         90 |
 |    branchingAngle2 :         48 |          5 |         90 |
 |  widthDecreaseRate :       0.67 |        0.1 |        0.9 |
 -------------------------------------------------------------
 |  Integer Parameter :      Value |    Minimum |    Maximum |
 -------------------------------------------------------------
 |    iterationNumber :          6 |          3 |          8 |
 -------------------------------------------------------------
Produced Sympodial Rules = /(67)A(1,0.2)
)";

  CHECK(refTreeState == actualTreeState.str());
  actualTreeState.str("");

  // Check the ternary initial state
  auto ternaryTree = TreeFactory::instance()->getTree("ternary");
  ternaryTree->print(actualTreeState);

  refTreeState =
      R"( -------------------------------------------------------------
 |   Double Parameter :      Value |    Minimum |    Maximum |
 -------------------------------------------------------------
 |      initialHeight :       0.07 |       0.04 |        0.1 |
 |       initialWidth :       0.02 |       0.01 |       0.04 |
 | initialOrientation :         67 |          0 |        360 |
 |     elongationRate :        1.2 |          1 |        1.4 |
 |  widthIncreaseRate :        1.9 |        1.2 |        1.9 |
 |     branchingAngle :      18.95 |          5 |         30 |
 |   divergenceAngle1 :      94.74 |         70 |        140 |
 |   divergenceAngle2 :     132.63 |         70 |        140 |
 |        lengthScale :        0.2 |        0.1 |        0.3 |
 -------------------------------------------------------------
 |  Integer Parameter :      Value |    Minimum |    Maximum |
 -------------------------------------------------------------
 |    iterationNumber :          7 |          4 |          7 |
 -------------------------------------------------------------
Produced Ternary Rules = !(0.02)F(0.07)/(67)A
)";

  CHECK(refTreeState == actualTreeState.str());
  actualTreeState.str("");
}
