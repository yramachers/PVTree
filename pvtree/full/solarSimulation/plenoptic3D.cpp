#include "pvtree/full/solarSimulation/plenoptic3D.hpp"
#include "pvtree/full/weightedParticleGun.hpp"
#include "pvtree/utils/equality.hpp"

#include <iostream>
#include <cmath>
#include <map>
#include <algorithm>
#include <random>

#include "TH2D.h"
#include "TH3D.h"
#include "G4Event.hh"
#include "G4ThreeVector.hh"
#include "Randomize.hh"
#include "G4SystemOfUnits.hh"

ClassImp(Plenoptic3D)

    Plenoptic3D::Plenoptic3D()
    : m_histogram(nullptr),
      m_surfaceScale(1.0),
      m_hasOptimizedSampling(false),
      m_histogramDefinitionChanged(true),
      m_seedChanged(true),
      m_generator(nullptr) {
  // Create initial bin edge vectors
  setBinning(AZIMUTH, m_axisDefaultBinNumbers[AZIMUTH]);
  setBinning(ELEVATION, m_axisDefaultBinNumbers[ELEVATION]);
  setBinning(ENERGY, m_axisDefaultBinNumbers[ENERGY]);

  // Setup the standard lightfield geometry.
  // Fixed to a cube at the moment.
  setSurfaceGeometry();
}

Plenoptic3D::~Plenoptic3D() {
  if (m_histogram != nullptr) {
    delete m_histogram;
  }
  if (m_generator != nullptr) {
    delete m_generator;
  }
}

Plenoptic3D::Plenoptic3D(int azimuthBinNumber, int elevationBinNumber,
                         int energyBinNumber)
    : m_histogram(nullptr),
      m_surfaceScale(1.0),
      m_hasOptimizedSampling(false),
      m_histogramDefinitionChanged(true),
      m_seedChanged(true),
      m_generator(nullptr) {
  // Create initial bin edge vectors
  setBinning(AZIMUTH, azimuthBinNumber);
  setBinning(ELEVATION, elevationBinNumber);
  setBinning(ENERGY, energyBinNumber);

  // Setup the standard lightfield geometry.
  // Fixed to a cube at the moment.
  setSurfaceGeometry();
}

Plenoptic3D::Plenoptic3D(const Plenoptic3D& original) : TObject(original) {
  // Copy everything !
  m_binLowEdges = original.m_binLowEdges;
  m_axisDefaultBinNumbers = original.m_axisDefaultBinNumbers;
  m_axisMinimumAllowed = original.m_axisMinimumAllowed;
  m_axisMaximumAllowed = original.m_axisMaximumAllowed;

  m_surfaces = original.m_surfaces;
  m_surfaceFlux = original.m_surfaceFlux;
  m_surfaceID = original.m_surfaceID;
  m_surfaceAreas = original.m_surfaceAreas;
  m_surfaceNormals = original.m_surfaceNormals;
  m_surfaceScale = original.m_surfaceScale;

  m_hasOptimizedSampling = original.m_hasOptimizedSampling;
  m_histogramDefinitionChanged = original.m_histogramDefinitionChanged;

  m_histogram = new TH3D(*(original.m_histogram));
  m_histogram->SetDirectory(0);
}

void Plenoptic3D::setBinning(Axis selectedAxis, int binNumber) {
  setBinning(selectedAxis, binNumber, m_axisMinimumAllowed[selectedAxis],
             m_axisMaximumAllowed[selectedAxis]);
}

void Plenoptic3D::setBinning(Axis selectedAxis, int binNumber,
                             double minimumValue, double maximumValue) {
  double binSize = (maximumValue - minimumValue) / binNumber;

  std::vector<double> binLowEdges;
  for (int binIndex = 0; binIndex < binNumber + 1; binIndex++) {
    binLowEdges.push_back(minimumValue + binSize * binIndex);
  }

  setBinning(selectedAxis, binLowEdges);
}

void Plenoptic3D::setBinning(Axis selectedAxis,
                             std::vector<double> binLowEdges) {
  m_binLowEdges[selectedAxis] = binLowEdges;
  m_histogramDefinitionChanged = true;
}

void Plenoptic3D::fill(double azimuth, double elevation, double energy,
                       double weight) {
  if (m_histogram == nullptr || m_histogramDefinitionChanged) {
    // If the histogram has not yet been created construct it
    // or if the definition has changed.
    constructHistogram();
  }

  // Very simply use the histogram filling function
  m_histogram->Fill(azimuth, elevation, energy, weight);

  // After filling will need to re-optimize
  m_hasOptimizedSampling = false;
}

void Plenoptic3D::clear() {
  // Just delete the histogram so it is created again
  if (m_histogram != nullptr) {
    delete m_histogram;
    m_histogram = nullptr;
  }
}

void Plenoptic3D::setSurfaceScale(double surfaceScale) {
  m_surfaceScale = surfaceScale;
}

void Plenoptic3D::estimateSurfaceFluxes() {
  if (m_hasOptimizedSampling) {
    return;  // don't need to do anything.
  }

  if (m_surfaceAreas.size() != m_surfaceNormals.size()) {
    std::cerr << "Number of surface areas is different from the number of "
                 "normal vectors" << std::endl;
    throw;
  }

  m_surfaceFlux.clear();

  for (unsigned int surfaceIndex = 0; surfaceIndex < m_surfaceAreas.size();
       surfaceIndex++) {
    m_surfaceFlux.push_back(calculateSurfaceFlux(
        m_surfaceAreas[surfaceIndex], m_surfaceNormals[surfaceIndex]));
  }

  m_hasOptimizedSampling = true;
}

void Plenoptic3D::generate(G4Event* event, WeightedParticleGun* particleGun,
                           unsigned int number) {
  // Make sure that the surface sampling has been optimized already.
  if (!m_hasOptimizedSampling) {
    estimateSurfaceFluxes();
  }

  // Need to keep track of the number of particles created per surface for
  // weighting.
  std::map<int, int> particleCounts;

  for (unsigned int e = 0; e < m_surfaceFlux.size(); e++) {
    particleCounts[e] = 0;
  }

  std::piecewise_constant_distribution<> surfaceElementDistribution(
      m_surfaceID.begin(), m_surfaceID.end(), m_surfaceFlux.begin());

  std::uniform_real_distribution<> posFractionDist(0.0, 1.0);

  double totalHistogramWeight = m_histogram->Integral();

  // Handle generator seeding if necessary.
  if (m_seedChanged || m_generator == nullptr) {
    if (m_generator != nullptr) {
      delete m_generator;
      m_generator = nullptr;
    }

    std::seed_seq seedSequence(m_seedSequence.begin(), m_seedSequence.end());
    m_generator = new std::mt19937(seedSequence);
    m_seedChanged = false;
  }

  // Attempt to create particles on surfaces
  unsigned int successNumber = 0;
  std::vector<TVector3> photonStartingPositions;
  std::vector<TVector3> photonDirections;
  std::vector<double> photonEnergy;
  std::vector<double> photonWeightNumerators;
  std::vector<int> photonSelectedSurfaces;
  while (successNumber < number) {
    // Throw some random numbers first
    // Select a surface at random with importance sampling
    int selectedSurface = std::floor(surfaceElementDistribution(*m_generator));

    if (selectedSurface < 0 || selectedSurface >= (int)(m_surfaceFlux.size())) {
      std::cerr << "Selected surface number " << selectedSurface
                << " which does not exist!" << std::endl;
      throw;
    }

    // Keep track of the attempt numbers
    particleCounts[selectedSurface] = particleCounts[selectedSurface] + 1;

    // Get the plenoptic function random values
    double currentAzimuth = 0.0;
    double currentElevation = 0.0;
    double currentEnergy = 0.0;
    m_histogram->GetRandom3(currentAzimuth, currentElevation, currentEnergy);

    if (currentEnergy > 5.0 || currentEnergy < 0.2) {
      std::cerr << "oops " << std::endl;
      throw;
    }

    // Probably should triple check this!
    TVector3 currentLightVector(0.0, 1.0, 0.0);
    currentLightVector.RotateX(currentElevation);
    currentLightVector.RotateZ(currentAzimuth);
    currentLightVector =
        currentLightVector * -1.0;  // invert direction ( sun -> leaf )

    // Check that the angle with respect to the surface normal is < pi/2
    // (passing through the surface in the right direction)
    double angleBetweenVectors =
        currentLightVector.Angle(m_surfaceNormals[selectedSurface]);

    if (std::fabs(angleBetweenVectors) > M_PI / 2.0) {
      continue;
    }

    // Can now start saving information
    photonDirections.push_back(currentLightVector);
    photonEnergy.push_back(currentEnergy * eV);
    photonSelectedSurfaces.push_back(selectedSurface);

    // Generate a random position on the surface
    // Don't forget the need to scale it up
    TVector3 currentStartingPosition;
    currentStartingPosition.SetX(
        m_surfaceScale *
        getOrderedFractionalValue(posFractionDist(*m_generator),
                                  m_surfaces[selectedSurface].first.X(),
                                  m_surfaces[selectedSurface].second.X()));
    currentStartingPosition.SetY(
        m_surfaceScale *
        getOrderedFractionalValue(posFractionDist(*m_generator),
                                  m_surfaces[selectedSurface].first.Y(),
                                  m_surfaces[selectedSurface].second.Y()));
    currentStartingPosition.SetZ(
        m_surfaceScale *
        getOrderedFractionalValue(posFractionDist(*m_generator),
                                  m_surfaces[selectedSurface].first.Z(),
                                  m_surfaces[selectedSurface].second.Z()));

    photonStartingPositions.push_back(currentStartingPosition);

    // Calculate the weight numerator where
    // weight = totalHistogramWeight * |cos(DeltaAngle)| * area of surface /
    // total number of particles tried
    // Probably should triple check this!
    // Surface scale is in mm (hence the factor of 1000!)
    double currentWeightNumerator = totalHistogramWeight *
                                    std::fabs(std::cos(angleBetweenVectors)) *
                                    m_surfaceAreas[selectedSurface] *
                                    std::pow(m_surfaceScale / 1000.0, 2.0);

    photonWeightNumerators.push_back(currentWeightNumerator);

    successNumber++;
  }

  // Use the particle gun to add photons to the event
  for (unsigned int p = 0; p < photonStartingPositions.size(); p++) {
    // Set the direction of the photon
    particleGun->SetParticleMomentumDirection(
        G4ThreeVector(photonDirections[p].X(), photonDirections[p].Y(),
                      photonDirections[p].Z()));

    // Set the initial position
    particleGun->SetParticlePosition(G4ThreeVector(
        photonStartingPositions[p].X(), photonStartingPositions[p].Y(),
        photonStartingPositions[p].Z()));

    // Set the energy of the photon
    particleGun->SetParticleEnergy(photonEnergy[p]);

    // Randomize the polarization
    setRandomPhotonPolarisation(particleGun);

    // Finally add the photon to the event
    particleGun->GenerateWeightedPrimaryVertex(
        event,
        photonWeightNumerators[p] / particleCounts[photonSelectedSurfaces[p]]);
  }
}

TH2D* Plenoptic3D::getEnergyProjectedHistogram() const {
  if (m_histogram != nullptr) {
    return static_cast<TH2D*>(m_histogram->Project3D("yx"));
  }

  // No histogram to project!
  std::cerr
      << "No plenoptic histogram created. Therefore nothing can be projected."
      << std::endl;
  return nullptr;
}

void Plenoptic3D::append(const Plenoptic3D& source) {
  // Check configuration is the same for both, do they have
  // identical binning?
  bool hasDifferentBinning = false;
  int precision = 10;
  for (auto& axisType : {AZIMUTH, ELEVATION, ENERGY}) {
    if (m_binLowEdges[axisType].size() !=
        source.m_binLowEdges.at(axisType).size()) {
      hasDifferentBinning = true;
      break;
    }

    if (!std::equal(m_binLowEdges[axisType].begin(),
                    m_binLowEdges[axisType].end(),
                    source.m_binLowEdges.at(axisType).cbegin(),
                    [precision](double x, double y)
                        -> bool { return almost_equal(x, y, precision); })) {
      hasDifferentBinning = true;
      break;
    }
  }

  if (hasDifferentBinning) {
    std::cerr << "Plenoptic3D cannot be appended as the binning is different"
              << std::endl;
    throw;
  }

  // Everything seems consistant so add onto the histogram
  bool additionResult = m_histogram->Add(source.m_histogram);

  if (!additionResult) {
    std::cerr << "Plenoptic3D histogram not added succesfully for some reason."
              << std::endl;
    throw;
  }

  // Will need to re-calculate the optimized surface sampling
  m_hasOptimizedSampling = false;
}

void Plenoptic3D::setRandomNumberSeedSequence(std::vector<int> seedSequence) {
  // Store the sequence (and the fact it has changed)
  m_seedSequence = seedSequence;
  m_seedChanged = true;
}

void Plenoptic3D::setSurfaceGeometry() {
  // Set surface vertices
  m_surfaces.clear();
  m_surfaces.push_back(
      std::make_pair(TVector3(-0.5, -0.5, 1.0), TVector3(0.5, 0.5, 1.0)));
  m_surfaces.push_back(
      std::make_pair(TVector3(-0.5, -0.5, 0.0), TVector3(0.5, -0.5, 1.0)));
  m_surfaces.push_back(
      std::make_pair(TVector3(-0.5, 0.5, 0.0), TVector3(0.5, 0.5, 1.0)));
  m_surfaces.push_back(
      std::make_pair(TVector3(-0.5, -0.5, 0.0), TVector3(-0.5, 0.5, 1.0)));
  m_surfaces.push_back(
      std::make_pair(TVector3(0.5, -0.5, 0.0), TVector3(0.5, 0.5, 1.0)));

  // Set the surface normals
  m_surfaceNormals.clear();
  m_surfaceNormals.push_back(TVector3(0.0, 0.0, -1.0));
  m_surfaceNormals.push_back(TVector3(0.0, 1.0, 0.0));
  m_surfaceNormals.push_back(TVector3(0.0, -1.0, 0.0));
  m_surfaceNormals.push_back(TVector3(1.0, 0.0, 0.0));
  m_surfaceNormals.push_back(TVector3(-1.0, 0.0, 0.0));

  // Calculate the areas under certain assumptions
  m_surfaceAreas.clear();
  for (auto& surface : m_surfaces) {
    m_surfaceAreas.push_back(calculateSurfaceArea(surface));
  }

  // Set the surface IDs for random selection
  m_surfaceID.clear();
  for (unsigned int id = 0; id < m_surfaces.size(); id++) {
    m_surfaceID.push_back(id);
  }
  // For piecewise constant distribution you need an extra bin.
  m_surfaceID.push_back(m_surfaceID.back() + 1);
}

void Plenoptic3D::constructHistogram() {
  if (m_histogram != nullptr) {
    // Always recreate
    delete m_histogram;
    m_histogram = nullptr;
  }

  // Create arrays for each axis
  double* azimuthBinLowEdgesArray = new double[m_binLowEdges[AZIMUTH].size()];
  std::copy(m_binLowEdges[AZIMUTH].begin(), m_binLowEdges[AZIMUTH].end(),
            azimuthBinLowEdgesArray);

  double* elevationBinLowEdgesArray =
      new double[m_binLowEdges[ELEVATION].size()];
  std::copy(m_binLowEdges[ELEVATION].begin(), m_binLowEdges[ELEVATION].end(),
            elevationBinLowEdgesArray);

  double* energyBinLowEdgesArray = new double[m_binLowEdges[ENERGY].size()];
  std::copy(m_binLowEdges[ENERGY].begin(), m_binLowEdges[ENERGY].end(),
            energyBinLowEdgesArray);

  m_histogram =
      new TH3D("plenoptic3D", "plenoptic3D", m_binLowEdges[AZIMUTH].size() - 1,
               azimuthBinLowEdgesArray, m_binLowEdges[ELEVATION].size() - 1,
               elevationBinLowEdgesArray, m_binLowEdges[ENERGY].size() - 1,
               energyBinLowEdgesArray);

  // Tell root we will manage the memory
  m_histogram->SetDirectory(0);

  m_histogramDefinitionChanged = false;
  m_hasOptimizedSampling = false;

  // Clean up
  delete[] azimuthBinLowEdgesArray;
  delete[] elevationBinLowEdgesArray;
  delete[] energyBinLowEdgesArray;
}

double Plenoptic3D::calculateSurfaceFlux(double surfaceArea,
                                         TVector3 normal) const {
  double flux = 0.0;

  // Iterate over the plenoptic function
  for (int bx = 1; bx < m_histogram->GetNbinsX() + 1; bx++) {
    // Get the current azimuth angle
    double currentAzimuth = m_histogram->GetXaxis()->GetBinCenter(bx);

    for (int by = 1; by < m_histogram->GetNbinsY() + 1; by++) {
      // Get the current elevation angle then build the light vector
      double currentElevation = m_histogram->GetYaxis()->GetBinCenter(by);

      TVector3 currentLightVector(0.0, 1.0, 0.0);
      currentLightVector.RotateX(currentElevation);
      currentLightVector.RotateZ(currentAzimuth);
      currentLightVector =
          currentLightVector * -1.0;  // invert direction ( sun -> leaf )

      // Check if anglular difference with respect to normal vector is
      // within pi/2
      double angleBetweenVectors = currentLightVector.Angle(normal);

      if (std::fabs(angleBetweenVectors) > M_PI / 2.0) {
        continue;
      }

      double angleFactor =
          std::fabs(std::cos(angleBetweenVectors)) * surfaceArea;

      // Finally sum up the energy
      for (int bz = 1; bz < m_histogram->GetNbinsZ() + 1; bz++) {
        double currentEnergy = m_histogram->GetZaxis()->GetBinCenter(bz);
        double currentWeight = m_histogram->GetBinContent(bx, by, bz);

        flux += currentEnergy * currentWeight * angleFactor;
      }
    }
  }

  return flux;
}

double Plenoptic3D::calculateSurfaceArea(
    std::pair<TVector3, TVector3> surface) const {
  // Make assumption that vertex pair is axially aligned plane surface
  std::vector<double> deltas;
  deltas.push_back(std::fabs(surface.second.X() - surface.first.X()));
  deltas.push_back(std::fabs(surface.second.Y() - surface.first.Y()));
  deltas.push_back(std::fabs(surface.second.Z() - surface.first.Z()));

  // Use the two biggest values (don't know which axis we are aligned to)
  std::sort(deltas.begin(), deltas.end());
  double area = deltas[1] * deltas[2];

  return area;
}

double Plenoptic3D::deltaAzimuth(double angle1, double angle2) const {
  double delta = std::fabs(angle1 - angle2);

  if (delta > M_PI) {
    delta = 2.0 * M_PI - delta;
  }

  return delta;
}

double Plenoptic3D::wrapAngle(double angle) const {
  double wrappedAngle = std::remainder(angle, 2.0 * M_PI);

  return wrappedAngle;
}

double Plenoptic3D::getOrderedFractionalValue(double fraction, double valueA,
                                              double valueB) const {
  double minimum, maximum;

  if (valueA < valueB) {
    minimum = valueA;
    maximum = valueB;
  } else {
    minimum = valueB;
    maximum = valueA;
  }

  return fraction * (maximum - minimum) + minimum;
}

void Plenoptic3D::setRandomPhotonPolarisation(
    WeightedParticleGun* particleGun) const {
  G4double angle = G4UniformRand() * 360.0 * deg;

  G4ThreeVector normal(1.0, 0.0, 0.0);
  G4ThreeVector kphoton = particleGun->GetParticleMomentumDirection();
  G4ThreeVector product = normal.cross(kphoton);
  G4double modul2 = product * product;

  G4ThreeVector e_perpend(0.0, 0.0, 1.0);
  if (modul2 > 0.) e_perpend = (1. / std::sqrt(modul2)) * product;
  G4ThreeVector e_paralle = e_perpend.cross(kphoton);

  G4ThreeVector polar =
      std::cos(angle) * e_paralle + std::sin(angle) * e_perpend;

  particleGun->SetParticlePolarization(polar);
}
