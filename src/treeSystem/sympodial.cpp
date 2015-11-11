#include "treeSystem/sympodial.hpp"
#include "treeSystem/treeConstructionInterface.hpp"

//To shorten the following statements
using LSysPtr = std::shared_ptr<TreeSystemInterface>;

//Trunk formation
SympodialBranching::F::F(TreeConstructionInterface* constructor, double elongation) : TreeSystemInterface(constructor) {
  m_elongation = elongation;
}

std::vector<LSysPtr> SympodialBranching::F::applyRule() {

  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new F(m_constructor, m_elongation)) );

  return results;
} 

void SympodialBranching::F::processTurtles(std::vector<Turtle*>& turtleStack,
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

void SympodialBranching::F::print(std::ostream& os) const {
  os << "F(" << m_elongation << ")";
}


//Width
SympodialBranching::Exclame::Exclame(TreeConstructionInterface* constructor, double width) : TreeSystemInterface(constructor) {
  m_width = width;
}

std::vector<LSysPtr> SympodialBranching::Exclame::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new Exclame(m_constructor, m_width)) );
  return results;
} 

void SympodialBranching::Exclame::processTurtles(std::vector<Turtle*>& turtleStack,
						 std::vector<Turtle*>& /*retiredTurtles*/){
  turtleStack.back()->width = m_width;
}

void SympodialBranching::Exclame::print(std::ostream& os) const {
  os << "!(" << m_width << ")";
}


//Store the current state on the stack
SympodialBranching::LeftBracket::LeftBracket(TreeConstructionInterface* constructor) : TreeSystemInterface(constructor) { }

std::vector<LSysPtr> SympodialBranching::LeftBracket::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr(new LeftBracket(m_constructor)));
  return results;
} 

void SympodialBranching::LeftBracket::processTurtles(std::vector<Turtle*>& turtleStack,
						     std::vector<Turtle*>& /*retiredTurtles*/){
  turtleStack.push_back(new Turtle( *(turtleStack.back()) ));
}

void SympodialBranching::LeftBracket::print(std::ostream& os) const {
  os << "[";
}


//Pull the last state onto the stack
SympodialBranching::RightBracket::RightBracket(TreeConstructionInterface* constructor) : TreeSystemInterface(constructor) { }

std::vector<LSysPtr> SympodialBranching::RightBracket::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr(new RightBracket(m_constructor)));
  return results;
} 

void SympodialBranching::RightBracket::processTurtles(std::vector<Turtle*>& turtleStack,
						      std::vector<Turtle*>& /*retiredTurtles*/){
  delete turtleStack.back();
  turtleStack.pop_back(); 
}

void SympodialBranching::RightBracket::print(std::ostream& os) const {
  os << "]";
}


//Rotate around vector H by an angle in degrees.
SympodialBranching::Slash::Slash(TreeConstructionInterface* constructor, double angle) : TreeSystemInterface(constructor) {
  m_angle = angle;
}

std::vector<LSysPtr> SympodialBranching::Slash::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new Slash(m_constructor, m_angle)) );
  return results;
} 

void SympodialBranching::Slash::processTurtles(std::vector<Turtle*>& turtleStack,
					       std::vector<Turtle*>& /*retiredTurtles*/){
  //Get rotation angle in radians
  double angleToRotate = m_angle * (M_PI/180.0);
  
  //Rotate lVector around orientation
  turtleStack.back()->lVector.Rotate(angleToRotate, turtleStack.back()->orientation);
}

void SympodialBranching::Slash::print(std::ostream& os) const {
  os << "/(" << m_angle << ")";
}


//Rotate around vector L by an angle in degrees.
SympodialBranching::Ampersand::Ampersand(TreeConstructionInterface* constructor, double angle) : TreeSystemInterface(constructor) {
  m_angle = angle;
}

std::vector<LSysPtr> SympodialBranching::Ampersand::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new Ampersand(m_constructor, m_angle)) );
  return results;
}

void SympodialBranching::Ampersand::processTurtles(std::vector<Turtle*>& turtleStack,
						   std::vector<Turtle*>& /*retiredTurtles*/){
  //Get rotation angle in radians
  double angleToRotate = m_angle * (M_PI/180.0);
  
  //Rotate orientation around L vector
  turtleStack.back()->orientation.Rotate(angleToRotate, turtleStack.back()->lVector);
} 

void SympodialBranching::Ampersand::print(std::ostream& os) const {
  os << "&(" << m_angle << ")";
}

//Rotate around vertical vector in the clockwise direction
SympodialBranching::Plus::Plus(TreeConstructionInterface* constructor, double angle) : TreeSystemInterface(constructor) {
  m_angle = angle;
}

std::vector<LSysPtr> SympodialBranching::Plus::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new Plus(m_constructor, m_angle)) );
  return results;
}

void SympodialBranching::Plus::processTurtles(std::vector<Turtle*>& turtleStack,
					      std::vector<Turtle*>& /*retiredTurtles*/){
  //Get rotation angle in radians
  double angleToRotate = m_angle * (M_PI/180.0);
  
  //Rotate orientation around vertical vector
  TVector3 verticalVector(0.0, 0.0, 1.0);
  turtleStack.back()->orientation.Rotate(angleToRotate, verticalVector);

  //Also rotate the lVector!
  turtleStack.back()->lVector.Rotate(angleToRotate, verticalVector);
} 

void SympodialBranching::Plus::print(std::ostream& os) const {
  os << "+(" << m_angle << ")";
}

//Rotate around vertical vector in the anti-clockwise direction
SympodialBranching::Minus::Minus(TreeConstructionInterface* constructor, double angle) : TreeSystemInterface(constructor) {
  m_angle = angle;
}

std::vector<LSysPtr> SympodialBranching::Minus::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new Minus(m_constructor, m_angle)) );
  return results;
}

void SympodialBranching::Minus::processTurtles(std::vector<Turtle*>& turtleStack,
					       std::vector<Turtle*>& /*retiredTurtles*/){
  //Get rotation angle in radians
  double angleToRotate = - m_angle * (M_PI/180.0);
  
  //Rotate orientation around vertical vector
  TVector3 verticalVector(0.0, 0.0, 1.0);
  turtleStack.back()->orientation.Rotate(angleToRotate, verticalVector);

  //Also rotate the lVector!
  turtleStack.back()->lVector.Rotate(angleToRotate, verticalVector);
} 

void SympodialBranching::Minus::print(std::ostream& os) const {
  os << "-(" << m_angle << ")";
}


//Rotate around turtle orientation such that the left vector is horizontal
SympodialBranching::Dollar::Dollar(TreeConstructionInterface* constructor) : TreeSystemInterface(constructor) {}

std::vector<LSysPtr> SympodialBranching::Dollar::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new Dollar(m_constructor)) );
  return results;
}

void SympodialBranching::Dollar::processTurtles(std::vector<Turtle*>& /*turtleStack*/,
						std::vector<Turtle*>& /*retiredTurtles*/){
  //At the moment not implemented
} 

void SympodialBranching::Dollar::print(std::ostream& os) const {
  os << "$";
}


//Controls the growth (initial growth at least...)
SympodialBranching::A::A(TreeConstructionInterface* constructor, double length, double width) : TreeSystemInterface(constructor) {
  m_length = length;
  m_width = width;
}

std::vector<LSysPtr> SympodialBranching::A::applyRule() {
  std::vector<LSysPtr> results;

  results.push_back( LSysPtr( new Exclame(m_constructor, m_width) ) );
  results.push_back( LSysPtr( new F(m_constructor, m_length) ) );

  results.push_back( LSysPtr( new LeftBracket(m_constructor) ) );
  results.push_back( LSysPtr( new Ampersand(m_constructor, m_constructor->getDoubleParameter("branchingAngle1") ) ) );
  results.push_back( LSysPtr( new B(m_constructor, m_length*m_constructor->getDoubleParameter("contractionRatio1"), m_width*m_constructor->getDoubleParameter("widthDecreaseRate")) ) );
  results.push_back( LSysPtr( new RightBracket(m_constructor) ) );

  results.push_back( LSysPtr( new Slash(m_constructor, 180.0) ) );
  results.push_back( LSysPtr( new LeftBracket(m_constructor) ) );
  results.push_back( LSysPtr( new Ampersand(m_constructor, m_constructor->getDoubleParameter("branchingAngle2")) ) );
  results.push_back( LSysPtr( new B(m_constructor, m_length*m_constructor->getDoubleParameter("contractionRatio2"), m_width*m_constructor->getDoubleParameter("widthDecreaseRate")) ) );
  results.push_back( LSysPtr( new RightBracket(m_constructor) ) );

  return results;
} 

void SympodialBranching::A::processTurtles(std::vector<Turtle*>& /*turtleStack*/,
					   std::vector<Turtle*>& /*retiredTurtles*/){
  //Do nothing...
}

void SympodialBranching::A::print(std::ostream& os) const {
  os << "A(" << m_length << "," << m_width << ")";
}


//Controls the growth (Doesn't draw anything!)
SympodialBranching::B::B(TreeConstructionInterface* constructor, double length, double width) : TreeSystemInterface(constructor) {
  m_length = length;
  m_width = width;
}

std::vector<LSysPtr> SympodialBranching::B::applyRule() {
  std::vector<LSysPtr> results;

  results.push_back( LSysPtr( new Exclame(m_constructor, m_width) ) );
  results.push_back( LSysPtr( new F(m_constructor, m_length) ) );

  results.push_back( LSysPtr( new LeftBracket(m_constructor) ) );
  results.push_back( LSysPtr( new Plus(m_constructor, m_constructor->getDoubleParameter("branchingAngle1") ) ) );
  results.push_back( LSysPtr( new Dollar(m_constructor) ) );
  results.push_back( LSysPtr( new B(m_constructor, m_length*m_constructor->getDoubleParameter("contractionRatio1"), m_width*m_constructor->getDoubleParameter("widthDecreaseRate") ) ) );
  results.push_back( LSysPtr( new RightBracket(m_constructor) ) );

  results.push_back( LSysPtr( new LeftBracket(m_constructor) ) );
  results.push_back( LSysPtr( new Minus(m_constructor, m_constructor->getDoubleParameter("branchingAngle2")) ) );
  results.push_back( LSysPtr( new Dollar(m_constructor) ) );
  results.push_back( LSysPtr( new B(m_constructor, m_length*m_constructor->getDoubleParameter("contractionRatio2"), m_width*m_constructor->getDoubleParameter("widthDecreaseRate") ) ) );
  results.push_back( LSysPtr( new RightBracket(m_constructor) ) );

  return results;
} 

void SympodialBranching::B::processTurtles(std::vector<Turtle*>& /*turtleStack*/,
					   std::vector<Turtle*>& /*retiredTurtles*/){
  //Do nothing...
}

void SympodialBranching::B::print(std::ostream& os) const {
  os << "B(" << m_length << "," << m_width << ")";
}
