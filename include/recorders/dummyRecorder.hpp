#ifndef RECORDERS_DUMMY_RECORDER_HPP
#define RECORDERS_DUMMY_RECORDER_HPP

/*!
 * @file
 * \brief Dummy analysis for the occasions where nothing
 *        needs to be done.
 *
 */

#include "recorders/recorderBase.hpp"

class DummyRecorder :  public RecorderBase {

public:

  DummyRecorder();
  ~DummyRecorder();
  
  void recordBeginOfRun(const G4Run* run);
  void recordEndOfRun(const G4Run* run);
};


#endif //RECORDERS_DUMMY_RECORDER_HPP
