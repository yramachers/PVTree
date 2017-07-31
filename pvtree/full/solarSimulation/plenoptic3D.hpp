#ifndef PVTREE_SOLARSIMULATION_PLENOPTIC3D
#define PVTREE_SOLARSIMULATION_PLENOPTIC3D

#include <vector>
#include <tuple>
#include <map>
#include <random>

// save diagnostic state
#pragma GCC diagnostic push

// turn off the specific warning.
#pragma GCC diagnostic ignored "-Wshadow"

#include "TObject.h"
#include "TVector3.h"

// turn the warnings back on
#pragma GCC diagnostic pop

class TH2D;
class TH3D;
class G4Event;
class WeightedParticleGun;

/*! \brief A three dimensional plenoptic function.
 *
 */
class Plenoptic3D : public TObject {
 public:
  /*! \brief Enumeration unifying the axis setting and getting.
   *
   */
  enum Axis { AZIMUTH = 1, ELEVATION = 2, ENERGY = 3 };

  /*! \brief Create a 3D Plenoptic function with default settings.
   *
   */
  Plenoptic3D();

  ~Plenoptic3D();

  /*! \brief Create a Plenoptic function that subdivides the
   *         complete phase space by a given number of equally
   *         sized bins. Default ranges are still used.
   *
   * @param[in] azimuthBinNumber The number of bins to subdivide azimuth.
   * @param[in] elevationBinNumber The number of bins to subdivide elevation.
   * @param[in] energyBinNumber The number of bins to subdivide the
   *            photon energy (in the range 280.0 nm to 1500.0 nm).
   */
  Plenoptic3D(int azimuthBinNumber, int elevationBinNumber,
              int energyBinNumber);

  /*! \brief Copy a Plenoptic3D function
   */
  Plenoptic3D(const Plenoptic3D& original);

  /*! \brief Set equal sized binning between default minimum and maximum
   *
   * @param[in] selectedAxis Enumeration value for axis you wish to set.
   * @param[in] binNumber The total number of bins
   */
  void setBinning(Axis selectedAxis, int binNumber);

  /*! \brief Set equal sized binning between specified minimum and maximum
   *
   * @param[in] selectedAxis Enumeration value for axis you wish to set.
   * @param[in] binNumber The total number of bins
   * @param[in] minimumValue The first bins low value.
   * @param[in] maximumValue The last bins high value.
   */
  void setBinning(Axis selectedAxis, int binNumber, double minimumValue,
                  double maximumValue);

  /*! \brief Set bin low edges
   *
   * @param[in] selectedAxis Enumeration value for axis you wish to set.
   * @param[in] binLowEdges A vector of bin starting values.
   */
  void setBinning(Axis selectedAxis, std::vector<double> binLowEdges);

  /*! \brief Fill the plenoptic function with a weight at a specific
   *         angle and energy.
   *
   * @param[in] azimuth The azimuth angle in radians.
   * @param[in] elevation The elevation angle in radians.
   * @param[in] energy The energy of the photon in eV.
   * @param[in] weight The weight for this entry.
   */
  void fill(double azimuth, double elevation, double energy, double weight);

  /*! \brief Reset the plenoptic function histogram to zero
   *
   */
  void clear();

  /*! \brief Set the lightfield scale.
   *
   * Simple scale factor to be applied to the surface where the photons are
   * being generated from. The surface being used is a cube. Important for
   * ensuring the simulation scene is entirely surrounded by the lightfield.
   *
   * @param[in] surfaceScale Factor to be applied to a 1x1x1 cube.
   */
  void setSurfaceScale(double surfaceScale);

  /*! \brief Make a simple estimate of the normalized surface fluxes for the
   *         current plenoptic function.
   *
   * Publically available so user can decide when to carry out the estimate.
   */
  void estimateSurfaceFluxes();

  /*! \brief Randomly generate a set of photons from the lightfield and
   *         add them to a Geant 4 event for simulation.
   *
   * @param[in] event The Geant event to be simulated.
   * @param[in] particleGun The object which constructs the photons.
   * @param[in] number The number of photons to added to the event.
   */
  void generate(G4Event* event, WeightedParticleGun* particleGun,
                unsigned int number);

  /*! \brief Retreive the elevation vs azimuth histogram with energy projected
   *         out.
   *
   * Mainly for a visual inspection of the plenoptic function.
   */
  TH2D* getEnergyProjectedHistogram() const;

  /*! \brief Add another plenoptic function onto this instance.
   *
   * @param[in] source The plenoptic function to copy from.
   */
  void append(const Plenoptic3D& source);

  /*! \brief Set the random number seed sequence for generation step
   *
   * @param[in] seedSequence An arbitrary length integer vector for seeding
   *                         the random number generator.
   */
  void setRandomNumberSeedSequence(std::vector<int> seedSequence);

 private:
  //! Histogram which stores the binned values of the plenoptic function.
  TH3D* m_histogram;

  //! Current binning vectors
  std::map<Axis, std::vector<double>> m_binLowEdges;
  std::map<Axis, int> m_axisDefaultBinNumbers = {
      {AZIMUTH, 50}, {ELEVATION, 50}, {ENERGY, 100}};
  std::map<Axis, double> m_axisMinimumAllowed = {
      {AZIMUTH, 0.0}, {ELEVATION, 0.0}, {ENERGY, 1.0}};
  std::map<Axis, double> m_axisMaximumAllowed = {
      {AZIMUTH, 2.0 * M_PI}, {ELEVATION, M_PI / 2.0}, {ENERGY, 10.0}};

  //! Surface definition
  std::vector<std::pair<TVector3, TVector3>> m_surfaces;

  //! The energy flux through each element of the surface
  std::vector<double> m_surfaceFlux;

  //! Hack to get random sampling working with piecewise constant distribution
  std::vector<double> m_surfaceID;

  //! The length of each surface element
  std::vector<double> m_surfaceAreas;

  //! Normals of generating surface
  std::vector<TVector3> m_surfaceNormals;

  //! Uniform scale applied to the surface vertex positions.
  // Units are [mm]!
  double m_surfaceScale;

  //! Record that the flux through the lightfield surface has been
  //  evaluated (if a little roughly).
  bool m_hasOptimizedSampling;

  //! Keep track of the properties of histogram
  bool m_histogramDefinitionChanged;

  //! Random number generator seed sequence
  std::vector<int> m_seedSequence = {1, 2, 1234};
  bool m_seedChanged;

  //! Random number generator (that should not be persisted)
  std::mt19937* m_generator;  //! transient

 private:
  /*! \brief Construct the lightfield surface geometry
   *
   * Currently just builds a five sided cube (missing the bottom out) where
   * the sides are axially aligned.
   */
  void setSurfaceGeometry();

  /*! \brief Create the plenoptic function histogram using the current
   *         settings.
   *
   */
  void constructHistogram();

  /*! \brief Calculate the flux from the plenoptic function through a surface
   *         segment.
   *
   * @param[in] surfaceArea Surface area for the segment considered
   * @param[in] normal Surface normal vector.
   * \returns The sum of energy flux through the surface.
   */
  double calculateSurfaceFlux(double surfaceArea, TVector3 normal) const;

  /*! \brief Calculate the flux from the plenoptic function through a surface
   *         segment.
   *
   * @param[in] surface Vertex positions definining a plane.
   * \returns The surface area of the plane.
   */
  double calculateSurfaceArea(std::pair<TVector3, TVector3> surface) const;

  /*! \brief Calculate minimal angular separation.
   *
   * @param[in] angle1 Angle in radians (range -pi to pi)
   * @param[in] angle2 Angle in radians (range -pi to pi)
   * \returns The angular separation in radians ( range 0 to pi ).
   */
  double deltaAzimuth(double angle1, double angle2) const;

  /*! \brief Calculate wrapped angle (range -pi to pi).
   *
   * @param[in] angle Angle in radians.
   * \returns The angle in range -pi to pi.
   */
  double wrapAngle(double angle) const;

  /*! \brief Get fractional position between two values always starting from
   *         the smaller value.
   *
   * @param[in] fraction value between 0.0 and 1.0
   * @param[in] valueA Arbitrary value.
   * @param[in] valueB Arbitrary value.
   */
  double getOrderedFractionalValue(double fraction, double valueA,
                                   double valueB) const;

  /*! \brief Choose a random photon polarisation for the next shot.
   *
   * @param[in] particleGun the gun producing the current particles.
   */
  void setRandomPhotonPolarisation(WeightedParticleGun* particleGun) const;

  ClassDef(Plenoptic3D, 1);
};

#endif  // PVTREE_SOLARSIMULATION_PLENOPTIC3D
