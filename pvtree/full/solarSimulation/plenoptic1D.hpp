#ifndef PVTREE_SOLARSIMULATION_PLENOPTIC1D
#define PVTREE_SOLARSIMULATION_PLENOPTIC1D

#include <vector>
#include <tuple>
#include "TObject.h"

class TH1D;

/*! \brief A one dimensional plenoptic function.
 *
 * Testing out idea of creating a plenoptic function which
 * is just a function of theta. It will wrap of course. It
 * allows the use of variable binning.
 *
 * In the future it would be good if it could generate the
 * binning automatically.
 *
 * It would good if these could be saved to disk and merged
 * safely.
 *
 * Angle is in radians and has the maximal range 0->2 PI
 */
class Plenoptic1D : public TObject {
 public:
  /*! \brief Create an empty Plenoptic function, this is just
   *         for root persistence.
   *
   */
  Plenoptic1D();
  ~Plenoptic1D();

  /*! \brief Create a Plenoptic function that subdivides the
   *         complete phase space by a given number of equally
   *         sized bins.
   *
   * @param[in] binNumber The number of bins to subdivide theta.
   */
  explicit Plenoptic1D(int binNumber);

  /*! \brief Create a Plenoptic function that subdivides the
   *         a specified range of theta by a given number of equally
   *         sized bins.
   *
   * @param[in] binNumber The number of bins to subdivide theta.
   * @param[in] minValue The minimum theta value in radians.
   * @param[in] maxValue The maximum theta value in radians.
   */
  Plenoptic1D(int binNumber, double minValue, double maxValue);

  /*! \brief Create a Plenoptic function that subdivides the
   *         a specified range of theta by an arbitrary set of
   *         bins.
   *
   * @param[in] binLowEdges A set of bin low edges for theta
   */
  explicit Plenoptic1D(std::vector<double> binLowEdges);

  /*! \brief Fill the plenoptic function with a given value
   *         at a certain angle.
   *
   * @param[in] angle The angle in raditions at which to fill
   * @param[in] value The fill value
   */
  void fill(double angle, double value);

  /*! \brief Reset the plenoptic function histogram to zero
   *
   */
  void clear();

  /*! \brief Set the lightfield surface geometry
   *
   * The chosen surface must not have overlaps. e.g. A ray to infinity must
   * not pass through the surface twice from the inside.
   *
   * @param[in] vertexPositions A vector of (x, y) positions which defines the
   *                            generation surface.
   */
  void setSurfaceGeometry(
      std::vector<std::tuple<double, double> > vertexPositions);

  /*! \brief Generate a set of particles with starting position, angle and
   *weight
   *
   * The default root random number generator is used, so set the ROOT seed the
   * standard way.
   *
   * @param[in] number The number of particles to generate, which defaults to
   *one.
   * \returns A vector of tuples containing x, y, theta and weight.
   */
  std::vector<std::tuple<double, double, double, double> > generate(
      unsigned int number = 1u) const;

 private:
  //! Histogram which stores the binned values of the plenoptic function.
  TH1D* m_histogram;

  //! The vertex positions of surface used for generation
  std::vector<std::tuple<double, double> > m_vertexPositions;

  //! The energy flux through each element of the surface
  std::vector<double> m_surfaceElementFlux;

  //! Hack to get random sampling working with piecewise constant distribution
  std::vector<double> m_surfaceElementID;

  //! The length of each surface element
  std::vector<double> m_surfaceElementLengths;

  //! The normal angle of each surface element
  std::vector<double> m_surfaceElementAngles;

  //! The total energy flux through the surface
  double m_totalSurfaceFlux;

  //! Record that the surface geometry has been provided
  bool m_hasSurfaceGeometry;

 private:
  /*! \brief Calculate the flux from the plenoptic function through a line
   *         segment.
   *
   * \param[in] x1 The starting x coordinate.
   * \param[in] y1 The starting y coordinate.
   * \param[in] x2 The ending x coordinate.
   * \param[in] y2 The ending y coordinate.
   * \returns The sum of energy flux through the segment
   */
  double calculateElementFlux(double x1, double y1, double x2, double y2) const;

  /*! \brief Calculate the length of a segment.
   *
   * \param[in] x1 The starting x coordinate.
   * \param[in] y1 The starting y coordinate.
   * \param[in] x2 The ending x coordinate.
   * \param[in] y2 The ending y coordinate.
   * \returns The length of a segment.
   */
  double calculateElementLength(double x1, double y1, double x2,
                                double y2) const;

  /*! \brief Calculate the normal angle of a segment.
   *
   * \param[in] x1 The starting x coordinate.
   * \param[in] y1 The starting y coordinate.
   * \param[in] x2 The ending x coordinate.
   * \param[in] y2 The ending y coordinate.
   * \returns The angle of a segment.
   */
  double calculateElementAngle(double x1, double y1, double x2,
                               double y2) const;

  /*! \brief Calculate minimal angular separation.
   *
   * \param[in] angle1 Angle in radians (range -pi to pi)
   * \param[in] angle2 Angle in radians (range -pi to pi)
   * \returns The angular separation in radians ( range 0 to pi ).
   */
  double deltaTheta(double angle1, double angle2) const;

  /*! \brief Calculate wrapped angle (range -pi to pi).
   *
   * \param[in] angle Angle in radians.
   * \returns The angle in range -pi to pi.
   */
  double wrapAngle(double angle) const;

  ClassDef(Plenoptic1D, 1);
};

#endif  // PVTREE_SOLARSIMULATION_PLENOPTIC1D
