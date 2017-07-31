#ifndef PV_FULL_LIGHTFIELD_GENERATOR_ACTION_HPP
#define PV_FULL_LIGHTFIELD_GENERATOR_ACTION_HPP

/*! @file
 * \brief Handles the generation of primary particles for the Geant Simulation
 *        using a previously evaluated lightfield.
 *
 * Currently will generate photons on the surface of a box surrounding the
 * detector according to the passed lightfield.
 */

#include "G4VUserPrimaryGeneratorAction.hh"

class G4Event;
class WeightedParticleGun;
class Plenoptic3D;

/* \brief Handles the generation of primary particles for the Geant Simulation
 *        using a previously evaluated lightfield.
 *
 * Currently will generate photons on the surface of a box surrounding the
 * detector according to the passed lightfield.
 */
class LightfieldGeneratorAction : public G4VUserPrimaryGeneratorAction {
 public:
  /*! \brief Construct the generator action which employs the use of
   *         3d plenoptic function.
   *
   * @param[in] photonNumber The number of photons to generate per event.
   * @param[in] lightfield The 3d plenoptic function describing how to produce
   *            the photons.
   */
  LightfieldGeneratorAction(unsigned int photonNumber, Plenoptic3D* lightfield);
  virtual ~LightfieldGeneratorAction();

  /*! \brief Called at the start of event generation. Initial vertices
   *         /particles are produced in this function.
   *
   * @param[in] event The event which should have primary particles added.
   */
  virtual void GeneratePrimaries(G4Event* event);

  /*! \brief Set the number of photons to be randomly generated
   *         in each event.
   *
   * @param[in] photonNumber Change the number of photons to generate per event.
   */
  void SetPhotonNumber(unsigned int photonNumber);

 private:
  unsigned int m_photonNumber;

  WeightedParticleGun* m_particleGun;

  Plenoptic3D* m_lightfield;
};

#endif  // PV_FULL_LIGHTFIELD_GENERATOR_ACTION_HPP
