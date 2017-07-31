#ifndef PVTREE_SOLAR_SIMULATION_SPECTRUM
#define PVTREE_SOLAR_SIMULATION_SPECTRUM

/*! @file
 * \brief Solar spectrum handler for SMARTS outputs.
 *
 * Class to generate photons from SMARTS spectra.
 *
 */

#include <string>
#include <iosfwd>
#include <vector>
#include <map>
#include <tuple>
#include <memory>

class TH1D;

class Spectrum {
 public:
  explicit Spectrum(std::string inputFilePath);
  Spectrum(std::vector<std::string> columnNames,
           std::map<std::string, std::vector<double> > data);
  ~Spectrum();

  /*! \brief Produce a set of photons with the
   *         bias of more photons in regions with
   *         higher irradiance.
   *
   * \returns A vector of tuples containing the photon
   *          energy in eV and the direct normal irradiance.
   *          To recover the original distribution still need
   *          to divide by the total number of photons considered.
   */
  std::vector<std::tuple<double, double> > generatePhotons(
      unsigned int photonNumber);

  /*! \brief Retrieve the raw SMARTS column names
   *         for the spectrum
   */
  std::vector<std::string> getSMARTSColumnNames() const;

  /*! \brief Retrieve the raw SMARTS binned data
   *         values for the spectrum. For each of
   *         the requested variables.
   *
   * \returns a map containing bin values for each variable
   *          where the map is indexed by the column name.
   */
  std::map<std::string, std::vector<double> > getSMARTSData() const;

  /*! \brief Retrieve the SMARTS column data binned in wavelength
   *
   * @param[in] columnName The name of the column produced by SMARTS.
   *
   * \returns a shared pointer to the requested histogram
   */
  std::shared_ptr<TH1D> getHistogram(std::string columnName);

  /*! \brief Check that two spectra are the same
   *         within the precision of the data.
   */
  bool operator==(const Spectrum& otherSpectrum);

  /*! \brief Check that two spectra are not the same
   *         within the precision of the data.
   */
  bool operator!=(const Spectrum& otherSpectrum);

 private:
  /*! \brief Extract the spectra definition
   *         from a text file from SMARTS.
   */
  void extractFile(std::ifstream& inputFile);

  /*! \brief Convert input data into histogram.
   *
   * @param[in] columnName the name of the column produced by SMARTS.
   */
  void createHistogram(std::string columnName);

  //! The raw data from the file
  std::map<std::string, std::vector<double> > m_data;

  //! Store the column names seperately as well
  std::vector<std::string> m_columnNames;

  //! Store the histograms by name
  std::map<std::string, std::shared_ptr<TH1D> > m_histograms;

  //! Precision of import format (to handle standard smarts export)
  int m_dataPrecision;
};

#endif  // PVTREE_SOLAR_SIMULATION_SPECTRUM
