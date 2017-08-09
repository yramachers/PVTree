#include "pvtree/full/layeredLeafConstruction.hpp"
#include "pvtree/full/leafTrackerSD.hpp"
#include "pvtree/leafSystem/leafConstructionInterface.hpp"
#include "pvtree/geometry/polygon.hpp"
#include "pvtree/geometry/turtle.hpp"
#include "pvtree/full/material/materialFactory.hpp"
#include "assert.h"
#include <algorithm>

#include "G4LogicalBorderSurface.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4OpticalSurface.hh"
#include "G4LogicalVolume.hh"
#include "G4ThreeVector.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4NistManager.hh"
#include "G4SDManager.hh"
#include "G4Orb.hh"

// Need a tessellated solid to represent the leaf.
#include "G4TessellatedSolid.hh"
#include "G4TriangularFacet.hh"

LayeredLeafConstruction::LayeredLeafConstruction(
    std::shared_ptr<LeafConstructionInterface> leafSystem,
    Turtle* initialTurtle)
    : G4VUserDetectorConstruction(),
      m_leafSystem(leafSystem),
      m_initialTurtle(initialTurtle),
      m_worldLogicalVolume(nullptr),
      m_trackerSD(nullptr),
      m_airMaterialName("pv-air"),
      m_frontMaterialName("pv-glass"),
      m_sensitiveMaterialName("pv-silicon"),
      m_backMaterialName("pv-glass"),
      //    m_backMaterialName("pv-aluminium"),
      m_constructedSensitiveDetectors(false),
      m_sensitiveArea(0.0) {
  // Set colours for diffent parts of leaves
  m_frontAttributes.SetColour(
      G4Colour(0.0, 0.6, 1.0, 1.0));  // Blue (transparent)
  m_sensitiveAttributes.SetColour(G4Colour(0.32, 0.84, 0.18, 1.0));  // Green
  m_backAttributes.SetColour(G4Colour(0.73, 0.51, 0.13, 1.0));  // Brown

  // Hide the envelope by default
  G4bool visibility;
  m_envelopeAttributes.SetVisibility(visibility = false);

  // Make sure the world orb doesnt hide everything
  m_worldVisualAttributes.SetForceSolid(
      true);  // Wireframe doesn't work for orbs...
  m_worldVisualAttributes.SetColour(
      G4Colour(0.0, 0.6, 1.0, 0.1));  // Transparent light blue
}

LayeredLeafConstruction::LayeredLeafConstruction()
    : G4VUserDetectorConstruction(),
      m_leafSystem(nullptr),
      m_initialTurtle(nullptr),
      m_worldLogicalVolume(nullptr),
      m_trackerSD(nullptr),
      m_airMaterialName("pv-air"),
      //     m_frontMaterialName("pv-glass"),
      m_frontMaterialName("pv-glass"),
      m_sensitiveMaterialName("pv-silicon"),
      m_backMaterialName("pv-glass"),
      m_constructedSensitiveDetectors(false),
      m_sensitiveArea(0.0) {
  m_frontAttributes.SetColour(
      G4Colour(0.0, 0.6, 1.0, 1.0));  // Blue (transparent)
  m_sensitiveAttributes.SetColour(G4Colour(0.32, 0.84, 0.18, 1.0));  // Green
  m_backAttributes.SetColour(G4Colour(0.73, 0.51, 0.13, 1.0));  // Brown

  // Hide the envelope by default
  G4bool visibility;
  m_envelopeAttributes.SetVisibility(visibility = false);
}

LayeredLeafConstruction::~LayeredLeafConstruction() {}

G4LogicalVolume* LayeredLeafConstruction::getLogicalVolume() {
  return m_worldLogicalVolume;  // For drawing...
}

double LayeredLeafConstruction::getSensitiveSurfaceArea() {
  return m_sensitiveArea;
}

G4VPhysicalVolume* LayeredLeafConstruction::Construct() {
  // Get the leaf logical geometry from current settings
  G4LogicalVolume* leafEnvelope = constructLeafLogicalVolume();

  // Use the leaf envelope solid to get the max and min extents so that
  // the world volume can be defined to be a reasonable size.
  G4TessellatedSolid* envelopeSolid =
      static_cast<G4TessellatedSolid*>(leafEnvelope->GetSolid());

  G4ThreeVector totalMaximums(envelopeSolid->GetMaxXExtent(),
                              envelopeSolid->GetMaxYExtent(),
                              envelopeSolid->GetMaxZExtent());
  G4ThreeVector totalMinimums(envelopeSolid->GetMinXExtent(),
                              envelopeSolid->GetMinYExtent(),
                              envelopeSolid->GetMinZExtent());

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

  // Applying a scale to the world size. Doesn't matter too greatly in the case
  // of leaf
  // only simulation/visualization.
  double fudgeScaleFactor = 1.5;
  maximumBoundingBoxX *= fudgeScaleFactor;
  maximumBoundingBoxY *= fudgeScaleFactor;
  maximumBoundingBoxZ *= fudgeScaleFactor;

  double boundingRadius = std::sqrt(std::pow(maximumBoundingBoxX, 2.0) +
                                    std::pow(maximumBoundingBoxY, 2.0) +
                                    std::pow(maximumBoundingBoxZ, 2.0));

  // Create the 'World' (which has to be centred at coordinates 0.0, 0.0, 0.0)
  // Using a sphere so that the photon field is always disk shaped
  // Multiply by root3 to account for the fact that the photons have to
  // originate outside the 'snug' bounding volume
  double pRmax;
  G4Orb* worldOrb = new G4Orb("World", pRmax = std::sqrt(3.0) * boundingRadius);

  // Get air material from factory
  G4Material* airMaterial =
      MaterialFactory::instance()->getMaterial(m_airMaterialName);

  m_worldLogicalVolume =
      new G4LogicalVolume(worldOrb, airMaterial, "World", 0, 0, 0);
  m_worldLogicalVolume->SetVisAttributes(m_worldVisualAttributes);

  G4VPhysicalVolume* worldPhysicalVolume =
      new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, 0.0), m_worldLogicalVolume,
                        "World", 0, false, 0);

  // Convert the logical volumes into physical volumes embeddeded in the world
  // Put into envelope initially!
  G4Transform3D
      identityTransform;  // Already positioned within world with initial turtle

  //(const G4Transform3D &Transform3D, G4LogicalVolume *pCurrentLogical, const
  //G4String &pName, G4LogicalVolume *pMotherLogical, G4bool pMany, G4int
  //pCopyNo)
  new G4PVPlacement(identityTransform, leafEnvelope, "LeafEnvelope",
                    m_worldLogicalVolume, false, 0);

  return worldPhysicalVolume;
}

G4LogicalVolume* LayeredLeafConstruction::constructForTree(
    std::shared_ptr<LeafConstructionInterface> leafSystem,
    Turtle* initialTurtle) {
  // Make sure to update these settings (may have not been specified in the
  // constructor)
  m_leafSystem = leafSystem;
  m_initialTurtle = initialTurtle;

  // Get the leaf logical geometry from current settings
  G4LogicalVolume* leafEnvelope = constructLeafLogicalVolume();

  return leafEnvelope;
}

void LayeredLeafConstruction::ConstructSDandField() {
  // Turn all the leaves into sensitive detectors.
  //  if (!m_constructedSensitiveDetectors) {
  //  std::cout << "SIM: in Leaf SD Construct()" << std::endl;
  G4String photovoltaicCellsName = "PVTree/LeafSensitiveDetector";

  // Check if the sensitive detector has already been constructed elsewhere
  bool showSearchWarning = false;
  m_trackerSD = static_cast<LeafTrackerSD*>(
      G4SDManager::GetSDMpointer()->FindSensitiveDetector(photovoltaicCellsName,
                                                          showSearchWarning));

  if (m_trackerSD == 0) {
    m_trackerSD =
        new LeafTrackerSD(photovoltaicCellsName, "TrackerHitsCollection");
    G4SDManager::GetSDMpointer()->AddNewDetector(m_trackerSD);
  }
  //  std::cout << "SIM: in Leaf SD Construct(), tracker SD check 2: " 
  //	    << m_trackerSD->GetName() << " is active? "
  //	    << m_trackerSD->isActive() << std::endl;

  // Set as sensitive all the leave's logical volumes
  SetSensitiveDetector("LeafSensitive", m_trackerSD, true);

}

void LayeredLeafConstruction::iterateLSystem() {
  // Reset to initial conditions
  m_leafConditions.clear();
  m_leafConditions = m_leafSystem->getInitialConditions();

  int leafIterationNumber =
      m_leafSystem->getIntegerParameter("iterationNumber");

  for (int i = 0; i < leafIterationNumber; i++) {
    std::vector<std::shared_ptr<LeafSystemInterface>> latestConditions;

    for (const auto& condition : m_leafConditions) {
      for (const auto& newCondition : condition->applyRule()) {
        latestConditions.push_back(newCondition);
      }
    }

    // Use the new iteration
    m_leafConditions.clear();
    m_leafConditions = latestConditions;
  }
}

std::vector<Polygon*> LayeredLeafConstruction::generateSurface() {
  std::vector<Polygon*> candidateSurfacePolygons;

  std::vector<Turtle*> activeTurtles;
  std::vector<Turtle*> retiredTurtles;

  // Create initial active turtle (start from end of last turtle)
  TVector3 startPosition =
      m_initialTurtle->position +
      m_initialTurtle->length * m_initialTurtle->orientation;
  activeTurtles.push_back(new Turtle(
      startPosition, m_initialTurtle->orientation, m_initialTurtle->lVector));

  // Process all the conditions (convert into turtles)
  for (auto& condition : m_leafConditions) {
    condition->processTurtles(activeTurtles, retiredTurtles,
                              candidateSurfacePolygons);
  }

  // Remove the last active turtle
  delete activeTurtles.back();
  activeTurtles.pop_back();

  assert(activeTurtles.size() == 0);

  // Don't actually want to keep the turtles (they are not important in this
  // case).
  for (auto& turtle : retiredTurtles) {
    delete turtle;
  }

  retiredTurtles.clear();

  // Remove some problematic triangles here.
  std::vector<Polygon*> surfacePolygons;
  for (auto& polygon : candidateSurfacePolygons) {
    // Check that the polygon is valid (3 vertices and all at different
    // positions).
    if (polygon->size() != 3) {
      continue;
    }

    bool badTriangle = false;
    for (unsigned int v = 1; v < polygon->size(); v++) {
      if ((polygon->getVertex(0)->getPosition() -
           polygon->getVertex(v)->getPosition()).Mag() < 0.0001) {
        badTriangle = true;
      }
    }

    if (badTriangle) {
      continue;
    }

    surfacePolygons.push_back(polygon);
  }

  return surfacePolygons;
}

G4LogicalVolume* LayeredLeafConstruction::constructLeafLogicalVolume() {
  // Check all necessary inputs have been provided
  if (m_leafSystem == nullptr || m_initialTurtle == nullptr) {
    std::cerr << "Did not construct LayeredLeafConstruction with appropriate "
                 "information." << std::endl;
    throw;
  }

  // Iterate the LSystem conditions
  iterateLSystem();

  // Construct the surface of the leaf
  std::vector<Polygon*> initialSystemSurface = generateSurface();

  // Obtain the total thickness to be used for the leaf
  double thickness = m_leafSystem->getDoubleParameter("thickness");

  // Create the meshes by extrapolating the system surface
  std::vector<Polygon*> frontMesh = extrapolateSurfaceIntoMesh(
      initialSystemSurface, 0.5 * thickness, 0.03 * thickness);
  std::vector<Polygon*> sensitiveMesh = extrapolateSurfaceIntoMesh(
      initialSystemSurface, 0.03 * thickness, 0.0 * thickness);
  std::vector<Polygon*> backMesh =
      extrapolateSurfaceIntoMesh(initialSystemSurface, 0.0, -0.5 * thickness);
  std::vector<Polygon*> envelopeMesh = extrapolateSurfaceIntoMesh(
      initialSystemSurface, 0.5 * thickness, -0.5 * thickness);

  // Calculate the surface area of the front of the sensitive mesh
  double currentSurfaceArea = calculateExtrapolatedSurfaceArea(
      initialSystemSurface, 0.03 * thickness, 0.0 * thickness);
  m_sensitiveArea = currentSurfaceArea;
  //  std::cout << "SIM: Layered Leaf Build - sensitive area = " <<
  //  currentSurfaceArea << std::endl;

  // Convert the meshes into Geant4 solids
  G4TessellatedSolid* frontSolid =
      convertMeshToTessellatedSolid(frontMesh, std::string("LeafFrontSolid"));
  G4TessellatedSolid* sensitiveSolid = convertMeshToTessellatedSolid(
      sensitiveMesh, std::string("LeafSensitiveSolid"));
  G4TessellatedSolid* backSolid =
      convertMeshToTessellatedSolid(backMesh, std::string("LeafBackSolid"));
  G4TessellatedSolid* envelopeSolid = convertMeshToTessellatedSolid(
      envelopeMesh, std::string("LeafEnvelopeSolid"));

  // Get the materials from the factory
  G4Material* frontMaterial =
      MaterialFactory::instance()->getMaterial(m_frontMaterialName);
  G4Material* sensitiveMaterial =
      MaterialFactory::instance()->getMaterial(m_sensitiveMaterialName);
  G4Material* backMaterial =
      MaterialFactory::instance()->getMaterial(m_backMaterialName);
  G4Material* airMaterial =
      MaterialFactory::instance()->getMaterial(m_airMaterialName);

  // Finally make the leaf geometric sandwich.
  G4LogicalVolume* frontLogicalVolume =
      new G4LogicalVolume(frontSolid, frontMaterial, "LeafFront");
  G4LogicalVolume* sensitiveLogicalVolume =
      new G4LogicalVolume(sensitiveSolid, sensitiveMaterial, "LeafSensitive");
  G4LogicalVolume* backLogicalVolume =
      new G4LogicalVolume(backSolid, backMaterial, "LeafBack");
  G4LogicalVolume* envelopeLogicalVolume =
      new G4LogicalVolume(envelopeSolid, airMaterial, "LeafEnvelope");

  // Set Visual attributes for each volume
  frontLogicalVolume->SetVisAttributes(m_frontAttributes);
  sensitiveLogicalVolume->SetVisAttributes(m_sensitiveAttributes);
  backLogicalVolume->SetVisAttributes(m_backAttributes);
  envelopeLogicalVolume->SetVisAttributes(m_envelopeAttributes);

  // Get the optical surface properties from the factory
  G4OpticalSurface* frontOpticalSurface =
      MaterialFactory::instance()->getOpticalSurface(m_frontMaterialName);
  G4OpticalSurface* sensitiveOpticalSurface =
      MaterialFactory::instance()->getOpticalSurface(m_sensitiveMaterialName);
  G4OpticalSurface* backOpticalSurface =
      MaterialFactory::instance()->getOpticalSurface(m_backMaterialName);

  // Set the skin surface properties (but not silicon?)
  new G4LogicalSkinSurface("LeafFrontSkin", frontLogicalVolume,
                           frontOpticalSurface);
  new G4LogicalSkinSurface("LeafBackSkin", backLogicalVolume,
                           backOpticalSurface);

  // Convert the logical volumes into physical volumes embeddeded in the
  // envelope
  G4Transform3D identityTransform;
  G4VPhysicalVolume* frontPhysicalVolume =
      new G4PVPlacement(identityTransform, frontLogicalVolume, "LeafFront",
                        envelopeLogicalVolume, false, 0);
  G4VPhysicalVolume* sensitivePhysicalVolume =
      new G4PVPlacement(identityTransform, sensitiveLogicalVolume,
                        "LeafSensitive", envelopeLogicalVolume, false, 0);
  G4VPhysicalVolume* backPhysicalVolume =
      new G4PVPlacement(identityTransform, backLogicalVolume, "LeafBack",
                        envelopeLogicalVolume, false, 0);

  // Setup the optical boundary between the front and sensitive part
  new G4LogicalBorderSurface("Front_Sensitve_Border", frontPhysicalVolume,
                             sensitivePhysicalVolume, sensitiveOpticalSurface);
  new G4LogicalBorderSurface("Back_Sensitve_Border", backPhysicalVolume,
                             sensitivePhysicalVolume, sensitiveOpticalSurface);

  // Clean up the polygon lists to prevent a memory leak.
  clearPolygonList(initialSystemSurface);
  clearPolygonList(frontMesh);
  clearPolygonList(sensitiveMesh);
  clearPolygonList(backMesh);
  clearPolygonList(envelopeMesh);

  return envelopeLogicalVolume;
}

void LayeredLeafConstruction::getExtent(std::vector<Polygon*>& polygons,
                                        G4ThreeVector& minExtent,
                                        G4ThreeVector& maxExtent) {
  for (auto& polygon : polygons) {
    for (unsigned int v = 0; v < polygon->size(); v++) {
      G4ThreeVector g4Position(
          convertVector(polygon->getVertex(v)->getPosition()));

      auto result = std::minmax({g4Position.x(), minExtent.x(), maxExtent.x()});
      minExtent.setX(result.first);
      maxExtent.setX(result.second);

      result = std::minmax({g4Position.y(), minExtent.y(), maxExtent.y()});
      minExtent.setY(result.first);
      maxExtent.setY(result.second);

      result = std::minmax({g4Position.z(), minExtent.z(), maxExtent.z()});
      minExtent.setZ(result.first);
      maxExtent.setZ(result.second);
    }
  }
}

void LayeredLeafConstruction::getExtentForTree(G4ThreeVector& minExtent,
                                               G4ThreeVector& maxExtent) {
  // Check all necessary inputs have been provided
  if (m_leafSystem == nullptr || m_initialTurtle == nullptr) {
    std::cerr << "Did not construct LayeredLeafConstruction with appropriate "
                 "information." << std::endl;
    throw;
  }

  // Iterate the LSystem conditions
  iterateLSystem();

  // Construct the surface of the leaf
  std::vector<Polygon*> initialSystemSurface = generateSurface();

  // Obtain the total thickness to be used for the leaf
  double thickness = m_leafSystem->getDoubleParameter("thickness");

  // Just create the envelope mesh
  std::vector<Polygon*> envelopeMesh = extrapolateSurfaceIntoMesh(
      initialSystemSurface, 0.5 * thickness, -0.5 * thickness);

  // Use the envelope to check if the extent is outside current maximum range
  getExtent(envelopeMesh, minExtent, maxExtent);

  // Clean up
  clearPolygonList(envelopeMesh);
}

void LayeredLeafConstruction::getExtentForTree(
    std::shared_ptr<LeafConstructionInterface> leafSystem,
    Turtle* initialTurtle, G4ThreeVector& minExtent, G4ThreeVector& maxExtent) {
  // Make sure to update these settings (may have not been specified in the
  // constructor)
  m_leafSystem = leafSystem;
  m_initialTurtle = initialTurtle;

  getExtentForTree(minExtent, maxExtent);
}

G4ThreeVector LayeredLeafConstruction::convertVector(const TVector3& input) {
  G4ThreeVector output(input.X() * m, input.Y() * m, input.Z() * m);
  return output;
}

std::vector<std::shared_ptr<Vertex>> LayeredLeafConstruction::mergeVertices(
    std::vector<Polygon*>& polygons) {
  double mergeDistance = 0.00000001;
  std::vector<std::shared_ptr<Vertex>> uniqueVertices;

  // This might be a little slow...
  for (auto& face : polygons) {
    for (unsigned int v = 0; v < face->size(); v++) {
      // Check if vertex is within merging distance of previous unique vertex
      bool isUnique = true;

      for (auto& uniqueVertex : uniqueVertices) {
        double separation = (uniqueVertex->getPosition() -
                             face->getVertex(v)->getPosition()).Mag();

        if (separation < mergeDistance) {
          // replace the vertex in the current face with previous unique vertex
          face->replaceVertex(face->getVertex(v), uniqueVertex);
          isUnique = false;
          break;
        }
      }

      if (isUnique) {
        uniqueVertices.push_back(face->getVertex(v));
      }
    }
  }

  return uniqueVertices;
}

std::vector<Polygon*> LayeredLeafConstruction::extrapolateSurfaceIntoMesh(
    const std::vector<Polygon*>& polygons, double frontSurfaceOffsetFactor,
    double backSurfaceOffsetFactor) {
  // Duplicate the surface polygons to create the two surfaces
  // also keep a copy of the source surface so that the edge can
  // be created.
  std::vector<Polygon*> frontSurface;
  std::vector<Polygon*> backSurface;
  std::vector<Polygon*> sourceSurface;

  for (auto& polygon : polygons) {
    frontSurface.push_back(new Polygon(*polygon));
    backSurface.push_back(new Polygon(*polygon));
    sourceSurface.push_back(new Polygon(*polygon));
  }

  // Merge together vertices before extrapolation
  std::vector<std::shared_ptr<Vertex>> frontUniqueVertices =
      mergeVertices(frontSurface);
  std::vector<std::shared_ptr<Vertex>> backUniqueVertices =
      mergeVertices(backSurface);
  mergeVertices(sourceSurface);

  // Extrapolate surfaces along the vertex normals
  // Don't set straight away, otherwise normal calculation changes!
  std::vector<TVector3> updatedPositions;
  for (unsigned int v = 0; v < frontUniqueVertices.size(); v++) {
    updatedPositions.push_back(frontUniqueVertices[v]->getPosition() +
                               frontUniqueVertices[v]->getNormal() *
                                   frontSurfaceOffsetFactor);
  }
  for (unsigned int v = 0; v < frontUniqueVertices.size(); v++) {
    frontUniqueVertices[v]->setPosition(updatedPositions[v]);
  }
  updatedPositions.clear();

  for (unsigned int v = 0; v < backUniqueVertices.size(); v++) {
    updatedPositions.push_back(backUniqueVertices[v]->getPosition() +
                               backUniqueVertices[v]->getNormal() *
                                   backSurfaceOffsetFactor);
  }
  for (unsigned int v = 0; v < backUniqueVertices.size(); v++) {
    backUniqueVertices[v]->setPosition(updatedPositions[v]);
  }
  updatedPositions.clear();

  // Invert the back surface faces
  for (unsigned int p = 0; p < backSurface.size(); p++) {
    backSurface[p]->invertNormal();
  }

  // Create the edge surface
  std::vector<Polygon*> edgeSurface = createEdgeSurface(
      sourceSurface, frontSurfaceOffsetFactor, backSurfaceOffsetFactor);

  // Put all the polygons into a single vector to be returned
  std::vector<Polygon*> meshSurfacePolygons;
  meshSurfacePolygons.insert(meshSurfacePolygons.end(), frontSurface.begin(),
                             frontSurface.end());
  meshSurfacePolygons.insert(meshSurfacePolygons.end(), backSurface.begin(),
                             backSurface.end());
  meshSurfacePolygons.insert(meshSurfacePolygons.end(), edgeSurface.begin(),
                             edgeSurface.end());

  // Clear the source surface polygons
  clearPolygonList(sourceSurface);

  // Return the constructed mesh
  return meshSurfacePolygons;
}

double LayeredLeafConstruction::calculateExtrapolatedSurfaceArea(
    const std::vector<Polygon*>& polygons, double frontSurfaceOffsetFactor,
    double backSurfaceOffsetFactor) {
  // Duplicate the surface polygons to create the two surfaces
  // also keep a copy of the source surface so that the edge can
  // be created.
  std::vector<Polygon*> frontSurface;
  std::vector<Polygon*> sourceSurface;

  for (auto& polygon : polygons) {
    frontSurface.push_back(new Polygon(*polygon));
    sourceSurface.push_back(new Polygon(*polygon));
  }

  // Merge together vertices before extrapolation
  std::vector<std::shared_ptr<Vertex>> frontUniqueVertices =
      mergeVertices(frontSurface);
  mergeVertices(sourceSurface);

  // Extrapolate surfaces along the vertex normals
  // Don't set straight away, otherwise normal calculation changes!
  std::vector<TVector3> updatedPositions;
  for (unsigned int v = 0; v < frontUniqueVertices.size(); v++) {
    updatedPositions.push_back(frontUniqueVertices[v]->getPosition() +
                               frontUniqueVertices[v]->getNormal() *
                                   frontSurfaceOffsetFactor);
  }
  for (unsigned int v = 0; v < frontUniqueVertices.size(); v++) {
    frontUniqueVertices[v]->setPosition(updatedPositions[v]);
  }
  updatedPositions.clear();

  // Create the edges
  std::vector<Polygon*> edgeSurface = createEdgeSurface(
      sourceSurface, frontSurfaceOffsetFactor, backSurfaceOffsetFactor);

  // Use the front and edge surfaces to calculate the current 'visible'
  // sensitive area
  double surfaceArea = 0.0;

  for (auto& polygon : frontSurface) {
    surfaceArea += polygon->getArea();
  }
  for (auto& polygon : edgeSurface) {
    surfaceArea += polygon->getArea();
  }

  clearPolygonList(frontSurface);
  clearPolygonList(edgeSurface);
  clearPolygonList(sourceSurface);

  return surfaceArea;
}

std::vector<Polygon*> LayeredLeafConstruction::createEdgeSurface(
    const std::vector<Polygon*>& surfacePolygons,
    double frontSurfaceOffsetFactor, double backSurfaceOffsetFactor) {
  // Construct the edge connecting the two surfaces
  std::vector<std::pair<std::shared_ptr<Vertex>, std::shared_ptr<Vertex>>>
      allEdges;
  std::vector<std::pair<std::shared_ptr<Vertex>, std::shared_ptr<Vertex>>>
      uniqueEdges;

  for (auto& polygon : surfacePolygons) {
    // Check that the polygon is valid (3 vertices and all at different
    // positions).
    if (polygon->size() != 3) {
      continue;
    }

    bool badTriangle = false;
    for (unsigned int v = 1; v < polygon->size(); v++) {
      if ((polygon->getVertex(0)->getPosition() -
           polygon->getVertex(v)->getPosition()).Mag() < 0.0000001) {
        badTriangle = true;
      }
    }

    if (badTriangle) {
      continue;
    }

    allEdges.push_back(
        std::pair<std::shared_ptr<Vertex>, std::shared_ptr<Vertex>>(
            polygon->getVertex(0), polygon->getVertex(1)));
    allEdges.push_back(
        std::pair<std::shared_ptr<Vertex>, std::shared_ptr<Vertex>>(
            polygon->getVertex(1), polygon->getVertex(2)));
    allEdges.push_back(
        std::pair<std::shared_ptr<Vertex>, std::shared_ptr<Vertex>>(
            polygon->getVertex(2), polygon->getVertex(0)));
  }

  for (unsigned int e = 0; e < allEdges.size(); e++) {
    bool isUnique = true;

    for (unsigned int c = 0; c < allEdges.size(); c++) {
      if (c == e) {
        // Skip obvious fail
        continue;
      }

      if (allEdges[e].first == allEdges[c].first &&
          allEdges[e].second == allEdges[c].second) {
        isUnique = false;
        break;
      }

      if (allEdges[e].first == allEdges[c].second &&
          allEdges[e].second == allEdges[c].first) {
        isUnique = false;
        break;
      }
    }

    if (isUnique) {
      uniqueEdges.push_back(allEdges[e]);
    }
  }

  // For each unique edge create create a pair of faces to fill the gap
  std::vector<Polygon*> edgeSurface;
  for (unsigned int e = 0; e < uniqueEdges.size(); e++) {
    // Get the edge normal (needed to check that edge polygons are pointing in
    // the right direction)
    TVector3 edgeFaceNormal =
        uniqueEdges[e].first->getEdgeNormal(uniqueEdges[e].second);

    // Copy and extrapolate vertices
    TVector3 frontPosition1 =
        uniqueEdges[e].first->getPosition() +
        uniqueEdges[e].first->getNormal() * frontSurfaceOffsetFactor;
    TVector3 frontPosition2 =
        uniqueEdges[e].second->getPosition() +
        uniqueEdges[e].second->getNormal() * frontSurfaceOffsetFactor;
    TVector3 backPosition1 =
        uniqueEdges[e].first->getPosition() +
        uniqueEdges[e].first->getNormal() * backSurfaceOffsetFactor;
    TVector3 backPosition2 =
        uniqueEdges[e].second->getPosition() +
        uniqueEdges[e].second->getNormal() * backSurfaceOffsetFactor;

    Polygon* face1 = new Polygon();
    face1->addVertex(std::shared_ptr<Vertex>(new Vertex(frontPosition1)));
    face1->addVertex(std::shared_ptr<Vertex>(new Vertex(backPosition1)));
    face1->addVertex(std::shared_ptr<Vertex>(new Vertex(frontPosition2)));

    if (edgeFaceNormal.Dot(face1->getNormal()) < 0.0) {
      face1->invertNormal();
    }

    edgeSurface.push_back(face1);

    Polygon* face2 = new Polygon();
    face2->addVertex(std::shared_ptr<Vertex>(new Vertex(frontPosition2)));
    face2->addVertex(std::shared_ptr<Vertex>(new Vertex(backPosition1)));
    face2->addVertex(std::shared_ptr<Vertex>(new Vertex(backPosition2)));

    if (edgeFaceNormal.Dot(face2->getNormal()) < 0.0) {
      face2->invertNormal();
    }

    edgeSurface.push_back(face2);
  }

  return edgeSurface;
}

G4TessellatedSolid* LayeredLeafConstruction::convertMeshToTessellatedSolid(
    const std::vector<Polygon*>& polygons, std::string solidName) {
  // Create the tesselated solid in Geant4 geometry
  G4TessellatedSolid* solid = new G4TessellatedSolid(solidName);

  for (auto& polygon : polygons) {
    // Check that the polygon is valid (3 vertices and all at different
    // positions).
    if (polygon->size() != 3) {
      continue;
    }

    bool badTriangle = false;
    for (unsigned int v = 1; v < polygon->size(); v++) {
      if ((polygon->getVertex(0)->getPosition() -
           polygon->getVertex(v)->getPosition()).Mag() < 0.0000001) {
        badTriangle = true;
      }
    }

    if (badTriangle) {
      continue;
    }

    G4TriangularFacet* facet = new G4TriangularFacet(
        convertVector(polygon->getVertex(0)->getPosition()),
        convertVector(polygon->getVertex(1)->getPosition()),
        convertVector(polygon->getVertex(2)->getPosition()), ABSOLUTE);

    solid->AddFacet((G4VFacet*)facet);
  }

  solid->SetSolidClosed(true);

  return solid;
}

void LayeredLeafConstruction::clearPolygonList(
    std::vector<Polygon*>& polygons) {
  for (auto& polygon : polygons) {
    delete polygon;
  }

  polygons.clear();
}
