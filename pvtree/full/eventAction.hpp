#ifndef PV_FULL_EVENT_ACTION
#define PV_FULL_EVENT_ACTION

#include "G4UserEventAction.hh"

class G4Event;
class RecorderBase;

class EventAction : public G4UserEventAction {
private:
  int           m_verbosityLevel;
  RecorderBase* m_recorder;

public:
  EventAction(RecorderBase* recorder, int verbosityLevel = 0);
  virtual ~EventAction();

  virtual void BeginOfEventAction(const G4Event* event);
  virtual void EndOfEventAction(const G4Event* event);

};


#endif //PV_FULL_EVENT_ACTION
