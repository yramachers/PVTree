#include "pvtree/treeSystem/ternary.hpp"
#include "pvtree/treeSystem/treeConstructionInterface.hpp"

//To shorten the following statements
using LSysPtr = std::shared_ptr<TreeSystemInterface>;

//Trunk formation
TernaryBranching::F::F(TreeConstructionInterface* constructor, double elongation) : TreeSystemInterface(constructor) {
  m_elongation = elongation;
}

std::vector<LSysPtr> TernaryBranching::F::applyRule() {

  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new F(m_constructor, m_constructor->getDoubleParameter("elongationRate")*m_elongation )) );

  return results;
} 

void TernaryBranching::F::processTurtles(std::vector<Turtle*>& turtleStack,
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

void TernaryBranching::F::print(std::ostream& os) const {
  os << "F(" << m_elongation << ")";
}


//Width
TernaryBranching::Exclame::Exclame(TreeConstructionInterface* constructor, double width) : TreeSystemInterface(constructor) {
  m_width = width;
}

std::vector<LSysPtr> TernaryBranching::Exclame::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new Exclame(m_constructor, m_constructor->getDoubleParameter("widthIncreaseRate")*m_width)) );
  return results;
} 

void TernaryBranching::Exclame::processTurtles(std::vector<Turtle*>& turtleStack,
					       std::vector<Turtle*>& /*retiredTurtles*/){
  turtleStack.back()->width = m_width;
}

void TernaryBranching::Exclame::print(std::ostream& os) const {
  os << "!(" << m_width << ")";
}


//Store the current state on the stack
TernaryBranching::LeftBracket::LeftBracket(TreeConstructionInterface* constructor) : TreeSystemInterface(constructor) { }

std::vector<LSysPtr> TernaryBranching::LeftBracket::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr(new LeftBracket(m_constructor)));
  return results;
} 

void TernaryBranching::LeftBracket::processTurtles(std::vector<Turtle*>& turtleStack,
						   std::vector<Turtle*>& /*retiredTurtles*/){
  turtleStack.push_back(new Turtle( *(turtleStack.back()) ));
}

void TernaryBranching::LeftBracket::print(std::ostream& os) const {
  os << "[";
}


//Pull the last state onto the stack
TernaryBranching::RightBracket::RightBracket(TreeConstructionInterface* constructor) : TreeSystemInterface(constructor) { }

std::vector<LSysPtr> TernaryBranching::RightBracket::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr(new RightBracket(m_constructor)));
  return results;
} 

void TernaryBranching::RightBracket::processTurtles(std::vector<Turtle*>& turtleStack,
						    std::vector<Turtle*>& /*retiredTurtles*/){
  delete turtleStack.back();
  turtleStack.pop_back(); 
}

void TernaryBranching::RightBracket::print(std::ostream& os) const {
  os << "]";
}


//Rotate around vector H by an angle in degrees.
TernaryBranching::Slash::Slash(TreeConstructionInterface* constructor, double angle) : TreeSystemInterface(constructor) {
  m_angle = angle;
}

std::vector<LSysPtr> TernaryBranching::Slash::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new Slash(m_constructor, m_angle)) );
  return results;
} 

void TernaryBranching::Slash::processTurtles(std::vector<Turtle*>& turtleStack,
					     std::vector<Turtle*>& /*retiredTurtles*/){
  //Get rotation angle in radians
  double angleToRotate = m_angle * (M_PI/180.0);
  
  //Rotate lVector around orientation
  turtleStack.back()->lVector.Rotate(angleToRotate, turtleStack.back()->orientation);
}

void TernaryBranching::Slash::print(std::ostream& os) const {
  os << "/(" << m_angle << ")";
}


//Rotate around vector L by an angle in degrees.
TernaryBranching::Ampersand::Ampersand(TreeConstructionInterface* constructor, double angle) : TreeSystemInterface(constructor) {
  m_angle = angle;
}

std::vector<LSysPtr> TernaryBranching::Ampersand::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new Ampersand(m_constructor, m_angle)) );
  return results;
}

void TernaryBranching::Ampersand::processTurtles(std::vector<Turtle*>& turtleStack,
						 std::vector<Turtle*>& /*retiredTurtles*/){
  //Get rotation angle in radians
  double angleToRotate = m_angle * (M_PI/180.0);
  
  //Rotate orientation around L vector
  turtleStack.back()->orientation.Rotate(angleToRotate, turtleStack.back()->lVector);
} 

void TernaryBranching::Ampersand::print(std::ostream& os) const {
  os << "&(" << m_angle << ")";
}


//Controls the growth (Doesn't draw anything!)
TernaryBranching::A::A(TreeConstructionInterface* constructor) : TreeSystemInterface(constructor) {}

std::vector<LSysPtr> TernaryBranching::A::applyRule() {
  std::vector<LSysPtr> results;
  
  results.push_back( LSysPtr(new Exclame(m_constructor, m_constructor->getDoubleParameter("initialWidth") )) );
  results.push_back( LSysPtr(new F(m_constructor, m_constructor->getDoubleParameter("lengthScale") )) );
  
  results.push_back( LSysPtr(new LeftBracket(m_constructor)) );
  results.push_back( LSysPtr(new Ampersand(m_constructor, m_constructor->getDoubleParameter("branchingAngle") )) );
  results.push_back( LSysPtr(new F(m_constructor, m_constructor->getDoubleParameter("lengthScale") )) );
  results.push_back( LSysPtr(new A(m_constructor)) );
  results.push_back( LSysPtr(new RightBracket(m_constructor)) );
  results.push_back( LSysPtr(new Slash(m_constructor, m_constructor->getDoubleParameter("divergenceAngle1") )) );
  
  results.push_back( LSysPtr(new LeftBracket(m_constructor)) );
  results.push_back( LSysPtr(new Ampersand(m_constructor, m_constructor->getDoubleParameter("branchingAngle") )) );
  results.push_back( LSysPtr(new F(m_constructor, m_constructor->getDoubleParameter("lengthScale") )) );
  results.push_back( LSysPtr(new A(m_constructor)) );
  results.push_back( LSysPtr(new RightBracket(m_constructor)) );
  results.push_back( LSysPtr(new Slash(m_constructor, m_constructor->getDoubleParameter("divergenceAngle2") )) );

  results.push_back( LSysPtr(new LeftBracket(m_constructor)) );
  results.push_back( LSysPtr(new Ampersand(m_constructor, m_constructor->getDoubleParameter("branchingAngle") )) );
  results.push_back( LSysPtr(new F(m_constructor, m_constructor->getDoubleParameter("lengthScale") )) );
  results.push_back( LSysPtr(new A(m_constructor)) );
  results.push_back( LSysPtr(new RightBracket(m_constructor)) );
  
  return results;
} 

void TernaryBranching::A::processTurtles(std::vector<Turtle*>& /*turtleStack*/,
					 std::vector<Turtle*>& /*retiredTurtles*/){
  //Do nothing...
}

void TernaryBranching::A::print(std::ostream& os) const {
  os << "A";
}
