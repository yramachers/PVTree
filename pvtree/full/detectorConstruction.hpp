#ifndef PV_FULL_DETECTOR_CONSTRUCTION
#define PV_FULL_DETECTOR_CONSTRUCTION

#include "pvtree/full/layeredLeafConstruction.hpp"

#include <vector>
#include <memory>
#include "globals.hh"
#include "G4VUserDetectorConstruction.hh"
#include "G4ThreeVector.hh"
#include "G4VisAttributes.hh"

class Turtle;
class G4Material;
class G4LogicalVolume;
class G4PhysicalVolume;
class TreeConstructionInterface;
class LeafConstructionInterface;
class TreeSystemInterface;

/*! \brief A class used to describe how to translate an L-System into a
 *         Geant4 geometry.
 */
class DetectorConstruction : public G4VUserDetectorConstruction {
 public:
  DetectorConstruction(std::shared_ptr<TreeConstructionInterface> treeSystem,
                       std::shared_ptr<LeafConstructionInterface> leafSystem,
                       unsigned int treeNumber);
  DetectorConstruction(std::shared_ptr<TreeConstructionInterface> treeSystem,
                       std::shared_ptr<LeafConstructionInterface> leafSystem);
  virtual ~DetectorConstruction();

  virtual G4VPhysicalVolume* Construct();
  virtual void ConstructSDandField();

  G4LogicalVolume* getLogicalVolume();

  /*! \brief Let the detector know that a call has been made to
   *         ReinitializeGeometry() on the run manager, so we need
   *         to re-run construction.
   */
  void resetGeometry();

  /*! \brief Method allows the tree and leaf system to be changes, but
   *         need to call ReinitializeGeometry() on the run manager to
   *         allow a clean re-running of construction.
   */
  void resetGeometry(std::shared_ptr<TreeConstructionInterface> treeSystem,
                     std::shared_ptr<LeafConstructionInterface> leafSystem,
                     unsigned int treeNumber);
  void resetGeometry(std::shared_ptr<TreeConstructionInterface> treeSystem,
                     std::shared_ptr<LeafConstructionInterface> leafSystem);

  /*! \brief Get the surface area of all the sensitive geometry.
   *
   * \returns Area in units of meters squared.
   */
  double getSensitiveSurfaceArea();

  /*! \brief Get the total number of leaves attached to tree
   *
   * \returns number of leaves.
   */
  unsigned int getNumberOfLeaves();

  /*! \brief Get the total number of leaves that were rejected due
   *         to overlaps.
   *
   * \returns number of rejected leaves.
   */
  unsigned int getNumberOfRejectedLeaves();

  /*! \brief Get the size of the structure in the X-axis direction
   *
   * \returns length in meters
   */
  double getXSize();

  /*! \brief Get the size of the structure in the Y-axis direction
   *
   * \returns length in meters
   */
  double getYSize();

  /*! \brief Get the size of the structure in the Z-axis direction
   *
   * \returns length in meters
   */
  double getZSize();

 private:
  void constructWorld();
  void placeTree(unsigned int i, unsigned int j);
  double calculateWorldSize();
  void iterateLSystem();
  void generateTurtles();
  void getTurtleTreeExtent(const Turtle* turtle, G4ThreeVector& minExtent,
                           G4ThreeVector& maxExtent, int maxDepth = -1);
  G4ThreeVector convertVector(const TVector3& input);
  void recursiveTreeBuild(Turtle* turtle, int depthStep,
                          G4LogicalVolume* parentVolume,
                          G4ThreeVector parentPosition);

  /*! \brief Construct the acceptable leaves from the candidate leaf
   *         list.
   */
  void candidateLeafBuild();

  /*! \brief Check if a candidate leaf would overlap with previously
   *         placed leaves.
   *
   * based upon method in  source/geometry/volumes/src/G4PVPlacement.cc
   *
   * @param[in] candidateLeafLogicalVolume The new leaf to be tested.
   * @param[in] parentBranchVolume The branch volume from which the leaf
   *            originates, so don't do overlap check against.
   * @param[in] resolution The number of points to be used in the check.
   * @param[in] tolerence The precision of the overlap check.
   * @param[in] maximumErrorNumber The number of overlaps to be displayed in
   *                               standard output.
   *
   * \returns true if the leaf is overlapping.
   */
  bool checkForLeafOverlaps(G4LogicalVolume* candidateLeafLogicalVolume,
                            G4VPhysicalVolume* parentBranchVolume,
                            G4int resolution = 1000, G4double tolerence = 0.0,
                            G4int maximumErrorNumber = 1);

  // Leaf detector construction
  LayeredLeafConstruction m_leafConstructor;

  // L-System Constructors
  std::shared_ptr<TreeConstructionInterface> m_treeSystem;
  std::vector<std::shared_ptr<TreeSystemInterface> > m_treeConditions;
  std::shared_ptr<LeafConstructionInterface> m_leafSystem;
  std::vector<Turtle*> m_turtles;

  // Candidate leaves and their spawn points
  std::vector<std::pair<G4LogicalVolume*, G4VPhysicalVolume*> >
      m_candidateLeaves;

  // Number of trees to construct
  unsigned int m_treeNumber;

  // Volumes
  G4LogicalVolume* m_worldLogicalVolume;
  G4VPhysicalVolume* m_worldPhysicalVolume;

  // Materials
  std::string m_airMaterialName;
  std::string m_trunkMaterialName;
  std::string m_floorMaterialName;

  // Visualization attributes
  G4VisAttributes m_trunkVisualAttributes;
  G4VisAttributes m_worldVisualAttributes;
  G4VisAttributes m_floorVisualAttributes;

  // For re-construction
  bool m_constructedSensitiveDetectors;
  bool m_constructed;

  // General structural details
  unsigned int m_treesConstructed;
  double m_sensitiveSurfaceArea;
  unsigned int m_leafNumber;
  unsigned int m_rejectedLeafNumber;
  double m_structureXSize;
  double m_structureYSize;
  double m_structureZSize;
  double m_treeRadius;
  double m_shiftedOrigin;
};

#endif  // PV_FULL_DETECTOR_CONSTRUCTION
