#ifndef PV_FULL_LEAF_CONSTRUCTION
#define PV_FULL_LEAF_CONSTRUCTION

#include <vector>
#include "globals.hh"
#include "G4VUserDetectorConstruction.hh"
#include "leafSystem/leafSystemInterface.hpp"
#include "G4ThreeVector.hh"
#include "G4VisAttributes.hh"

class Turtle;
class Polygon;
class G4Material;
class G4LogicalVolume;
class G4TessellatedSolid;
class G4OpticalSurface;
class TVector3;
class Vertex;
class LeafConstructionInterface;
class LeafTrackerSD;

/*! \brief A class used to describe how to translate a leaf L-System into a
 *           Geant4 geometry.
 */
class LeafConstruction : public G4VUserDetectorConstruction {
public:
  /*! \brief Constructor with full specification of leaf system for the 
   *        case of standalone use.
   */
  LeafConstruction(std::shared_ptr<LeafConstructionInterface> leafConstructor,
		   Turtle* initialTurtle);

  /*! \brief Constructor without the specification of the L-System or initial
   *        turtle. For the case where there is a need to construct many leaves
   *        of a similar type however the initial conditions may change.
   */
  LeafConstruction();

  virtual ~LeafConstruction();

  virtual G4VPhysicalVolume* Construct();
  virtual void               ConstructSDandField();

  /*! \brief Construct a logical volume for a leaf with a specified L-System
   *        and an initial turtle.
   */
  G4LogicalVolume* constructForTree(std::shared_ptr<LeafConstructionInterface> leafConstructor,
				    Turtle* initialTurtle); 

  void getExtentForTree(G4ThreeVector& minExtent, 
			G4ThreeVector& maxExtent);
  void getExtentForTree(std::shared_ptr<LeafConstructionInterface> leafConstructor,
			Turtle* initialTurtle,
			G4ThreeVector& minExtent, 
			G4ThreeVector& maxExtent);

  G4LogicalVolume* getLogicalVolume();

  /*! \brief Get the surface area of all the sensitive geometry.
   *
   * \returns Area in units of meters squared.
   */
  double           getSensitiveSurfaceArea();

private:

  void                                 iterateLSystem();
  void                                 clearPolygonLists();
  void                                 generateSurface();
  std::vector<std::shared_ptr<Vertex>> mergeVertices(std::vector<Polygon* > polygons);
  void                                 solidifyLeaf();
  void                                 getExtent(std::vector<Polygon*> polygons,
						 G4ThreeVector& minExtent,
						 G4ThreeVector& maxExtent);
  G4ThreeVector                        convertVector(const TVector3& input);


  std::shared_ptr<LeafConstructionInterface>         m_leafConstructor;
  std::vector<std::shared_ptr<LeafSystemInterface> > m_leafConditions;
  Turtle*                                            m_initialTurtle;
  std::vector<Polygon*>                              m_leafSurface;
  std::vector<Polygon*>                              m_completeLeaf;

  //Solids
  G4TessellatedSolid* m_leafSolid;

  //Volumes
  G4LogicalVolume* m_worldLogicalVolume;

  //Sensitive volumes
  LeafTrackerSD* m_trackerSD;

  // Materials
  std::string m_airMaterialName;
  std::string m_sensitiveMaterialName;

  //Visualization attributes
  G4VisAttributes m_leafVisualAttributes;
  G4VisAttributes m_worldVisualAttributes;

  //For re-construction
  bool m_constructedSensitiveDetectors;

  //Important leaf properties
  double m_leafArea;
};


#endif //PV_FULL_LEAF_CONSTRUCTION
