#include "pvtree/full/solarSimulation/plenoptic1D.hpp"

#include <iostream>
#include <cmath>
#include <random>
#include <map>

#include "TH1D.h"

ClassImp(Plenoptic1D)

    Plenoptic1D::Plenoptic1D()
    : m_histogram(nullptr),
      m_totalSurfaceFlux(0.0),
      m_hasSurfaceGeometry(false) {}

Plenoptic1D::Plenoptic1D(int binNumber)
    : m_totalSurfaceFlux(0.0), m_hasSurfaceGeometry(false) {
  m_histogram =
      new TH1D("plenoptic1D", "plenoptic", binNumber, 0.0, 2.0 * M_PI);

  // I will manage the memory
  m_histogram->SetDirectory(0);
}

Plenoptic1D::Plenoptic1D(int binNumber, double minValue, double maxValue)
    : m_totalSurfaceFlux(0.0), m_hasSurfaceGeometry(false) {
  m_histogram =
      new TH1D("plenoptic1D", "plenoptic", binNumber, minValue, maxValue);

  // I will manage the memory
  m_histogram->SetDirectory(0);
}

Plenoptic1D::Plenoptic1D(std::vector<double> binLowEdges)
    : m_totalSurfaceFlux(0.0), m_hasSurfaceGeometry(false) {
  double* binLowEdgesArray = new double[binLowEdges.size()];
  std::copy(binLowEdges.begin(), binLowEdges.end(), binLowEdgesArray);

  m_histogram = new TH1D("plenoptic1D", "plenoptic", binLowEdges.size() - 1,
                         binLowEdgesArray);

  // I will manage the memory
  m_histogram->SetDirectory(0);

  delete[] binLowEdgesArray;
}

Plenoptic1D::~Plenoptic1D() {
  if (m_histogram != nullptr) {
    delete m_histogram;
  }
}

void Plenoptic1D::fill(double angle, double value) {
  // Very simple!
  m_histogram->Fill(angle, value);
}

void Plenoptic1D::clear() {
  // Clean up the histogram
  m_histogram->Reset();
}

void Plenoptic1D::setSurfaceGeometry(
    std::vector<std::tuple<double, double> > vertexPositions) {
  // Store a copy of the vertices
  m_vertexPositions = vertexPositions;

  // Clean up in case this is called twice
  m_surfaceElementFlux.clear();
  m_surfaceElementID.clear();
  m_surfaceElementLengths.clear();
  m_surfaceElementAngles.clear();
  m_totalSurfaceFlux = 0.0;

  // Pre-process surface for generation step
  // Integrate the energy traversing each surface
  for (unsigned int v = 0; v < vertexPositions.size() - 1; v++) {
    double flux = calculateElementFlux(std::get<0>(vertexPositions[v]),
                                       std::get<1>(vertexPositions[v]),
                                       std::get<0>(vertexPositions[v + 1]),
                                       std::get<1>(vertexPositions[v + 1]));
    m_surfaceElementID.push_back(v);
    m_surfaceElementFlux.push_back(flux);

    m_totalSurfaceFlux += flux;

    // Store the length as well
    double length = calculateElementLength(std::get<0>(vertexPositions[v]),
                                           std::get<1>(vertexPositions[v]),
                                           std::get<0>(vertexPositions[v + 1]),
                                           std::get<1>(vertexPositions[v + 1]));
    m_surfaceElementLengths.push_back(length);

    // Store the angle as well
    double angle = calculateElementAngle(std::get<0>(vertexPositions[v]),
                                         std::get<1>(vertexPositions[v]),
                                         std::get<0>(vertexPositions[v + 1]),
                                         std::get<1>(vertexPositions[v + 1]));
    m_surfaceElementAngles.push_back(angle);
  }

  // For piecewise constant distribution you need an extra bin.
  m_surfaceElementID.push_back(m_surfaceElementID.back() + 1);

  m_hasSurfaceGeometry = true;
}

std::vector<std::tuple<double, double, double, double> > Plenoptic1D::generate(
    unsigned int number /* = 1*/) const {
  std::vector<std::tuple<double, double, double, double> > particles;
  std::vector<std::tuple<double, double, double, double, int> >
      particlesPerElement;
  std::map<int, int> elementParticleCounts;

  for (unsigned int e = 0; e < m_surfaceElementFlux.size(); e++) {
    elementParticleCounts[e] = 0;
  }

  if (!m_hasSurfaceGeometry) {
    std::cerr << "Unable to produce any particles if no surface is specified."
              << std::endl;
    return particles;
  }

  // Use non-deterministic random device to seed a PRNG
  std::random_device rd;
  std::mt19937 gen(rd());

  std::piecewise_constant_distribution<> surfaceElementDistribution(
      m_surfaceElementID.begin(), m_surfaceElementID.end(),
      m_surfaceElementFlux.begin());

  std::uniform_real_distribution<> initialPositionDistribution(0.0, 1.0);

  double totalEnergy = m_histogram->Integral();

  // Generate on surface according to plenoptic function
  unsigned int n = 0;
  while (n < number) {
    // Select a surface at random with importance sampling
    int selectedSurface = std::floor(surfaceElementDistribution(gen));

    if (selectedSurface < 0 ||
        selectedSurface >= (int)(m_vertexPositions.size()) - 1) {
      std::cerr << "Selected surface number " << selectedSurface
                << " which does not exist!" << std::endl;
      throw;
    }

    // Choose an initial position
    double fractionAlongSurface = initialPositionDistribution(gen);
    double currentX = (std::get<0>(m_vertexPositions[selectedSurface + 1]) -
                       std::get<0>(m_vertexPositions[selectedSurface])) *
                          fractionAlongSurface +
                      std::get<0>(m_vertexPositions[selectedSurface]);
    double currentY = (std::get<1>(m_vertexPositions[selectedSurface + 1]) -
                       std::get<1>(m_vertexPositions[selectedSurface])) *
                          fractionAlongSurface +
                      std::get<1>(m_vertexPositions[selectedSurface]);

    // Choose a particle angle
    double currentAngle = m_histogram->GetRandom();

    // Count number of particles trialed
    elementParticleCounts[selectedSurface] =
        elementParticleCounts[selectedSurface] + 1;

    // Only use if within 90 degrees of the normal direction of the surface
    if (deltaTheta(currentAngle, m_surfaceElementAngles[selectedSurface]) <
        M_PI / 2.0) {
      // Calculate weight = totalEnergy * |cos(DeltaAngle)| * length of surface
      // / total number of particles
      double weight =
          totalEnergy *
          std::fabs(std::cos(currentAngle -
                             m_surfaceElementAngles[selectedSurface])) *
          m_surfaceElementLengths[selectedSurface];
      // no longer divide by number here

      // Create the tuple
      particlesPerElement.push_back(std::make_tuple(
          currentX, currentY, currentAngle, weight, selectedSurface));

      n++;
    }
  }

  // Weight by number of particles per surface
  for (auto& particle : particlesPerElement) {
    particles.push_back(std::make_tuple(
        std::get<0>(particle), std::get<1>(particle), std::get<2>(particle),
        std::get<3>(particle) / elementParticleCounts[std::get<4>(particle)]));
  }

  return particles;
}

double Plenoptic1D::calculateElementFlux(double x1, double y1, double x2,
                                         double y2) const {
  double flux = 0.0;

  // Calculate the length
  double elementLength = calculateElementLength(x1, y1, x2, y2);

  // and get the element normal
  double elementAngle = calculateElementAngle(x1, y1, x2, y2);

  // Iterate over plenoptic function
  for (int b = 1; b < m_histogram->GetNbinsX() + 1; b++) {
    double angle = m_histogram->GetBinCenter(b);
    double energy = m_histogram->GetBinContent(b);

    // Only add to flux that is within 90 degrees of the normal direction
    if (deltaTheta(angle, elementAngle) < M_PI / 2.0) {
      flux +=
          elementLength * std::fabs(std::cos(angle - elementAngle)) * energy;
    }
  }

  return flux;
}

double Plenoptic1D::calculateElementLength(double x1, double y1, double x2,
                                           double y2) const {
  // Calculate the length
  double elementLength =
      std::sqrt(std::pow(x2 - x1, 2.0) + std::pow(y2 - y1, 2.0));

  return elementLength;
}

double Plenoptic1D::calculateElementAngle(double x1, double y1, double x2,
                                          double y2) const {
  // Calculate the angle
  double elementAngle = std::atan2((x2 - x1), (y2 - y1));

  // Convert to the normal
  elementAngle -= M_PI / 2.0;

  return wrapAngle(elementAngle);
}

double Plenoptic1D::deltaTheta(double angle1, double angle2) const {
  double delta = std::fabs(angle1 - angle2);

  if (delta > M_PI) {
    delta = 2.0 * M_PI - delta;
  }

  return delta;
}

double Plenoptic1D::wrapAngle(double angle) const {
  double wrappedAngle = std::remainder(angle, 2.0 * M_PI);

  return wrappedAngle;
}
