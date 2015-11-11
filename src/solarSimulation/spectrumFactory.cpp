#include "solarSimulation/spectrumFactory.hpp"
#include "solarSimulation/smartsWrap.hpp"

#include <cstring>
#include <algorithm>
#include <iostream>

extern"C" {
  void runsmarts_();
}

void SpectrumFactory::convertToFortran(char* fstring, std::size_t fstring_len, const char* cstring){
  std::size_t inlen = std::strlen(cstring);
  std::size_t cpylen = std::min(inlen, fstring_len);
  
  if (inlen > fstring_len){
    std::cerr << "Input string length (" << inlen << ") is longer than largest possible Fortran string ("
	      << cpylen << ")." << std::endl;
    std::cerr << "\tSMARTS may not be configured correctly because of this." << std::endl;
    std::cerr << "\tInput string involved = " << cstring << std::endl;
  }

  std::copy(cstring, cstring + cpylen, fstring);
  std::fill(fstring + cpylen, fstring + fstring_len, ' ');
}


SpectrumFactory::SpectrumFactory() {

  // Set default SMARTS options
  setDefaults();
}

void SpectrumFactory::setDefaults() {
  // Control verbosity of smarts, where we only want
  // errors reported and no output files produced.
  generalsmarts_.verbosity = 1;
  generalsmarts_.writeOutputFiles = 0;

  // Card 1 
  convertToFortran(inputcard1_.comment, 64, "Spectrum Factory Setup");

  // Card 2
  setDefaultAtmosphericPressure();

  // Card 3
  setDefaultAtmosphereProperties();

  // Card 4
  setDefaultPrecipitableWater();

  // Card 5
  setDefaultOzoneAbundance();
  
  // Card 6
  setDefaultGasLoad();

  // Card 7
  inputcard7_.carbonDioxideConcentration = 370.0;

  // Card 7a 
  inputcard7a_.extraterrestrialSpectrum = 1;

  // Card 8
  convertToFortran(inputcard8_.aerosolModel, 64, "S&F_URBAN");

  // Card 9
  inputcard9_.mode = 0;
  inputcard9_.aerosolOpticalDepth500 = 0.084;

  // Card 10
  inputcard10_.mode = 38;
  
  // Card 10b
  inputcard10b_.mode = 0;

  // Card 11
  inputcard11_.minWavelength       = 280.0;
  inputcard11_.maxWavelength       = 4000.0;
  inputcard11_.sunCorrectionFactor = 1.0;
  inputcard11_.solarConstant       = 1367.0;

  // Card 12
  inputcard12_.mode                    = 2;
  inputcard12_.minWavelength           = 280.0;
  inputcard12_.maxWavelength           = 4000.0;
  inputcard12_.wavelengthInterval      = 0.5;

  if ( m_outputVariablesSelected.size() > 54 ) {
    throw std::string("Too many output variables selected.");
  }

  inputcard12_.numberOfOutputVariables = m_outputVariablesSelected.size();
  for (unsigned int variableIndex=0; variableIndex < m_outputVariablesSelected.size(); variableIndex++) {
    inputcard12_.variablesSelected[variableIndex] = m_outputVariablesSelected[variableIndex];
  }

  // Card 13
  inputcard13_.mode = 0;

  // Card 14
  inputcard14_.mode = 0;

  // Card 15
  inputcard15_.mode = 0;

  // Card 16
  inputcard16_.mode = 0;

  // Card 17
  inputcard17_.mode = 2;
  inputcard17_.relativeAirMass = 1.5;

  // Non-SMARTS
  m_cloudCover = 0.0;

  // Parameters changed so smarts needs to be re-run
  clearCache();
}

SpectrumFactory::SpectrumFactory(SpectrumFactory& /*spectrumFactory*/) {}

SpectrumFactory* SpectrumFactory::instance(){
  static SpectrumFactory spectrumFactory;
  return &spectrumFactory;
}

std::shared_ptr<Spectrum> SpectrumFactory::getSpectrum() {

  if (!m_parametersChanged){
    
    //If nothing has changed return previously constructed spectrum
    return m_previousSpectrum;    
  }

  // Run SMARTS
  runsmarts_();

  // Extract the results from smarts 
  // Start with the header names
  std::vector<std::string>                    headerNames;
  std::map<std::string, std::vector<double> > binValues;
  for (int x = 0; x < smartsoutputs_.outputHeaderNumber; x++) {
    std::string headerName = std::string(smartsoutputs_.outputHeaders[x]);
    headerNames.push_back( headerName );
    binValues[headerName] = std::vector<double>();
  } 

  // Then the bin values
  for (int w = 0; w < smartsoutputs_.outputBinNumber; w++) {
    for (int h = 0; h < smartsoutputs_.outputHeaderNumber; h++) {
      binValues[headerNames[h]].push_back( smartsoutputs_.outputBinValues[w][h] );
    }
  }

  //! \todo Replace the use of incredibly simple model of cloud cover.
  for ( unsigned int b=0; b<binValues[std::string("Direct_normal_irradiance")].size(); b++){
    binValues[std::string("Direct_normal_irradiance")][b] *= 1.00001 - m_cloudCover;
  }

  // Create the spectrum (don't manually delete!)
  Spectrum* spectrum = new Spectrum(headerNames, binValues);
  m_previousSpectrum = std::shared_ptr<Spectrum>(spectrum);

  m_parametersChanged = false;
  return m_previousSpectrum;
}

void SpectrumFactory::clearCache() {
  m_parametersChanged = true;
}

void SpectrumFactory::setSolarPositionWithElevationAzimuth(double solarElevation, double solarAzimuth) {
  
  // Card 17
  inputcard17_.mode = 1;
  inputcard17_.elevationAngle = solarElevation;
  inputcard17_.azimuthalAngle = solarAzimuth;

  clearCache();
}

void SpectrumFactory::setDefaultAtmosphericPressure() {

  // Card 2
  inputcard2_.mode     = 1;
  inputcard2_.pressure = 1015.0;
  inputcard2_.altitude = 0.088;
  inputcard2_.height   = 0.0;

  clearCache();
}

void SpectrumFactory::setAtmosphericPressure(double pressure) {

  // Card 2
  inputcard2_.pressure = pressure;

  if (inputcard2_.mode == 2) {
    // Wrong mode for using pressure, switch by default (but also warn!)
    // Mode 1 is actually recommended!
    inputcard2_.mode = 1;
    std::cerr << "Inconsistant mode for using atmospheric pressure, switching to mode 1." << std::endl;
  }

  clearCache();
}

void SpectrumFactory::setAltitude(double altitude) {

  // Card 2
  inputcard2_.altitude = altitude;

  if (inputcard2_.mode == 0) {
    // Wrong mode for using altitude
    std::cerr << "Inconsistant mode for using altitude." << std::endl;
  }

  clearCache();
}

void SpectrumFactory::setDefaultPrecipitableWater() {

  // Card 4
  // Use default value for current atmosphere.
  inputcard4_.mode = 1;

  clearCache();
}

void SpectrumFactory::setPrecipitableWater(double precipitableWater) {

  // Card 4
  inputcard4_.mode = 0;
  inputcard4_.precipitableWater = precipitableWater;

  clearCache();
}

void SpectrumFactory::setDefaultOzoneAbundance() {

  // Card 5
  inputcard5_.mode = 1;

  clearCache();
}

void SpectrumFactory::setOzoneAbundance(double ozoneAbundance, int altitudeCorrectionMode /* = 0 */) {

  // Card 5
  inputcard5_.mode = 0;
  inputcard5_.altitudeCorrectionMode = altitudeCorrectionMode;
  inputcard5_.ozoneTotalColumnAbundance = ozoneAbundance;

  clearCache();
}

void SpectrumFactory::setDefaultAtmosphereProperties() {

  // Card 3
  inputcard3_.mode = 1;
  convertToFortran(inputcard3_.reference, 4, "USSA");

  clearCache();
}

void SpectrumFactory::setAtmosphereProperties(double airTemperature, double relativeHumidity, time_t /*time*/, double averageDailyTemperature) {
  
  // Card 3 -- Setting up a 'realisitic' atmosphere
  inputcard3_.mode = 0;

  inputcard3_.temperature = airTemperature;
  inputcard3_.relativeHumidity = relativeHumidity;
  inputcard3_.dailyTemperature = averageDailyTemperature;

  //! \todo Pick a season based upon the month and location.
  convertToFortran(inputcard3_.season, 6, "SUMMER");

  clearCache();
}

void SpectrumFactory::setDefaultGasLoad() {

  // Card 6
  inputcard6_.mode = 1;

  clearCache();
}

void SpectrumFactory::setGasLoad(GasLoad loadChoice) {

  int translatedLoadMode = 1;

  switch(loadChoice) {
  case PRISTINE:
    translatedLoadMode = 1;
    break;
  case LIGHT_POLLUTION:
    translatedLoadMode = 2;
    break;
  case MODERATE_POLLUTION:
    translatedLoadMode = 3;
    break;
  case SEVERE_POLLUTION:
    translatedLoadMode = 4;
    break;
  default:
    translatedLoadMode = 1;
  }

  // Card 6
  inputcard6_.mode     = 0;
  inputcard6_.loadMode = translatedLoadMode;

  clearCache();
}

void SpectrumFactory::setCloudCover(double cloudCover) {
  m_cloudCover = cloudCover;

  clearCache();
}

void SpectrumFactory::setTiltAngles(double elevation, double azimuth) {

  // Card 10b
  inputcard10b_.mode           = 1;
  inputcard10b_.tiltAngle      = elevation;
  inputcard10b_.surfaceAzimuth = azimuth;
  
  clearCache();
}

void SpectrumFactory::setTiltLocalAlbedo(int referenceAlbedoIndex) {

  // Card 10b
  inputcard10b_.foregroundAlbedoMode = referenceAlbedoIndex;

  clearCache();
}

void SpectrumFactory::appendOutputVariable(int extraVariableIndex) {

  // First check if the variable was already present
  if ( std::find( begin(m_outputVariablesSelected), end(m_outputVariablesSelected), extraVariableIndex) != m_outputVariablesSelected.end() ){
    return;
  }

  m_outputVariablesSelected.push_back(extraVariableIndex);

  if ( m_outputVariablesSelected.size() > 54 ) {
    std::cerr << "Too many output variables selected." << std::endl;
    throw std::string("Too many output variables selected.");
  }

  inputcard12_.numberOfOutputVariables = m_outputVariablesSelected.size();
  for (unsigned int variableIndex=0; variableIndex < m_outputVariablesSelected.size(); variableIndex++) {
    inputcard12_.variablesSelected[variableIndex] = m_outputVariablesSelected[variableIndex];
  }

  clearCache();
}

