#ifndef PVTREE_LOCATION_LOCATION_DETAILS
#define PVTREE_LOCATION_LOCATION_DETAILS

/*! @file
 * \brief Location details for simulation.
 *
 * Class used to store details of location of the
 * device to be simulated.
 *
 */

#include <string>
#include <iosfwd>

class LocationDetails {
 public:
  explicit LocationDetails(std::string inputFilePath);
  explicit LocationDetails(double longitude, double latitiude, double altitude);
  LocationDetails(const LocationDetails& original);
  ~LocationDetails();

  /*! \brief Get the longitude of the location
   *
   * \returns the longitude
   */
  double getLongitude() const;

  /*! \brief Get the latitude of the location
   *
   * \returns the latitude
   */
  double getLatitude() const;

  /*! \brief Get the altitude of the location
   *
   * \returns the altitude [km]
   */
  double getAltitude() const;

 private:
  /*! \brief Extract the location configuration
   *         from specified input file
   */
  void extractFile(std::string configFilePath);

  double m_longitude;
  double m_latitude;
  double m_altitude;
};

#endif  // PVTREE_LOCATION_LOCATION_DETAILS
