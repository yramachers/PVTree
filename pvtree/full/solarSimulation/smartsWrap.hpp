#ifndef PVTREE_SOLAR_SIMULATION_SMARTS_WRAP_HPP
#define PVTREE_SOLAR_SIMULATION_SMARTS_WRAP_HPP

/*! @file
 * \brief Testing common block access. Will need to create a class
 *        for the management of this configuration.
 *
 */

extern "C" {
/*! \brief Additional controls for SMARTS code
 *
 */
extern struct {
  //! \brief = 0 print nothing, >= 1 print errors, >= 2 print debug, >=3 print
  //everything.
  int verbosity;
  //! \brief = 0 write no files, = 1 write files.
  int writeOutputFiles;
} generalsmarts_;

/*! \brief Outputs from SMARTS code
 *
 * I expect at maximum 2000 bins, but I
 * leave a little spare room in case I can't
 * count.
 *
 * Also note the column/row ordering switch
 * between C++ and Fortran.
 */
extern struct {
  double outputBinValues[2100][55];
  int outputBinNumber;
  int outputHeaderNumber;
  char outputHeaders[55][25];
} smartsoutputs_;

/*! \brief First input card which is simply a string comment.
 *
 */
extern struct { char comment[64]; } inputcard1_;

/*! \brief Input card describing the site pressure.
 *
 */
extern struct {
  double pressure;
  double altitude;
  double height;
  double latitude;
  int mode;
} inputcard2_;

/*! \brief Input card selecting proper default atmosphere.
 *
 */
extern struct {
  double temperature;
  double relativeHumidity;
  double dailyTemperature;
  int mode;
  char season[6];
  char reference[4];
} inputcard3_;

/*! \brief Input card selecting atmostpheric water content
 *
 */
extern struct {
  double precipitableWater;
  int mode;
} inputcard4_;

/*! \brief Input card selecting ozone abundance
 *
 */
extern struct {
  double ozoneTotalColumnAbundance;
  int altitudeCorrectionMode;
  int mode;
} inputcard5_;

/*! \brief Input card controlling gaseous absorption
 *         and pollution.
 */
extern struct {
  double formaldehydeConcentration;
  double methaneConcentration;
  double carbonMonoxideConcentration;
  double nitrousAcidConcentration;
  double nitricAcidConcentration;
  double nitricOxideConcentration;
  double nitrogenDioxideConcentration;
  double nitrogenTrioxideConcentration;
  double ozoneConcentration;
  double sulfurDioxideConcentration;
  int loadMode;
  int mode;
} inputcard6_;

/*! \brief Input card selecting carbon dioxide concentration.
 *
 */
extern struct { double carbonDioxideConcentration; } inputcard7_;

/*! \brief Input card selecting the proper extraterrestrial
 *         spectrum.
 */
extern struct { int extraterrestrialSpectrum; } inputcard7a_;

/*! \brief Input card selecting the aerosol model.
 *
 */
extern struct {
  double alphaShortWavelength;
  double alphaLongWavelength;
  double singleScatteringAlbedo;
  double aerosolAsymmetry;
  char aerosolModel[64];
} inputcard8_;

/*! \brief Input card selecting the turbidity data.
 *
 */
extern struct {
  double aerosolOpticalDepth500;
  double angstromTurbidityCoefficient;
  double schueppTurbidityCoefficient;
  double meteorologicalRange;
  double prevailingVisibility;
  double aerosolOpticalDepth550;
  int mode;
} inputcard9_;

/*! \brief Input card selecting the correct zonal albedo.
 *
 */
extern struct {
  double broadbandLambertianAlbedo;
  int mode;
} inputcard10_;

/*! \brief Input card selecting tilt calculations.
 *
 */
extern struct {
  double tiltAngle;
  double surfaceAzimuth;
  double foregroundAlbedo;
  int foregroundAlbedoMode;
  int mode;
} inputcard10b_;

/*! \brief Input card selecting spectral range.
 *
 */
extern struct {
  double minWavelength;
  double maxWavelength;
  double sunCorrectionFactor;
  double solarConstant;
} inputcard11_;

/*! \brief Input card selecting how results should
 *         be printed to file.
 */
extern struct {
  double minWavelength;
  double maxWavelength;
  double wavelengthInterval;
  int numberOfOutputVariables;
  int mode;
  int variablesSelected[54];
} inputcard12_;

/*! \brief Input card selecting circumsolar calculation.
 *
 */
extern struct {
  double slopeAngle;
  double halfApertureOpeningAngle;
  double limitAngle;
  int mode;
} inputcard13_;

/*! \brief Input card selecting scanning/smoothing filter.
 *
 */
extern struct {
  double minWavelength;
  double maxWavelength;
  double irradianceStep;
  double fullWidthHalfMaximum;
  int transmittanceMode;
  int mode;
} inputcard14_;

/*! \brief Input card selecting illuminance, luminous efficacy
 *         and photosynthetically active radiation calculations.
 */
extern struct { int mode; } inputcard15_;

/*! \brief Input card selecting special broadband UV calculations.
 *
 */
extern struct { int mode; } inputcard16_;

/*! \brief Input card selecting solar position and air mass
 *         calculations.
 */
extern struct {
  double zenithAngle;
  double azimuthalAngle;
  double elevationAngle;
  double relativeAirMass;
  double latitude;
  double longitude;
  double timeInterval;
  int year;
  int month;
  int day;
  int hour;
  int zone;
  int mode;
} inputcard17_;
}

#endif  // PVTREE_SOLAR_SIMULATION_SMARTS_WRAP_HPP
