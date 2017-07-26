#ifndef PVTREE_CLIMATE_CLIMATE_DATA_HPP
#define PVTREE_CLIMATE_CLIMATE_DATA_HPP

/* @file
 * \brief Simple class used to store the extracted GRIB
 *        parameter values for a single point in time. A
 *        few helper methods are available for access to 
 *        particular data times.
 *
 */

#include <map>
#include <string>
#include <time.h>
#include <memory>

/*! \brief Simple class used to store the extracted GRIB
 *         parameter values for a single point in time. A
 *         few helper methods are available for access to 
 *         particular data times.
 *
 */
class ClimateData {
private:

  //! \brief List of values indexed by the parameter ID
  std::map<int, double> m_parameterValues;

  //! \brief The time when the variables where measured/calculated
  time_t m_time;

  /*! \brief The mapping of parameter names to their ID
   *         where it is assumed to be the same for all data items.
   */
  std::shared_ptr<std::map<std::string,int>> m_nameToParameterID;

public:
  /*! \brief Construct climate data object with a specific parameter set mapping
   *         and time.
   *
   * @param[in] nameToParameterID A shared pointer to a map correlating the name 
   *            of parameters and their IDs.
   * @param[in] time The time of measurement/calculation.
   */
  ClimateData(std::shared_ptr<std::map<std::string,int>> nameToParameterID, time_t time);

  ~ClimateData();

  /*! \brief Set the parameter mapping
   *
   * @param[in] nameToParameterID A shared pointer to a map correlating the name 
   *            of parameters and their IDs.
   */
  void   setParameterMapping(std::shared_ptr<std::map<std::string,int>> nameToParameterID);

  /*! \brief Retrieve the value for a specific named parameter.
   *
   * @param[in] parameterName The name of the parameter as defined
   *                          in the input GRIB file.
   */
  double getValue(std::string parameterName) const;

  /*! \brief Retrieve the value for a specific parameter ID.
   *
   * @param[in] parameterID The ID of the parameter as defined in
   *                        the input GRIB file.
   */
  double getValue(int parameterID) const;

  /*! \brief Set the value of a parameter of specific ID
   *
   * @param[in] parameterID The ID of the parameter as defined in
   *                        the input GRIB file.
   * @param[in] value       The value for the parameter.
   */
  void   setValue(int parameterID, double value);

  /*! \brief Check if there is a value present for the parameter at
   *         the specific time
   * @param[in] parameterName The name of the parameter as defined
   *                          in the input GRIB file.
   *
   * \returns True if a value is present.
   */
  bool   hasValue(std::string parameterName) const;

  /*! \brief Check if there is a value present for the parameter at
   *         the specific time
   * @param[in] parameterID The ID of the parameter as defined
   *                        in the input GRIB file.
   *
   * \returns True if a value is present.
   */
  bool   hasValue(int parameterID) const;

  /*! \brief Retrieve the time of measurement/calculation
   */
  time_t getTime() const;

  /*! \brief Set the time of mesaurement/calculation.
   *
   * @param[in] time The time of the measurement/calculation.
   */
  void   setTime(time_t time);
};



#endif //PVTREE_CLIMATE_CLIMATE_DATA_HPP
