#include "testing/catch.hpp"
#include "utils/equality.hpp"
#include "location/locationDetails.hpp"
#include <time.h>

TEST_CASE( "location/locationDetails", "[location]" ) {

  // Grab the default location
  LocationDetails deviceLocation("location.cfg");

  // Check that the location could be read in correctly
  double longitude = deviceLocation.getLongitude();
  double latitude  = deviceLocation.getLatitude();
  double altitude  = deviceLocation.getAltitude();

  int checkPrecision = 10;
  CHECK( almost_equal( longitude, -1.563645, checkPrecision) );
  CHECK( almost_equal( latitude,  52.383109, checkPrecision) );
  CHECK( almost_equal( altitude,   0.088,    checkPrecision) );

  // Try cloning the location
  LocationDetails copiedLocation(deviceLocation);

  // Are the settings identical?
  CHECK( almost_equal( deviceLocation.getLongitude(), copiedLocation.getLongitude(), checkPrecision) );
  CHECK( almost_equal( deviceLocation.getLatitude(),  copiedLocation.getLatitude(),  checkPrecision) );
  CHECK( almost_equal( deviceLocation.getAltitude(),  copiedLocation.getAltitude(),  checkPrecision) );

  // Try constructor which uses longitude, latitude and altitude
  LocationDetails specificLocation(deviceLocation.getLongitude(), deviceLocation.getLatitude(), deviceLocation.getAltitude() );

  // Are the settings identical?
  CHECK( almost_equal( deviceLocation.getLongitude(), specificLocation.getLongitude(), checkPrecision) );
  CHECK( almost_equal( deviceLocation.getLatitude(),  specificLocation.getLatitude(),  checkPrecision) );
  CHECK( almost_equal( deviceLocation.getAltitude(),  specificLocation.getAltitude(),  checkPrecision) );

}

