#include "pvtree/leafSystem/simple.hpp"
#include "pvtree/geometry/turtle.hpp"
#include "pvtree/geometry/polygon.hpp"
#include "pvtree/leafSystem/leafConstructionInterface.hpp"

//To shorten the following statements
using LSysPtr = std::shared_ptr<LeafSystemInterface>;

//Move but don't create a structure by itself (reserved for vertex creation rule)
Simple::G::G(LeafConstructionInterface* constructor, double elongation, double growthRate) : LeafSystemInterface(constructor) {
  m_elongation = elongation;
  m_growthRate = growthRate;
}

std::vector<LSysPtr> Simple::G::applyRule() {

  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new G(m_constructor, m_elongation*m_growthRate, m_growthRate)) );

  return results;
}

void Simple::G::processTurtles(std::vector<Turtle*>& turtleStack,
			       std::vector<Turtle*>& /*retiredTurtles*/,
			       std::vector<Polygon*>& /*leafSegments*/){
  Turtle* activeTurtle = turtleStack.back();

  //Move the turtle
  activeTurtle->length += m_elongation;
  turtleStack.back()->move();
  turtleStack.back()->length=0.0;
}

void Simple::G::print(std::ostream& os) const {
  os << "G(" << m_elongation << "," << m_growthRate << ")";
}

//Move in the turtles down direction
Simple::Down::Down(LeafConstructionInterface* constructor, double distance) : LeafSystemInterface(constructor) {
  m_distance = distance;
}

std::vector<LSysPtr> Simple::Down::applyRule() {

  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new Down(m_constructor, m_distance)) );

  return results;
}

void Simple::Down::processTurtles(std::vector<Turtle*>& turtleStack,
				  std::vector<Turtle*>& /*retiredTurtles*/,
				  std::vector<Polygon*>& /*leafSegments*/){
  TVector3 downVector(0.0, 0.0, -1.0*m_distance);
  
  Turtle* activeTurtle = turtleStack.back();
  activeTurtle->moveAlongVector(downVector);
}

void Simple::Down::print(std::ostream& os) const {
  os << "D(" << m_distance << ")";
}


//Store the current state on the stack
Simple::LeftBracket::LeftBracket(LeafConstructionInterface* constructor) : LeafSystemInterface(constructor) { }

std::vector<LSysPtr> Simple::LeftBracket::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr(new LeftBracket(m_constructor)));
  return results;
}

void Simple::LeftBracket::processTurtles(std::vector<Turtle*>& turtleStack,
					 std::vector<Turtle*>& /*retiredTurtles*/,
					 std::vector<Polygon*>& /*leafSegments*/){
  turtleStack.push_back(new Turtle( *(turtleStack.back()) ));
}

void Simple::LeftBracket::print(std::ostream& os) const {
  os << "[";
}

//Retrieve the state from the stack
Simple::RightBracket::RightBracket(LeafConstructionInterface* constructor) : LeafSystemInterface(constructor) { }

std::vector<LSysPtr> Simple::RightBracket::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr(new RightBracket(m_constructor)));
  return results;
}

void Simple::RightBracket::processTurtles(std::vector<Turtle*>& turtleStack,
					  std::vector<Turtle*>& /*retiredTurtles*/,
					  std::vector<Polygon*>& /*leafSegments*/){
  delete turtleStack.back();
  turtleStack.pop_back();
}

void Simple::RightBracket::print(std::ostream& os) const {
  os << "]";
}

//Rotate around vector H by an angle in degrees.
Simple::Slash::Slash(LeafConstructionInterface* constructor, double angle) : LeafSystemInterface(constructor) {
  m_angle = angle;
}

std::vector<LSysPtr> Simple::Slash::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new Slash(m_constructor, m_angle)) );
  return results;
}

void Simple::Slash::processTurtles(std::vector<Turtle*>& turtleStack,
				   std::vector<Turtle*>& /*retiredTurtles*/,
				   std::vector<Polygon*>& /*leafSegments*/){
  //Get rotation angle in radians
  double angleToRotate = m_angle * (M_PI/180.0);

  //Rotate lVector around orientation
  turtleStack.back()->lVector.Rotate(angleToRotate, turtleStack.back()->orientation);
}

void Simple::Slash::print(std::ostream& os) const {
  os << "/(" << m_angle << ")";
}

//Rotate around vector L by an angle in degrees.
Simple::Ampersand::Ampersand(LeafConstructionInterface* constructor, double angle) : LeafSystemInterface(constructor) {
  m_angle = angle;
}

std::vector<LSysPtr> Simple::Ampersand::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new Ampersand(m_constructor, m_angle)) );
  return results;
}

void Simple::Ampersand::processTurtles(std::vector<Turtle*>& turtleStack,
				       std::vector<Turtle*>& /*retiredTurtles*/,
				       std::vector<Polygon*>& /*leafSegments*/){
  //Get rotation angle in radians
  double angleToRotate = m_angle * (M_PI/180.0);

  //Rotate orientation around L vector
  turtleStack.back()->orientation.Rotate(angleToRotate, turtleStack.back()->lVector);
}

void Simple::Ampersand::print(std::ostream& os) const {
  os << "&(" << m_angle << ")";
}


//Rotate around vertical vector in clockwise direction.
Simple::Plus::Plus(LeafConstructionInterface* constructor, double angle) : LeafSystemInterface(constructor) {
  m_angle = angle;
}

std::vector<LSysPtr> Simple::Plus::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new Plus(m_constructor, m_angle)) );
  return results;
}

void Simple::Plus::processTurtles(std::vector<Turtle*>& turtleStack,
				  std::vector<Turtle*>& /*retiredTurtles*/,
				  std::vector<Polygon*>& /*leafSegments*/){
  //Get rotation angle in radians
  double angleToRotate = m_angle * (M_PI/180.0);

  //Rotate orientation around vertical vector
  TVector3 verticalVector(0.0, 0.0, 1.0);
  turtleStack.back()->orientation.Rotate(angleToRotate, verticalVector);

  //Also rotate the lVector!
  turtleStack.back()->lVector.Rotate(angleToRotate, verticalVector);
}

void Simple::Plus::print(std::ostream& os) const {
  os << "+(" << m_angle << ")";
}

//Rotate around vertical vector in the anti-clockwise direction
Simple::Minus::Minus(LeafConstructionInterface* constructor, double angle) : LeafSystemInterface(constructor) {
  m_angle = angle;
}

std::vector<LSysPtr> Simple::Minus::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new Minus(m_constructor, m_angle)) );
  return results;
}

void Simple::Minus::processTurtles(std::vector<Turtle*>& turtleStack,
				   std::vector<Turtle*>& /*retiredTurtles*/,
				   std::vector<Polygon*>& /*leafSegments*/){
  //Get rotation angle in radians
  double angleToRotate = - m_angle * (M_PI/180.0);

  //Rotate orientation around vertical vector
  TVector3 verticalVector(0.0, 0.0, 1.0);
  turtleStack.back()->orientation.Rotate(angleToRotate, verticalVector);

  //Also rotate the lVector!
  turtleStack.back()->lVector.Rotate(angleToRotate, verticalVector);
}

void Simple::Minus::print(std::ostream& os) const {
  os << "-(" << m_angle << ")";
}


//Start a new polygon
Simple::CurlyLeft::CurlyLeft(LeafConstructionInterface* constructor) : LeafSystemInterface(constructor) {}

std::vector<LSysPtr> Simple::CurlyLeft::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new CurlyLeft(m_constructor)) );
  return results;
}

void Simple::CurlyLeft::processTurtles(std::vector<Turtle*>& /*turtleStack*/,
				       std::vector<Turtle*>& /*retiredTurtles*/,
				       std::vector<Polygon*>& leafSegments){
  leafSegments.push_back(new Polygon());
}

void Simple::CurlyLeft::print(std::ostream& os) const {
  os << "{";
}

//Finish a polygon (doesn't do anything!)
Simple::CurlyRight::CurlyRight(LeafConstructionInterface* constructor) : LeafSystemInterface(constructor) {}

std::vector<LSysPtr> Simple::CurlyRight::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new CurlyRight(m_constructor)) );
  return results;
}

void Simple::CurlyRight::processTurtles(std::vector<Turtle*>& /*turtleStack*/,
					std::vector<Turtle*>& /*retiredTurtles*/,
					std::vector<Polygon*>& /*leafSegments*/){
  //Doesn't need to do anything just yet
}

void Simple::CurlyRight::print(std::ostream& os) const {
  os << "}";
}


//Create a vertex
Simple::Dot::Dot(LeafConstructionInterface* constructor) : LeafSystemInterface(constructor) {}

std::vector<LSysPtr> Simple::Dot::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new Dot(m_constructor)) );
  return results;
}

void Simple::Dot::processTurtles(std::vector<Turtle*>& turtleStack,
				 std::vector<Turtle*>& /*retiredTurtles*/,
				 std::vector<Polygon*>& leafSegments){
  leafSegments.back()->addVertex(turtleStack.back()->position);
}

void Simple::Dot::print(std::ostream& os) const {
  os << ".";
}


//Control the growth
Simple::A::A(LeafConstructionInterface* constructor, double timeIndex) : LeafSystemInterface(constructor) { 
  m_timeIndex = timeIndex;
}

std::vector<LSysPtr> Simple::A::applyRule() {
  std::vector<LSysPtr> results;

  if (fabs(m_timeIndex) > 0.0001) {
    results.push_back(LSysPtr( new G(m_constructor, m_constructor->getDoubleParameter("mainInitialLength"), m_constructor->getDoubleParameter("mainGrowthRate")) ));
  }

  //T1
  results.push_back(LSysPtr( new LeftBracket(m_constructor) ));
  results.push_back(LSysPtr( new CurlyLeft(m_constructor) ));
  results.push_back(LSysPtr( new Ampersand(m_constructor, -m_constructor->getDoubleParameter("divergenceAngle")) ));
  results.push_back(LSysPtr( new B(m_constructor, m_timeIndex) ));
  results.push_back(LSysPtr( new Dot(m_constructor) ));
  results.push_back(LSysPtr( new RightBracket(m_constructor) ));

  results.push_back(LSysPtr( new LeftBracket(m_constructor) ));
  results.push_back(LSysPtr( new Dot(m_constructor) ));
  results.push_back(LSysPtr( new G(m_constructor, -m_constructor->getDoubleParameter("mainInitialLength"), m_constructor->getDoubleParameter("mainGrowthRate")) ));
  results.push_back(LSysPtr( new Dot(m_constructor) ));
  results.push_back(LSysPtr( new CurlyRight(m_constructor) ));
  results.push_back(LSysPtr( new RightBracket(m_constructor) ));

  //T2
  results.push_back(LSysPtr( new LeftBracket(m_constructor) ));
  results.push_back(LSysPtr( new CurlyLeft(m_constructor) ));
  results.push_back(LSysPtr( new G(m_constructor, -m_constructor->getDoubleParameter("mainInitialLength"), m_constructor->getDoubleParameter("mainGrowthRate")) ));
  results.push_back(LSysPtr( new Dot(m_constructor) ));
  results.push_back(LSysPtr( new Ampersand(m_constructor, -m_constructor->getDoubleParameter("divergenceAngle")) ));
  results.push_back(LSysPtr( new G(m_constructor, m_constructor->getDoubleParameter("lateralInitialLength"), m_constructor->getDoubleParameter("lateralGrowthRate")) ));
  results.push_back(LSysPtr( new B(m_constructor, m_timeIndex-1) ));
  results.push_back(LSysPtr( new Dot(m_constructor) ));
  results.push_back(LSysPtr( new RightBracket(m_constructor) ));

  results.push_back(LSysPtr( new LeftBracket(m_constructor) ));
  results.push_back(LSysPtr( new Ampersand(m_constructor, -m_constructor->getDoubleParameter("divergenceAngle")) ));
  results.push_back(LSysPtr( new B(m_constructor, m_timeIndex) ));
  results.push_back(LSysPtr( new Dot(m_constructor) ));
  results.push_back(LSysPtr( new CurlyRight(m_constructor) ));
  results.push_back(LSysPtr( new RightBracket(m_constructor) ));

  //Produce some growth
  results.push_back(LSysPtr( new LeftBracket(m_constructor) ));
  results.push_back(LSysPtr( new A(m_constructor, m_timeIndex+1.0) ));
  results.push_back(LSysPtr( new RightBracket(m_constructor) ));

  //T3
  results.push_back(LSysPtr( new LeftBracket(m_constructor) ));
  results.push_back(LSysPtr( new CurlyLeft(m_constructor) ));
  results.push_back(LSysPtr( new Dot(m_constructor) ));
  results.push_back(LSysPtr( new Ampersand(m_constructor, m_constructor->getDoubleParameter("divergenceAngle")) ));
  results.push_back(LSysPtr( new B(m_constructor, m_timeIndex) ));
  results.push_back(LSysPtr( new Dot(m_constructor) ));
  results.push_back(LSysPtr( new RightBracket(m_constructor) ));

  results.push_back(LSysPtr( new LeftBracket(m_constructor) ));
  results.push_back(LSysPtr( new G(m_constructor, -m_constructor->getDoubleParameter("mainInitialLength"), m_constructor->getDoubleParameter("mainGrowthRate")) ));
  results.push_back(LSysPtr( new Dot(m_constructor) ));
  results.push_back(LSysPtr( new CurlyRight(m_constructor) ));
  results.push_back(LSysPtr( new RightBracket(m_constructor) ));

  //T4
  results.push_back(LSysPtr( new LeftBracket(m_constructor) ));
  results.push_back(LSysPtr( new CurlyLeft(m_constructor) ));
  results.push_back(LSysPtr( new G(m_constructor, -m_constructor->getDoubleParameter("mainInitialLength"), m_constructor->getDoubleParameter("mainGrowthRate")) ));
  results.push_back(LSysPtr( new Dot(m_constructor) ));
  results.push_back(LSysPtr( new RightBracket(m_constructor) ));

  results.push_back(LSysPtr( new LeftBracket(m_constructor) ));
  results.push_back(LSysPtr( new Ampersand(m_constructor, m_constructor->getDoubleParameter("divergenceAngle")) ));
  results.push_back(LSysPtr( new B(m_constructor, m_timeIndex) ));
  results.push_back(LSysPtr( new Dot(m_constructor) ));
  results.push_back(LSysPtr( new RightBracket(m_constructor) ));

  results.push_back(LSysPtr( new LeftBracket(m_constructor) ));
  results.push_back(LSysPtr( new G(m_constructor, -m_constructor->getDoubleParameter("mainInitialLength"), m_constructor->getDoubleParameter("mainGrowthRate")) ));
  results.push_back(LSysPtr( new Ampersand(m_constructor, m_constructor->getDoubleParameter("divergenceAngle")) ));
  results.push_back(LSysPtr( new G(m_constructor, m_constructor->getDoubleParameter("lateralInitialLength"), m_constructor->getDoubleParameter("lateralGrowthRate")) ));
  results.push_back(LSysPtr( new B(m_constructor, m_timeIndex-1.0) ));
  results.push_back(LSysPtr( new Dot(m_constructor) ));
  results.push_back(LSysPtr( new CurlyRight(m_constructor) ));
  results.push_back(LSysPtr( new RightBracket(m_constructor) ));

  return results;
}

void Simple::A::processTurtles(std::vector<Turtle*>& /*turtleStack*/,
			       std::vector<Turtle*>& /*retiredTurtles*/,
			       std::vector<Polygon*>& /*leafSegments*/){
  //do nothing
}

void Simple::A::print(std::ostream& os) const {
  os << "A(" << m_timeIndex << ")";
}


//Control the growth
Simple::B::B(LeafConstructionInterface* constructor, double timeIndex) : LeafSystemInterface(constructor) { 
  m_timeIndex = timeIndex;
}

std::vector<LSysPtr> Simple::B::applyRule() {
  std::vector<LSysPtr> results;

  results.push_back(LSysPtr( new G(m_constructor, m_constructor->getDoubleParameter("lateralInitialLength"), m_constructor->getDoubleParameter("lateralGrowthRate")) ));
  results.push_back(LSysPtr( new B(m_constructor, m_timeIndex-m_constructor->getDoubleParameter("growthPotentialDecrease")) ));


  return results;
}

void Simple::B::processTurtles(std::vector<Turtle*>& /*turtleStack*/,
			       std::vector<Turtle*>& /*retiredTurtles*/,
			       std::vector<Polygon*>& /*leafSegments*/){
  //do nothing
}

void Simple::B::print(std::ostream& os) const {
  os << "B(" << m_timeIndex << ")";
}




