#include "full/leafTrackerHit.hpp"
#include "G4UnitsTable.hh"
#include "G4VVisManager.hh"
#include "G4Circle.hh"
#include "G4Colour.hh"
#include "G4VisAttributes.hh"

#include <iomanip>

G4ThreadLocal G4Allocator<LeafTrackerHit>* LeafTrackerHitAllocator=0;

LeafTrackerHit::LeafTrackerHit() : 
  G4VHit(), 
  m_trackID(-1),
  m_chamberNumber(-1),
  m_energyDeposited(0.0),
  m_position(0.0, 0.0, 0.0){

}

LeafTrackerHit::LeafTrackerHit(const LeafTrackerHit& leafTrackerHit) : G4VHit() {
  m_trackID         = leafTrackerHit.m_trackID;
  m_chamberNumber   = leafTrackerHit.m_chamberNumber;
  m_energyDeposited = leafTrackerHit.m_energyDeposited;
  m_position        = leafTrackerHit.m_position;
}

LeafTrackerHit::~LeafTrackerHit() {}

const LeafTrackerHit& LeafTrackerHit::operator=(const LeafTrackerHit& right)
{
  m_trackID         = right.m_trackID;
  m_chamberNumber   = right.m_chamberNumber;
  m_energyDeposited = right.m_energyDeposited;
  m_position        = right.m_position;

  return *this;
}

G4int LeafTrackerHit::operator==(const LeafTrackerHit& right) const
{
  return ( this == &right ) ? 1 : 0;
}


//Base class methods
void LeafTrackerHit::Draw() {
  G4VVisManager* pVVisManager = G4VVisManager::GetConcreteInstance();
  if(pVVisManager)
  {
    G4Circle circle(m_position);
    circle.SetScreenSize(4.);
    circle.SetFillStyle(G4Circle::filled);
    G4Colour colour(1.,0.,0.);
    G4VisAttributes attribs(colour);
    circle.SetVisAttributes(attribs);
    pVVisManager->Draw(circle);
  }
}

void LeafTrackerHit::Print() {

  G4cout << "Track ID: " << m_trackID << " Chamber Number: " << m_chamberNumber
	 << " Energy Deposited: " << std::setw(7) << G4BestUnit(m_energyDeposited, "Energy")
	 << "Position: " << std::setw(7) << G4BestUnit(m_position, "Length")
	 << G4endl;
  
}


//Setters
void LeafTrackerHit::setTrackID(G4int trackID) {
  m_trackID = trackID;
}

void LeafTrackerHit::setChamberNumber(G4int chamberNumber) {
  m_chamberNumber = chamberNumber;
}

void LeafTrackerHit::setEnergyDeposited(G4double energy) {
  m_energyDeposited = energy;
}

void LeafTrackerHit::setPosition(G4ThreeVector position) {
  m_position = position;
}

  
//Getters
G4int LeafTrackerHit::getTrackID() {
  return m_trackID;
}

G4int LeafTrackerHit::getChamberNumber() {
  return m_chamberNumber;
}

G4double LeafTrackerHit::getEnergyDeposited() {
  return m_energyDeposited;
}

G4ThreeVector LeafTrackerHit::getPosition() {
  return m_position;
}


