#include "full/primaryGeneratorAction.hpp"
#include "full/weightedParticleGun.hpp"

#include "G4Event.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"

#include "G4LogicalVolumeStore.hh"
#include "G4LogicalVolume.hh"
#include "G4Orb.hh"

#include "TRandom.h"
#include "Randomize.hh"
#include "solarSimulation/sun.hpp"
#include <iostream>

PrimaryGeneratorAction::PrimaryGeneratorAction(unsigned int photonNumber, Sun* sun) :
  G4VUserPrimaryGeneratorAction(),
  m_photonNumber(photonNumber),
  m_sun(sun) {

  m_particleGun = new WeightedParticleGun();

  // default particle kinematic
  G4ParticleDefinition* particleDefinition = G4ParticleTable::GetParticleTable()->FindParticle("opticalphoton");

  m_particleGun->SetParticleDefinition(particleDefinition);
  m_particleGun->SetParticleMomentumDirection(G4ThreeVector(0.0,0.0,-1.0));
  m_particleGun->SetParticleEnergy(3.0*eV);
}

PrimaryGeneratorAction::~PrimaryGeneratorAction() {
  delete m_particleGun;
}

void PrimaryGeneratorAction::SetPhotonNumber(unsigned int photonNumber) {
  m_photonNumber = photonNumber;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event) {

  TRandom rnd;
  double ret_x, ret_y;
  //Now need to translate the start position based upon the current light vector of the sun
  TVector3 currentLightVector = m_sun->getLightVector();

  //Define a tangent surface in which the 
  TVector3 orthogonalVector1 = currentLightVector.Orthogonal().Unit();
  TVector3 orthogonalVector2 = currentLightVector.Cross(orthogonalVector1).Unit();

  // In order to avoid dependence of PrimaryGeneratorAction
  // on DetectorConstruction class we get world volume
  // from G4LogicalVolumeStore.
  G4LogicalVolume* worldLV = G4LogicalVolumeStore::GetInstance()->GetVolume("World");
  G4Orb* worldOrb = NULL;

  if ( worldLV ) worldOrb = dynamic_cast<G4Orb*>(worldLV->GetSolid());
  if ( worldOrb ) { 
    
    //However only create photons within a restricted disk
    G4double worldSurfaceRadius = worldOrb->GetRadius();
    G4double generationRadius = worldSurfaceRadius*(1.0/std::sqrt(3.0));
    double eps_r = 0.001 * generationRadius; // small spray radius
    
    /*! \todo Need to set ROOT random number generator seed here for repeatability 
     *        as the spectrum uses TH1D to generate a random energy photon.
     */
    std::vector<std::tuple<double, double> > photonEnergies = m_sun->getSpectrum()->generatePhotons(m_photonNumber);
    
    double photonWeight = 0.0;
    for (unsigned int particleNumber=0; particleNumber<m_photonNumber; particleNumber++){
      
      G4double candidateX=0.0,candidateY=0.0;
      bool acceptablePoint = false;
      
      while (acceptablePoint != true){
	//Keep randomly generating x,y coordinates until they lie close enough to the origin
	candidateX = (G4UniformRand()*2.0 - 1.0)*generationRadius;
	candidateY = (G4UniformRand()*2.0 - 1.0)*generationRadius;
	
	if ( (std::pow(candidateX, 2.0) + std::pow(candidateY, 2.0)) > std::pow(generationRadius,2.0) ) {
	  acceptablePoint = false;
	} else {
	  acceptablePoint = true;
	}
      }
      
      TVector3 candidatePoint = TVector3(orthogonalVector1)*candidateX + TVector3(orthogonalVector2)*candidateY;
      rnd.Circle(ret_x, ret_y, eps_r); // random tiny rad vector
      TVector3 EpsVector = TVector3(orthogonalVector1)*ret_x + TVector3(orthogonalVector2)*ret_y;
      TVector3 sprayVector = currentLightVector + EpsVector;
      // Set the direction of the photon
      m_particleGun->SetParticleMomentumDirection(G4ThreeVector(sprayVector.X(),
								sprayVector.Y(),
								sprayVector.Z()));
      //      m_particleGun->SetParticleMomentumDirection(G4ThreeVector(currentLightVector.X(),
      //								currentLightVector.Y(),
      //								currentLightVector.Z()));

      //Then change the starting position of the photons in the opposite direction
      TVector3 toStartingPoint = (-1.0 * generationRadius) * currentLightVector;
      candidatePoint += toStartingPoint;
      
      m_particleGun->SetParticlePosition(G4ThreeVector(candidatePoint.X(), 
						       candidatePoint.Y(), 
						       candidatePoint.Z()));

      // Set the polarisation of the photon
      setRandomPhotonPolarisation();
      
      double photonEnergy = std::get<0>(photonEnergies[particleNumber]);
      photonWeight = std::get<1>(photonEnergies[particleNumber])/m_photonNumber;

      // Weight also needs to take into account the surface area over which photons are
      // being generated.
      photonWeight *= CLHEP::pi*std::pow(generationRadius/CLHEP::meter, 2.0);

      m_particleGun->SetParticleEnergy(photonEnergy*eV);
      m_particleGun->GenerateWeightedPrimaryVertex(event, photonWeight);
    }
    //    std::cout << "Weight: " << photonWeight << std::endl;
  }
  else {
    G4cerr << "Orb world volume not found." << G4endl;
    G4cerr << "Perhaps you have changed geometry." << G4endl;
    G4cerr << "The gun will be place in the center" << G4endl;
    G4cerr << "and symbolically fire one into the ground." << G4endl;
      
    m_particleGun->SetParticlePosition(G4ThreeVector(0.0, 0.0, 0.0));
    m_particleGun->SetParticleMomentumDirection(G4ThreeVector(0.0,0.0,-1.0));
    m_particleGun->SetParticleEnergy(3.0*eV);
    m_particleGun->GeneratePrimaryVertex(event);
  }
}

void PrimaryGeneratorAction::setRandomPhotonPolarisation() {
  G4double angle = G4UniformRand() * 360.0 * deg;
  
  G4ThreeVector normal(1.0, 0.0, 0.0);
  G4ThreeVector kphoton = m_particleGun->GetParticleMomentumDirection();
  G4ThreeVector product = normal.cross(kphoton);
  G4double modul2       = product*product;

  G4ThreeVector e_perpend(0.0, 0.0, 1.0);
  if (modul2 > 0.) e_perpend = (1./std::sqrt(modul2))*product;
  G4ThreeVector e_paralle    = e_perpend.cross(kphoton);

  G4ThreeVector polar = std::cos(angle)*e_paralle + std::sin(angle)*e_perpend;

  m_particleGun->SetParticlePolarization(polar);
}
