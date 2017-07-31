#include "pvtree/full/eventAction.hpp"
#include "pvtree/full/recorders/recorderBase.hpp"
#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4TrajectoryContainer.hh"
#include "G4Trajectory.hh"
#include "G4ios.hh"

EventAction::EventAction(RecorderBase* recorder, int verbosityLevel)
    : G4UserEventAction(),
      m_verbosityLevel(verbosityLevel),
      m_recorder(recorder) {}
EventAction::~EventAction() {}

void EventAction::BeginOfEventAction(const G4Event* event) {
  m_recorder->recordBeginOfEvent(event);
}

void EventAction::EndOfEventAction(const G4Event* event) {
  // Periodic printing
  if (m_verbosityLevel > 0) {
    // Get number of stored trajectories
    G4TrajectoryContainer* trajectoryContainer =
        event->GetTrajectoryContainer();
    G4int n_trajectories = 0;
    if (trajectoryContainer) n_trajectories = trajectoryContainer->entries();

    G4int eventID = event->GetEventID();

    if (eventID < 100 || eventID % 100 == 0) {
      G4cout << ">>> Event: " << eventID << G4endl;

      if (trajectoryContainer) {
        G4cout << "    " << n_trajectories
               << " trajectories stored in this event." << G4endl;
      }
      G4cout << "    " << event->GetHCofThisEvent()->GetNumberOfCollections()
             << " hit collection(s) in event." << G4endl;

      G4VHitsCollection* hc = event->GetHCofThisEvent()->GetHC(0);
      G4cout << "    " << hc->GetSize() << " hit(s) stored in this event"
             << G4endl;
    }
  }

  // Analyse complete event
  m_recorder->recordEndOfEvent(event);
}
