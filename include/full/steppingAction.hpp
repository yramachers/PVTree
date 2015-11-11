#ifndef PVTREE_FULL_STEPPING_ACTION_HPP
#define PVTREE_FULL_STEPPING_ACTION_HPP

#include "globals.hh"
#include "G4UserSteppingAction.hh"
#include "G4OpBoundaryProcess.hh"

class SteppingAction : public G4UserSteppingAction
{
  public:

  SteppingAction();
  virtual ~SteppingAction();
  virtual void UserSteppingAction(const G4Step* step);
  
  void SetOneStepPrimaries(G4bool usesOneStepPrimaries);
  G4bool GetOneStepPrimaries();
  
private:
  
  G4bool m_oneStepPrimaries;
  G4OpBoundaryProcessStatus m_expectedNextStatus;
};

#endif //PVTREE_FULL_STEPPING_ACTION_HPP
