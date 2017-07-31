#ifndef PVTREE_CLIMATE_CLIMATE_FACTORY_HPP
#define PVTREE_CLIMATE_CLIMATE_FACTORY_HPP

/* @file
 * \brief Factory which will provide access to the
 *        atmospheric conditions stored in GRIB
 *        files.
 */
#include "pvtree/location/locationDetails.hpp"
#include <string>
#include <map>
#include <memory>
#include <vector>

class Climate;

/*! \brief Factory which will provide access to the
 *        atmospheric conditions stored in GRIB
 *        files.
 *
 * Follows the singleton pattern so all access to methods
 * is made through a static instance.
 *
 */
class ClimateFactory {
 private:
  /*! \brief Name of configuration file describing climate.
   */
  std::string m_climateConfiguration;

  /*! \brief Instance of climate. Deletion of this object is
   *         handled by the climate factory.
   */
  Climate* m_climate;

  /*! \brief Monitor climate configuration name
   */
  bool m_climateConfigurationChanged;

  /*! \brief Device location information
   */
  LocationDetails m_deviceLocation;

  /*! \brief Prevent construction of additional instances.
   *
   */
  ClimateFactory();

  /*! \brief Prevent copy construction of instances.
   *
   */
  ClimateFactory(ClimateFactory& climateFactory);

  /*! \brief Clean up.
   *
   */
  ~ClimateFactory();

 public:
  /*! \brief Retrieve the singleton reference to this factory.
   */
  static ClimateFactory* instance();

  /*! \brief Set the configuration file describing the climate.
   */
  void setConfigurationFile(std::string configurationFileName);

  /*! \brief Set the device location for climate evaluation.
   *
   * @param[in] deviceLocation Location details instance for device.
   */
  void setDeviceLocation(LocationDetails deviceLocation);

  /*! \brief Retrieve the instance of the climate construted.
   *
   * Will lazily construct the climate when first requested.
   */
  const Climate* getClimate();
};

#endif  // PVTREE_CLIMATE_CLIMATE_FACTORY_HPP
