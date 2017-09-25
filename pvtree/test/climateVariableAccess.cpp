#include "pvtree/test/catch.hpp"
#include "pvtree/climate/climateFactory.hpp"
#include "pvtree/climate/climate.hpp"
#include "pvtree/utils/equality.hpp"
#include "pvtree/location/locationDetails.hpp"
#include <time.h>

#include <iostream>
#include <iomanip>

time_t getTestTime() {
  struct tm calendarTime;
  calendarTime.tm_sec = 0;
  calendarTime.tm_min = 0;
  calendarTime.tm_hour = 12;
  calendarTime.tm_mday = 12;
  calendarTime.tm_mon = 3;
  calendarTime.tm_year = 114;
  calendarTime.tm_isdst = 1;
  return mktime(&calendarTime);
}

TEST_CASE("climate/climateFactory", "[climate]") {
  /*! \todo Fix climate variable access so the same climate file can be
   *        opened twice. It seems to be an issue on ecCodes side, perhaps
   *        a future update will fix this. Currently uses climate factory
   *        configured elsewhere.
   */

  // Get the device location details
  LocationDetails deviceLocation("location.cfg");

  // Prepare the climate factory with the default configuration
  ClimateFactory::instance()->setDeviceLocation(deviceLocation);
  ClimateFactory::instance()->setConfigurationFile("default.cfg");

  // Get the default climate
  const Climate* climate = ClimateFactory::instance()->getClimate();

  // Check that the interpolation number is still the same
  REQUIRE(climate->getInterpolationPointNumber() == 5);

  // Get the time to attempt to retrieve values
  time_t testTime = getTestTime();
  CHECK((long)testTime == 1397300400);

  // Check that requesting a non-existant parameter breaks;
  bool threwException = false;
  try {
    climate->getInterpolatedValue("doesntexist", testTime);
  } catch (const std::string&) {
    threwException = true;
  }

  CHECK(threwException);

  threwException = false;
  try {
    climate->getInterpolatedValue(1000, testTime);
  } catch (const std::string&) {
    threwException = true;
  }

  CHECK(threwException);

  // Check the parameter units have not changed
  CHECK(climate->getParameterUnits("2 metre temperature") == "K");
  CHECK(climate->getParameterUnits("Total column water") == "kg m**-2");
  CHECK(climate->getParameterUnits("Surface pressure") == "Pa");
  CHECK(climate->getParameterUnits("Total cloud cover") == "(0 - 1)");
  CHECK(climate->getParameterUnits("Total column ozone") == "kg m**-2");

  // Check the interpolation values being retrieved have not changed
  // signficantly
  int checkPrecision = 10;
  CHECK(almost_equal(
      (float)climate->getInterpolatedValue("2 metre temperature", testTime),
      283.5870361328f, checkPrecision));
  CHECK(almost_equal(
      (float)climate->getInterpolatedValue("Total column water", testTime),
      16.9258f, /*checkPrecision*/ 100));
  CHECK(almost_equal(
      (float)climate->getInterpolatedValue("Surface pressure", testTime),
      100382.5f, checkPrecision));
  CHECK(almost_equal(
      (float)climate->getInterpolatedValue("Total cloud cover", testTime),
      0.951996f, checkPrecision));
  CHECK(almost_equal(
      (float)climate->getInterpolatedValue("Total column ozone", testTime),
      0.00662565f, checkPrecision));

  // Check the climate data directly
  std::vector<std::shared_ptr<const ClimateData>> directClimateData =
      climate->getData();

  REQUIRE(directClimateData.size() > 0u);

  // Check variable presence with different route
  CHECK(directClimateData[0]->hasValue("2 metre temperature"));
  CHECK(directClimateData[0]->hasValue("Total column water"));
  CHECK(directClimateData[0]->hasValue("Surface pressure"));
  CHECK(directClimateData[0]->hasValue("Total cloud cover"));
  CHECK(directClimateData[0]->hasValue("Total column ozone"));

  // Check it cant find missing variables
  CHECK(!directClimateData[0]->hasValue(-1));

  // Check more direct value access
  CHECK(
      almost_equal((float)directClimateData[0]->getValue("2 metre temperature"),
                   279.6376953125f, checkPrecision));

  // Check time ordering of climate data
  bool timeOrdered = true;

  if (directClimateData.size() > 1u) {
    for (unsigned int climateIndex = 1; climateIndex < directClimateData.size();
         climateIndex++) {
      if (directClimateData[climateIndex]->getTime() <
          directClimateData[climateIndex - 1]->getTime()) {
        timeOrdered = false;
        break;
      }
    }
  }
  CHECK(timeOrdered);
}
