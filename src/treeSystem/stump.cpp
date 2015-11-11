#include "treeSystem/stump.hpp"
#include "treeSystem/treeConstructionInterface.hpp"

//To shorten the following statements
using LSysPtr = std::shared_ptr<TreeSystemInterface>;

//Trunk formation
StumpBranching::F::F(TreeConstructionInterface* constructor, double elongation) : TreeSystemInterface(constructor) {
  m_elongation = elongation;
}

std::vector<LSysPtr> StumpBranching::F::applyRule() {

  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new F(m_constructor, m_elongation )) );

  return results;
} 

void StumpBranching::F::processTurtles(std::vector<Turtle*>& turtleStack,
				       std::vector<Turtle*>& retiredTurtles){
  Turtle* activeTurtle = turtleStack.back();
  
  //Set the turtle speed
  activeTurtle->length += m_elongation;
  activeTurtle->complete = true;
  
  //Retire the turtle
  retiredTurtles.push_back(activeTurtle);
  turtleStack.pop_back();
  
  //Create a new active turtle to replace it
  turtleStack.push_back(new Turtle( *activeTurtle ));
  
  //Move the active turtle to the new starting position
  turtleStack.back()->move();
  turtleStack.back()->length=0.0;
}

void StumpBranching::F::print(std::ostream& os) const {
  os << "F(" << m_elongation << ")";
}


//Width
StumpBranching::Exclame::Exclame(TreeConstructionInterface* constructor, double width) : TreeSystemInterface(constructor) {
  m_width = width;
}

std::vector<LSysPtr> StumpBranching::Exclame::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new Exclame(m_constructor, m_width)) );
  return results;
} 

void StumpBranching::Exclame::processTurtles(std::vector<Turtle*>& turtleStack,
					     std::vector<Turtle*>& /*retiredTurtles*/){
  turtleStack.back()->width = m_width;
}

void StumpBranching::Exclame::print(std::ostream& os) const {
  os << "!(" << m_width << ")";
}


//Rotate around vector H by an angle in degrees.
StumpBranching::Slash::Slash(TreeConstructionInterface* constructor, double angle) : TreeSystemInterface(constructor) {
  m_angle = angle;
}

std::vector<LSysPtr> StumpBranching::Slash::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new Slash(m_constructor, m_angle)) );
  return results;
} 

void StumpBranching::Slash::processTurtles(std::vector<Turtle*>& turtleStack,
					     std::vector<Turtle*>& /*retiredTurtles*/){
  //Get rotation angle in radians
  double angleToRotate = m_angle * (M_PI/180.0);
  
  //Rotate lVector around orientation
  turtleStack.back()->lVector.Rotate(angleToRotate, turtleStack.back()->orientation);
}

void StumpBranching::Slash::print(std::ostream& os) const {
  os << "/(" << m_angle << ")";
}


//Rotate around vector L by an angle in degrees.
StumpBranching::Ampersand::Ampersand(TreeConstructionInterface* constructor, double angle) : TreeSystemInterface(constructor) {
  m_angle = angle;
}

std::vector<LSysPtr> StumpBranching::Ampersand::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new Ampersand(m_constructor, m_angle)) );
  return results;
}

void StumpBranching::Ampersand::processTurtles(std::vector<Turtle*>& turtleStack,
					       std::vector<Turtle*>& /*retiredTurtles*/){
  //Get rotation angle in radians
  double angleToRotate = m_angle * (M_PI/180.0);
  
  //Rotate orientation around L vector
  turtleStack.back()->orientation.Rotate(angleToRotate, turtleStack.back()->lVector);
} 

void StumpBranching::Ampersand::print(std::ostream& os) const {
  os << "&(" << m_angle << ")";
}


