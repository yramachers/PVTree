/*!
 * @file 
 * \brief Application to fill a lightfield over an extended
 *        period of time with diffuse irradiance across the
 *        sky.
 *
 * This will probably not function with SMARTS. Need to replace
 * with another atmospheric radiative transport model to calculate
 * the Radiance( azimuth, elevation ). See for example LBLRTM. It 
 * also probably doesn't matter too much for clear sky conditions.
 */

#include "solarSimulation/sun.hpp"
#include "solarSimulation/plenoptic3D.hpp"
#include "utils/getopt_pp.h"
#include "climate/climateFactory.hpp"
#include "location/locationDetails.hpp"
#include "solarSimulation/spectrumFactory.hpp"

#include <iostream>
#include <vector>
#include <memory>
#include <chrono>
#include <sstream>
#include <random>
#include <regex>

#include "CLHEP/Units/PhysicalConstants.h"

// save diagnostic state
#pragma GCC diagnostic push 

// turn off the specific warning.
#pragma GCC diagnostic ignored "-Wshadow"

#include "TFile.h"
#include "TH2D.h"

// turn the warnings back on
#pragma GCC diagnostic pop

void showHelp() {
  std::cout << "yearlyDiffuseLightfieldScan help" << std::endl;
  std::cout << "\t --timeSegments <INTEGER> :\t default 50" << std::endl;
  std::cout << "\t --startDate <INTEGER> :\t default 1/1/2014" << std::endl;
  std::cout << "\t --endDate <INTEGER> :\t default 1/1/2015" << std::endl;
  std::cout << "\t --yearSegments <INTEGER> :\t default 10" << std::endl;
  std::cout << "\t --startSegmentIndex <INTEGER> :\t default 0" << std::endl;
  std::cout << "\t --endSegmentIndex <INTEGER> :\t default last index" << std::endl;
  std::cout << "\t --azimuthBinNumber <INTEGER> :\t default 20u" << std::endl;
  std::cout << "\t --elevationBinNumber <INTEGER> :\t default 20u" << std::endl;
  std::cout << "\t --energyBinNumber <INTEGER> :\t default 100u" << std::endl;
  std::cout << "\t --outputFileName <ROOT FILENAME> : \t default 'lightfield.root'" << std::endl;
}

bool isSameDay(time_t time1, time_t time2) {

  // Convert to calendar time
  struct tm* calendarTime = gmtime(&time1);

  int monthDay1 = calendarTime->tm_mday;
  int month1 = calendarTime->tm_mon;
  int year1 = calendarTime->tm_year;

  calendarTime = gmtime(&time2);

  int monthDay2 = calendarTime->tm_mday;
  int month2 = calendarTime->tm_mon;
  int year2 = calendarTime->tm_year;

  if (monthDay1 == monthDay2 && month1 == month2 && year1 == year2){
    return true;
  }
  return false;
}

/*! \brief Convert date in format DD/MM/YYYY into the time
 *         since epoch.
 *
 * @param[in] inputDate String representing date in format DD/MM/YYYY
 * \returns The time since epoch.
 */
time_t interpretDate(std::string inputDate) {

  // Use regular expressions to extract important quantities
  std::regex regularExpression("(\\d+)/(\\d+)/(\\d+)");

  std::smatch dateMatches;
  std::regex_match(inputDate, dateMatches, regularExpression);

  // Is the match the correct size?
  if ( dateMatches.size() != 4 ){
    throw std::string("Cannot interpret date: " + inputDate);
  }

  struct tm calendarTime;

  calendarTime.tm_sec = 0;
  calendarTime.tm_min = 0; 
  calendarTime.tm_hour = 12; 
  calendarTime.tm_mday = std::stoi(dateMatches[1]); 
  calendarTime.tm_mon =  std::stoi(dateMatches[2])-1; 
  calendarTime.tm_year = std::stoi(dateMatches[3])-1900;
  calendarTime.tm_isdst = 1;

  return mktime(&calendarTime);  
}

/*! \brief Diffuse lightfield creation main.
 *
 * Provides a mechanism to build a diffuse lightfield for a specific period of time.
 *
 * @param[in] argc Number of command line arguments.
 * @param[in] argv Accepts a number of command line arguments. See help for more 
 *                 details
 * 
 */
int main(int argc, char** argv) {

  unsigned int simulationTimeSegments;
  std::string startDate;
  std::string endDate;
  unsigned int yearSegments;
  unsigned int startSegmentIndex;
  unsigned int endSegmentIndex;
  unsigned int azimuthBinNumber;
  unsigned int elevationBinNumber;
  unsigned int energyBinNumber;
  std::string outputFileName;
  

  GetOpt::GetOpt_pp ops(argc, argv);

  // Check for help request
  if (ops >> GetOpt::OptionPresent('h', "help")){
    showHelp();
    return 0;
  }

  ops >> GetOpt::Option("timeSegments",       simulationTimeSegments,    50u);
  ops >> GetOpt::Option("startDate",          startDate,          "1/1/2014");
  ops >> GetOpt::Option("endDate",            endDate,            "1/1/2015");
  ops >> GetOpt::Option("yearSegments",       yearSegments,              10u);
  ops >> GetOpt::Option("startSegmentIndex",  startSegmentIndex,          0u);
  ops >> GetOpt::Option("endSegmentIndex",    endSegmentIndex,  yearSegments);
  ops >> GetOpt::Option("azimuthBinNumber",   azimuthBinNumber,          20u);
  ops >> GetOpt::Option("elevationBinNumber", elevationBinNumber,        20u);
  ops >> GetOpt::Option("energyBinNumber",    energyBinNumber,          100u);
  ops >> GetOpt::Option("outputFileName",     outputFileName, "diffuse.lightfield.root");

  if (yearSegments == 0) {
    std::cerr << "Need at least one year time segment." << std::endl;
    return -1;
  }

  std::cerr << "THIS WILL PRODUCE GARBAGE RESULTS WHILST USING SMARTS!" << std::endl;

  // Report input parameters
  std::cout << "Simulating in " << simulationTimeSegments << " time segments." << std::endl;
  std::cout << "Starting from day " << startDate << " and finishing on " << endDate << " splitting into " 
	    << yearSegments << " segments." << std::endl;
  std::cout << "In this job considering year segments " << startSegmentIndex << " to " << endSegmentIndex << std::endl;
  std::cout << "Splitting plenoptic funciton into " << azimuthBinNumber << " azimuth bins " << std::endl;
  std::cout << "\t and " << elevationBinNumber << " elevation bins" << std::endl;
  std::cout << "\t and " << energyBinNumber << " energy bins." << std::endl;
  std::cout << "Recording results in " << outputFileName << std::endl;

  // Also do not run if other arguments are present
  if (ops.options_remain()) {
    std::cerr << "Oops! Unexpected options." << std::endl;
    showHelp();
    return -1;
  }

  // Attempt to interpret the start and end dates.
  time_t interpretedStartDate = interpretDate(startDate);
  time_t interpretedEndDate =   interpretDate(endDate);

  // Get the device location details
  LocationDetails deviceLocation("location.cfg");

  // Set the altitude of the spectrum factory using location details
  int albedoIndex = 38; //light soil
  int diffuseVariableID = 7;
  SpectrumFactory::instance()->setAltitude(deviceLocation.getAltitude());
  SpectrumFactory::instance()->setTiltLocalAlbedo(albedoIndex);
  SpectrumFactory::instance()->appendOutputVariable(diffuseVariableID);

  // Prepare the climate factory with the default configuration
  ClimateFactory::instance()->setConfigurationFile("default.cfg");
  ClimateFactory::instance()->setDeviceLocation(deviceLocation);

  // Obtain the simulation sun
  Sun sun(deviceLocation);
  sun.setClimateOption(Sun::CLOUDCOVER, false); // Ignore clouds!

  // Prepare a root file to store the results
  TFile resultsFile(outputFileName.c_str(), "RECREATE");

  double minimumAzimuth = 0.0;
  double maximumAzimuth = 2.0*M_PI;
  double minimumElevation = 0.0;
  double maximumElevation = M_PI/2.0;
  
  double azimuthBinStepSize = (maximumAzimuth -minimumAzimuth)/azimuthBinNumber;
  double elevationBinStepSize = (maximumElevation -minimumElevation)/elevationBinNumber;

  // The lightfield object to fill
  Plenoptic3D lightfield;
  lightfield.setBinning(Plenoptic3D::AZIMUTH,   azimuthBinNumber,   minimumAzimuth,   maximumAzimuth);
  lightfield.setBinning(Plenoptic3D::ELEVATION, elevationBinNumber, minimumElevation, maximumElevation);
  // Maximal range allowed by smarts is ~0.31 -> 4.43 eV
  lightfield.setBinning(Plenoptic3D::ENERGY,    energyBinNumber,    0.2, 5.0);
  
  std::vector<time_t > dayTimes;
  std::vector<time_t > selectedDayTimes;
  std::vector<double > dayEnergySums;

  // Create a list of days (avoiding duplication).
  double yearSegmentSize = (interpretedEndDate-interpretedStartDate)/yearSegments;

  for (unsigned int segmentIndex=0; segmentIndex<yearSegments+1; segmentIndex++ ) {

    time_t candidateDay = interpretedStartDate + yearSegmentSize*segmentIndex;
      
    // Check that it is on a different day
    if ( dayTimes.size() > 0 && isSameDay(candidateDay, dayTimes.back()) ) {
      continue;
    }
    
    dayTimes.push_back(candidateDay);

    // If in specified year segment index range use for evaluation
    if ( segmentIndex >= startSegmentIndex && segmentIndex <= endSegmentIndex ) {
      selectedDayTimes.push_back(candidateDay);
    }
  }
  
  // Repeat simulation for each day
  for (unsigned int dayIndex=0; dayIndex<selectedDayTimes.size(); dayIndex++) {

    // Perform the simulation between the sunrise and sunset on the selected day.
    sun.setDate(selectedDayTimes[dayIndex]);
    int simulationStartingTime = sun.getSunriseTime()*60; //s
    int simulationEndingTime   = sun.getSunsetTime()*60; //s, 
    int simulationStepTime     = (double)(simulationEndingTime-simulationStartingTime)/simulationTimeSegments;

    // Integrate over the representative day
    // Simulate at all time points with the same number of events...
    for (unsigned int timeIndex=0; timeIndex<simulationTimeSegments; timeIndex++) {
      
      // Set the time to the mid-point of the day-time segment
      sun.setTime( (int)(simulationStartingTime + timeIndex*simulationStepTime + simulationStepTime/2.0) );

      for (unsigned int azimuthBin = 0; azimuthBin < azimuthBinNumber; azimuthBin++){
	double currentAzimuth = minimumAzimuth + azimuthBin*azimuthBinStepSize + 0.5*azimuthBinStepSize;

	for (unsigned int elevationBin = 0; elevationBin < elevationBinNumber; elevationBin++){
	  double currentElevation = minimumElevation + elevationBin*elevationBinStepSize + 0.5*elevationBinStepSize;
      
	  // Fill the lightfield
	  // Get the tilted irradiance for the current angle
	  SpectrumFactory::instance()->setTiltAngles(currentElevation*(180.0/M_PI), currentAzimuth*(180.0/M_PI));

	  std::shared_ptr<Spectrum> currentSpectrum = sun.getSpectrum();
	  std::vector<double> wavelengths = currentSpectrum->getSMARTSData()["Wvlgth"];
	  //! \todo This doesn't equal the radiance, it is completely wrong
	  std::vector<double> irradiances = currentSpectrum->getSMARTSData()["Difuse_tilted_irradiance"];
	  
	  for (unsigned int b=0; b<wavelengths.size(); b++) {
	    double currentWeight = irradiances[b] * simulationStepTime;
	    
	    //Convert the wavelength (nm) into energy (eV)
	    double currentEnergy = CLHEP::h_Planck * CLHEP::c_light / (wavelengths[b] * CLHEP::nm); //Joules
	    currentEnergy /= CLHEP::eV; //to eV 
	    
	    lightfield.fill(currentAzimuth, currentElevation, currentEnergy, currentWeight);
	  }
	}
      }
    }
  }
  
  // Write results out to the root file
  lightfield.Write("lightfield");

  // Also save the projected histogram separately as well
  lightfield.getEnergyProjectedHistogram()->Write("projectedLightfield");

  // Close the root file
  resultsFile.Close();

  return 0;
}







