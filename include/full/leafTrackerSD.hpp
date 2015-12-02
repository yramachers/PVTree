#ifndef LEAF_TRACKER_SD_HPP
#define LEAF_TRACKER_SD_HPP

#include "G4VSensitiveDetector.hh"

#include "leafTrackerHit.hpp"

#include <vector>

class G4Step;
class G4HCofThisEvent;

/* Leaf tracker sensitive detector class
//
 */

class LeafTrackerSD : public G4VSensitiveDetector
{
public:
  LeafTrackerSD(const G4String& name, const G4String& hitsCollectionName);
  virtual ~LeafTrackerSD();

  // methods from base class
  virtual void   Initialize(G4HCofThisEvent* hitCollection);
  virtual G4bool ProcessHits(G4Step* step, G4TouchableHistory* history);
  virtual void   EndOfEvent(G4HCofThisEvent* hitCollection);
  G4bool ProcessHits_user(const G4Step* step, G4TouchableHistory* history);

private:
  LeafTrackerHitsCollection* m_hitsCollection;
};


#endif //LEAF_TRACKER_SD_HPP
