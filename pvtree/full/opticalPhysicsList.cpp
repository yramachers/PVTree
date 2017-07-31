#include "pvtree/full/opticalPhysicsList.hpp"
#include "G4OpAbsorption.hh"
#include "G4OpBoundaryProcess.hh"
#include "G4OpRayleigh.hh"
#include "G4ParticleTypes.hh"
#include "G4ParticleTable.hh"
#include "G4ProcessManager.hh"

OpticalPhysicsList::OpticalPhysicsList()
    : G4VUserPhysicsList(), m_verbosityLevel(0) {}

OpticalPhysicsList::~OpticalPhysicsList() {}

void OpticalPhysicsList::ConstructParticle() {
  // Just want optical photons
  G4OpticalPhoton::OpticalPhotonDefinition();
}

void OpticalPhysicsList::ConstructProcess() {
  AddTransportation();

  // optical processes
  G4bool theAbsorptionProcessNeverUsed = true;
  G4OpAbsorption* theAbsorptionProcess = new G4OpAbsorption();
  G4bool theBoundaryProcessNeverUsed = true;
  G4OpBoundaryProcess* theBoundaryProcess = new G4OpBoundaryProcess();
  G4bool theRayleighProcessNeverUsed = true;
  G4OpRayleigh* rayleighScatteringProcess = new G4OpRayleigh();
  theAbsorptionProcess->SetVerboseLevel(m_verbosityLevel);
  theBoundaryProcess->SetVerboseLevel(m_verbosityLevel);
  rayleighScatteringProcess->SetVerboseLevel(m_verbosityLevel);

  auto theParticleIterator = GetParticleIterator();
  theParticleIterator->reset();
  while ((*(theParticleIterator))()) {
    G4ParticleDefinition* particle = theParticleIterator->value();
    G4ProcessManager* pmanager = particle->GetProcessManager();
    G4String particleName = particle->GetParticleName();

    if (particleName == "opticalphoton") {
      pmanager->AddDiscreteProcess(theAbsorptionProcess);
      pmanager->AddDiscreteProcess(theBoundaryProcess);
      pmanager->AddDiscreteProcess(rayleighScatteringProcess);
      theAbsorptionProcessNeverUsed = false;
      theBoundaryProcessNeverUsed = false;
      theRayleighProcessNeverUsed = false;
    }
  }
  if (theBoundaryProcessNeverUsed) delete theBoundaryProcess;
  if (theAbsorptionProcessNeverUsed) delete theAbsorptionProcess;
  if (theRayleighProcessNeverUsed) delete rayleighScatteringProcess;
}

void OpticalPhysicsList::AddTransportation() {
  G4VUserPhysicsList::AddTransportation();
}

void OpticalPhysicsList::SetCuts() {
  //  " G4VUserPhysicsList::SetCutsWithDefault" method sets
  //   the default cut value for all particle types
  SetCutsWithDefault();
}
