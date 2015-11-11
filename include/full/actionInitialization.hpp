#ifndef PV_FULL_ACTION_INITIALIZATION
#define PV_FULL_ACTION_INITIALIZATION

/*! @file
 * \brief Prepare standard set of user actions to generate primary particles,
 *        handle new runs and handle new events. 
 *        
 * A number of elements of the simulation are configured here including the
 * analysis performed, number of photons and the light source.
 */

#include "G4VUserActionInitialization.hh"
#include "G4VUserPrimaryGeneratorAction.hh"
#include <functional>

class RecorderBase;
class Sun;

class ActionInitialization : public G4VUserActionInitialization {

public:
  ActionInitialization(RecorderBase* recorder, std::function<G4VUserPrimaryGeneratorAction*()> primaryGenerator);
  virtual ~ActionInitialization();

  virtual void BuildForMaster() const;
  virtual void Build() const;

private:
  /*! \brief Number of photons to generate per event. */
  unsigned int  m_photonNumber;

  /*! \brief Inteface to analysis code. */
  RecorderBase* m_recorder;

  /*! \brief The primary generator creator function. */
  std::function<G4VUserPrimaryGeneratorAction*()> m_primaryGenerator;

  /*! \brief Solar model */
  Sun* m_sun;
};




#endif // PV_FULL_ACTION_INITIALIZATION
