#include "pvtree/location/locationDetails.hpp"
#include "pvtree/utils/resource.hpp"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <libconfig.h++>

/*! @file
 * \brief Location details for simulation.
 *
 * Class used to store details of location of the
 * device to be simulated.
 *
 */

LocationDetails::LocationDetails(std::string inputFilePath) {
  // Need to start by finding the file
  // First try to find it w.r.t the local directory
  std::ifstream localTest(inputFilePath.c_str());

  if (localTest.is_open()) {
    localTest.close();

    // If found use the local file
    extractFile(inputFilePath);
    return;
  }

  // Not a local file so look in the installed share directory
  std::string shareFilePath = pvtree::getConfigFile("config/" + inputFilePath);
  std::ifstream shareTest(shareFilePath.c_str());

  if (shareTest.is_open()) {
    shareTest.close();

    extractFile(shareFilePath);
    return;
  }

  // If reaching here then unable to extract a file's contents!
  std::cerr << "LocationDetails::LocationDetails - Unable to find the "
               "specified input file " << inputFilePath << std::endl;
  throw std::invalid_argument("Can't find location configuration file.");
}

LocationDetails::LocationDetails(double longitude, double latitude,
                                 double altitude, int tzone) {
  m_longitude = longitude;
  m_latitude = latitude;
  m_altitude = altitude;
  m_timezone = tzone;
}

LocationDetails::LocationDetails(const LocationDetails& original) {
  m_longitude = original.m_longitude;
  m_latitude = original.m_latitude;
  m_altitude = original.m_altitude;
  m_timezone = original.m_timezone;
}

LocationDetails::~LocationDetails() {}

double LocationDetails::getLongitude() const { return m_longitude; }

double LocationDetails::getLatitude() const { return m_latitude; }

double LocationDetails::getAltitude() const { return m_altitude; }

int    LocationDetails::getTimeZone() const { return m_timezone; }

void LocationDetails::extractFile(std::string configFilePath) {
  libconfig::Config* cfg = new libconfig::Config;

  try {
    cfg->readFile(configFilePath.c_str());

  } catch (const libconfig::FileIOException& fioex) {
    std::cerr << "I/O error while reading file." << std::endl;
    throw;
  } catch (const libconfig::ParseException& pex) {
    std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
              << " - " << pex.getError() << std::endl;
    throw;
  }

  // Make sure everything is filled
  int readCount = 0;

  if (cfg->exists("longitude")) {
    cfg->lookupValue("longitude", m_longitude);
    readCount++;
  }

  if (cfg->exists("latitude")) {
    cfg->lookupValue("latitude", m_latitude);
    readCount++;
  }

  if (cfg->exists("altitude")) {
    cfg->lookupValue("altitude", m_altitude);
    readCount++;
  }

  if (cfg->exists("timezone")) {
    cfg->lookupValue("timezone", m_timezone);
    readCount++;
  }

  if (readCount != 4) {
    throw std::string(
        "Failed to read correct amount of information for LocationDetails.");
  }
}
