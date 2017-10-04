#include "pvtree/full/solarSimulation/sun.hpp"
#include "pvtree/full/solarSimulation/spectrumFactory.hpp"
#include "pvtree/climate/climateFactory.hpp"
#include "pvtree/climate/climate.hpp"
#include <cmath>
#include <iostream>
#include <cassert>
#include <time.h>

#include <CLHEP/Units/SystemOfUnits.h>
#include <CLHEP/Units/PhysicalConstants.h>

using CLHEP::gram;
using CLHEP::cm2;
using CLHEP::kilogram;
using CLHEP::m2;

void Sun::updateEnvironment() {
  // hmm need this run first to get the month :D
  S_solpos(&(this->m_solarPositionData));

  // Construct the current time
  struct tm calendarTime;
  calendarTime.tm_sec = this->m_solarPositionData.second;
  calendarTime.tm_min = this->m_solarPositionData.minute;
  calendarTime.tm_hour = this->m_solarPositionData.hour;
  calendarTime.tm_mday = this->m_solarPositionData.day;
  calendarTime.tm_mon = this->m_solarPositionData.month - 1;
  calendarTime.tm_year = this->m_solarPositionData.year - 1900;
  calendarTime.tm_isdst = 1;
  time_t currentTime = mktime(&calendarTime);

  // Get albedo
  m_albedo = ClimateFactory::instance()->getClimate()->getInterpolatedValue(
         "Albedo", currentTime);

  if (getClimateOption(TEMPERATURE)) {
    // Get current settings from climate factory static instance
    // using interpolation to fill in the gaps.
    double currentTemperature =
        ClimateFactory::instance()->getClimate()->getInterpolatedValue(
            "2 metre temperature", currentTime);
    this->m_solarPositionData.temp =
        currentTemperature -
        CLHEP::STP_Temperature;  // Need to convert from Kelvin.
  }

  if (getClimateOption(PRESSURE)) {
    double currentPressure =
        ClimateFactory::instance()->getClimate()->getInterpolatedValue(
            "Surface pressure", currentTime);
    this->m_solarPositionData.press = currentPressure * 0.01;  // Pa to hPa.

    // Also update the spectrum factory as well
    SpectrumFactory::instance()->setAtmosphericPressure(currentPressure *
                                                        0.01);  // Pa to mb
  }

  if (getClimateOption(COLUMNWATER)) {
    double currentPrecipitableWater =
        ClimateFactory::instance()->getClimate()->getInterpolatedValue(
            "Total column water", currentTime);
    SpectrumFactory::instance()->setPrecipitableWater(
        currentPrecipitableWater /
        ((gram / cm2) * (m2 / kilogram)));  // kg/m^2 to g/cm^2
  }

  if (getClimateOption(COLUMNOZONE)) {
    // Need to convert ozone density from kg m**-2 to ozone depth atm-cm
    double oxygen3Mass = 3.0 * 2.6568e-26;  // kg
    double loschmidtConstant = 2.6868e25;   // m-3
    double ozoneDensity =
        ClimateFactory::instance()->getClimate()->getInterpolatedValue(
            "Total column ozone", currentTime);
    double ozoneAbundance =
        ((ozoneDensity / oxygen3Mass) / loschmidtConstant) * 100.0;  // atm-cm
    SpectrumFactory::instance()->setOzoneAbundance(ozoneAbundance);
  }

  // Also need the total cloud cover
  if (getClimateOption(CLOUDCOVER)) {
    double currentCloudCover =
        ClimateFactory::instance()->getClimate()->getInterpolatedValue(
            "Total cloud cover", currentTime);
    SpectrumFactory::instance()->setCloudCover(currentCloudCover);
  }

  this->m_recalculateEnvironment = false;
}

void Sun::updateSolarPosition() {
  if (this->m_recalculateEnvironment) {
    updateEnvironment();  // make sure this has been updated
  }

  long retval = S_solpos(&(this->m_solarPositionData));
  S_decode(retval, &(this->m_solarPositionData));

  if (retval != 0) {
    // Try to catch problems with settings
    std::cout << this->m_solarPositionData.month << std::endl;
    std::cout << this->m_solarPositionData.hour << std::endl;
    std::cout << this->m_solarPositionData.minute << std::endl;
    std::cout << this->m_solarPositionData.second << std::endl;
    std::cout << this->m_solarPositionData.temp << std::endl;
    throw "Solar Positioning software badly initialized.";
  }

  this->m_recalculateSolarPosition = false;

  // Update spectrum factory to use this new position
  SpectrumFactory* factory = SpectrumFactory::instance();
  factory->setSolarPositionWithElevationAzimuth(
      this->getElevationAngle() * (180.0 / M_PI),
      this->getAzimuthalAngle() * (180.0 / M_PI));
}

Sun::Sun(LocationDetails deviceLocation)
    : m_recalculateEnvironment(true),
      m_recalculateSolarPosition(true),
      m_deviceLocation(deviceLocation) {
  // Initialize all the default values
  S_init(&(this->m_solarPositionData));

  // Set location for evaluation.
  this->m_solarPositionData.longitude = m_deviceLocation.getLongitude();
  this->m_solarPositionData.latitude =
      m_deviceLocation.getLatitude(); /* in DECIMAL DEGREES, not Deg/Min/Sec */

  // Tilt at latitude -- shouldn't matter
  // pd.tilt      = 0.0;
  // pd.aspect    = 135.0; //South east facing

  // Initialize all the time values
  this->m_solarPositionData.timezone =
    m_deviceLocation.getTimeZone(); 
  /* = GMT; DO NOT ADJUST FOR DAYLIGHT SAVINGS TIME. */
  this->m_solarPositionData.year = 2014;
  this->m_solarPositionData.daynum = 0;
  this->m_solarPositionData.hour = 12;
  this->m_solarPositionData.minute = 0;
  this->m_solarPositionData.second = 0;
}

Sun::~Sun() {
  // I dont think anything is necessary here. At the very least their test did
  // not clean anything up.
}

void Sun::setDate(time_t date) {
  // Convert to calendar time
  struct tm* calendarTime = gmtime(&date);

  this->m_solarPositionData.day = calendarTime->tm_mday;
  this->m_solarPositionData.month = calendarTime->tm_mon + 1;
  this->m_solarPositionData.year = calendarTime->tm_year + 1900;

  // Tell solpos to calculate the day of year number
  this->m_solarPositionData.function &= ~S_DOY;

  this->m_recalculateEnvironment = true;
  this->m_recalculateSolarPosition = true;
}

void Sun::setDate(int dayNumber, int yearNumber) {
  this->m_solarPositionData.daynum = dayNumber;
  this->m_solarPositionData.year = yearNumber;

  // Tell solpos to calculate the day of month number
  this->m_solarPositionData.function |= S_DOY;

  this->m_recalculateEnvironment = true;
  this->m_recalculateSolarPosition = true;
}

void Sun::setTime(int hour, int minute, int second) {
  this->m_solarPositionData.hour = hour;
  this->m_solarPositionData.minute = minute;
  this->m_solarPositionData.second = second;
  this->m_recalculateEnvironment = true;
  this->m_recalculateSolarPosition = true;
}

void Sun::setTime(int secondOfDay) {
  // need to convert into hours/minutes/seconds
  // Get the number of hours
  int hour = floor(secondOfDay / 3600.0);
  int secondOfHour = secondOfDay - hour * 3600;

  // Get the number of minutes
  int minute = floor(secondOfHour / 60.0);
  int secondOfMinute = secondOfHour - minute * 60;

  this->m_solarPositionData.hour = hour;
  this->m_solarPositionData.minute = minute;
  this->m_solarPositionData.second = secondOfMinute;

  this->m_recalculateEnvironment = true;
  this->m_recalculateSolarPosition = true;
}

void Sun::setDeviceLocation(LocationDetails deviceLocation) {
  m_deviceLocation = deviceLocation;

  this->m_solarPositionData.longitude = m_deviceLocation.getLongitude();
  this->m_solarPositionData.latitude =
      m_deviceLocation.getLatitude(); /* in DECIMAL DEGREES, not Deg/Min/Sec */

  this->m_recalculateEnvironment = true;
  this->m_recalculateSolarPosition = true;
}

double Sun::getAzimuthalAngle() {
  if (this->m_recalculateEnvironment) {
    updateEnvironment();
  }
  if (this->m_recalculateSolarPosition) {
    updateSolarPosition();
  }

  return this->m_solarPositionData.azim * (M_PI / 180.0);
}

double Sun::getElevationAngle() {
  if (this->m_recalculateEnvironment) {
    updateEnvironment();
  }
  if (this->m_recalculateSolarPosition) {
    updateSolarPosition();
  }

  return this->m_solarPositionData.elevref * (M_PI / 180.0);
}

TVector3 Sun::getLightVector() {
  if (this->m_recalculateEnvironment) {
    updateEnvironment();
  }
  if (this->m_recalculateSolarPosition) {
    updateSolarPosition();
  }

  double azimuth =
      this->m_solarPositionData.azim;  // N=0.0, E=90.0, S=180.0, W=270.0
  double elevation = this->m_solarPositionData
                         .elevref;  // Refracted elevation angle of the sun

  TVector3 sunVector(0.0, 1.0, 0.0);

  sunVector.RotateX(elevation * (M_PI / 180.0));
  sunVector.RotateZ(azimuth * (M_PI / 180.0));

  // Invert direction of vector ( sun -> leaf! )
  sunVector = sunVector * -1.0;

  return sunVector;
}

double Sun::getIrradiance() {
  if (this->m_recalculateEnvironment) {
    updateEnvironment();
  }
  if (this->m_recalculateSolarPosition) {
    updateSolarPosition();
  }

  return this->m_solarPositionData
      .etrn;  // Returning the ET direct normal solar irradiance
}

double Sun::getSunsetTime() {
  if (this->m_recalculateEnvironment) {
    updateEnvironment();
  }
  if (this->m_recalculateSolarPosition) {
    updateSolarPosition();
  }

  return this->m_solarPositionData.ssetr;  // Returning the minutes since
                                           // midnight for sunset time (no
                                           // refraction)
}

double Sun::getSunriseTime() {
  if (this->m_recalculateEnvironment) {
    updateEnvironment();
  }
  if (this->m_recalculateSolarPosition) {
    updateSolarPosition();
  }

  return this->m_solarPositionData.sretr;  // Returning the minutes since
                                           // midnight for sunrise time (no
                                           // refraction)
}

std::shared_ptr<Spectrum> Sun::getSpectrum() {
  if (this->m_recalculateEnvironment) {
    updateEnvironment();
  }
  if (this->m_recalculateSolarPosition) {
    updateSolarPosition();
  }

  SpectrumFactory* factory = SpectrumFactory::instance();
  return factory->getSpectrum();
}

bool Sun::isTimeDuringDay(time_t time) {
  setDate(time);

  // Start and end time of day
  double sunsetTime = getSunsetTime() * 60.0;
  double sunriseTime = getSunriseTime() * 60.0;

  // Get the time in seconds since start of day
  // Convert to calendar time
  struct tm* calendarTime = gmtime(&time);

  double timeFromStartOfDay = calendarTime->tm_sec +
                              calendarTime->tm_min * 60.0 +
                              calendarTime->tm_hour * 60.0 * 60.0;

  if (timeFromStartOfDay > sunriseTime && timeFromStartOfDay < sunsetTime) {
    return true;
  }

  return false;
}

bool Sun::getClimateOption(RealClimateOption option) {
  return m_climateOptions[option];
}

void Sun::setClimateOption(RealClimateOption option, bool isEnabled) {
  m_climateOptions[option] = isEnabled;
}

double Sun::getAlbedo() {
  if (this->m_recalculateEnvironment) {
    updateEnvironment();
  }
  return m_albedo;
}
