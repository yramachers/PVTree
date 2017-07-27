#include "pvtree/full/recorders/convergenceRecorder.hpp"
#include "pvtree/full/leafTrackerHit.hpp"
#include "G4Run.hh"
#include "G4Event.hh"
#include "G4SystemOfUnits.hh"

ConvergenceRecorder::ConvergenceRecorder() : RecorderBase(), m_eventAborted(false) {

}

ConvergenceRecorder::~ConvergenceRecorder() {

}

void ConvergenceRecorder::recordBeginOfRun(const G4Run* /*run*/) {
  
  // Extend results array for this new run
  m_photons.push_back(           std::vector<long>() );
  m_hits.push_back(              std::vector<long>() );
  m_summedHitEnergies.push_back( std::vector<double>() );

  m_eventAborted = false;
}

void ConvergenceRecorder::recordEndOfRun(const G4Run* /*run*/) {

}

void ConvergenceRecorder::recordBeginOfEvent(const G4Event* event) {

  // Store total number of photons being generated
  G4int numberOfPhotons = event->GetNumberOfPrimaryVertex();
  m_photons.back().push_back( numberOfPhotons );
}

void ConvergenceRecorder::recordEndOfEvent(const G4Event* event) {
  
  // Check if event aborted 
  if ( event->IsAborted() ) {
    m_eventAborted = true;
  }

  // Store the total number of hits
  G4VHitsCollection* hitCollection = event->GetHCofThisEvent()->GetHC(0);
  m_hits.back().push_back( hitCollection->GetSize() );

  // Store the total energy deposited in hits
  double energyDeposited = 0.0;
  for (unsigned int h=0; h<hitCollection->GetSize(); h++) {
    LeafTrackerHit* hit = static_cast<LeafTrackerHit*>(hitCollection->GetHit(h));
    energyDeposited += hit->getEnergyDeposited();
  }

    // The units of energy deposited is [W]
  m_summedHitEnergies.back().push_back( energyDeposited );
}

void ConvergenceRecorder::reset() {
  m_photons.clear();
  m_hits.clear();
  m_summedHitEnergies.clear();
  m_eventAborted = false;
}

std::vector<std::vector<long > > ConvergenceRecorder::getPhotonCounts() {
  return m_photons;
}

std::vector<std::vector<long > > ConvergenceRecorder::getHitCounts() {
  return m_hits;
}

std::vector<std::vector<double > > ConvergenceRecorder::getSummedHitEnergies() {
  return m_summedHitEnergies;
}

bool ConvergenceRecorder::wasAborted() {
  return m_eventAborted;
}
