#ifndef LEAF_TRACKER_HIT_HPP
#define LEAF_TRACKER_HIT_HPP

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"
#include "tls.hh"

/* Leaf tracker hit class
//
 */

class LeafTrackerHit : public G4VHit
{
public:
  LeafTrackerHit();
  LeafTrackerHit(const LeafTrackerHit&);
  virtual ~LeafTrackerHit();

  const LeafTrackerHit& operator=(const LeafTrackerHit&);
  G4int operator==(const LeafTrackerHit&) const;
  
  inline void* operator new(size_t);
  inline void  operator delete(void*);

  // methods from base class
  virtual void Draw();
  virtual void Print();

  //Setters
  void setTrackID(G4int trackID);    
  void setChamberNumber(G4int chamberNumber);
  void setEnergyDeposited(G4double energy);
  void setPosition(G4ThreeVector position);
  
  //Getters
  G4int         getTrackID();
  G4int         getChamberNumber();
  G4double      getEnergyDeposited();
  G4ThreeVector getPosition();

private:
  G4int         m_trackID;
  G4int         m_chamberNumber;
  G4double      m_energyDeposited;
  G4ThreeVector m_position;
};


typedef G4THitsCollection<LeafTrackerHit> LeafTrackerHitsCollection;

extern  G4ThreadLocal G4Allocator<LeafTrackerHit>* LeafTrackerHitAllocator;

inline void* LeafTrackerHit::operator new(size_t)
{
  if(!LeafTrackerHitAllocator)
      LeafTrackerHitAllocator = new G4Allocator<LeafTrackerHit>;
  return static_cast<void *>(LeafTrackerHitAllocator->MallocSingle());
}


inline void LeafTrackerHit::operator delete(void *hit)
{
  LeafTrackerHitAllocator->FreeSingle( static_cast<LeafTrackerHit*>(hit) );
}


#endif //LEAF_TRACKER_HIT_HPP
