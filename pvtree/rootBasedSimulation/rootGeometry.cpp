#include "pvtree/rootBasedSimulation/rootGeometry.hpp"
#include "pvtree/geometry/turtle.hpp"
#include "TGeoManager.h"
#include "TGeoMaterial.h"
#include "TGeoMedium.h"
#include "TGeoVolume.h"
#include "TLorentzRotation.h"
#include <iostream>
#include "pvtree/solarSimulation/sun.hpp"
#include "TH1D.h"

ROOTGeometry::ROOTGeometry(TGeoManager* manager) : m_manager(manager),
						   top(nullptr),
						   volumeCount(0),
						   boundingBoxesVisible(false),
						   depthStepNumber(8) {

  //Build some mediums -- if not used these will be a memory leak....?
  TGeoMaterial* vacuumMaterial = new TGeoMaterial("Vacuum", 0, 0, 0);
  this->vacuumMedium           = new TGeoMedium("Vacuum", 1, vacuumMaterial);
  TGeoMaterial* branchMaterial = new TGeoMaterial("Branch", 0, 0, 0);
  this->branchMedium           = new TGeoMedium("Branch", 2, branchMaterial);
  TGeoMaterial* leafMaterial   = new TGeoMaterial("Leaf", 0, 0, 0);
  this->leafMedium             = new TGeoMedium("Leaf", 3, leafMaterial);

}

//This still doesn't provide a perfect bounding box when there is a 'width' to the turtles
void ROOTGeometry::getTurtleTreeExtent(const Turtle* turtle, TVector3& minExtent, TVector3& maxExtent, int maxDepth /* = -1 */) {

  TVector3 endPosition(turtle->position);
  endPosition = endPosition + (turtle->orientation*turtle->length);

  auto result = std::minmax({turtle->position.X(),endPosition.X(),minExtent.X(),maxExtent.X()});
  minExtent.SetX(result.first);
  maxExtent.SetX(result.second);

  result = std::minmax({turtle->position.Y(),endPosition.Y(),minExtent.Y(),maxExtent.Y()});
  minExtent.SetY(result.first);
  maxExtent.SetY(result.second);

  result = std::minmax({turtle->position.Z(),endPosition.Z(),minExtent.Z(),maxExtent.Z()});
  minExtent.SetZ(result.first);
  maxExtent.SetZ(result.second);

  //Call for child turtles (if necessary)
  for (const Turtle* child : turtle->children) {
    if (maxDepth != 0){
      getTurtleTreeExtent(child, minExtent, maxExtent, maxDepth-1);
    }
  }

}

//Normalize the turtles to fit in a box of a given height
void ROOTGeometry::normalizeTurtlesHeight(std::vector<Turtle*> turtles, double height){

  //Assume that the first turtle is the top node
  TVector3 maximums(turtles[0]->position);
  TVector3 minimums(turtles[0]->position);

  getTurtleTreeExtent(turtles[0], minimums, maximums);

  double scale = height/(maximums.Z()-minimums.Z());
  for (Turtle* turtle : turtles) {
    turtle->length *= scale;
    turtle->width *= scale;
    turtle->position = turtle->position * scale;
  }
}

void ROOTGeometry::buildLists(Turtle* turtle, std::vector<Turtle*>& toDraw, std::vector<Turtle*>& toSeed, int depthCount){

  if (depthCount <= 0){
    //Just add to seeds
    toSeed.push_back(turtle);
    return;
  }

  toDraw.push_back(turtle);

  depthCount--;
  for (Turtle* child : turtle->children){
    buildLists(child, toDraw, toSeed, depthCount);
  }

}

void ROOTGeometry::constructLeaf(const Turtle* endTurtle, TGeoVolume* parentVolume, TVector3 parentPosition){

  double coneRadius = endTurtle->width*5.0;
  double coneArea   = M_PI*pow(coneRadius,2.0);
  double coneHeight = endTurtle->length/15.0;

  //Quick initial representation as a cone (like the branches)
  TGeoVolume* leaf = this->m_manager->MakeCone("Leaf", this->leafMedium,
					     coneHeight,
					     0.0,
					     coneRadius,
					     0.0,
					     coneRadius);

  leaf->SetFillColor(kGreen-2);
  leaf->SetLineColor(kGreen-2);

  TGeoRotation rotationMatrix("rotate",
			      TMath::RadToDeg()*endTurtle->orientation.Phi()+90.0,
			      TMath::RadToDeg()*endTurtle->orientation.Theta(),
			      0);

  TVector3 centralPosition(endTurtle->position);
  centralPosition = centralPosition + (endTurtle->orientation*(endTurtle->length+coneHeight/2.0)) - parentPosition;
  TGeoTranslation translationMatrix(centralPosition.X(),
				    centralPosition.Y(),
				    centralPosition.Z());

  TGeoCombiTrans* combinedMatrix = new TGeoCombiTrans(translationMatrix, rotationMatrix); //this is deleted automatically

  this->volumeCount++;
  parentVolume->AddNodeOverlap(leaf, this->volumeCount, combinedMatrix );


  //Construct another 'leaf' to be used later for simulation.
  TVector3 normalVector(endTurtle->orientation);
  normalVector *= 1.0 / normalVector.Mag();

  TVector3 surfaceOffset(normalVector);
  surfaceOffset *= endTurtle->length+coneHeight/2.0;

  TVector3 surfaceSamplePosition(centralPosition + parentPosition + surfaceOffset); //can use the normal vector to move to the surface

  leaves.push_back(Leaf(surfaceSamplePosition, normalVector, coneArea, this->volumeCount));
}

void ROOTGeometry::recursiveTreeBuild(Turtle* startTurtle, int depthStep, TGeoVolume* parentVolume, TVector3 parentPosition){

  //First construct the bounding box
  TVector3 maximums(startTurtle->position);
  TVector3 minimums(startTurtle->position);

  getTurtleTreeExtent(startTurtle, minimums, maximums);

  TGeoVolume* boundingBox = this->m_manager->MakeBox("BoundingBox",this->vacuumMedium,
						   (maximums.X()-minimums.X())/2.0,
						   (maximums.Y()-minimums.Y())/2.0,
						   (maximums.Z()-minimums.Z())/2.0 );

  boundingBox->SetInvisible(); //never draw these bounding boxes

  TVector3 boundingBoxPosition( (maximums.X()-minimums.X())/2.0 + minimums.X(),
				(maximums.Y()-minimums.Y())/2.0 + minimums.Y(),
				(maximums.Z()-minimums.Z())/2.0 + minimums.Z());
  TVector3 boundingBoxPositionToParent = boundingBoxPosition - parentPosition;

  this->volumeCount++;
  parentVolume->AddNodeOverlap(boundingBox, this->volumeCount, new TGeoTranslation(boundingBoxPositionToParent.X(),
										   boundingBoxPositionToParent.Y(),
										   boundingBoxPositionToParent.Z() ) );

  //In case the user wants to visualize the bounding boxes.
  if (this->boundingBoxesVisible){
    this->volumeCount++;
    TGeoVolume* drawboundingBox = this->m_manager->MakeBox("BoundingBox",this->vacuumMedium,
							 (maximums.X()-minimums.X())/2.0,
							 (maximums.Y()-minimums.Y())/2.0,
							 (maximums.Z()-minimums.Z())/2.0 );
    drawboundingBox->SetLineColor(kRed-2);

    parentVolume->AddNodeOverlap(drawboundingBox, this->volumeCount, new TGeoTranslation(boundingBoxPositionToParent.X(),
											 boundingBoxPositionToParent.Y(),
											 boundingBoxPositionToParent.Z() ) );
  }


  //Make a flat list of turtles to draw, and a list of new seed turtles (if any)
  std::vector<Turtle* > turtlesToDraw;
  std::vector<Turtle* > seedTurtles;

  // build the lists
  buildLists(startTurtle, turtlesToDraw, seedTurtles, depthStep);

  //Create geometry for current turtles
  for (const Turtle* turtle : turtlesToDraw) {
    TGeoVolume* turtleBox;

    if (turtle->children.size() == 0){
      //No children, assume at the end of the branch
      turtleBox = this->m_manager->MakeCone("Turtle", this->branchMedium,
					  turtle->length/2.0,
					  0.0,
					  turtle->width/2.0,
					  0.0,
					  turtle->width/2.0);
    }else{
      //If we have children end the cone on the child width! -- assume all children the same
      turtleBox = this->m_manager->MakeCone("Turtle", this->branchMedium,
					  turtle->length/2.0,
					  0.0,
					  turtle->width/2.0,
					  0.0,
					  turtle->children[0]->width/2.0);
    }
    turtleBox->SetFillColor(kOrange-2);
    turtleBox->SetLineColor(kOrange-2);

    TGeoRotation rotationMatrix("rotate",
				TMath::RadToDeg()*turtle->orientation.Phi()+90.0,
				TMath::RadToDeg()*turtle->orientation.Theta(),
				0);

    TVector3 centralPosition(turtle->position);
    centralPosition = centralPosition + (turtle->orientation*(turtle->length/2.0)) - boundingBoxPosition;
    TGeoTranslation translationMatrix(centralPosition.X(),
				      centralPosition.Y(),
				      centralPosition.Z());

    TGeoCombiTrans* combinedMatrix = new TGeoCombiTrans(translationMatrix, rotationMatrix); //this is deleted automatically

    this->volumeCount++;
    boundingBox->AddNodeOverlap(turtleBox, this->volumeCount, combinedMatrix );

    //Add a leaf as well if at end of branch
    if (turtle->children.size() ==0){
      constructLeaf(turtle, boundingBox, boundingBoxPosition);
    }

  }


  //Then call this function for new seeds
  for (Turtle* turtle : seedTurtles){
    recursiveTreeBuild(turtle, depthStep, boundingBox, boundingBoxPosition);
  }

}

void ROOTGeometry::constructTreeFromTurtles(std::vector<Turtle*> turtles, double maxZ){

  //Normalize the height of the turtles.
  normalizeTurtlesHeight(turtles, maxZ);

  //After normalizing get the absolute extents
  TVector3 totalMaximums(turtles[0]->position);
  TVector3 totalMinimums(turtles[0]->position);

  getTurtleTreeExtent(turtles[0], totalMinimums, totalMaximums);

  this->top = this->m_manager->MakeBox("Top", this->vacuumMedium,
				     totalMaximums.X()-totalMinimums.X(),
				     totalMaximums.Y()-totalMinimums.Y(),
				     totalMaximums.Z()-totalMinimums.Z() );

  //Start recursive geometry building
  TVector3 startPosition(0.0, 0.0, 0.0);
  recursiveTreeBuild(turtles[0], this->depthStepNumber, this->top, startPosition);

}


void ROOTGeometry::close() {
  this->m_manager->SetTopVolume(this->top);
  this->m_manager->CloseGeometry();
}

void ROOTGeometry::draw(std::string options /* ="" */) {
  if (this->top != 0) {
    this->top->Draw(options.c_str());
  } else {
    std::cout << "No geometry defined yet!" << std::endl;
  }
}

std::vector<Leaf>& ROOTGeometry::getLeaves() {
  return this->leaves;
}

TGeoManager* ROOTGeometry::getManager() {
  return this->m_manager;
}

/// Temporarily put this here whilst I think of a better layout...
void ROOTGeometry::evaluateEnergyCollection(Sun& sun) {

  //Invert to get vector from leaf surface
  TVector3 solarVector = sun.getLightVector()*(-1.0);

  for ( Leaf& leaf : this->leaves ) {

    //Initially do a simple test of if the leaf is facing the sun.
    double dotProduct = leaf.getNormal() * solarVector;

    if (dotProduct < 0.0){
      //facing wrong way, don't do intersection test (would hit ourselves anyway...)
      leaf.setEnergy(leaf.getEnergy()+0.0); //If I want the getLastEnergy() to function!
      continue;
    }

    //Calculate energy intercepted
    double energyIntercepted = leaf.getArea() * sun.getIrradiance() * dotProduct/ fabs(leaf.getNormal().Mag() * solarVector.Mag());

    //Start an intersection test from the leaf in the direction of the sun.
    TVector3 startingPoint = leaf.getPosition();
    this->m_manager->SetCurrentPoint(startingPoint.X(), startingPoint.Y(), startingPoint.Z());
    this->m_manager->SetCurrentDirection(solarVector.X(), solarVector.Y(), solarVector.Z());

    //Keep stepping whilst not hitting anything real and step size is small
    bool hitSomethingReal = false;
    int stepCounter = 0;

    while (this->m_manager->GetStep() < 100000.0 || stepCounter == 0) {
      TGeoNode* nodeHit = this->m_manager->FindNextBoundaryAndStep();

      stepCounter++;
      //std::cout << "Step " << stepCounter  <<" size = " << this->m_manager->GetStep() << std::endl;

      if (nodeHit == 0){
	//Hit Nothing!
	continue; //?
      }

      //Use node visibility to distinguish the bounding boxes from actual tree structure.
      if ( nodeHit->IsVisible() ) {
	//doh, hit sometihg
	hitSomethingReal = true;
	break;
      }
    }

    //Create a particle and check if it reaches the leaf
    if ( ! hitSomethingReal ){
      leaf.setEnergy(leaf.getEnergy()+energyIntercepted);
    } else {
      leaf.setEnergy(leaf.getEnergy()+0.0); //If I want the getLastEnergy() to function!
    }
  }

}
