#include "full/actionInitialization.hpp"
#include "full/runAction.hpp"
#include "full/eventAction.hpp"
#include "full/steppingAction.hpp"
#include "recorders/recorderBase.hpp"
#include "solarSimulation/sun.hpp"

ActionInitialization::ActionInitialization(RecorderBase* recorder, 
					   std::function<G4VUserPrimaryGeneratorAction*()> primaryGenerator) : 
  G4VUserActionInitialization(),  
  m_recorder(recorder),
  m_primaryGenerator(primaryGenerator) {}


ActionInitialization::~ActionInitialization() {}

void ActionInitialization::BuildForMaster() const{
  SetUserAction(new RunAction(m_recorder));
}

void ActionInitialization::Build() const{
  SetUserAction( m_primaryGenerator() );
  SetUserAction(new RunAction(m_recorder));
  SetUserAction(new EventAction(m_recorder));
  //SetUserAction(new SteppingAction());
}
