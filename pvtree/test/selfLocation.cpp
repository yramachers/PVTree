#include "pvtree/test/catch.hpp"

#include "pvtree/utils/resource.hpp"
#include <fstream>

TEST_CASE("Basic self location","") {
  std::string expectedPath = pvtree::getConfigFile("config/location.cfg");
  std::ifstream shouldBeOpenable {expectedPath, std::ios::in};

  REQUIRE_FALSE( expectedPath.empty() );
  REQUIRE( shouldBeOpenable.is_open() );
}

TEST_CASE("Climate data","") {
  REQUIRE_THROWS( pvtree::getClimateDataFile("foo") );
}
