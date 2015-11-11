#include "full/runAction.hpp"
#include "recorders/recorderBase.hpp"
#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4TransportationManager.hh"

RunAction::RunAction(RecorderBase* recorder) : G4UserRunAction(),
					       m_recorder(recorder) {}

RunAction::~RunAction() {}

void RunAction::BeginOfRunAction(const G4Run* run) {
  //inform the runManager to save random number seed
  G4RunManager::GetRunManager()->SetRandomNumberStore(false);

  // Turn off push messages for now
  G4TransportationManager::GetTransportationManager()->GetNavigator("World")->SetPushVerbosity(false);

  //perform analysis
  m_recorder->recordBeginOfRun(run);
}

void RunAction::EndOfRunAction(const G4Run* run) {
  //perform analysis
  m_recorder->recordEndOfRun(run);
}


