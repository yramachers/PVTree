#ifndef RECORDERS_RECORDER_BASE_HPP
#define RECORDERS_RECORDER_BASE_HPP

/*!
 * @file
 * \brief Pure virtual class defining common methods that
 *        will interact with Geant4 objects at different
 *        points in the simulation.
 *
 * Analysis code will need to inherit from this class to
 * access the results of simulation. This is based upon
 * the recorder implemented in the LXe optical example.
 *
 * This seperates the analysis implementation details from
 * the general simulation (for the most part).
 */

class G4Run;
class G4Event;
class G4Track;
class G4Step;

class RecorderBase {
 public:
  virtual ~RecorderBase(){};

  virtual void recordBeginOfRun(const G4Run*) = 0;
  virtual void recordEndOfRun(const G4Run*) = 0;

  virtual void recordBeginOfEvent(const G4Event*){};
  virtual void recordEndOfEvent(const G4Event*){};
  virtual void recordTrack(const G4Track*){};
  virtual void recordStep(const G4Step*){};
};

#endif  // RECORDERS_RECORDER_BASE_HPP
