#ifndef RECORDERS_FOREST_RECORDER_HPP
#define RECORDERS_FOREST_RECORDER_HPP

/*!
 * @file
 * \brief Analysis package for monitoring the convergence
 *        properties of the simulation.
 *
 * Monitors the number of hits on the detector geometry
 * during each event of a run. Designed to extract the
 * convergence of the detector efficiency, which is a
 * necessary component of establishing overall simulation
 * accuracy.
 */

#include "pvtree/full/recorders/recorderBase.hpp"
#include <vector>
#include <unordered_map>

class ForestRecorder : public RecorderBase {
  /*! \brief Total number of optical photons per run
   *         per event.
   */
  std::vector<std::vector<long> > m_photons;

  /*! \brief Total number of hits per run
   *         per event.
   */
  std::vector<std::vector<long> > m_hits;

  /*! \brief Total energy deposited by hits per run
   *         per event per tree. [W]
   */
  std::vector<std::vector<std::unordered_map<unsigned int, double> > > m_summedHitEnergies;

  /*! \brief Keep track of any aborted events.
   *
   */
  bool m_eventAborted;

 public:
  ForestRecorder();
  ~ForestRecorder();

  void recordBeginOfRun(const G4Run* run);
  void recordEndOfRun(const G4Run* run);

  void recordBeginOfEvent(const G4Event* event);
  void recordEndOfEvent(const G4Event* event);

  /*! \brief Reset the stored results to initial values.
   */
  void reset();

  /*! \brief Get the photon total counts
   */
  std::vector<std::vector<long> > getPhotonCounts();

  /*! \brief Get the hit total counts
   */
  std::vector<std::vector<long> > getHitCounts();

  /*! \brief Get the total energy deposited
   *
   * \returns The total energy deposited in an event for
   *          a number of events and runs per tree. The units are [W].
   */
  std::vector<std::vector<std::unordered_map<unsigned int, double> > > getSummedHitEnergies();

  /*! \brief Check if any event in run was aborted.
   *
   * \returns True if any of the events in the run were
   *          aborted.
   */
  bool wasAborted();
};

#endif  // RECORDERS_FOREST_RECORDER_HPP
