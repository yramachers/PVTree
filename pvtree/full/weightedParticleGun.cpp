#include "pvtree/full/weightedParticleGun.hpp"
#include "G4Event.hh"

WeightedParticleGun::WeightedParticleGun() : G4ParticleGun() {}

WeightedParticleGun::~WeightedParticleGun() {}

void WeightedParticleGun::GenerateWeightedPrimaryVertex(G4Event* evt,
                                                        double weight) {
  if (particle_definition == 0) return;

  // create a new vertex
  G4PrimaryVertex* vertex =
      new G4PrimaryVertex(particle_position, particle_time);

  // create new primaries and set them to the vertex
  G4double mass = particle_definition->GetPDGMass();
  for (G4int i = 0; i < NumberOfParticlesToBeGenerated; i++) {
    G4PrimaryParticle* particle = new G4PrimaryParticle(particle_definition);
    particle->SetKineticEnergy(particle_energy);
    particle->SetMass(mass);
    particle->SetMomentumDirection(particle_momentum_direction);
    particle->SetCharge(particle_charge);
    particle->SetPolarization(particle_polarization.x(),
                              particle_polarization.y(),
                              particle_polarization.z());
    particle->SetWeight(weight / NumberOfParticlesToBeGenerated);
    vertex->SetPrimary(particle);
  }
  evt->AddPrimaryVertex(vertex);
}
