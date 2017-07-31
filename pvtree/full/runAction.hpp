#ifndef PV_FULL_RUN_ACTION
#define PV_FULL_RUN_ACTION

#include "G4UserRunAction.hh"

class G4Run;
class RecorderBase;

class RunAction : public G4UserRunAction {
 public:
  explicit RunAction(RecorderBase* recorder);
  virtual ~RunAction();
  virtual void BeginOfRunAction(const G4Run*);
  virtual void EndOfRunAction(const G4Run*);

 private:
  RecorderBase* m_recorder;
};

#endif  // PV_FULL_RUN_ACTION
