#ifndef PVTREE_FULL_LAYERED_LEAF_CONSTRUCTION
#define PVTREE_FULL_LAYERED_LEAF_CONSTRUCTION

#include <vector>
#include "globals.hh"
#include "G4VUserDetectorConstruction.hh"
#include "pvtree/leafSystem/leafSystemInterface.hpp"
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
 *         Geant4 geometry.
 *
 * Goes beyond previous leaf construction by composing a solid out of
 * multiple materials in up to three layers.
 *
 * A simple improvement would be to add parameterisation of the leaf layers 
 * (e.g. thickness/number/type).
 */
class LayeredLeafConstruction : public G4VUserDetectorConstruction {
public:
  /* \brief Constructor with full specification of leaf system for the 
   *        case of standalone use.
   */
  LayeredLeafConstruction(std::shared_ptr<LeafConstructionInterface> leafSystem,
			  Turtle* initialTurtle);

  /* \brief Constructor without the specification of the L-System or initial
   *        turtle. For the case where there is a need to construct many leaves
   *        of a similar type however the initial conditions may change.
   */
  LayeredLeafConstruction();

  virtual ~LayeredLeafConstruction();

  virtual G4VPhysicalVolume* Construct();
  virtual void               ConstructSDandField();

  /* \brief Construct a logical volume for a leaf with a specified L-System
   *        and an initial turtle.
   */
  G4LogicalVolume* constructForTree(std::shared_ptr<LeafConstructionInterface> leafSystem,
				    Turtle* initialTurtle); 

  void getExtentForTree(G4ThreeVector& minExtent, 
			G4ThreeVector& maxExtent);
  void getExtentForTree(std::shared_ptr<LeafConstructionInterface> leafSystem,
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

  /*! \brief Iterate Lindenmeyer system for leaf.
   *
   */
  void iterateLSystem();

  /*! \brief Convert the results of the Lindenmeyer iterations
   *         into a surface.
   */
  std::vector<Polygon*> generateSurface();


  /*! \brief Construct leaf logical volume using current settings
   *         for initial turtle and Lindenmeyer system.
   */
  G4LogicalVolume* constructLeafLogicalVolume();

  /*! \brief Remove degenerate vertices without destroying the polygons. 
   *
   * After merge should expect there to be many shared vertices between
   * polygons.
   *
   * @param[in] polygons List of input polygons containing vertices which should 
   *            have a merge check performed.
   *
   * \returns List of unique vertices for all polygons.
  */
  std::vector<std::shared_ptr<Vertex>> mergeVertices(std::vector<Polygon* >& polygons);


  /*! \brief Convert a surface defined by a set of polygons into a 3d mesh
   *         by duplicating the surface and extrapolating along the vertex
   *         normals by specified factors.
   *
   * @param[in] polygons A list of triangles defining a surface.
   * @param[in] frontSurfaceOffsetFactor The front surface is moved along its
   *            normal vector by this factor.
   * @param[in] backSurfaceOffsetFactor The back surface is moved along its
   *            normal vector by this factor.
   *
   * \returns A vector of polygons (which need to be deleted) describing the
   *          extrapolated mesh.
   */
  std::vector<Polygon*> extrapolateSurfaceIntoMesh(const std::vector<Polygon*>& polygons,
						   double frontSurfaceOffsetFactor, 
						   double backSurfaceOffsetFactor);
  

  /*! \brief Function to calculate the surface area of a
   *         surface after extrapolation along normals has taken
   *         place.
   *
   * Assumes that the back face is covered. Therefore only includes
   * edges and front.
   *
   * \returns The area of the surface in meters squared.
   */
  double calculateExtrapolatedSurfaceArea(const std::vector<Polygon*>& polygons,
					  double frontSurfaceOffsetFactor,
					  double backSurfaceOffsetFactor);

  /*! \brief Create the edge mesh between two extrapolated surfaces.
   *
   * \returns A vector of polygons representing the edge.
   */
  std::vector<Polygon*> createEdgeSurface(const std::vector<Polygon*>& surfacePolygons, 
					  double frontSurfaceOffsetFactor,
					  double backSurfaceOffsetFactor);

  /*! \brief Convert mesh into a tessellated solid.
   *
   * Also performs some basic checks that the triangles that make
   * up the mesh are not degenerate. Currently ignores degenerate
   * triangles in construction.
   *
   * @param[in] polygons A list of triangles defining a closed volume.
   * @param[in] solidName Name to give the Geant4 solid.
   *
   * \returns A Geant4 Tessellated Solid.
   *          
   */
  G4TessellatedSolid* convertMeshToTessellatedSolid(const std::vector<Polygon*>& polygons,
						    std::string solidName);

  void getExtent(std::vector<Polygon*>& polygons,
		 G4ThreeVector& minExtent,
		 G4ThreeVector& maxExtent);

  /*! \brief Translate a ROOT TVector into a Geant4 vector.
   *
   * Also at the same time converts to Geant4 meters.
   *
   * @param[in] input ROOT TVector to be converted.
   * \returns A Geant4 Three Vector.
   */
  G4ThreeVector convertVector(const TVector3& input);

  /*! \brief Handle the deletion of a vector of polygons
   *
   */
  void clearPolygonList(std::vector<Polygon*>& polygons);
  
  /*! \brief Obtain Geant4 materials for each layer.
   *
   */
  void defineMaterials();


  std::shared_ptr<LeafConstructionInterface>         m_leafSystem;
  std::vector<std::shared_ptr<LeafSystemInterface> > m_leafConditions;
  Turtle*                                            m_initialTurtle;

  //Volumes
  G4LogicalVolume* m_worldLogicalVolume;

  //Sensitive volumes
  LeafTrackerSD* m_trackerSD;

  // Materials
  std::string m_airMaterialName;
  std::string m_frontMaterialName;
  std::string m_sensitiveMaterialName;
  std::string m_backMaterialName;

  //Visualization attributes
  G4VisAttributes m_frontAttributes;
  G4VisAttributes m_sensitiveAttributes;
  G4VisAttributes m_backAttributes;
  G4VisAttributes m_worldVisualAttributes;
  G4VisAttributes m_envelopeAttributes;

  //For re-construction
  bool m_constructedSensitiveDetectors;

  //Important leaf properties
  double m_sensitiveArea;
};


#endif //PVTREE_FULL_LAYERED_LEAF_CONSTRUCTION
