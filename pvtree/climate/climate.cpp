#include "pvtree/climate/climate.hpp"

#include <libconfig.h++>

#include <fstream>
#include <iomanip>
#include <algorithm>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <list>

// Construct a map of parameter names and IDs
// to be shared around. This might only apply to
// this specific GRIB file
Climate::Climate(std::string configurationFileName,
		 LocationDetails deviceLocation) : m_nameToParameterID( std::make_shared<std::map<std::string,int> >() ),
						   m_deviceLocation(deviceLocation) {
  // Use the configuration file
  if (!openConfiguration(configurationFileName)) throw;

  // Now extract relevant information from the GRIB file.
  if (!findGRIB())  throw;
  if (!parseGRIB()) throw;

}

Climate::~Climate() {}

bool Climate::fileExists(std::string filePath) {

  std::ifstream localTest(filePath.c_str());

  if (localTest.is_open()) {
    return true;
  }

  return false;
}

bool Climate::openConfiguration(std::string configPath){

  libconfig::Config* cfg = new libconfig::Config;

  if ( fileExists(configPath) ) {
    bool isFileOpen = parseConfiguration(configPath, cfg);

    if (!isFileOpen){
      delete cfg;
      return false;
    }

  } else {

    // Not a local file so look in the installed share directory
    const char* environmentVariableContents = std::getenv("PVTREE_SHARE_PATH");

    if (environmentVariableContents != 0 ){
      std::string shareFilePath(std::string(environmentVariableContents) + "/config/climate/" + configPath);

      if ( fileExists(shareFilePath) ) {
	bool isFileOpen = parseConfiguration(shareFilePath, cfg);

	if (!isFileOpen) {
	  delete cfg;
	  return false;
	}
      } else {

	// Not in either place so give up!
	std::cerr << "Unable to locate file " << configPath << " locally or in the shared config."
		  << std::endl;
	delete cfg;
	return false;
      }
    } else {

      // Not in either place so give up!
      std::cerr << "Unable to locate file " << configPath << " locally or in the shared config."
		<< std::endl;
      delete cfg;
      return false;
    }

  }

  delete cfg;
  return true;
}

bool Climate::parseConfiguration(std::string fileName, libconfig::Config* cfg) {

  // Read the file. If there is an error, report it and exit.
  try{
    cfg->readFile(fileName.c_str());
  } catch(const libconfig::FileIOException &fioex){
    std::cerr << "I/O error while reading file." << std::endl;
    return false;
  } catch(const libconfig::ParseException &pex) {
    std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
	      << " - " << pex.getError() << std::endl;
    return false;
  }

  // Extract from the configuration file the location of the GRIB file described
  if ( !cfg->exists("grib.fileName") ) {
    std::cerr << "Missing grib file name in " << fileName << std::endl;
    return false;
  }
  std::string test = cfg->lookup("grib.fileName");
  m_gribFileName = test;

  if ( cfg->exists("grib.parameters") ){
    libconfig::Setting& parameterList = cfg->lookup("grib.parameters");

    // Iterate over each item and check for extents.f
    for (int s=0; s<parameterList.getLength(); s++) {
      int parameterIndex;
      parameterList[s].lookupValue("index", parameterIndex);

      if (parameterList[s].exists("minimumValue")) {
	double minimumValue;
	parameterList[s].lookupValue("minimumValue", minimumValue);
	m_parameterIDMinValueAllowed[parameterIndex] = minimumValue;
      }

      if (parameterList[s].exists("maximumValue")) {
	double maximumValue;
	parameterList[s].lookupValue("maximumValue", maximumValue);
	m_parameterIDMaxValueAllowed[parameterIndex] = maximumValue;
      }
    }
  }

  return true;
}

bool Climate::findGRIB() {

  if ( !fileExists(m_gribFileName) ) {

    // Try under the data directory instead
    const char* environmentVariableContents = std::getenv("PVTREE_CLIMATE_DATA_PATH");

    if (environmentVariableContents != 0 ){
      std::string dataFilePath(std::string(environmentVariableContents) + "/" + m_gribFileName);

      if ( fileExists(dataFilePath) ) {
	// Alter the stored grib file name
	m_gribFileName = dataFilePath;
      } else {
	// Can't find the file
	std::cerr << "Unable to locate the grib file " << m_gribFileName << std::endl;
	return false;
      }
    } else {
      // Can't find the file
      std::cerr << "Unable to locate the grib file " << m_gribFileName << " and no climate data path specified."
		<< std::endl;
      return false;
    }

  }

  return true;
}

time_t Climate::getTimeFromMessage(codes_handle* handle) {

  // First extract from the message
  long month;
  long day;
  long hour;
  long minute;
  long second;
  long year;
  CODES_CHECK(codes_get_long(handle, "month",    &month), 0);
  CODES_CHECK(codes_get_long(handle, "day",      &day),   0);
  CODES_CHECK(codes_get_long(handle, "hour",     &hour),  0);
  CODES_CHECK(codes_get_long(handle, "minute",   &minute),0);
  CODES_CHECK(codes_get_long(handle, "second",   &second),0);
  CODES_CHECK(codes_get_long(handle, "year",     &year),  0);

  // Construct a time object
  struct tm calendarTime;

  // Need to be careful about the different conventions here :)
  calendarTime.tm_sec=second;
  calendarTime.tm_min=minute;
  calendarTime.tm_hour=hour;
  calendarTime.tm_mday=day;
  calendarTime.tm_mon=month - 1;
  calendarTime.tm_year=year - 1900;

  //! \todo need to actually get the correct time for the data.
  calendarTime.tm_isdst=1;

  // Convert to epoch time
  return mktime(&calendarTime);
}

bool Climate::parseGRIB() {

  // Just handling single file case for now
  int numberOfFiles = 1;
  char** fileNames = (char**)malloc(sizeof(char*)*numberOfFiles);
  fileNames[0] =     (char*)strdup(m_gribFileName.c_str());

  // Order by the date/time
  int errorValue = 0;
  char* orderBy = (char*)"dataDate,dataTime";
  codes_fieldset* set = codes_fieldset_new_from_files(0, fileNames, numberOfFiles, 0, 0, 0, orderBy, &errorValue);

  CODES_CHECK(errorValue, 0);

  // Save to a vector
  m_climateData.clear();

  // Use a map to prevent duplication
  std::map<time_t, std::shared_ptr<ClimateData>> climateDataMap;

  // For distance check assume all grids are the same! (cache for speed)
  int mode = CODES_NEAREST_SAME_GRID | CODES_NEAREST_SAME_POINT;

  codes_nearest* nearest = NULL;
  codes_handle* handle = NULL;

  // Iterate over all messages in file(s)
  while (( handle = codes_fieldset_next_handle(set, &errorValue)) != NULL){

    CODES_CHECK(errorValue, 0);

    // Get the time and convert to time_t
    time_t currentTime = getTimeFromMessage(handle);

    // Get previously made climate data (or make a new one if necessary)
    std::shared_ptr<ClimateData> currentData;

    if ( climateDataMap.find(currentTime) != climateDataMap.end() ){
      currentData = climateDataMap[currentTime];
    } else {
      currentData = std::make_shared<ClimateData>(m_nameToParameterID, currentTime);
      climateDataMap[currentTime] = currentData;
      m_climateData.push_back(currentData);
    }

    // Find the closest grid point
    if (!nearest) {
      nearest = codes_grib_nearest_new(handle, &errorValue);
      CODES_CHECK(errorValue, 0);
    }

    const size_t closestGridPointNumber = 4;
    double closestLatitudes[closestGridPointNumber]  = {0.0,};
    double closestLongitudes[closestGridPointNumber] = {0.0,};
    double closestValues[closestGridPointNumber]     = {0.0,};
    double closestDistances[closestGridPointNumber]  = {0.0,};
    int    closestIndicies[closestGridPointNumber]   = {0,};
    size_t passedSize = closestGridPointNumber;

    errorValue = codes_grib_nearest_find(nearest,
					 handle,
					 m_deviceLocation.getLatitude(),
					 m_deviceLocation.getLongitude(),
					 mode,
					 closestLatitudes,
					 closestLongitudes,
					 closestValues,
					 closestDistances,
					 closestIndicies,
					 &passedSize);

    CODES_CHECK(errorValue, 0);

    // Use closest grid point
    int closestIndex = 0;
    double closestDistance = closestDistances[0];
    for (unsigned int d=0; d<closestGridPointNumber; d++) {
      if (closestDistances[d] < closestDistance) {
	closestIndex = d;
	closestDistance = closestDistances[d];
      }
    }

    // Watch out for 'large' and negative distances (units are in km)
    double maximumAllowedDistance = 500.0;//km
    if ( closestDistance > maximumAllowedDistance ) {
      std::cerr << "Warning closest grid point for climate variable access is " << closestDistance
		<< "km away." << std::endl;
    }
    if ( closestDistance < 0.0 ) {
      std::cerr << "Error closest grid point for climate variable access is " << closestDistance
		<< "km away, it should not be negative!" << std::endl;
      throw;
    }

    // Check what parameter the message refers to
    long parameterIdentification;
    CODES_CHECK(codes_get_long(handle, "paramId", &parameterIdentification), 0);

    // Check if we need to get and store the variable name
    if ( m_parameterIDToName.find( (int)parameterIdentification ) == m_parameterIDToName.end() ){

      // Get the variable name and units
      size_t maximumLength = 255;
      char variableName[255];
      CODES_CHECK(grib_get_string (handle, "name",  variableName,  &maximumLength), 0);

      maximumLength = 255;
      char variableUnits[255];
      CODES_CHECK(grib_get_string (handle, "units", variableUnits, &maximumLength), 0);

      std::string variableNameString(variableName);
      std::string variableUnitsString(variableUnits);

      m_parameterIDToName[parameterIdentification] = variableNameString;
      m_parameterIDToUnits[parameterIdentification] = variableUnitsString;
      (*m_nameToParameterID)[variableNameString] = parameterIdentification;
    }

    // Store the closest value
    double currentValue = closestValues[closestIndex];

    currentData->setValue(parameterIdentification, currentValue);

    codes_handle_delete(handle);
  }
  CODES_CHECK(errorValue, 0);

  if (nearest) {
    errorValue = codes_grib_nearest_delete(nearest);
    CODES_CHECK(errorValue, 0);
  }

  if (set) codes_fieldset_delete(set);

  return true;
}

double Climate::getInterpolatedValue(std::string valueName, time_t time, ROOT::Math::Interpolation::Type interpolationType /*= ROOT::Math::Interpolation::kCSPLINE*/) const{

  if ( m_nameToParameterID->find(valueName) != m_nameToParameterID->end() ) {
    int valueID = m_nameToParameterID->at(valueName);

    // Actually use other function
    return getInterpolatedValue(valueID, time, interpolationType);
  }

  throw std::string("Unable to find value with name " + valueName);
}

double Climate::getInterpolatedValue(int valueID, time_t time, ROOT::Math::Interpolation::Type interpolationType /*= ROOT::Math::Interpolation::kCSPLINE*/) const{

  // First check the value ID actually exists
  // Only checks if there was at least one GRIB message with this parameter
  if ( m_parameterIDToName.find(valueID) == m_parameterIDToName.end() ) {
    throw std::string("Unable to find value with ID " + std::to_string(valueID));
  }

  // Search for ClimateData instance in the vector which is the
  // first ClimateData to have a time greater than passed 'time'
  auto nextClimateData = std::lower_bound(begin(m_climateData), end(m_climateData), time,
					  [] (const std::shared_ptr<ClimateData> &c, const double &t) {return c->getTime() < t;});
  auto previousClimateData = nextClimateData;

  // Fill up some vectors with nearby points
  std::list<double> xValues; // time
  std::list<double> yValues; // parameter value

  // Go forwards by m_interpolationPointNumber times
  // where missing values are skipped!
  int nextFoundValues = 0;
  while (nextFoundValues < m_interpolationPointNumber) {

    // No more values at later times
    if ( nextClimateData == m_climateData.end() ) {
      break;
    }

    // Check if value present, if so use it for interpolation
    if ( (*nextClimateData)->hasValue(valueID) ) {
      xValues.push_back( (*nextClimateData)->getTime() );
      yValues.push_back( (*nextClimateData)->getValue(valueID) );
      nextFoundValues++;
    }

    nextClimateData++;
  }

  // Go backwards by m_interpolationPointNumber times
  // where missing values are skipped!
  int previousFoundValues = 0;
  while (previousFoundValues < m_interpolationPointNumber) {

    // Check if at the begining of data
    // if so can't go further back!
    if ( previousClimateData == m_climateData.begin() ) {
      break;
    }

    previousClimateData--;

    // Check if value present, if so use it for interpolation
    if ( (*previousClimateData)->hasValue(valueID) ) {
      xValues.push_front( (*previousClimateData)->getTime() );
      yValues.push_front( (*previousClimateData)->getValue(valueID) );
      previousFoundValues++;
    }
  }

  if (nextFoundValues == 0) {
    // Currently just report a problem
    std::cerr << "WARNING: Interpolation not valid at this time point, using last available data point." << std::endl;

    if (previousFoundValues > 0){
      // Then return the last recorded value
      return yValues.back();
    } else {
      // Actually can't find any value...
      throw std::string( "Found no applicable values.");
    }
  }

  if (previousFoundValues == 0){
    // Currently just report a problem
    std::cerr << "WARNING: Interpolation not valid at this time point, using first available data point." << std::endl;

    if (nextFoundValues > 0){
      // Then return the first recorded value
      return yValues.front();
    } else {
      // Actually can't find any value...
      throw std::string("Found no applicable values.");
    }
  }

  // Copy into vectors
  std::vector<double> xValueVector;
  std::vector<double> yValueVector;

  std::copy(begin(xValues), end(xValues), std::back_inserter(xValueVector));
  std::copy(begin(yValues), end(yValues), std::back_inserter(yValueVector));

  // Build an interpolator
  ROOT::Math::Interpolator interpolator(xValueVector, yValueVector, interpolationType);

  double candidateValue = interpolator.Eval( (double)time );

  // Check the value is within any special requirements
  if ( m_parameterIDMaxValueAllowed.find(valueID) != m_parameterIDMaxValueAllowed.end() ){
    if (candidateValue > m_parameterIDMaxValueAllowed.at(valueID)) {
      candidateValue = m_parameterIDMaxValueAllowed.at(valueID);
    }
  }

  if ( m_parameterIDMinValueAllowed.find(valueID) != m_parameterIDMinValueAllowed.end() ){
    if (candidateValue < m_parameterIDMinValueAllowed.at(valueID)) {
      candidateValue = m_parameterIDMinValueAllowed.at(valueID);
    }
  }

  return candidateValue;
}


int Climate::getInterpolationPointNumber() const {
  return m_interpolationPointNumber;
}


void Climate::setInterpolationPointNumber(int interpolationPointNumber) {
  m_interpolationPointNumber = interpolationPointNumber;
}


std::string Climate::getParameterUnits(std::string parameterName) const {

  if ( m_nameToParameterID->find(parameterName) != m_nameToParameterID->end() ) {
    int valueID = m_nameToParameterID->at(parameterName);

    // Actually use other function
    return getParameterUnits(valueID);
  }

  throw std::string("Unable to find parameter with name " + parameterName);
}


std::string Climate::getParameterUnits(int parameterID) const {

  // First check the value ID actually exists
  // Only checks if there was at least one GRIB message with this parameter
  if ( m_parameterIDToUnits.find(parameterID) == m_parameterIDToUnits.end() ) {
    throw std::string("Unable to find value with ID " + std::to_string(parameterID));
  }

  return m_parameterIDToUnits.at(parameterID);
}


std::vector<std::shared_ptr<const ClimateData>> Climate::getData() const {

  std::vector<std::shared_ptr<const ClimateData>> rawData;

  // Copy all the climate data into a const form
  for (auto& data : m_climateData) {
    rawData.push_back(data);
  }

  return rawData;
}
