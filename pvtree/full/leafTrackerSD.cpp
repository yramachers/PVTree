#include "pvtree/full/leafTrackerSD.hpp"
#include "G4SDManager.hh"

LeafTrackerSD::LeafTrackerSD(const G4String& name,
                             const G4String& hitsCollectionName)
    : G4VSensitiveDetector(name), m_hitsCollection(NULL) {
  collectionName.insert(hitsCollectionName);
}

LeafTrackerSD::~LeafTrackerSD() {}

// Base class methods
void LeafTrackerSD::Initialize(G4HCofThisEvent* eventHitCollection) {
  // Create a collection to store the hits
  if (m_hitsCollection) m_hitsCollection = 0;
  m_hitsCollection =
      new LeafTrackerHitsCollection(SensitiveDetectorName, collectionName[0]);

  // Add collection to the event hit collection
  G4int hitCollectionID =
      G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
  eventHitCollection->AddHitsCollection(hitCollectionID, m_hitsCollection);
}

G4bool LeafTrackerSD::ProcessHits(G4Step* /*step*/,
                                  G4TouchableHistory* /*history*/) {
  return false;
}

G4bool LeafTrackerSD::ProcessHits_user(const G4Step* step,
                                       G4TouchableHistory* /*history*/) {
  // Assume the weight contains all the energy the
  // 'photon' is representing.
  G4double energyDeposit = step->GetTrack()->GetWeight();

  if (energyDeposit == 0.0) return false;

  //  step->GetTrack()->SetWeight(0.0); // into absorber - gone.

  // Create a hit object storing all the information
  LeafTrackerHit* hit = new LeafTrackerHit();

  hit->setTrackID(step->GetTrack()->GetTrackID());
  hit->setChamberNumber(
      step->GetPreStepPoint()->GetTouchableHandle()->GetCopyNumber());
  hit->setEnergyDeposited(energyDeposit);
  hit->setPosition(step->GetPostStepPoint()->GetPosition());

  m_hitsCollection->insert(hit);

  return true;
}

void LeafTrackerSD::EndOfEvent(G4HCofThisEvent* /*eventHitCollection*/) {
  if (verboseLevel > 1) {
    G4int numberOfHits = m_hitsCollection->entries();
    G4cout << G4endl << "-------->Hits Collection: in this event there are "
           << numberOfHits << " hits in the tracker chambers: " << G4endl;
    for (G4int i = 0; i < numberOfHits; i++) (*m_hitsCollection)[i]->Print();
  }
}
