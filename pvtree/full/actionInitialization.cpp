#include "pvtree/full/actionInitialization.hpp"
#include "pvtree/full/runAction.hpp"
#include "pvtree/full/eventAction.hpp"
#include "pvtree/full/steppingAction.hpp"
#include "pvtree/full/recorders/recorderBase.hpp"
#include "pvtree/full/solarSimulation/sun.hpp"

ActionInitialization::ActionInitialization(
    RecorderBase* recorder,
    std::function<G4VUserPrimaryGeneratorAction*()> primaryGenerator)
    : G4VUserActionInitialization(),
      m_recorder(recorder),
      m_primaryGenerator(primaryGenerator) {}

ActionInitialization::~ActionInitialization() {}

void ActionInitialization::BuildForMaster() const {
  SetUserAction(new RunAction(m_recorder));
}

void ActionInitialization::Build() const {
  SetUserAction(m_primaryGenerator());
  SetUserAction(new RunAction(m_recorder));
  SetUserAction(new EventAction(m_recorder));
  SetUserAction(new SteppingAction());
}
