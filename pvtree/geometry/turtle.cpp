#include "pvtree/geometry/turtle.hpp"
#include "TVector3.h"
#include <algorithm>
#include <vector>

Turtle::Turtle(){
  this->position = TVector3(0.0, 0.0, 0.0);
  this->orientation = TVector3(0.0, 0.0, 1.0);
  this->lVector = this->orientation.Orthogonal();
  this->width = 0.0;
  this->length = 0.0;
  this->parent = 0;
  this->complete = false;
}

Turtle::Turtle(Turtle& turtle){
  this->position = TVector3(turtle.position);
  this->orientation = TVector3(turtle.orientation);
  this->lVector = TVector3(turtle.lVector);
  this->width = turtle.width;
  this->length = turtle.length;
  this->complete = false;

  //Parent choice should in fact be a little more involved, it should be one which is 'complete'
  Turtle* currentParent = &turtle;
  this->parent = 0;
  while (this->parent == 0){
    if (currentParent == 0){
      break;
    }
    if (currentParent->complete) {
      this->parent = currentParent;
      this->parent->children.push_back(this);
    }
    currentParent = currentParent->parent;
  }
}

Turtle::Turtle(TVector3 initialPosition, TVector3 initialOrientation, TVector3 initialLVector){
  this->position = initialPosition;
  this->orientation = initialOrientation;
  this->lVector = initialLVector;
  this->width = 0.0;
  this->length = 0.0;
  this->parent = 0;
  this->complete = false;
}

Turtle::~Turtle(){
  //Before we delete remove this turtle from its children and parent!
  //whilst trying to maintain inheritence.
  std::vector<Turtle* >::iterator  it;
  if (this->parent != 0){
    it = std::find(this->parent->children.begin(), this->parent->children.end(), this);

    if (it != this->parent->children.end()){
      this->parent->children.erase(it);
    }

    //Add all the children of this turtle onto the parent to maintain the inheritence structure
    for (auto& child : this->children){
      this->parent->children.push_back(child);
      child->parent = this->parent;
    }

  } else {
    for (auto& child : this->children) {
      child->parent=0;
    }
  }
}

void Turtle::move(){
  //Change the starting position based upon length and heading
  TVector3 heading(this->orientation);
  heading.SetMag(this->length);
  this->position = this->position + heading;
}


void Turtle::moveAlongVector(const TVector3& displacement) {
  this->position = this->position + displacement;
}

