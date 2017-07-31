#ifndef PVTREE_SOLAR_SIMULATION_SPECTRUM_FACTORY_HPP
#define PVTREE_SOLAR_SIMULATION_SPECTRUM_FACTORY_HPP

#include "pvtree/full/solarSimulation/spectrum.hpp"

#include <string>
#include <map>
#include <memory>
#include <functional>

/*! \brief Factory which will provide access to SMARTS
 *         spectra.
 *
 * Follows the singleton pattern so all access to methods
 * is made through a static instance. This is because of
 * the need to access SMARTS via a set of structs->common
 * block.
 */
class SpectrumFactory {
 private:
  //! Prevent construction of additional instances
  SpectrumFactory();
  //! Prevent copy construction of instances
  SpectrumFactory(SpectrumFactory& spectrumFactory);

 public:
  /*! \brief Set the configurations values to default
   *         settings.
   */
  void setDefaults();

  /*! \brief Retrieve the singleton reference to this factory.
   */
  static SpectrumFactory* instance();

  /*! \brief Retrieve the spectrum produced by current configuration.
   * \returns A shared pointer to a spectrum.
   */
  std::shared_ptr<Spectrum> getSpectrum();

  /*! \brief Force the factory to re-run SMARTS even if the
   *         parameters are unchanged since last run.
   */
  void clearCache();

  /*! \brief Set the solar position (also for air mass calculation).
   *
   * @param[in] solarElevation True astronomical elevation plus refraction
   *correction [deg]
   * @param[in] solarAzimuth Solar azimuth counted clockwise from North (0.0)
   *[deg]
   *
   * Azimuth is only used if calculating radiation on a tilted plane (can be set
   *to zero).
   */
  void setSolarPositionWithElevationAzimuth(double solarElevation,
                                            double solarAzimuth);

  /*! \brief Set the default atmospheric pressure.
   *
   * Sets the atmospheric pressure to 1015mb and uses the mode
   * where altitude and height are specified (set to 0.088 and 0.0).
   */
  void setDefaultAtmosphericPressure();

  /*! \brief Set the atmospheric pressure at location.
   *
   * @param[in] pressure The atmospheric pressure at site [mb]
   */
  void setAtmosphericPressure(double pressure);

  /*! \brief Set the altitude above sea level.
   *
   * It should not exceed 100km.
   *
   * @param[in] altitude The altitude above sea level at the site [km]
   */
  void setAltitude(double altitude);

  /*! \brief Set the precipitable water above the site to the default.
   *
   * Uses the SMARTS mode where the precipitable water value originates
   * from selected atmosphere.
   */
  void setDefaultPrecipitableWater();

  /*! \brief Set the precipitable water above the site.
   *
   * @param[in] precipitableWater The precipitable water above the site [g/cm^2]
   */
  void setPrecipitableWater(double precipitableWater);

  /*! \brief Set the ozone abundance above the site to the default.
   *
   * Use the reference atmosphere to obtain the ozone abundance.
   */
  void setDefaultOzoneAbundance();

  /*! \brief Set the ozone abundance above the site.
   *
   * @param[in] ozoneAbundance The ozone abundance above the site [atm-cm]
   * @param[in] altitudeCorrectionMode The default of 0 will result in no
   *altitude correction
   *            being applied. Whilst 1 will apply a correction for an elevated
   *site (above sea
   *            level).
   */
  void setOzoneAbundance(double ozoneAbundance, int altitudeCorrectionMode = 0);

  /*! \brief Set the default atmosphere properties.
   *
   * Setting the atmosphere to the reference atmosphere 'USSA'. Thus giving an
   * ideal atmosphere.
   */
  void setDefaultAtmosphereProperties();

  /*! \brief Set atmosphere properties.
   *
   * Not the best name I think...
   *
   * @param[in] airTemperature The current atmospheric temperature at the site
   *[deg C]
   * @param[in] relativeHumidity The relative humidity at site level [%]
   * @param[in] time The current time, which is used to select an appropriate
   *season.
   * @param[in] averageDailyTemperature The average daily temperature at site
   *level [deg C]
   */
  void setAtmosphereProperties(double airTemperature, double relativeHumidity,
                               time_t time, double averageDailyTemperature);

  /*! \brief Set default gas load.
   *
   * Describes gaseous absorption and atmospheric pollution. The default is to
   *use average vertical
   * profiles for all gas abundances (except for CO2, O3 and water vapour).
   */
  void setDefaultGasLoad();

  /*! \brief Enumeration describing options available for pollution levels.
   *
   */
  enum GasLoad {
    PRISTINE,
    LIGHT_POLLUTION,
    MODERATE_POLLUTION,
    SEVERE_POLLUTION
  };

  /*! \brief Set gas pollution Load
   *
   * @param[in] loadChoice Choice of gas concenctration sets as defined by the
   *GasLoad
   *                       enumeration.
   */
  void setGasLoad(GasLoad loadChoice);

  /*! \brief Set the cloud cover fraction
   *
   * This is a non-smarts parameter.
   */
  void setCloudCover(double cloudCover);

  /*! \brief Set the surface direction for evaluating irradiance on tilted
   *         surfaces.
   *
   * @param[in] elevation The angle of the surface with respect to the vertical
   *                      plane. (90deg is a vertical plane). Set to -999 to
   *                      track the sun.
   * @param[in] azimuth Surface azimuth counted clockwise from the North. Set to
   *                    -999 to track the sun.
   */
  void setTiltAngles(double elevation, double azimuth);

  /*! \brief Set the local albedo reference for calculations related to tilted
   *         surfaces.
   *
   * @param[in] referenceAlbedoIndex The index of the reference albedo to be
   *used
   *                                 for backscattering calculation. See SMARTS
   *                                 documentation for all the options (65 of
   *them).
   */
  void setTiltLocalAlbedo(int referenceAlbedoIndex);

  /*! \brief Add extra variable to be outputted by the SMARTS.
   *
   * @param[in] extraVariableIndex The SMARTS index of the output variable to be
   *                               calculated.
   */
  void appendOutputVariable(int extraVariableIndex);

 private:
  bool m_parametersChanged;
  std::shared_ptr<Spectrum> m_previousSpectrum;
  double m_cloudCover;

  /*! \brief Output variables to be calculated by SMARTS.
   *
   * Defaults are : -
   *
   *   Extraterrestrial spectrum (1) [Wm^-2]
   *
   *   Direct normal irradiance (2) [Wm^-2]
   *
   *   Diffuse horizontal irradiance (3) [Wm^-2]
   *
   *   Spectral photonic energy (39) [eV]
   *
   *   Direct normal photon flux per eV (41) [cm^-2 s^-1 eV^-1]
   */
  std::vector<int> m_outputVariablesSelected = {1, 2, 3, 39, 41};

  /*! \brief Convert c-strings to Fortran strings which are space padded and
   *         not null terminated.
   *
   * from
   *http://stackoverflow.com/questions/10163485/passing-char-arrays-from-c-to-fortran
   *
   * @param[out] fstring Pointer to character array in the fortran common block.
   * @param[in]  fstring_len The length of the fortran character array.
   * @param[in]  cstring The C character array to copy from.
   */
  void convertToFortran(char* fstring, std::size_t fstring_len,
                        const char* cstring);
};

#endif  // PVTREE_SOLAR_SIMULATION_SPECTRUM_FACTORY_HPP
