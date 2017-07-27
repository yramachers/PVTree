#ifndef PV_VISUALIZATION_ACTION
#define PV_VISUALIZATION_ACTION

#include "globals.hh"
#include "G4VUserVisAction.hh"
#include "G4VisAttributes.hh"

class G4LogicalVolume;
class G4VVisManager;
class G4VPhysicalVolume;

/*! \brief Visualization action for case where no
 *         simulation needs to be performed. Should
 *         work for generic world logical volume.
 * 
 */
class VisualizationAction : public G4VUserVisAction {
public:
  explicit VisualizationAction(G4LogicalVolume* volumeToDraw);
  virtual void Draw();

private:
  //! \brief Top volume (world)
  G4LogicalVolume* m_volumeToDraw;

  //! \brief Use a default visual attribute for volumes without one specified
  G4VisAttributes m_defaultVisualAttributes;

  /*! \brief Draw each volume and its sub-volumes according
   *         to the appropriate visual attributes.
   *
   */
  void DrawRecursively(G4VVisManager* visManager, G4VPhysicalVolume* currentPhysicalVolume);

};


#endif //PV_VISUALIZATION_ACTION
