#ifndef RECORDERS_CONVERGENCE_RECORDER_HPP
#define RECORDERS_CONVERGENCE_RECORDER_HPP

/*!
 * @file
 * \brief Analysis package for monitoring the convergence
 *        properties of the simulation.
 *
 * Monitors the number of hits on the detector geometry
 * during each event of a run. Designed to extract the 
 * convergence of the detector efficiency, which is a
 * necessary component of establishing overall simulation 
 * accuarcy.
 */

#include "recorders/recorderBase.hpp"
#include <vector>

class ConvergenceRecorder :  public RecorderBase {

  /*! \brief Total number of optical photons per run
   *         per event.
   */
  std::vector<std::vector<long > >  m_photons;

  /*! \brief Total number of hits per run
   *         per event.
   */
  std::vector<std::vector<long > >  m_hits;

  /*! \brief Total energy deposited by hits per run
   *         per event. [W]
   */
  std::vector<std::vector<double> > m_summedHitEnergies;

  /*! \brief Keep track of any aborted events.
   *
   */
  bool m_eventAborted;

public:

  ConvergenceRecorder();
  ~ConvergenceRecorder();
  
  void recordBeginOfRun(const G4Run* run);
  void recordEndOfRun(const G4Run* run);
    
  void recordBeginOfEvent(const G4Event* event);
  void recordEndOfEvent(const G4Event* event);


  /*! \brief Reset the stored results to initial values.
   */
  void reset();

  /*! \brief Get the photon total counts
   */
  std::vector<std::vector<long > > getPhotonCounts();

  /*! \brief Get the hit total counts
   */
  std::vector<std::vector<long > > getHitCounts();

  /*! \brief Get the total energy deposited
   *
   * \returns The total energy deposited in an event for 
   *          a number of events and runs. The units are [W].
   */
  std::vector<std::vector<double > > getSummedHitEnergies();

  /*! \brief Check if any event in run was aborted.
   *
   * \returns True if any of the events in the run were 
   *          aborted.
   */
  bool wasAborted();
};


#endif //RECORDERS_CONVERGENCE_RECORDER_HPP
