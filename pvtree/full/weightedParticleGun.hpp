#ifndef PV_FULL_WEIGHTED_PARTICLE_GUN
#define PV_FULL_WEIGHTED_PARTICLE_GUN

#include "G4ParticleGun.hh"

class G4Event;

/*! \brief A quick wrapping of the default Geant4 Particle
 *         gun class to add the option of giving the primary
 *         particle a weight.
 */
class WeightedParticleGun : public G4ParticleGun {
 public:
  WeightedParticleGun();
  virtual ~WeightedParticleGun();

  /*! \brief Add a new vertex to the event using the
   *         current particle definition.
   *
   * Identical to the GeneratePrimaryVertex method in base
   * class but with the option to pass a weight.
   */
  void GenerateWeightedPrimaryVertex(G4Event* evt, double weight);

 private:
};

#endif  // PV_FULL_WEIGHTED_PARTICLE_GUN
