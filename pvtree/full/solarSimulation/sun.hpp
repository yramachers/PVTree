#ifndef PVTREE_SOLAR_SIMULATION_SUN
#define PVTREE_SOLAR_SIMULATION_SUN

/*! @file
 * \brief Wrapper of the SolPos C library.
 *
 * Uses an underlying implementation from the solar library SolPos 2.0
 * http://rredc.nrel.gov/solar/codesandalgorithms/solpos/
 */

#include "pvtree/full/solarSimulation/spectrum.hpp"
#include "pvtree/location/locationDetails.hpp"
#include <vector>
#include <memory>

// save diagnostic state
#pragma GCC diagnostic push

// turn off the specific warning.
#pragma GCC diagnostic ignored "-Wshadow"

#include "TVector3.h"

// turn the warnings back on
#pragma GCC diagnostic pop

extern "C" {
#include "pvtree/solpos/solpos00.h"
}

/*! \brief A class used to keep track of the properties of the simulated sun
 * used
 *  in the simulation.
 */
class Sun {
 public:
  /// Control the use of climate information
  enum RealClimateOption {
    TEMPERATURE,
    PRESSURE,
    COLUMNWATER,
    COLUMNOZONE,
    CLOUDCOVER
  };

 private:
  /// As an underlying implementation use the solar library SolPos 2.0
  /// http://rredc.nrel.gov/solar/codesandalgorithms/solpos/
  struct posdata m_solarPositionData;

  /// Keep track of the need to update the temperature and pressure
  bool m_recalculateEnvironment;
  bool m_recalculateSolarPosition;

  /// Location of the device in the world for which calculations should
  /// be made
  LocationDetails m_deviceLocation;

  //! Climate corrections to apply (by default)
  std::map<RealClimateOption, bool> m_climateOptions = {{TEMPERATURE, true},
                                                        {PRESSURE, true},
                                                        {COLUMNWATER, true},
                                                        {COLUMNOZONE, true},
                                                        {CLOUDCOVER, false}};

  double m_albedo;

  /*! \brief Setting environment variables from climate factory.
   *
   */
  void updateEnvironment();

  /*! \brief Calls the underlying library with changed parameters.
   *
   */
  void updateSolarPosition();

 public:
  explicit Sun(LocationDetails deviceLocation);
  ~Sun();

  /*! \brief Get the current azimuthal angle.
   *
   * \returns The azimuthal angle of the sun in the sky in radians.
   *          where N=0.0, E=90.0, S=180.0, w=270.0 deg
   */
  double getAzimuthalAngle();

  /*! \brief Get the current elevation angle.
   *
   * \returns The elevation angle of the sun in the sky in radians.
   */
  double getElevationAngle();

  /*! \brief Get the direction of the light ray from sun.
   *
   * \returns A unit TVector3 from the sun.
   */
  TVector3 getLightVector();

  /*! \brief Get the direct normal solar irradiance at the current time.
   *
   * This uses the more simple solpos model to evaluate irradiance. The
   * SMARTS implementation supersedes this. Might still be useful as a
   * cross check.
   *
   * \returns The total normal solar irradiance.
   */
  double getIrradiance();

  /*! \brief Get the time of the sunset for the current day.
   *
   * Uses a method which does not account for refraction.
   *
   * \returns The time in minutes since midnight.
   */
  double getSunsetTime();

  /*! \brief Get the time of the sunrise for the current day.
   *
   * Uses a method which does not account for refraction.
   *
   * \returns The time in minutes since midnight.
   */
  double getSunriseTime();

  /*! \brief Get the current photon energy spectrum.
   *
   * \returns The photon energy spectrum.
   */
  std::shared_ptr<Spectrum> getSpectrum();

  /*! \brief Set the date for which sun should be evaluated
   *
   * The allowed range of year number is 1950 to 2050 due
   * to limits of the algorithm. The allowed range of day
   * number is 1 to 365. These will be checked by the solar
   * positioning code.
   *
   * @param[in] dayNumber  The day number of the year.
   * @param[in] yearNumber The four digit year number.
   */
  void setDate(int dayNumber, int yearNumber);

  /*! \brief Set the date for which sun should be evaluated
   *
   * @param[in] date The time since the epoch time.
   */
  void setDate(time_t date);

  /*! \brief Set the time of the chosen day for which sun should be evaluated
   *
   * @param[in] hour    The hour of day.
   * @param[in] minute  The minute of the hour.
   * @param[in] second  The second of the minute.
   */
  void setTime(int hour, int minute, int second);

  /*! \brief Set the time of the chosen day for which sun should be evaluated
   *
   * @param[in] secondOfDay  The second of the day.
   */
  void setTime(int secondOfDay);

  /*! \brief Set the location of the device being simulated.
   *
   * @param[in] deviceLocation Location of the device.
   */
  void setDeviceLocation(LocationDetails deviceLocation);

  /*! \brief Check if specified time is during the daytime
   *
   * @param[in] time The time to check.
   *
   * \returns True if time is during the daytime.
   */
  bool isTimeDuringDay(time_t time);

  /*! \brief Check if a climate option should be applied
   *
   * @param[in] option The option as found in the RealClimateOption enumeration
   *
   * \returns True if the climate variable from real data is used in simulation
   *config.
   */
  bool getClimateOption(RealClimateOption option);

  /*! \brief Set if a climate variable should be used
   *
   * @param[in] option The option as found in the RealClimateOption enumeration
   * @param[in] isEnabled Set to true if you want the climate variable from real
   *data to be used.
   */
  void setClimateOption(RealClimateOption option, bool isEnabled);
  
  /*! \brief Get the surface albedo from the climate data.
   *
   * \returns The surface albedo.
   */
  double getAlbedo();
};

#endif  // PVTREE_SOLAR_SIMULATION_SUN
