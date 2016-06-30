#include "full/primaryGeneratorAction.hpp"
#include "full/weightedParticleGun.hpp"

#include "G4Event.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"

#include "G4LogicalVolumeStore.hh"
#include "G4LogicalVolume.hh"
#include "G4Orb.hh"

//#include "TRandom.h"
#include "Randomize.hh"
#include "solarSimulation/sun.hpp"
#include "solarSimulation/HosekSkyModel.hpp"
#include "TF2.h"
#include "TH1D.h"
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

  //  TRandom rnd;
  //  double ret_x, ret_y;
  //Now need to translate the start position based upon the current unit light vector of the sun
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
    
    double pi = acos(-1.0);

    G4double worldSurfaceRadius = worldOrb->GetRadius();
    double generationRadius = worldSurfaceRadius*(1.0/std::sqrt(3.0) / 10.1 * 0.75); // fit to size

    double solar_rad = m_sun->getElevationAngle(); // [rad], counts from horizon
    //    double solar_zenith = pi/2.0 - solar_rad;
    double solar_azimuth = m_sun->getAzimuthalAngle(); // [rad], counts from north=0 to west=270degr

    auto normalIrradianceHistogram = m_sun->getSpectrum()->getHistogram("Direct_normal_irradiance");
    auto diffuseIrradianceHistogram = m_sun->getSpectrum()->getHistogram("Difuse_horizn_irradiance");
    auto normalExtraterrIrrHistogram = m_sun->getSpectrum()->getHistogram("Extraterrestrial_spectrm");
    double totalNormal = normalIrradianceHistogram->Integral("width");
    double totalDiffuse = diffuseIrradianceHistogram->Integral("width");
    double totalextraterr = normalExtraterrIrrHistogram->Integral("width");

    // Perez brightness
    double bright = 1.5 * totalDiffuse / totalextraterr; // 1.5 air mass

    if (bright < 0.1) bright = 0.1; // limit to minimum 10% diffuse light or 5deg sun disc size
    if (bright > 1.0) bright = 1.0; // 
    
    double turb = 1.0 / (1.1 - bright); // experimental link to Perez bright parameter
    if (turb>10.0) turb = 10.0;
    if (turb<1.0) turb = 1.0;

    double albedo = 0.9; // hard-wired for now, high average albedo
    if (albedo<0.0) albedo = 0.0;
    if (albedo>1.0) albedo = 1.0;
    
    SkyFunction* sf = new SkyFunction(solar_rad,turb,albedo);
    TF2* fsky = new TF2("myf",sf,&SkyFunction::Eval,0.0, pi/2.0, 0.0, 2.0*pi, 0,"SkyFunction","Eval");
    //    std::cout << "fsky calling Eval(0.1,0.1) " << fsky->Eval(0.1,0.1) << std::endl;
    //    std::cout << "fsky calling Integral " << fsky->Integral(0.0,pi/2.0,0.0,2.0*pi) << std::endl;
    fsky->Integral(0.0,pi/2.0,0.0,2.0*pi); // activate function, ROOT issue
    
    // from Perez clearness parameter
    G4double ratio = 1.0 + totalNormal / totalDiffuse; // =1+normal/diffuse irradiance
//     std::cout << "Climate conditions for Hosek: Normal/Diffuse = " << totalNormal / totalDiffuse << std::endl;
//     std::cout << "Climate conditions for Hosek: Diffuse/Extraterr = " << totalDiffuse / totalextraterr << std::endl;
//     std::cout << "Hosek: brightness = " << bright << std::endl;
//     std::cout << "Hosek: turbidity  = " << turb << std::endl;
//     std::cout << "Hosek: albedo     = " << albedo << std::endl;
//    std::cout << "Hosek: Sun at elevation = " << solar_rad*180.0/pi << std::endl;
//    std::cout << "Hosek: Sun at azimuth   = " << solar_azimuth*180.0/pi << std::endl;
    
    std::vector<std::tuple<double, double> > photonEnergies = m_sun->getSpectrum()->generatePhotons(m_photonNumber);
    
    double photonWeight = 0.0;
    double photonEnergy = 0.0;
    double theta = 0.0; // angles in radians on sky sphere
    double gamma = 0.0; // with theta=0 at zenith, gamma=0 at sun position azimuth
    double probability = 1.0 / ratio;
    TVector3 candidatePoint(0,0,0); // for location
    for (unsigned int particleNumber=0; particleNumber<m_photonNumber; particleNumber++){
      
      if (G4UniformRand() >= probability) { // from the sun disk or ...
	// 	theta = x + solar_zenith; // solar zenith since TVector3 counts theta from zenith
	// 	gamma = y + solar_azimuth; // symmetric around solar azimuth
	// Weight also needs to take into account the surface area over which photons are
	// being generated. 
	// Gives units of [Watt] since integral arrives as [W/m^2]
	currentLightVector = m_sun->getLightVector();
	candidatePoint = directSun(generationRadius,orthogonalVector1,orthogonalVector2,currentLightVector);
	photonWeight = totalNormal / (m_photonNumber * (1.0-probability));
	photonWeight *= pi * std::pow(generationRadius/CLHEP::meter, 2.0); // area of normal irradiation source
      }
      else { // ... from the sky function
	fsky->GetRandom2(theta, gamma); // theta, gamma random coordinates on sky
	gamma += solar_azimuth - pi/2.0; // offset for sun position, TVector3 out of phi phase by 90deg
	candidatePoint.SetMagThetaPhi(worldSurfaceRadius, theta, gamma);
	currentLightVector = -1.0 * candidatePoint;
	// Weight also needs to take into account the surface area over which photons are
	// being generated. 
	// Gives units of [Watt] since integral arrives as [W/m^2]
	photonWeight = totalDiffuse / (m_photonNumber * probability);
	photonWeight *= std::pow(0.75*generationRadius/CLHEP::meter, 2.0); // horizontal area of receiving structure
      }

      // Set the direction of the photon - point to centre, not to sky
      m_particleGun->SetParticleMomentumDirection(G4ThreeVector(currentLightVector.X(),
								currentLightVector.Y(),
								currentLightVector.Z()));

      m_particleGun->SetParticlePosition(G4ThreeVector(candidatePoint.X(), 
						       candidatePoint.Y(), 
						       candidatePoint.Z()));

      // Set the polarisation of the photon
      setRandomPhotonPolarisation();
      
      photonEnergy = std::get<0>(photonEnergies[particleNumber]);
      //      photonWeight = std::get<1>(photonEnergies[particleNumber])/m_photonNumber;

      m_particleGun->SetParticleEnergy(photonEnergy*eV);
      m_particleGun->GenerateWeightedPrimaryVertex(event, photonWeight);

    }

    fsky->Delete();
    delete sf;
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


TVector3 PrimaryGeneratorAction::directSun(double genrad, TVector3 v1, TVector3 v2, TVector3 lv) {  
  G4double candidateX=0.0, candidateY=0.0;
  bool acceptablePoint = false;
  
  while (!acceptablePoint){
    //Keep randomly generating x,y coordinates until they lie close enough to the origin
    candidateX = (G4UniformRand()*2.0 - 1.0)*genrad;
    candidateY = (G4UniformRand()*2.0 - 1.0)*genrad;
    
    if ( (std::pow(candidateX, 2.0) + std::pow(candidateY, 2.0)) > std::pow(genrad,2.0) ) {
      acceptablePoint = false;
    } else {
      acceptablePoint = true;
    }
  }
  TVector3 candidatePoint = TVector3(v1)*candidateX + TVector3(v2)*candidateY;

  //Then change the starting position of the photons in the opposite direction
  TVector3 toStartingPoint = (-1.5 * genrad) * lv;
  candidatePoint += toStartingPoint;
      
  return candidatePoint;
}

