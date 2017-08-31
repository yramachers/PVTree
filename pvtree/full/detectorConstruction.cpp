#include "pvtree/treeSystem/treeConstructionInterface.hpp"
#include "pvtree/leafSystem/leafConstructionInterface.hpp"
#include "pvtree/full/detectorConstruction.hpp"
#include "pvtree/geometry/turtle.hpp"
#include "pvtree/full/material/materialFactory.hpp"
#include "assert.h"
#include <algorithm>

#include "G4Material.hh"
#include "G4Element.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4OpticalSurface.hh"
#include "G4LogicalVolume.hh"
#include "G4ThreeVector.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4NistManager.hh"
#include "G4AffineTransform.hh"
#include "G4UnitsTable.hh"

// Need a number of solids to construct the detector
#include "G4Box.hh"
#include "G4Cons.hh"
#include "G4Orb.hh"
#include "G4Tubs.hh"
#include "G4Sphere.hh"
#include "G4TessellatedSolid.hh"

DetectorConstruction::DetectorConstruction(
    std::shared_ptr<TreeConstructionInterface> treeSystem,
    std::shared_ptr<LeafConstructionInterface> leafSystem,
    unsigned int treeNumber)
    : G4VUserDetectorConstruction(),
      m_treeSystem(treeSystem),
      m_leafSystem(leafSystem),
      m_treeNumber(treeNumber),
      m_worldLogicalVolume(nullptr),
      m_worldPhysicalVolume(nullptr),
      m_airMaterialName("pv-air"),
      m_trunkMaterialName("pv-aluminium"),
      m_floorMaterialName("pv-concrete"),
      m_constructedSensitiveDetectors(false),
      m_constructed(false),
      m_sensitiveSurfaceArea(0.0),
      m_leafNumber(0u),
      m_rejectedLeafNumber(0u),
      m_structureXSize(0.0),
      m_structureYSize(0.0),
      m_structureZSize(0.0) {
  // Set the colours of different geometry types.
  m_trunkVisualAttributes.SetColour(G4Colour(0.73, 0.51, 0.13, 1.0));  // Brown
  m_floorVisualAttributes.SetColour(
      G4Colour(0.87, 0.87, 0.87, 1.0));  // Transparent Grey

  // Make sure the world orb doesnt hide everything
  m_worldVisualAttributes.SetForceSolid(
      true);  // Wireframe doesn't work for orbs...
  m_worldVisualAttributes.SetColour(
      G4Colour(0.0, 0.6, 1.0, 0.1));  // Transparent light blue
}

DetectorConstruction::DetectorConstruction(
    std::shared_ptr<TreeConstructionInterface> treeSystem,
    std::shared_ptr<LeafConstructionInterface> leafSystem)
    : DetectorConstruction(treeSystem, leafSystem, 1) {}

DetectorConstruction::~DetectorConstruction() {
  // Ensure no turtles survive
  for (auto& turtle : m_turtles) {
    delete turtle;
  }

  m_turtles.clear();
}

G4LogicalVolume* DetectorConstruction::getLogicalVolume() {
  return m_worldLogicalVolume;  // For drawing...
}

void DetectorConstruction::resetGeometry() { m_constructed = false; }

void DetectorConstruction::resetGeometry(
    std::shared_ptr<TreeConstructionInterface> treeSystem,
    std::shared_ptr<LeafConstructionInterface> leafSystem,
    unsigned int treeNumber) {
  m_treeSystem = treeSystem;
  m_leafSystem = leafSystem;
  m_treeNumber = treeNumber;
  resetGeometry();
}

void DetectorConstruction::resetGeometry(
    std::shared_ptr<TreeConstructionInterface> treeSystem,
    std::shared_ptr<LeafConstructionInterface> leafSystem) {
  resetGeometry(treeSystem, leafSystem, 1);
}

double DetectorConstruction::getSensitiveSurfaceArea() {
  return m_sensitiveSurfaceArea;
}

unsigned int DetectorConstruction::getNumberOfLeaves() { return m_leafNumber; }

unsigned int DetectorConstruction::getNumberOfRejectedLeaves() {
  return m_rejectedLeafNumber;
}

double DetectorConstruction::getXSize() { return m_structureXSize; }

double DetectorConstruction::getYSize() { return m_structureYSize; }

double DetectorConstruction::getZSize() { return m_structureZSize; }

G4VPhysicalVolume* DetectorConstruction::Construct() {
  //  std::cout << "SIM: in Detector Construct()" << std::endl;
  // Check if already constructed
  if (m_constructed) {
    return m_worldPhysicalVolume;
  }

  // Reset some structural information extracted during construction
  m_sensitiveSurfaceArea = 0.0;
  m_leafNumber = 0u;
  m_rejectedLeafNumber = 0u;

  // Clear the candidate leaf list if not already empty
  for (auto& candidateLeafInfo : m_candidateLeaves) {
    G4LogicalVolume* leafLogicalVolume = candidateLeafInfo.first;
    delete leafLogicalVolume;
  }

  m_candidateLeaves.clear();

  // Construct the world
  constructWorld();

  // Construct a tree
  placeTree();

  // Attempt to create leaves from candidate pool
  candidateLeafBuild();

  // Record that the world has been build
  m_constructed = true;

  return m_worldPhysicalVolume;
}

void DetectorConstruction::ConstructSDandField() {
  // The leaves are the only sensitive element
  m_leafConstructor.ConstructSDandField();
}

void DetectorConstruction::constructWorld() {
  // Find size of 1 tree and calculate the size of the world box
  double boundingRadius = calculateWorldSize();

  // Create the 'World' (which has to be centred at coordinates 0.0, 0.0, 0.0)
  // Large enough for the sun disk to appear more point-like and hit regardless
  // Using a sphere so that the photon field is always disk shaped
  // G4Orb (const G4String &pName, G4double pRmax)
  // Multiply by root3 to account for the fact that the photons have to
  // originate outside the 'snug' bounding volume
  G4Orb* worldOrb = new G4Orb("World", std::sqrt(3.0) * boundingRadius);

  // Get air material from factory
  G4Material* airMaterial =
      MaterialFactory::instance()->getMaterial(m_airMaterialName);

  m_worldLogicalVolume =
      new G4LogicalVolume(worldOrb, airMaterial, "World", 0, 0, 0);
  m_worldLogicalVolume->SetVisAttributes(m_worldVisualAttributes);

  m_worldPhysicalVolume =
      new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, 0.0), m_worldLogicalVolume,
                        "World", 0, false, 0);

  // Add a very simple floor which almost reaches edge of world
  // For some inexplicable reason the surface on the inside is
  // not behaving itself so add a 'top'
  G4double topThickness = 0.001 * boundingRadius;
  if (topThickness > 0.05 * m) {
    topThickness = 0.05 * m;
  }

  G4double pRMin, pRMax, pSPhi, pDPhi, pSTheta, pDTheta, pDz;
  G4Tubs* floorTopSolid = new G4Tubs(
      "FloorTop", pRMin = 0.0, pRMax = std::sqrt(3.0) * 0.95 * boundingRadius,
      pDz = topThickness, pSPhi = 0.0, pDPhi = 2.0 * M_PI);
  G4Sphere* floorSolid = new G4Sphere(
      "Floor", pRMin = 0.0, pRMax = std::sqrt(3.0) * 0.95 * boundingRadius,
      pSPhi = 0.0, pDPhi = 2.0 * M_PI, pSTheta = M_PI / 2.0, pDTheta = M_PI);

  // Get floor material from factory
  G4Material* floorMaterial =
      MaterialFactory::instance()->getMaterial(m_floorMaterialName);
  G4OpticalSurface* floorOpticalSurface =
      MaterialFactory::instance()->getOpticalSurface(m_floorMaterialName);

  G4LogicalVolume* floorTopLogicalVolume =
      new G4LogicalVolume(floorTopSolid, floorMaterial, "FloorTop");
  G4LogicalVolume* floorLogicalVolume =
      new G4LogicalVolume(floorSolid, floorMaterial, "Floor");

  floorTopLogicalVolume->SetVisAttributes(m_floorVisualAttributes);
  floorLogicalVolume->SetVisAttributes(m_floorVisualAttributes);

  // Need to ensure there is no overlap between the floor and floor top.
  G4ThreeVector floorPosition(0.0, 0.0, -topThickness);
  G4RotationMatrix* identityRotation = new G4RotationMatrix();
  /*G4VPhysicalVolume* floorPhysicalVolume =*/new G4PVPlacement(
      identityRotation, floorPosition, floorLogicalVolume, "Floor",
      m_worldLogicalVolume, false, 0);

  G4Transform3D identityTransform;  // Positioned at 0, 0, 0
  /*G4VPhysicalVolume* floorPhysicalVolume =*/new G4PVPlacement(
      identityTransform, floorTopLogicalVolume, "FloorTop",
      m_worldLogicalVolume, false, 0);

  // Set the surface properties
  new G4LogicalSkinSurface("FloorSkin", floorLogicalVolume,
                           floorOpticalSurface);
  new G4LogicalSkinSurface("FloorTopSkin", floorTopLogicalVolume,
                           floorOpticalSurface);
}

void DetectorConstruction::placeTree() {
  // Fudgey fix for now to test
  // Iterating the LSystem conditions
  iterateLSystem();

  // Construct the turtles from final L-System
  generateTurtles();

  // find all the parentless turtles (the starting points for recursive builds).
  std::vector<Turtle*> startingTurtles;
  for (auto& candidateTurtle : m_turtles) {
    if (candidateTurtle->parent == NULL) {
      startingTurtles.push_back(candidateTurtle);
    }
  }

  // Convert the turtles into Geant4 geometry representing the tree structure
  // through a recursive build
  G4ThreeVector startPosition(0.0, 0.0, 0.0);
  for (auto& startingTurtle : startingTurtles) {
    recursiveTreeBuild(startingTurtle, 100, m_worldLogicalVolume,
                       startPosition);
  }
}

double DetectorConstruction::calculateWorldSize() {
  // Iterating the LSystem conditions
  iterateLSystem();

  // Construct the turtles from final L-System
  generateTurtles();

  // find all the parentless turtles (the starting points for recursive builds).
  std::vector<Turtle*> startingTurtles;
  for (auto& candidateTurtle : m_turtles) {
    if (candidateTurtle->parent == NULL) {
      startingTurtles.push_back(candidateTurtle);
    }
  }

  // Find the total extent of the tree geometry to build the world volume
  // Need to consider all the starting turtles in the evaluation
  G4ThreeVector totalMaximums(convertVector(m_turtles[0]->position));
  G4ThreeVector totalMinimums(convertVector(m_turtles[0]->position));

  for (auto& startingTurtle : startingTurtles) {
    getTurtleTreeExtent(startingTurtle, totalMinimums, totalMaximums);
  }

  // Calculate the rough bounding
  double maximumBoundingBoxX =
      (fabs(totalMaximums.x()) > fabs(totalMinimums.x())
           ? fabs(totalMaximums.x())
           : fabs(totalMinimums.x()));
  double maximumBoundingBoxY =
      (fabs(totalMaximums.y()) > fabs(totalMinimums.y())
           ? fabs(totalMaximums.y())
           : fabs(totalMinimums.y()));
  double maximumBoundingBoxZ =
      (fabs(totalMaximums.z()) > fabs(totalMinimums.z())
           ? fabs(totalMaximums.z())
           : fabs(totalMinimums.z()));

  m_structureXSize = maximumBoundingBoxX / meter;
  m_structureYSize = maximumBoundingBoxY / meter;
  m_structureZSize = maximumBoundingBoxZ / meter;

  // Applying a temporary scale to the world box size whilst I wait for better
  // construction code to evaluate the
  // true bounding box.
  double fudgeScaleFactor = 10.1;
  maximumBoundingBoxX *= fudgeScaleFactor;
  maximumBoundingBoxY *= fudgeScaleFactor;
  maximumBoundingBoxZ *= fudgeScaleFactor;

  double boundingRadius = std::sqrt(std::pow(maximumBoundingBoxX, 2.0) +
                                    std::pow(maximumBoundingBoxY, 2.0) +
                                    std::pow(maximumBoundingBoxZ, 2.0));

  return boundingRadius;
}

void DetectorConstruction::iterateLSystem() {
  // Clean up any old results
  m_treeConditions.clear();

  int treeIterationNumber =
      m_treeSystem->getIntegerParameter("iterationNumber");
  m_treeConditions = m_treeSystem->getInitialConditions();

  for (int i = 0; i < treeIterationNumber; i++) {
    std::vector<std::shared_ptr<TreeSystemInterface> > latestConditions;

    for (const auto& condition : m_treeConditions) {
      for (const auto& newCondition : condition->applyRule()) {
        latestConditions.push_back(newCondition);
      }
    }

    // Use the new iteration
    m_treeConditions.clear();
    m_treeConditions = latestConditions;
  }
}

void DetectorConstruction::generateTurtles() {
  // Clear previously generated turtles (if any!)
  for (auto& turtle : m_turtles) {
    delete turtle;
  }
  m_turtles.clear();

  std::vector<Turtle*> activeTurtles;

  // Create initial active turtle
  activeTurtles.push_back(new Turtle());

  // Process all the conditions (convert into turtles)
  for (auto& condition : m_treeConditions) {
    condition->processTurtles(activeTurtles, m_turtles);
  }

  // Remove the last active turtle
  delete activeTurtles.back();
  activeTurtles.pop_back();

  assert(activeTurtles.size() == 0);
}

//! \todo Need to improve the tree extent calculation so it takes into account
//the width of the trunk/branches.
void DetectorConstruction::getTurtleTreeExtent(const Turtle* turtle,
                                               G4ThreeVector& minExtent,
                                               G4ThreeVector& maxExtent,
                                               int maxDepth /* = -1 */) {
  TVector3 endPosition(turtle->position);
  endPosition = endPosition + (turtle->orientation * turtle->length);

  G4ThreeVector g4StartPosition(convertVector(turtle->position));
  G4ThreeVector g4EndPosition(convertVector(endPosition));

  auto result = std::minmax(
      {g4StartPosition.x(), g4EndPosition.x(), minExtent.x(), maxExtent.x()});
  minExtent.setX(result.first);
  maxExtent.setX(result.second);

  result = std::minmax(
      {g4StartPosition.y(), g4EndPosition.y(), minExtent.y(), maxExtent.y()});
  minExtent.setY(result.first);
  maxExtent.setY(result.second);

  result = std::minmax(
      {g4StartPosition.z(), g4EndPosition.z(), minExtent.z(), maxExtent.z()});
  minExtent.setZ(result.first);
  maxExtent.setZ(result.second);

  // Call for child turtles (if necessary)
  for (const Turtle* child : turtle->children) {
    if (maxDepth != 0) {
      getTurtleTreeExtent(child, minExtent, maxExtent, maxDepth - 1);
    }
  }

  // If no children then look at leaf size
  if (turtle->children.size() == 0) {
    Turtle copiedTurtle(turtle->position, turtle->orientation, turtle->lVector);
    m_leafConstructor.getExtentForTree(m_leafSystem, &copiedTurtle, minExtent,
                                       maxExtent);
  }
}

/*
  Translate a ROOT TVector into a Geant4 vector.
 */
G4ThreeVector DetectorConstruction::convertVector(const TVector3& input) {
  G4ThreeVector output(input.X() * m, input.Y() * m, input.Z() * m);
  return output;
}

void DetectorConstruction::recursiveTreeBuild(Turtle* turtle, int /*depthStep*/,
                                              G4LogicalVolume* parentVolume,
                                              G4ThreeVector parentPosition) {
  // Would need to use the depthStep in the future for the creation of bounding
  // volumes.

  // It is probably the case that the bounding box volumes can use air as their
  // material. I
  // assume that Geant4 is not additive.
  G4double pRmin1, pRmax1, pRmin2, pRmax2, pDz, pSPhi, pDPhi;
  G4Cons* trunkSolid;
  double widthCriteria = 0.04;
  double pi = 3.141592654;

  if (turtle->children.size() == 0) {
    trunkSolid = new G4Cons(
        "Trunk", pRmin1 = 0.0 * m, pRmax1 = (turtle->width / 2.0) * m,
        pRmin2 = 0.0 * m, pRmax2 = (turtle->width / 2.0) * m,
        pDz = (turtle->length / 2.0) * m, pSPhi = 0.0, pDPhi = 2.0 * M_PI);
  } else {
    // If we have children end the cone on the child width! -- assume all
    // children the same
    trunkSolid = new G4Cons(
        "Trunk", pRmin1 = 0.0 * m, pRmax1 = (turtle->width / 2.0) * m,
        pRmin2 = 0.0 * m, pRmax2 = (turtle->children[0]->width / 2.0) * m,
        pDz = (turtle->length / 2.0) * m, pSPhi = 0.0, pDPhi = 2.0 * M_PI);
  }

  // Get trunk material from factory
  G4Material* trunkMaterial =
      MaterialFactory::instance()->getMaterial(m_trunkMaterialName);
  G4OpticalSurface* trunkOpticalSurface =
      MaterialFactory::instance()->getOpticalSurface(m_trunkMaterialName);

  G4LogicalVolume* trunkLogicalVolume =
      new G4LogicalVolume(trunkSolid, trunkMaterial, "Trunk");

  trunkLogicalVolume->SetVisAttributes(m_trunkVisualAttributes);

  G4RotationMatrix* rotationMatrix = new G4RotationMatrix();
  rotationMatrix->set((turtle->orientation.Phi() + M_PI / 2.0),
                      turtle->orientation.Theta(), 0);

  TVector3 centralPosition(turtle->position);
  centralPosition =
      centralPosition + (turtle->orientation * (turtle->length / 2.0));

  // G4RotationMatrix *pRot, const G4ThreeVector &tlate, G4LogicalVolume
  // *pCurrentLogical, const G4String &pName, G4LogicalVolume *pMotherLogical,
  // G4bool pMany, G4int pCopyNo, G4bool pSurfChk=false
  G4VPhysicalVolume* trunkPhysicalVolume =
      new G4PVPlacement(rotationMatrix, convertVector(centralPosition),
                        trunkLogicalVolume, "Trunk", parentVolume, false, 0);

  // Set the surface properties
  new G4LogicalSkinSurface("TrunkSkin", trunkLogicalVolume,
                           trunkOpticalSurface);

  // Then call this function for new seeds
  for (Turtle* childTurtle : turtle->children) {
    recursiveTreeBuild(childTurtle, -1, parentVolume, parentPosition);
  }

  // If there are no child turtles present assume a leaf should be present
  if (turtle->children.size() == 0) {
    // If there are no children present assume leaf construction
    // For overlapping checks need to have rest of structure built already
    // So just storing the leaf logical volume and branch physical volume for
    // later!
    G4LogicalVolume* leafLogicalVolume =
        m_leafConstructor.constructForTree(m_leafSystem, turtle);
    m_candidateLeaves.push_back(std::pair<G4LogicalVolume*, G4VPhysicalVolume*>(
        leafLogicalVolume, trunkPhysicalVolume));

    int iterationNumber = m_treeSystem->getIntegerParameter("iterationNumber");

    // KIERAN - add two leaves at the base of the end pieve of branch
    if (iterationNumber != 0) {
      // Store the initial conditions of the turtle before making adjustments to
      // place leaf
      double storeLength = turtle->length;
      double storePhi = turtle->orientation.Phi();
      double storeLVectorPhi = turtle->lVector.Phi();
      // move the turtle to a point at the base of the branch, perpendicular to
      // it
      turtle->orientation.SetPhi(turtle->orientation.Phi() - (pi / 2));
      turtle->lVector.SetPhi(turtle->lVector.Phi() - (pi / 2));
      turtle->length = turtle->width / 2;
      // add it to the leaf list
      leafLogicalVolume =
          m_leafConstructor.constructForTree(m_leafSystem, turtle);
      m_candidateLeaves.push_back(
          std::pair<G4LogicalVolume*, G4VPhysicalVolume*>(leafLogicalVolume,
                                                          trunkPhysicalVolume));
      // restore turtle to its original position
      turtle->length = storeLength;
      turtle->orientation.SetPhi(storePhi);
      turtle->lVector.SetPhi(storeLVectorPhi);
      // move the turtle to a point on the other side of the branch, at the base
      turtle->orientation.SetPhi(turtle->orientation.Phi() + (pi / 2));
      turtle->lVector.SetPhi(turtle->lVector.Phi() + (pi / 2));
      turtle->length = turtle->width / 2;
      // add leaf to the list
      leafLogicalVolume =
          m_leafConstructor.constructForTree(m_leafSystem, turtle);
      m_candidateLeaves.push_back(
          std::pair<G4LogicalVolume*, G4VPhysicalVolume*>(leafLogicalVolume,
                                                          trunkPhysicalVolume));
      // restore turtle conditions
      turtle->length = storeLength;
      turtle->orientation.SetPhi(storePhi);
      turtle->lVector.SetPhi(storeLVectorPhi);
    }
  }
  // KIERAN - if the branch is sufficiently thin (arbitrarily chosen thickness)
  // then place two leaves at the base of that segment
  else if (turtle->children.size() != 0 &&
           (turtle->width / 2) < widthCriteria) {
    // if we are dealing with a cylindrical piece of branch
    if (pRmax1 == pRmax2) {
      // Store the initial conditions of the turtle before making adjustments to
      // place leaf
      double storeLength = turtle->length;
      double storePhi = turtle->orientation.Phi();
      double storeLVectorPhi = turtle->lVector.Phi();
      // move the turtle to a point at the base of the branch, perpendicular to
      // it
      turtle->orientation.SetPhi(turtle->orientation.Phi() - (pi / 2));
      turtle->lVector.SetPhi(turtle->lVector.Phi() - (pi / 2));
      turtle->length = turtle->width / 2;
      // add it to the leaf list
      G4LogicalVolume* leafLogicalVolume =
          m_leafConstructor.constructForTree(m_leafSystem, turtle);
      m_candidateLeaves.push_back(
          std::pair<G4LogicalVolume*, G4VPhysicalVolume*>(leafLogicalVolume,
                                                          trunkPhysicalVolume));
      // restore turtle to its original position
      turtle->length = storeLength;
      turtle->orientation.SetPhi(storePhi);
      turtle->lVector.SetPhi(storeLVectorPhi);
      // move the turtle to a point on the other side of the branch, at the base
      turtle->orientation.SetPhi(turtle->orientation.Phi() + (pi / 2));
      turtle->lVector.SetPhi(turtle->lVector.Phi() + (pi / 2));
      turtle->length = turtle->width / 2;
      // add leaf to the list
      leafLogicalVolume =
          m_leafConstructor.constructForTree(m_leafSystem, turtle);
      m_candidateLeaves.push_back(
          std::pair<G4LogicalVolume*, G4VPhysicalVolume*>(leafLogicalVolume,
                                                          trunkPhysicalVolume));
      // restore turtle conditions
      turtle->length = storeLength;
      turtle->orientation.SetPhi(storePhi);
      turtle->lVector.SetPhi(storeLVectorPhi);
    } else {  // add leaves to the base of sufficiently thin cones
      // Store the initial conditions of the turtle before making adjustments to
      // place leaf
      double storeLength = turtle->length;
      double storePhi = turtle->orientation.Phi();
      double storeLVectorPhi = turtle->lVector.Phi();
      double inclination = (pi / 2) - atan(pDz / (pRmax1 - pRmax2));
      // move the turtle to a point at the base of the branch, perpendicular to
      // it
      turtle->orientation.SetPhi(turtle->orientation.Phi() - (pi / 2) -
                                 inclination);
      turtle->lVector.SetPhi(turtle->lVector.Phi() - (pi / 2));
      turtle->length = (turtle->width / 2) * cos(inclination);
      // add it to the leaf list
      G4LogicalVolume* leafLogicalVolume =
          m_leafConstructor.constructForTree(m_leafSystem, turtle);
      m_candidateLeaves.push_back(
          std::pair<G4LogicalVolume*, G4VPhysicalVolume*>(leafLogicalVolume,
                                                          trunkPhysicalVolume));
      // restore turtle to its original position
      turtle->length = storeLength;
      turtle->orientation.SetPhi(storePhi);
      turtle->lVector.SetPhi(storeLVectorPhi);
      // move the turtle to a point on the other side of the branch, at the base
      turtle->orientation.SetPhi(turtle->orientation.Phi() + (pi / 2) -
                                 inclination);
      turtle->lVector.SetPhi(turtle->lVector.Phi() + (pi / 2));
      turtle->length = (turtle->width / 2) * cos(inclination);
      // add leaf to the list
      leafLogicalVolume =
          m_leafConstructor.constructForTree(m_leafSystem, turtle);
      m_candidateLeaves.push_back(
          std::pair<G4LogicalVolume*, G4VPhysicalVolume*>(leafLogicalVolume,
                                                          trunkPhysicalVolume));
      // restore turtle conditions
      turtle->length = storeLength;
      turtle->orientation.SetPhi(storePhi);
      turtle->lVector.SetPhi(storeLVectorPhi);
    }
  }
}

void DetectorConstruction::candidateLeafBuild() {
  // Leaf already positioned within world with initial turtle
  G4Transform3D identityTransform;

  //  std::cout << "SIM: N candidate leaves = " << m_candidateLeaves.size() <<
  //  std::endl;
  for (auto& candidateLeafInfo : m_candidateLeaves) {
    G4LogicalVolume* leafLogicalVolume = candidateLeafInfo.first;
    G4VPhysicalVolume* trunkPhysicalVolume = candidateLeafInfo.second;

    // Check for overlaps with everything in the world!
    bool isOverlapping =
        checkForLeafOverlaps(leafLogicalVolume, trunkPhysicalVolume);

    if (!isOverlapping) {
      // If not overlapping then place the leaf
      /*G4VPhysicalVolume* leafPhysicalVolume =*/new G4PVPlacement(
          identityTransform, leafLogicalVolume, "LeafEnvelope",
          trunkPhysicalVolume->GetMotherLogical(), false, 0);

      // Sum up the leaf area to get the total sensitive area of the
      // detector. (in units of meter squared)
      m_sensitiveSurfaceArea += m_leafConstructor.getSensitiveSurfaceArea();
      m_leafNumber += 1u;
      //      std::cout << "SIM: non-overlapping - PIECE sensitive area = " <<
      //      m_leafConstructor.getSensitiveSurfaceArea() << std::endl;
      //      std::cout << "SIM: non-overlapping - sensitive area = " <<
      //      m_sensitiveSurfaceArea << std::endl;
    } else {
      // Always reject overlapping leaves.
      delete leafLogicalVolume;

      m_rejectedLeafNumber += 1u;
    }
  }
  //  std::cout << "SIM: Leaf Build - sensitive area = " <<
  //  m_sensitiveSurfaceArea << std::endl;

  m_candidateLeaves.clear();
}

bool DetectorConstruction::checkForLeafOverlaps(
    G4LogicalVolume* candidateLeafLogicalVolume,
    G4VPhysicalVolume* parentBranchVolume, G4int resolution, G4double tolerence,
    G4int maximumErrorNumber) {
  G4LogicalVolume* parentLogicalVolume = parentBranchVolume->GetMotherLogical();
  G4VSolid* solid = candidateLeafLogicalVolume->GetSolid();

  if (!parentLogicalVolume) {
    return false;
  }

  G4int trials = 0;
  G4bool retval = false;

  G4VSolid* motherSolid = parentLogicalVolume->GetSolid();

  // Create the transformation from daughter to mother
  G4AffineTransform Tm;

  for (G4int n = 0; n < resolution; n++) {
    // Generate a random point on the solid's surface
    G4ThreeVector point = solid->GetPointOnSurface();

    // Transform the generated point to the mother's coordinate system
    G4ThreeVector mp = Tm.TransformPoint(point);

    // Checking overlaps with the mother volume
    if (motherSolid->Inside(mp) == kOutside) {
      G4double distanceToInside = motherSolid->DistanceToIn(mp);
      if (distanceToInside > tolerence) {
        trials++;
        retval = true;
        std::ostringstream message;

        message << "Overlap with mother volume !" << G4endl
                << "          Overlap is detected for volume "
                << candidateLeafLogicalVolume->GetName() << G4endl
                << "          with its mother volume "
                << parentLogicalVolume->GetName() << G4endl
                << "          at mother local point " << mp << ", "
                << "overlapping by at least: "
                << G4BestUnit(distanceToInside, "Length");

        if (trials >= maximumErrorNumber) {
          message << G4endl << "NOTE: Reached maximum fixed number -"
                  << maximumErrorNumber
                  << "- of overlaps reports for this volume !";
        }

        G4Exception("DetectorContruction::checkForLeafOverlaps()",
                    "LeafOverlap", JustWarning, message);
        if (trials >= maximumErrorNumber) {
          return true;
        }
      }
    }

    // Checking overlaps with each 'sister' volume which are also
    // leaves.
    for (G4int i = 0; i < parentLogicalVolume->GetNoDaughters(); i++) {
      G4VPhysicalVolume* daughter = parentLogicalVolume->GetDaughter(i);

      if (daughter == parentBranchVolume) {
        // Allow overlap with parent branch
        // perhaps should just increase the tolerence
        continue;
      }

      // Create the transformation for daughter volume and transform point
      G4AffineTransform Td(daughter->GetRotation(), daughter->GetTranslation());
      G4ThreeVector md = Td.Inverse().TransformPoint(mp);

      G4VSolid* daughterSolid = daughter->GetLogicalVolume()->GetSolid();
      if (daughterSolid->Inside(md) == kInside) {
        G4double distanceToOutside = daughterSolid->DistanceToOut(md);
        if (distanceToOutside > tolerence) {
          trials++;
          retval = true;
          /*
          std::ostringstream message;
          message << "Overlap with volume already placed !" << G4endl
                  << "          Overlap is detected for volume "
                  << candidateLeafLogicalVolume->GetName() << G4endl
                  << "          with " << daughter->GetName() << " volume's"
                  << G4endl
                  << "          local point " << md << ", "
                  << "overlapping by at least: "
                  << G4BestUnit(distanceToOutside,"Length");
          if (trials>=maximumErrorNumber){
            message << G4endl
                    << "NOTE: Reached maximum fixed number -" <<
          maximumErrorNumber
                    << "- of overlaps reports for this volume !";
          }
          G4Exception("DetectorContruction::checkForLeafOverlaps()",
          "LeafOverlap", JustWarning, message);
          */

          if (trials >= maximumErrorNumber) {
            return true;
          }
        }
      }

      // Now checking that 'sister' volume is not totally included and
      // overlapping. Do it only once, for the first point generated
      if (n == 0) {
        // Generate a single point on the surface of the 'sister' volume
        // and verify that the point is NOT inside the current volume

        G4ThreeVector dPoint = daughterSolid->GetPointOnSurface();

        // Transform the generated point to the mother's coordinate system
        // and finally to current volume's coordinate system
        G4ThreeVector mp2 = Td.TransformPoint(dPoint);
        G4ThreeVector msi = Tm.Inverse().TransformPoint(mp2);

        if (solid->Inside(msi) == kInside) {
          trials++;
          retval = true;
          /*
          std::ostringstream message;
          message << "Overlap with volume already placed !" << G4endl
                  << "          Overlap is detected for volume "
                  << candidateLeafLogicalVolume->GetName() << G4endl
                  << "          apparently fully encapsulating volume "
                  << daughter->GetName() << G4endl
                  << "          at the same level !";
          G4Exception("DetectorContruction::checkForLeafOverlaps()",
                      "LeafOverlap", JustWarning, message);
          */

          if (trials >= maximumErrorNumber) {
            return true;
          }
        }
      }
    }
  }

  return retval;
}
