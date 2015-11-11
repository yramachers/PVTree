#include "treeSystem/monopodial.hpp"
#include "treeSystem/treeConstructionInterface.hpp"

//To shorten the following statements
using LSysPtr = std::shared_ptr<TreeSystemInterface>;

//Trunk formation
MonopodialBranching::F::F(TreeConstructionInterface* constructor, double elongation) : TreeSystemInterface(constructor) {
  m_elongation = elongation;
}

std::vector<LSysPtr> MonopodialBranching::F::applyRule() {

  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new F(m_constructor, m_elongation)) );

  return results;
} 

void MonopodialBranching::F::processTurtles(std::vector<Turtle*>& turtleStack,
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

void MonopodialBranching::F::print(std::ostream& os) const {
  os << "F(" << m_elongation << ")";
}


//Width
MonopodialBranching::Exclame::Exclame(TreeConstructionInterface* constructor, double width) : TreeSystemInterface(constructor) {
  m_width = width;
}

std::vector<LSysPtr> MonopodialBranching::Exclame::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new Exclame(m_constructor, m_width)) );
  return results;
} 

void MonopodialBranching::Exclame::processTurtles(std::vector<Turtle*>& turtleStack,
						  std::vector<Turtle*>& /*retiredTurtles*/){
  turtleStack.back()->width = m_width;
}

void MonopodialBranching::Exclame::print(std::ostream& os) const {
  os << "!(" << m_width << ")";
}


//Store the current state on the stack
MonopodialBranching::LeftBracket::LeftBracket(TreeConstructionInterface* constructor) : TreeSystemInterface(constructor) { }

std::vector<LSysPtr> MonopodialBranching::LeftBracket::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr(new LeftBracket(m_constructor)));
  return results;
} 

void MonopodialBranching::LeftBracket::processTurtles(std::vector<Turtle*>& turtleStack,
						      std::vector<Turtle*>& /*retiredTurtles*/){
  turtleStack.push_back(new Turtle( *(turtleStack.back()) ));
}

void MonopodialBranching::LeftBracket::print(std::ostream& os) const {
  os << "[";
}


//Pull the last state onto the stack
MonopodialBranching::RightBracket::RightBracket(TreeConstructionInterface* constructor) : TreeSystemInterface(constructor) { }

std::vector<LSysPtr> MonopodialBranching::RightBracket::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr(new RightBracket(m_constructor)));
  return results;
} 

void MonopodialBranching::RightBracket::processTurtles(std::vector<Turtle*>& turtleStack,
						       std::vector<Turtle*>& /*retiredTurtles*/){
  delete turtleStack.back();
  turtleStack.pop_back(); 
}

void MonopodialBranching::RightBracket::print(std::ostream& os) const {
  os << "]";
}


//Rotate around vector H by an angle in degrees.
MonopodialBranching::Slash::Slash(TreeConstructionInterface* constructor, double angle) : TreeSystemInterface(constructor) {
  m_angle = angle;
}

std::vector<LSysPtr> MonopodialBranching::Slash::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new Slash(m_constructor, m_angle)) );
  return results;
} 

void MonopodialBranching::Slash::processTurtles(std::vector<Turtle*>& turtleStack,
						std::vector<Turtle*>& /*retiredTurtles*/){
  //Get rotation angle in radians
  double angleToRotate = m_angle * (M_PI/180.0);
  
  //Rotate lVector around orientation
  turtleStack.back()->lVector.Rotate(angleToRotate, turtleStack.back()->orientation);
}

void MonopodialBranching::Slash::print(std::ostream& os) const {
  os << "/(" << m_angle << ")";
}


//Rotate around vector L by an angle in degrees.
MonopodialBranching::Ampersand::Ampersand(TreeConstructionInterface* constructor, double angle) : TreeSystemInterface(constructor) {
  m_angle = angle;
}

std::vector<LSysPtr> MonopodialBranching::Ampersand::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new Ampersand(m_constructor, m_angle)) );
  return results;
}

void MonopodialBranching::Ampersand::processTurtles(std::vector<Turtle*>& turtleStack,
						    std::vector<Turtle*>& /*retiredTurtles*/){
  //Get rotation angle in radians
  double angleToRotate = m_angle * (M_PI/180.0);
  
  //Rotate orientation around L vector
  turtleStack.back()->orientation.Rotate(angleToRotate, turtleStack.back()->lVector);
} 

void MonopodialBranching::Ampersand::print(std::ostream& os) const {
  os << "&(" << m_angle << ")";
}

//Rotate around vertical vector in the clockwise direction
MonopodialBranching::Plus::Plus(TreeConstructionInterface* constructor, double angle) : TreeSystemInterface(constructor) {
  m_angle = angle;
}

std::vector<LSysPtr> MonopodialBranching::Plus::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new Plus(m_constructor, m_angle)) );
  return results;
}

void MonopodialBranching::Plus::processTurtles(std::vector<Turtle*>& turtleStack,
					       std::vector<Turtle*>& /*retiredTurtles*/){
  //Get rotation angle in radians
  double angleToRotate = m_angle * (M_PI/180.0);
  
  //Rotate orientation around vertical vector
  TVector3 verticalVector(0.0, 0.0, 1.0);
  turtleStack.back()->orientation.Rotate(angleToRotate, verticalVector);

  //Also rotate the lVector!
  turtleStack.back()->lVector.Rotate(angleToRotate, verticalVector);
} 

void MonopodialBranching::Plus::print(std::ostream& os) const {
  os << "+(" << m_angle << ")";
}

//Rotate around vertical vector in the anti-clockwise direction
MonopodialBranching::Minus::Minus(TreeConstructionInterface* constructor, double angle) : TreeSystemInterface(constructor) {
  m_angle = angle;
}

std::vector<LSysPtr> MonopodialBranching::Minus::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new Minus(m_constructor, m_angle)) );
  return results;
}

void MonopodialBranching::Minus::processTurtles(std::vector<Turtle*>& turtleStack,
						std::vector<Turtle*>& /*retiredTurtles*/){
  //Get rotation angle in radians
  double angleToRotate = - m_angle * (M_PI/180.0);
  
  //Rotate orientation around vertical vector
  TVector3 verticalVector(0.0, 0.0, 1.0);
  turtleStack.back()->orientation.Rotate(angleToRotate, verticalVector);

  //Also rotate the lVector!
  turtleStack.back()->lVector.Rotate(angleToRotate, verticalVector);
} 

void MonopodialBranching::Minus::print(std::ostream& os) const {
  os << "-(" << m_angle << ")";
}


//Rotate around turtle orientation such that the left vector is horizontal
MonopodialBranching::Dollar::Dollar(TreeConstructionInterface* constructor) : TreeSystemInterface(constructor) {}

std::vector<LSysPtr> MonopodialBranching::Dollar::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new Dollar(m_constructor)) );
  return results;
}

void MonopodialBranching::Dollar::processTurtles(std::vector<Turtle*>& /*turtleStack*/,
						 std::vector<Turtle*>& /*retiredTurtles*/){
  //At the moment not implemented
} 

void MonopodialBranching::Dollar::print(std::ostream& os) const {
  os << "$";
}


//Controls the growth (initial growth at least...)
MonopodialBranching::A::A(TreeConstructionInterface* constructor, double length, double width) : TreeSystemInterface(constructor) {
  m_length = length;
  m_width = width;
}

std::vector<LSysPtr> MonopodialBranching::A::applyRule() {
  std::vector<LSysPtr> results;

  results.push_back( LSysPtr( new Exclame(m_constructor, m_width) ) );
  results.push_back( LSysPtr( new F(m_constructor, m_length) ) );

  results.push_back( LSysPtr( new LeftBracket(m_constructor) ) );
  results.push_back( LSysPtr( new Ampersand(m_constructor, m_constructor->getDoubleParameter("branchingAngle1")) ) );
  results.push_back( LSysPtr( new B(m_constructor, m_length*m_constructor->getDoubleParameter("contractionRatio2"), m_width*m_constructor->getDoubleParameter("widthDecreaseRate")) ) );
  results.push_back( LSysPtr( new RightBracket(m_constructor) ) );

  results.push_back( LSysPtr( new Slash(m_constructor, m_constructor->getDoubleParameter("divergenceAngle") ) ) );
  results.push_back( LSysPtr( new A(m_constructor, m_length*m_constructor->getDoubleParameter("contractionRatio1"), m_width*m_constructor->getDoubleParameter("widthDecreaseRate")) ) );


  return results;
} 

void MonopodialBranching::A::processTurtles(std::vector<Turtle*>& /*turtleStack*/,
					    std::vector<Turtle*>& /*retiredTurtles*/){
  //Do nothing...
}

void MonopodialBranching::A::print(std::ostream& os) const {
  os << "A(" << m_length << "," << m_width << ")";
}


//Controls the growth (Doesn't draw anything!)
MonopodialBranching::B::B(TreeConstructionInterface* constructor, double length, double width) : TreeSystemInterface(constructor) {
  m_length = length;
  m_width = width;
}

std::vector<LSysPtr> MonopodialBranching::B::applyRule() {
  std::vector<LSysPtr> results;

  results.push_back( LSysPtr( new Exclame(m_constructor, m_width) ) );
  results.push_back( LSysPtr( new F(m_constructor, m_length) ) );

  results.push_back( LSysPtr( new LeftBracket(m_constructor) ) );
  results.push_back( LSysPtr( new Minus(m_constructor, m_constructor->getDoubleParameter("branchingAngle2")) ) );
  results.push_back( LSysPtr( new Dollar(m_constructor) ) );
  results.push_back( LSysPtr( new C(m_constructor, m_length*m_constructor->getDoubleParameter("contractionRatio2"), m_width*m_constructor->getDoubleParameter("widthDecreaseRate")) ) );
  results.push_back( LSysPtr( new RightBracket(m_constructor) ) );
  results.push_back( LSysPtr( new C(m_constructor, m_length*m_constructor->getDoubleParameter("contractionRatio1"), m_width*m_constructor->getDoubleParameter("widthDecreaseRate")) ) );


  return results;
} 

void MonopodialBranching::B::processTurtles(std::vector<Turtle*>& /*turtleStack*/,
					    std::vector<Turtle*>& /*retiredTurtles*/){
  //Do nothing...
}

void MonopodialBranching::B::print(std::ostream& os) const {
  os << "B(" << m_length << "," << m_width << ")";
}


//Controls the growth (Doesn't draw anything!)
MonopodialBranching::C::C(TreeConstructionInterface* constructor, double length, double width) : TreeSystemInterface(constructor)  {
  m_length = length;
  m_width = width;
}

std::vector<LSysPtr> MonopodialBranching::C::applyRule() {
  std::vector<LSysPtr> results;

  results.push_back( LSysPtr( new Exclame(m_constructor, m_width) ) );
  results.push_back( LSysPtr( new F(m_constructor, m_length) ) );

  results.push_back( LSysPtr( new LeftBracket(m_constructor) ) );
  results.push_back( LSysPtr( new Plus(m_constructor, m_constructor->getDoubleParameter("branchingAngle2")) ) );
  results.push_back( LSysPtr( new Dollar(m_constructor) ) );
  results.push_back( LSysPtr( new B(m_constructor, m_length*m_constructor->getDoubleParameter("contractionRatio2"), m_width*m_constructor->getDoubleParameter("widthDecreaseRate")) ) );
  results.push_back( LSysPtr( new RightBracket(m_constructor) ) );
  results.push_back( LSysPtr( new B(m_constructor, m_length*m_constructor->getDoubleParameter("contractionRatio1"), m_width*m_constructor->getDoubleParameter("widthDecreaseRate")) ) );


  return results;
} 

void MonopodialBranching::C::processTurtles(std::vector<Turtle*>& /*turtleStack*/,
					    std::vector<Turtle*>& /*retiredTurtles*/){
  //Do nothing...
}

void MonopodialBranching::C::print(std::ostream& os) const {
  os << "C(" << m_length << "," << m_width << ")";
}
