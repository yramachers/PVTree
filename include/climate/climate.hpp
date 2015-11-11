#ifndef PVTREE_CLIMATE_CLIMATE_HPP
#define PVTREE_CLIMATE_CLIMATE_HPP

/* @file
 * \brief Wrapper for the purpose of accessing the various
 *        climate properties stored in GRIB (Gridded binary)
 *        format.
 */

#include <string>
#include <map>
#include <memory>
#include <vector>
#include <time.h>
#include "eccodes.h"

#include "climate/climateData.hpp"
#include "location/locationDetails.hpp"

#include <Math/Interpolator.h>

namespace libconfig{
  class Config;
}

/*! \brief Wrapper for the purpose of accessing the various
 *         climate properties stored in GRIB (Gridded binary)
 *         format.
 * 
 */
class Climate {
private:
  
  // GRIB file name which contains climate variables
  std::string               m_gribFileName;

  // List the parameter ID and variable names
  std::map<int,std::string> m_parameterIDToName;
  std::map<int,std::string> m_parameterIDToUnits;

  // Allow the name to parameter ID map to be shared
  std::shared_ptr<std::map<std::string,int>> m_nameToParameterID;

  // Store maximums and minimums of values 
  // mainly to avoid issuses in interpolation
  std::map<int, double> m_parameterIDMaxValueAllowed;
  std::map<int, double> m_parameterIDMinValueAllowed;

  // Store the extracted data
  std::vector<std::shared_ptr<ClimateData>> m_climateData;

  /*! \brief Number of interpolation data points to use
   *         in both the forward and backward directions. 
   *
   * The default number used is five.
   */
  int m_interpolationPointNumber = 5;

  /*! \brief Location of the device
   */
  LocationDetails m_deviceLocation;

  /*! \brief Check if a file exists for a given path.
   *
   * @param[in] filePath The path whose existence is to be checked.
   *
   * \returns True if the file exists at the specified path.
   */
  bool fileExists(std::string filePath);

  /*! \brief Attempt to open a configuration file in a number
   *         of locations
   *
   * @param[in] configPath configuration path fragment which may
   *                       be found in a number of locations.
   *
   * \returns True if configuration file opened.
   */
  bool openConfiguration(std::string configPath);  

  /*! \brief Actual reading of the configuration file with
   *         some standard parse checking applied by libconfig.
   *
   * @param[in] fileName Configuration file path.
   * @param[in] cfg Configuration library instance.
   *
   * \returns True if configuraiton has been parsed.
   */
  bool parseConfiguration(std::string fileName, libconfig::Config* cfg);  

  /*! \brief Attempt to find a GRIB file from which to import the 
   *         climate variables.
   *
   * \returns True if the GRIB file has been found.
   */
  bool findGRIB();

  /*! \brief Extract the time from a GRIB message.
   *
   * @param[in] handle The handle to the GRIB message.
   *
   * \returns The time of the message.
   */
  time_t getTimeFromMessage(codes_handle* handle);

  /*! \brief Parse the contents of a GRIB file
   *
   * \returns True if the GRIB file could be parsed succesfully.
   */
  bool parseGRIB();

public:

  /*! \brief Construct climate object with configuration specified
   *         in a file.
   *
   * @param[in] configurationFileName The name of the configuration file 
   *                                  describing the data to load.
   * @param[in] deviceLocation Location of the device for which the climate
   *                           should be evaluated.
   */
  Climate(std::string configurationFileName,
	  LocationDetails deviceLocation);

  /*! \brief Clean up.
   */
  ~Climate();

  /*! \brief Access interpolated value using name of value.
   *
   * @param[in] valueName Name of parameter to be retrieved.
   * @param[in] time The time to which the parameter should be interpolated to.
   * @param[in] interpolationType The method of interpolation that should be used,
   *            where by default the cubic spline method is used.
   *
   * \returns The interpolated value.
   */
  double getInterpolatedValue(std::string valueName, 
			      time_t time, 
			      ROOT::Math::Interpolation::Type interpolationType = ROOT::Math::Interpolation::kCSPLINE) const;

  /*! \brief Access interpolated value using ID of value.
   *
   * @param[in] valueID ID of parameter to be retrieved.
   * @param[in] time The time to which the parameter should be interpolated to.
   * @param[in] interpolationType The method of interpolation that should be used,
   *            where by default the cubic spline method is used.
   *
   * \returns The interpolated value.
   */
  double getInterpolatedValue(int valueID, 
			      time_t time, 
			      ROOT::Math::Interpolation::Type interpolationType = ROOT::Math::Interpolation::kCSPLINE) const;

  /*! \brief Find out how many data points are used in interpolation.
   *
   * \returns The number of data points used around the time point specified to
   *          carry out interpolation (effectively doubled as both forward and 
   *          backward in time).
   */
  int getInterpolationPointNumber() const;

  /*! \brief Set how many data points are used in interpolation.
   *
   * @param[in] interpolationPointNumber The number of data points used around the 
   *            time point specified to carry out interpolation (effectively doubled 
   *            as both forward and backward in time).
   */
  void setInterpolationPointNumber(int interpolationPointNumber);

  /*! \brief Get the units of the requested parameter
   *
   * @param[in] parameterName Name of the parameter for which the units will
   *            be retrieved.
   *
   * \returns The units of the parameter in string format.
   */
  std::string getParameterUnits(std::string parameterName) const;

  /*! \brief Get the units of the requested parameter
   *
   * @param[in] parameterID ID of the parameter for which the units will
   *            be retrieved.
   *
   * \returns The units of the parameter in string format.
   */
  std::string getParameterUnits(int parameterID) const;
  
  /*! \brief Raw data access.
   *
   * \returns All the data extracted from the GRIB file for the 
   *          specified location. No modification is allowed.
   */
  std::vector<std::shared_ptr<const ClimateData>> getData() const;

};


#endif //PVTREE_CLIMATE_CLIMATE_HPP
