#include "full/leafConstruction.hpp"
#include "full/leafTrackerSD.hpp"
#include "leafSystem/leafConstructionInterface.hpp"
#include "geometry/polygon.hpp"
#include "geometry/turtle.hpp"
#include "material/materialFactory.hpp"
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
#include "G4SDManager.hh"
#include "G4Orb.hh"

//Need a tessellated solid to represent the leaf.
#include "G4TessellatedSolid.hh"
#include "G4TriangularFacet.hh"

LeafConstruction::LeafConstruction(std::shared_ptr<LeafConstructionInterface> leafConstructor,
				   Turtle* initialTurtle) 
  : G4VUserDetectorConstruction(), 
    m_leafConstructor(leafConstructor),
    m_initialTurtle(initialTurtle),
    m_leafSolid(nullptr),
    m_worldLogicalVolume(nullptr),
    m_trackerSD(nullptr),
    m_airMaterialName("pv-air"),
    m_sensitiveMaterialName("pv-silicon"),
    m_constructedSensitiveDetectors(false),
    m_leafArea(0.0) {

  m_leafVisualAttributes.SetColour( G4Colour(0.32, 0.84, 0.18, 0.7) ); //Green

  // Make sure the world orb doesnt hide everything
  m_worldVisualAttributes.SetForceSolid(true); //Wireframe doesn't work for orbs...
  m_worldVisualAttributes.SetColour( G4Colour(0.0, 0.6, 1.0, 0.1) ); // Transparent light blue
}

LeafConstruction::LeafConstruction() 
  : G4VUserDetectorConstruction(), 
    m_leafConstructor(nullptr),
    m_initialTurtle(nullptr),
    m_leafSolid(nullptr),
    m_worldLogicalVolume(nullptr),
    m_trackerSD(nullptr),
    m_airMaterialName("pv-air"),
    m_sensitiveMaterialName("pv-silicon"),
    m_constructedSensitiveDetectors(false),
    m_leafArea(0.0) {

  m_leafVisualAttributes.SetColour( G4Colour(0.32, 0.84, 0.18, 0.7) ); //Green

  //Make sure the world box doesn't hide everything
  m_worldVisualAttributes.SetForceWireframe(true);
  m_worldVisualAttributes.SetColour( G4Colour(0.0, 0.6, 1.0, 1.0) );
}

LeafConstruction::~LeafConstruction(){

  //Need to always clean these up
  clearPolygonLists();

}

G4LogicalVolume* LeafConstruction::getLogicalVolume() {
  return m_worldLogicalVolume; //For drawing...
}

double LeafConstruction::getSensitiveSurfaceArea() {
  return m_leafArea;
}

void LeafConstruction::clearPolygonLists() {

  //Delete any created polygons (choose the most inclusive that is filled!)
  if (m_completeLeaf.size() > 0){
    for (auto& polygon : m_completeLeaf){
      delete polygon;
    }

    m_completeLeaf.clear();
  } else { 
    for (auto& polygon : m_leafSurface){
      delete polygon;
    }
  }

  m_leafSurface.clear();

}

G4VPhysicalVolume* LeafConstruction::Construct() {

  //Start by iterating the LSystem conditions
  iterateLSystem();

  //Construct the surface of the leaf
  generateSurface();

  //Create the leaf solid
  solidifyLeaf();

  //Extract the area
  m_leafArea = m_leafSolid->GetSurfaceArea()/meter2;

  //Find the total extent of the leaf geometry to build the world volume
  G4ThreeVector totalMaximums( convertVector(m_completeLeaf[0]->getVertex(0)->getPosition()) );
  G4ThreeVector totalMinimums( convertVector(m_completeLeaf[0]->getVertex(0)->getPosition()) );

  getExtent(m_completeLeaf, totalMinimums, totalMaximums);

  //Calculate the rough bounding 
  double maximumBoundingBoxX = (fabs(totalMaximums.x()) > fabs(totalMinimums.x()) ? fabs(totalMaximums.x()) : fabs(totalMinimums.x()));
  double maximumBoundingBoxY = (fabs(totalMaximums.y()) > fabs(totalMinimums.y()) ? fabs(totalMaximums.y()) : fabs(totalMinimums.y()));
  double maximumBoundingBoxZ = (fabs(totalMaximums.z()) > fabs(totalMinimums.z()) ? fabs(totalMaximums.z()) : fabs(totalMinimums.z()));

  // Applying a scale to the world box size
  double fudgeScaleFactor = 1.5;
  maximumBoundingBoxX *= fudgeScaleFactor;
  maximumBoundingBoxY *= fudgeScaleFactor;
  maximumBoundingBoxZ *= fudgeScaleFactor;

  double boundingRadius = std::sqrt( std::pow(maximumBoundingBoxX,2.0) + std::pow(maximumBoundingBoxY,2.0) + std::pow(maximumBoundingBoxZ,2.0) );

  //Create the 'World' (which has to be centred at coordinates 0.0, 0.0, 0.0)
  //Using a sphere so that the photon field is always disk shaped
  //G4Orb (const G4String &pName, G4double pRmax)
  //Multiply by root3 to account for the fact that the photons have to originate outside the 'snug' bounding volume
  G4Orb* worldOrb = new G4Orb("World", std::sqrt(3.0) * boundingRadius); 

  // Get air material from factory
  G4Material* airMaterial = MaterialFactory::instance()->getMaterial(m_airMaterialName);

  m_worldLogicalVolume = new G4LogicalVolume(worldOrb, airMaterial,"World",0,0,0);
  m_worldLogicalVolume->SetVisAttributes(m_worldVisualAttributes);

  G4VPhysicalVolume* worldPhysicalVolume = new G4PVPlacement(0, 
							     G4ThreeVector(0.0, 0.0, 0.0), 
							     m_worldLogicalVolume, 
							     "World", 
							     0, 
							     false, 
							     0);

  // Get the sensitive material from factory
  G4Material* sensitiveMaterial = MaterialFactory::instance()->getMaterial(m_sensitiveMaterialName);
  G4OpticalSurface* sensitiveOpticalSurface = MaterialFactory::instance()->getOpticalSurface(m_sensitiveMaterialName);

  //Convert the solid into a physical volume embedded in the world
  G4LogicalVolume* leafLogicalVolume = new G4LogicalVolume(m_leafSolid, sensitiveMaterial, "Leaf");
  leafLogicalVolume->SetVisAttributes(m_leafVisualAttributes);
  new G4LogicalSkinSurface("LeafSkin", leafLogicalVolume, sensitiveOpticalSurface);

  //(const G4Transform3D &Transform3D, G4LogicalVolume *pCurrentLogical, const G4String &pName, G4LogicalVolume *pMotherLogical, G4bool pMany, G4int pCopyNo)
  G4Transform3D identityTransform; //Already positioned within world with initial turtle
  new G4PVPlacement(identityTransform, leafLogicalVolume, "Leaf", m_worldLogicalVolume, false, 0);

  return worldPhysicalVolume; 
}

G4LogicalVolume* LeafConstruction::constructForTree(std::shared_ptr<LeafConstructionInterface> leafConstructor,
						    Turtle* initialTurtle) {
  
  m_leafConstructor = leafConstructor;
  m_initialTurtle = initialTurtle;

  //Iterate the LSystem conditions
  iterateLSystem();

  //Construct the surface of the leaf
  generateSurface();

  //Convert the leaf surface polygons into a solid
  solidifyLeaf();

  //Extract the area
  m_leafArea = m_leafSolid->GetSurfaceArea()/meter2;

  // Get the sensitive material from factory
  G4Material* sensitiveMaterial = MaterialFactory::instance()->getMaterial(m_sensitiveMaterialName);
  G4OpticalSurface* sensitiveOpticalSurface = MaterialFactory::instance()->getOpticalSurface(m_sensitiveMaterialName);

  //Create the logical volume and set optical properties
  G4LogicalVolume* leafLogicalVolume = new G4LogicalVolume(m_leafSolid, sensitiveMaterial, "Leaf");
  leafLogicalVolume->SetVisAttributes(m_leafVisualAttributes);
  new G4LogicalSkinSurface("LeafSkin", leafLogicalVolume, sensitiveOpticalSurface);

  return leafLogicalVolume;
}

void LeafConstruction::ConstructSDandField() {

  // Turn all the leaves into sensitive detectors.
  if (!m_constructedSensitiveDetectors) {
    G4String photovoltaicCellsName = "PVTree/LeafSensitiveDetector";

    // Check if the sensitive detector has already been constructed elsewhere
    bool showSearchWarning = false;
    m_trackerSD = static_cast<LeafTrackerSD*>( G4SDManager::GetSDMpointer()->FindSensitiveDetector(photovoltaicCellsName, showSearchWarning) );

    if (m_trackerSD == 0){
      m_trackerSD = new LeafTrackerSD(photovoltaicCellsName, "TrackerHitsCollection");
    }

    //Set as sensitive all the leave's logical volumes 
    SetSensitiveDetector("Leaf", m_trackerSD, true);

    m_constructedSensitiveDetectors = true;
  } else { 
    //If the sensitive detectors were previously setup don't completely recreate...

    //Set as sensitive all the leave's logical volumes 
    SetSensitiveDetector("Leaf", m_trackerSD, true);
  }

}


void LeafConstruction::iterateLSystem() {
  
  //Reset to initial conditions
  m_leafConditions.clear();
  m_leafConditions = m_leafConstructor->getInitialConditions();

  int leafIterationNumber = m_leafConstructor->getIntegerParameter("iterationNumber");

  for (int i=0; i<leafIterationNumber; i++){
    std::vector<std::shared_ptr<LeafSystemInterface> > latestConditions;

    for (const auto & condition : m_leafConditions) {
      for (const auto & newCondition : condition->applyRule()) {
	latestConditions.push_back(newCondition);
      }
    }
    
    //Use the new iteration
    m_leafConditions.clear();
    m_leafConditions = latestConditions;
  }

}

void LeafConstruction::generateSurface() {

  //Always make sure that previous polygons are deleted
  clearPolygonLists();

  std::vector<Turtle*> activeTurtles;
  std::vector<Turtle*> retiredTurtles;

  //Create initial active turtle (start from end of last turtle)
  TVector3 startPosition = m_initialTurtle->position + m_initialTurtle->length*m_initialTurtle->orientation;
  activeTurtles.push_back(new Turtle(startPosition, m_initialTurtle->orientation, m_initialTurtle->lVector));

  //Process all the conditions (convert into turtles)
  for ( auto& condition : m_leafConditions ){
    condition->processTurtles(activeTurtles, retiredTurtles, m_leafSurface);
  }

  //Remove the last active turtle
  delete activeTurtles.back();
  activeTurtles.pop_back();

  assert(activeTurtles.size() == 0);

  //Also don't actually want to keep the turtles (they are not important in this case).
  for (auto& turtle : retiredTurtles){
    delete turtle;
  }

  retiredTurtles.clear();
}

void LeafConstruction::getExtent(std::vector<Polygon*> polygons,
				 G4ThreeVector& minExtent, 
				 G4ThreeVector& maxExtent) {

  for ( auto& polygon : polygons ) {
    for ( unsigned int v=0; v<polygon->size(); v++ ) {
      
      G4ThreeVector g4Position( convertVector(polygon->getVertex(v)->getPosition()) );

      auto result = std::minmax({g4Position.x(),minExtent.x(),maxExtent.x()});
      minExtent.setX(result.first);
      maxExtent.setX(result.second);
				
      result = std::minmax({g4Position.y(),minExtent.y(),maxExtent.y()});
      minExtent.setY(result.first);
      maxExtent.setY(result.second);
      
      result = std::minmax({g4Position.z(),minExtent.z(),maxExtent.z()});
      minExtent.setZ(result.first);
      maxExtent.setZ(result.second);
    }
  }
}

void LeafConstruction::getExtentForTree(G4ThreeVector& minExtent, 
					G4ThreeVector& maxExtent) {

  //Iterate the LSystem conditions
  iterateLSystem();

  //Construct the surface of the leaf
  generateSurface();

  //Convert the leaf surface polygons into a solid
  solidifyLeaf();

  getExtent(m_completeLeaf, minExtent, maxExtent);
}

void LeafConstruction::getExtentForTree(std::shared_ptr<LeafConstructionInterface> leafConstructor,
					Turtle* initialTurtle,
					G4ThreeVector& minExtent, 
					G4ThreeVector& maxExtent) {

  m_leafConstructor = leafConstructor;
  m_initialTurtle = initialTurtle;

  getExtentForTree(minExtent, maxExtent);
}

/*!
/ Translate a ROOT TVector into a Geant4 vector.
*/
G4ThreeVector LeafConstruction::convertVector(const TVector3& input) {
  G4ThreeVector output(input.X()*m, input.Y()*m, input.Z()*m);
  return output;
}

/*!
/ Remove degenerate vertices without destroying the polygons. Also return the
/ unique vertices.
*/
std::vector<std::shared_ptr<Vertex>> LeafConstruction::mergeVertices(std::vector<Polygon* > polygons) {
  
  double mergeDistance = 0.00000001;
  std::vector<std::shared_ptr<Vertex>> uniqueVertices;

  //This might be a little slow...
  for ( auto& face : polygons ){    
    for (unsigned int v=0; v<face->size(); v++) {
      
      //Check if vertex is within merging distance of previous unique vertex
      bool isUnique = true;
      
      for ( auto& uniqueVertex : uniqueVertices ) {
	double separation = (uniqueVertex->getPosition() - face->getVertex(v)->getPosition()).Mag();

	if ( separation < mergeDistance) {
	  //replace the vertex in the current face with previous unique vertex
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

void LeafConstruction::solidifyLeaf(){

  double thickness = m_leafConstructor->getDoubleParameter("thickness");

  //Duplicate the polygons to create a secondary surface
  std::vector<Polygon* > secondarySurface;

  for ( auto& polygon : m_leafSurface ) {
    secondarySurface.push_back( new Polygon(*polygon) );
  }

  //Merge together vertices before extrapolation
  //Need to have equivalent vertices for original surface (otherwise normals will vary!)
  std::vector<std::shared_ptr<Vertex>> uniqueVertices = mergeVertices(secondarySurface);
  std::vector<std::shared_ptr<Vertex>> surfaceUniqueVertices = mergeVertices(m_leafSurface);

  //Extrapolate surface along the vertex normals
  for (unsigned int v=0; v<uniqueVertices.size(); v++) {
    TVector3 secondaryPosition = uniqueVertices[v]->getPosition() + surfaceUniqueVertices[v]->getNormal() * (-1.0 * thickness );
    uniqueVertices[v]->setPosition(secondaryPosition);
  }

  //Invert the secondary surface faces
  for ( unsigned int p=0; p<secondarySurface.size(); p++) {
    secondarySurface[p]->invertNormal();
  }

  //Construct the edge connecting the two surfaces
  std::vector<std::pair<std::shared_ptr<Vertex>, std::shared_ptr<Vertex>>> allEdges;
  std::vector<std::pair<std::shared_ptr<Vertex>, std::shared_ptr<Vertex>>> uniqueEdges;

  for ( auto& polygon : m_leafSurface) {
    //Check that the polygon is valid (3 vertices and all at different positions).
    if (polygon->size() != 3) {
      continue; 
    }

    bool badTriangle = false;
    for (unsigned int v=1; v<polygon->size(); v++) {
      if ( (polygon->getVertex(0)->getPosition()-polygon->getVertex(v)->getPosition()).Mag() < 0.0000001 ) {
	badTriangle = true;
      }
    }

    if (badTriangle) {
      continue;
    }


    allEdges.push_back( std::pair<std::shared_ptr<Vertex>, std::shared_ptr<Vertex> >(polygon->getVertex(0), polygon->getVertex(1)) );
    allEdges.push_back( std::pair<std::shared_ptr<Vertex>, std::shared_ptr<Vertex> >(polygon->getVertex(1), polygon->getVertex(2)) );
    allEdges.push_back( std::pair<std::shared_ptr<Vertex>, std::shared_ptr<Vertex> >(polygon->getVertex(2), polygon->getVertex(0)) );
  }

  for ( unsigned int e=0; e<allEdges.size(); e++ ) {
    bool isUnique = true;

    for ( unsigned int c=0; c<allEdges.size(); c++ ) {
      
      if (c == e) {
	//Skip obvious fail
	continue;
      }

      if ( allEdges[e].first == allEdges[c].first && allEdges[e].second == allEdges[c].second ) {
	isUnique = false;
	break;
      }

      if ( allEdges[e].first == allEdges[c].second && allEdges[e].second == allEdges[c].first ) {
	isUnique = false;
	break;
      }

    }

    if (isUnique) {
      uniqueEdges.push_back( allEdges[e] );
    }
  }

  //For each unique edge create create a pair of faces to fill the gap
  std::vector<Polygon* > edgeSurface;
  for ( unsigned int e=0; e<uniqueEdges.size(); e++ ){

    //Get the edge normal (needed to check that edge polygons are pointing in the right direction)
    TVector3 edgeFaceNormal = uniqueEdges[e].first->getEdgeNormal( uniqueEdges[e].second );

    //Copy and extrapolate vertices
    TVector3 extrapolatedPosition1 = uniqueEdges[e].first->getPosition() + uniqueEdges[e].first->getNormal() * (-1.0 * thickness );
    TVector3 extrapolatedPosition2 = uniqueEdges[e].second->getPosition() + uniqueEdges[e].second->getNormal() * (-1.0 * thickness );

    Polygon* face1 = new Polygon();
    face1->addVertex(std::shared_ptr<Vertex>(new Vertex(uniqueEdges[e].first->getPosition())));
    face1->addVertex(std::shared_ptr<Vertex>(new Vertex(extrapolatedPosition1)));
    face1->addVertex(std::shared_ptr<Vertex>(new Vertex(uniqueEdges[e].second->getPosition())));

    if (edgeFaceNormal.Dot(face1->getNormal()) < 0.0) {
      face1->invertNormal();
    } 

    edgeSurface.push_back(face1);
    
    Polygon* face2 = new Polygon();
    face2->addVertex(std::shared_ptr<Vertex>(new Vertex(uniqueEdges[e].second->getPosition())));
    face2->addVertex(std::shared_ptr<Vertex>(new Vertex(extrapolatedPosition1)));
    face2->addVertex(std::shared_ptr<Vertex>(new Vertex(extrapolatedPosition2)));

    if (edgeFaceNormal.Dot(face2->getNormal()) < 0.0) {
      face2->invertNormal();
    } 

    edgeSurface.push_back(face2);
  }

  //Put all the polygons into one vector
  m_completeLeaf.insert(m_completeLeaf.end(), m_leafSurface.begin(),    m_leafSurface.end()   );
  m_completeLeaf.insert(m_completeLeaf.end(), secondarySurface.begin(), secondarySurface.end());
  m_completeLeaf.insert(m_completeLeaf.end(), edgeSurface.begin(),      edgeSurface.end()     );
  
  //Create the tesselated solid in Geant4 geometry
  m_leafSolid = new G4TessellatedSolid("LeafSolid");

  for ( auto& polygon : m_completeLeaf ) {

    //Check that the polygon is valid (3 vertices and all at different positions).
    if (polygon->size() != 3) {
      continue; 
    }

    bool badTriangle = false;
    for (unsigned int v=1; v<polygon->size(); v++) {
      if ( (polygon->getVertex(0)->getPosition()-polygon->getVertex(v)->getPosition()).Mag() < 0.0000001 ) {
	badTriangle = true;
      }
    }

    if (badTriangle) {
      continue;
    }

    G4TriangularFacet* facet = new G4TriangularFacet( convertVector(polygon->getVertex(0)->getPosition()),
						      convertVector(polygon->getVertex(1)->getPosition()),
						      convertVector(polygon->getVertex(2)->getPosition()),
						      ABSOLUTE);

    m_leafSolid->AddFacet((G4VFacet*)facet);
  }

  m_leafSolid->SetSolidClosed(true);
}


