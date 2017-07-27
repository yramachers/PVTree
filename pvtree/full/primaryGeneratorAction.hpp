#ifndef PV_FULL_PRIMARY_GENERATOR_ACTION_HPP
#define PV_FULL_PRIMARY_GENERATOR_ACTION_HPP

/*! @file
 * \brief Handles the generation of primary particles for the Geant Simulation
 *        using a very simple solar model.
 *
 * Currently will generate photons on a disk with a direction controlled by the
 * suns current position. Energy sprectrum isn't a spectrum at the moment...
 */

#include "G4VUserPrimaryGeneratorAction.hh"
#include "TVector3.h"

class G4Event;
class WeightedParticleGun;
class Sun;

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {

public:
  PrimaryGeneratorAction(unsigned int photonNumber, Sun* sun);
  virtual ~PrimaryGeneratorAction();

  /*! \brief Called at the start of event generation. Initial vertices
   *         /particles are produced in this function.
   *
   */
  virtual void GeneratePrimaries(G4Event* event);
  
  /*! \brief Set the number of photons to be randomly generated
   *         in each event.
   */
  void SetPhotonNumber(unsigned int photonNumber);

private:
  unsigned int         m_photonNumber;
  WeightedParticleGun* m_particleGun;
  Sun*                 m_sun;

  /*! \brief Assume photons should be generated with a random
   *         polarisation. This is the default but if it is 
   *         not done manually there will be warnings!
   *
   * Currently taking implementation from : -
   * source/examples/extended/optical/OpNovice/src/OpNovicePrimaryGeneratorAction.cc
   * and it is consistant with the implementation in : -
   * source/event/src/G4PrimaryTransformer.cc
   *
   */
  void setRandomPhotonPolarisation();

  /*! \brief source geometry as disk of world radius/sqrt(3)
   */
  TVector3 directSun(double genrad, TVector3 v1, TVector3 v2, TVector3 lv);
};



#endif //PV_FULL_PRIMARY_GENERATOR_ACTION_HPP
