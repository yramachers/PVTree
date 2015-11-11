#include "full/steppingAction.hpp"

#include "G4Step.hh"
#include "G4Track.hh"
#include "G4StepPoint.hh"
#include "G4VPhysicalVolume.hh"
#include "G4ProcessManager.hh"
#include "G4ParticleDefinition.hh"

SteppingAction::SteppingAction() : m_oneStepPrimaries(false) {

  m_expectedNextStatus = Undefined;
}

SteppingAction::~SteppingAction() {}

/* ! \brief Monitor the steps taking place within the Geant4 simulation
 *          for cases where optical photons are detected at a boundary
 *          of a sensitive detector.
 *
 * Based upon the LXeSteppingAction from the Geant4 examples.
 */
void SteppingAction::UserSteppingAction(const G4Step* step) {

  G4Track* theTrack = step->GetTrack();

  if ( theTrack->GetCurrentStepNumber() == 1 ) m_expectedNextStatus = Undefined;

  G4StepPoint* thePrePoint = step->GetPreStepPoint();
  /*G4VPhysicalVolume* thePrePV =*/ thePrePoint->GetPhysicalVolume();

  G4StepPoint* thePostPoint = step->GetPostStepPoint();
  G4VPhysicalVolume* thePostPV = thePostPoint->GetPhysicalVolume();

  G4OpBoundaryProcessStatus boundaryStatus=Undefined;
  static G4ThreadLocal G4OpBoundaryProcess* boundary=NULL;

  //find the boundary process only once
  if(!boundary){
    G4ProcessManager* pm = step->GetTrack()->GetDefinition()->GetProcessManager();
    G4int nprocesses = pm->GetProcessListLength();
    G4ProcessVector* pv = pm->GetProcessList();

    for(G4int i=0; i<nprocesses; i++){
      if((*pv)[i]->GetProcessName()=="OpBoundary"){
        boundary = (G4OpBoundaryProcess*)(*pv)[i];
        break;
      }
    }
  }

  //Ignore photons that have left the world volume
  if(!thePostPV){
    G4cout << "Leaving the world..." << G4endl;
    m_expectedNextStatus=Undefined;
    return;
  }

  G4ParticleDefinition* particleType = theTrack->GetDefinition();

  if(particleType==G4OpticalPhoton::OpticalPhotonDefinition()){
    //Optical photon only

    //Was the photon absorbed by the absorption process
    if(thePostPoint->GetProcessDefinedStep()->GetProcessName() == "OpAbsorption"){
      
    }

    boundaryStatus=boundary->GetStatus();


    //Check to see if the partcile was actually at a boundary
    //Otherwise the boundary status may not be valid
    //Prior to Geant4.6.0-p1 this would not have been enough to check
    //fGeomBoundary is an enum from G4StepStatus.hh
    if(thePostPoint->GetStepStatus() == fGeomBoundary){

      if(m_expectedNextStatus==StepTooSmall){
        if(boundaryStatus!=StepTooSmall){
          G4ExceptionDescription ed;

          ed << "full::SteppingAction::UserSteppingAction(): "
                << "No reallocation step after reflection!"
                << G4endl;

          G4Exception("full::SteppingAction::UserSteppingAction()", "FullSimulation",
          FatalException,ed,
          "Something is wrong with the surface normal or geometry");
        }
      }

      m_expectedNextStatus = Undefined;

      switch(boundaryStatus){
      case Absorption:
	G4cout << "Absorption by " << thePostPV->GetName() << G4endl;
	break;
      case Detection:
	G4cout << "Detection by " << thePostPV->GetName() << G4endl;
	break;
      case BackScattering:
	G4cout << "Back scattering by " << thePostPV->GetName() << G4endl;
	m_expectedNextStatus = StepTooSmall;
	break;
      case Transmission:
	G4cout << "Transmission by " << thePostPV->GetName() << G4endl;
	break;
      case Undefined:
	G4cout << "Undefined by " << thePostPV->GetName() << G4endl;
	break;
      default:
	G4cout << "Something else by " << thePostPV->GetName() << G4endl;
	break;
      }

    }
  }// particleType==opticalphoton
}

void SteppingAction::SetOneStepPrimaries(G4bool usesOneStepPrimaries){ 
  m_oneStepPrimaries = usesOneStepPrimaries;
}

G4bool SteppingAction::GetOneStepPrimaries(){
  return m_oneStepPrimaries;
}




