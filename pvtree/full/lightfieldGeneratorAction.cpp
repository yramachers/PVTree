#include "pvtree/full/lightfieldGeneratorAction.hpp"
#include "pvtree/full/weightedParticleGun.hpp"

#include "G4Event.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"

#include "G4LogicalVolumeStore.hh"
#include "G4LogicalVolume.hh"
#include "G4Orb.hh"

#include "Randomize.hh"
#include "pvtree/full/solarSimulation/plenoptic3D.hpp"

LightfieldGeneratorAction::LightfieldGeneratorAction(unsigned int photonNumber,
                                                     Plenoptic3D* lightfield)
    : G4VUserPrimaryGeneratorAction(),
      m_photonNumber(photonNumber),
      m_lightfield(lightfield) {
  m_particleGun = new WeightedParticleGun();

  // default particle kinematics
  // will be overriden by the lightfield!
  G4ParticleDefinition* particleDefinition =
      G4ParticleTable::GetParticleTable()->FindParticle("opticalphoton");

  m_particleGun->SetParticleDefinition(particleDefinition);
  m_particleGun->SetParticleMomentumDirection(G4ThreeVector(0.0, 0.0, -1.0));
  m_particleGun->SetParticleEnergy(3.0 * eV);
}

LightfieldGeneratorAction::~LightfieldGeneratorAction() {
  delete m_particleGun;
}

void LightfieldGeneratorAction::SetPhotonNumber(unsigned int photonNumber) {
  m_photonNumber = photonNumber;
}

void LightfieldGeneratorAction::GeneratePrimaries(G4Event* event) {
  // In order to avoid dependence of LightfieldGeneratorAction
  // on DetectorConstruction class we get world volume
  // from G4LogicalVolumeStore.
  G4LogicalVolume* worldLV =
      G4LogicalVolumeStore::GetInstance()->GetVolume("World");
  G4Orb* worldOrb = NULL;

  if (worldLV) worldOrb = dynamic_cast<G4Orb*>(worldLV->GetSolid());
  if (worldOrb) {
    // Evaluate the scale of the world for the lightfield
    G4double worldSurfaceRadius = worldOrb->GetRadius();

    // Lightfield geometry is a cube whilst world is an orb, probably
    // could match these a bit better.
    G4double generationRadius = worldSurfaceRadius * (1.0 / std::sqrt(3.0));

    // Set the world scale
    m_lightfield->setSurfaceScale(generationRadius);

    // Generate the photons
    m_lightfield->generate(event, m_particleGun, m_photonNumber);

  } else {
    G4cerr << "Orb world volume not found." << G4endl;
    G4cerr << "Perhaps you have changed geometry." << G4endl;
    G4cerr << "The gun will be place in the center" << G4endl;
    G4cerr << "and symbolically fire one into the ground." << G4endl;

    m_particleGun->SetParticlePosition(G4ThreeVector(0.0, 0.0, 0.0));
    m_particleGun->SetParticleMomentumDirection(G4ThreeVector(0.0, 0.0, -1.0));
    m_particleGun->SetParticleEnergy(3.0 * eV);
    m_particleGun->GeneratePrimaryVertex(event);
  }
}
