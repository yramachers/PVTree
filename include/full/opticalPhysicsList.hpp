#ifndef PV_FULL_OPTICAL_PHYSICS_LIST
#define PV_FULL_OPTICAL_PHYSICS_LIST

#include "G4VUserPhysicsList.hh"

class OpticalPhysicsList : public G4VUserPhysicsList {
private:
  bool m_verbosityLevel;

public:

  OpticalPhysicsList();
  virtual ~OpticalPhysicsList();

protected:
  virtual void ConstructParticle(); 
  virtual void ConstructProcess();

  virtual void AddTransportation();

public:
  virtual void SetCuts();
};



#endif //PV_FULL_OPTICAL_PHYSICS_LIST
