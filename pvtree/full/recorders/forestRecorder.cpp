#include "pvtree/full/recorders/forestRecorder.hpp"
#include "pvtree/full/leafTrackerHit.hpp"
#include "G4Run.hh"
#include "G4Event.hh"
#include "G4SystemOfUnits.hh"

ForestRecorder::ForestRecorder()
    : RecorderBase(), m_eventAborted(false) {}

ForestRecorder::~ForestRecorder() {}

void ForestRecorder::recordBeginOfRun(const G4Run* /*run*/) {
  // Extend results array for this new run
  m_photons.push_back(std::vector<long>());
  m_hits.push_back(std::vector<long>());
  m_summedHitEnergies.push_back(std::vector<std::unordered_map<unsigned int, 
                                                               double>>());
  m_eventAborted = false;
}

void ForestRecorder::recordEndOfRun(const G4Run* /*run*/) {}

void ForestRecorder::recordBeginOfEvent(const G4Event* event) {
  // Store total number of photons being generated
  G4int numberOfPhotons = event->GetNumberOfPrimaryVertex();
  m_photons.back().push_back(numberOfPhotons);
}

void ForestRecorder::recordEndOfEvent(const G4Event* event) {
  // Check if event aborted
  if (event->IsAborted()) {
    m_eventAborted = true;
  }

  // Store the total number of hits
  G4VHitsCollection* hitCollection = event->GetHCofThisEvent()->GetHC(0);
  m_hits.back().push_back(hitCollection->GetSize());

  // Store the total energy deposited in hits
  std::unordered_map<unsigned int, double> energyDeposited;
  for (unsigned int h = 0; h < hitCollection->GetSize(); h++) {
    LeafTrackerHit* hit =
        static_cast<LeafTrackerHit*>(hitCollection->GetHit(h));
    auto treeNumber = hit->getTreeNumber();
    auto energy = hit->getEnergyDeposited();
    auto wasInserted = energyDeposited.insert({treeNumber, energy});
    if (wasInserted.second == false) {
      energyDeposited[treeNumber] += energy;
    }
  }

  // The units of energy deposited is [W]
  m_summedHitEnergies.back().push_back(energyDeposited);
}

void ForestRecorder::reset() {
  m_photons.clear();
  m_hits.clear();
  m_summedHitEnergies.clear();
  m_eventAborted = false;
}

std::vector<std::vector<long> > ForestRecorder::getPhotonCounts() {
  return m_photons;
}

std::vector<std::vector<long> > ForestRecorder::getHitCounts() {
  return m_hits;
}

std::vector<std::vector<std::unordered_map<unsigned int, double> > > ForestRecorder::getSummedHitEnergies() {
  return m_summedHitEnergies;
}

bool ForestRecorder::wasAborted() { return m_eventAborted; }
