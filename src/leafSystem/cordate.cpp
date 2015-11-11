#include "leafSystem/cordate.hpp"
#include "geometry/turtle.hpp"
#include "geometry/polygon.hpp"
#include "leafSystem/leafConstructionInterface.hpp"

//To shorten the following statements
using LSysPtr = std::shared_ptr<LeafSystemInterface>;

//Move but don't create a structure by itself (reserved for vertex creation rule)
Cordate::G::G(LeafConstructionInterface* constructor, double elongation) : LeafSystemInterface(constructor) {
  m_elongation = elongation;
}

std::vector<LSysPtr> Cordate::G::applyRule() {

  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new G(m_constructor, m_elongation)) );

  return results;
}

void Cordate::G::processTurtles(std::vector<Turtle*>& turtleStack,
				std::vector<Turtle*>& /*retiredTurtles*/,
				std::vector<Polygon*>& /*leafSegments*/){
  Turtle* activeTurtle = turtleStack.back();

  //Move the turtle
  activeTurtle->length += m_elongation;
  turtleStack.back()->move();
  turtleStack.back()->length=0.0;
}

void Cordate::G::print(std::ostream& os) const {
  os << "G(" << m_elongation << ")";
}

//Move in the turtles down direction
Cordate::Down::Down(LeafConstructionInterface* constructor, double distance) : LeafSystemInterface(constructor) {
  m_distance = distance;
}

std::vector<LSysPtr> Cordate::Down::applyRule() {

  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new Down(m_constructor, m_distance)) );

  return results;
}

void Cordate::Down::processTurtles(std::vector<Turtle*>& turtleStack,
				   std::vector<Turtle*>& /*retiredTurtles*/,
				   std::vector<Polygon*>& /*leafSegments*/){
  TVector3 downVector(0.0, 0.0, -1.0*m_distance);
  
  Turtle* activeTurtle = turtleStack.back();
  activeTurtle->moveAlongVector(downVector);
}

void Cordate::Down::print(std::ostream& os) const {
  os << "D(" << m_distance << ")";
}


//Store the current state on the stack
Cordate::LeftBracket::LeftBracket(LeafConstructionInterface* constructor) : LeafSystemInterface(constructor) { }

std::vector<LSysPtr> Cordate::LeftBracket::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr(new LeftBracket(m_constructor)));
  return results;
}

void Cordate::LeftBracket::processTurtles(std::vector<Turtle*>& turtleStack,
					  std::vector<Turtle*>& /*retiredTurtles*/,
					  std::vector<Polygon*>& /*leafSegments*/){
  turtleStack.push_back(new Turtle( *(turtleStack.back()) ));
}

void Cordate::LeftBracket::print(std::ostream& os) const {
  os << "[";
}

//Retrieve the state from the stack
Cordate::RightBracket::RightBracket(LeafConstructionInterface* constructor) : LeafSystemInterface(constructor) { }

std::vector<LSysPtr> Cordate::RightBracket::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr(new RightBracket(m_constructor)));
  return results;
}

void Cordate::RightBracket::processTurtles(std::vector<Turtle*>& turtleStack,
					   std::vector<Turtle*>& /*retiredTurtles*/,
					   std::vector<Polygon*>& /*leafSegments*/){
  delete turtleStack.back();
  turtleStack.pop_back();
}

void Cordate::RightBracket::print(std::ostream& os) const {
  os << "]";
}

//Rotate around vector H by an angle in degrees.
Cordate::Slash::Slash(LeafConstructionInterface* constructor, double angle) : LeafSystemInterface(constructor) {
  m_angle = angle;
}

std::vector<LSysPtr> Cordate::Slash::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new Slash(m_constructor, m_angle)) );
  return results;
}

void Cordate::Slash::processTurtles(std::vector<Turtle*>& turtleStack,
				    std::vector<Turtle*>& /*retiredTurtles*/,
				    std::vector<Polygon*>& /*leafSegments*/){
  //Get rotation angle in radians
  double angleToRotate = m_angle * (M_PI/180.0);

  //Rotate lVector around orientation
  turtleStack.back()->lVector.Rotate(angleToRotate, turtleStack.back()->orientation);
}

void Cordate::Slash::print(std::ostream& os) const {
  os << "/(" << m_angle << ")";
}

//Rotate around vector L by an angle in degrees.
Cordate::Ampersand::Ampersand(LeafConstructionInterface* constructor, double angle) : LeafSystemInterface(constructor) {
  m_angle = angle;
}

std::vector<LSysPtr> Cordate::Ampersand::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new Ampersand(m_constructor, m_angle)) );
  return results;
}

void Cordate::Ampersand::processTurtles(std::vector<Turtle*>& turtleStack,
					std::vector<Turtle*>& /*retiredTurtles*/,
					std::vector<Polygon*>& /*leafSegments*/){
  //Get rotation angle in radians
  double angleToRotate = m_angle * (M_PI/180.0);

  //Rotate orientation around L vector
  turtleStack.back()->orientation.Rotate(angleToRotate, turtleStack.back()->lVector);
}

void Cordate::Ampersand::print(std::ostream& os) const {
  os << "&(" << m_angle << ")";
}


//Rotate around vertical vector in clockwise direction.
Cordate::Plus::Plus(LeafConstructionInterface* constructor, double angle) : LeafSystemInterface(constructor) {
  m_angle = angle;
}

std::vector<LSysPtr> Cordate::Plus::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new Plus(m_constructor, m_angle)) );
  return results;
}

void Cordate::Plus::processTurtles(std::vector<Turtle*>& turtleStack,
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

void Cordate::Plus::print(std::ostream& os) const {
  os << "+(" << m_angle << ")";
}

//Rotate around vertical vector in the anti-clockwise direction
Cordate::Minus::Minus(LeafConstructionInterface* constructor, double angle) : LeafSystemInterface(constructor) {
  m_angle = angle;
}

std::vector<LSysPtr> Cordate::Minus::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new Minus(m_constructor, m_angle)) );
  return results;
}

void Cordate::Minus::processTurtles(std::vector<Turtle*>& turtleStack,
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

void Cordate::Minus::print(std::ostream& os) const {
  os << "-(" << m_angle << ")";
}


//Start a new polygon
Cordate::CurlyLeft::CurlyLeft(LeafConstructionInterface* constructor) : LeafSystemInterface(constructor) {}

std::vector<LSysPtr> Cordate::CurlyLeft::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new CurlyLeft(m_constructor)) );
  return results;
}

void Cordate::CurlyLeft::processTurtles(std::vector<Turtle*>& /*turtleStack*/,
					std::vector<Turtle*>& /*retiredTurtles*/,
					std::vector<Polygon*>& leafSegments){
  leafSegments.push_back(new Polygon());
}

void Cordate::CurlyLeft::print(std::ostream& os) const {
  os << "{";
}

//Finish a polygon (doesn't do anything!)
Cordate::CurlyRight::CurlyRight(LeafConstructionInterface* constructor) : LeafSystemInterface(constructor) {}

std::vector<LSysPtr> Cordate::CurlyRight::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new CurlyRight(m_constructor)) );
  return results;
}

void Cordate::CurlyRight::processTurtles(std::vector<Turtle*>& /*turtleStack*/,
					 std::vector<Turtle*>& /*retiredTurtles*/,
					 std::vector<Polygon*>& /*leafSegments*/){
  //Doesn't need to do anything just yet
}

void Cordate::CurlyRight::print(std::ostream& os) const {
  os << "}";
}


//Create a vertex
Cordate::Dot::Dot(LeafConstructionInterface* constructor) : LeafSystemInterface(constructor) {}

std::vector<LSysPtr> Cordate::Dot::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new Dot(m_constructor)) );
  return results;
}

void Cordate::Dot::processTurtles(std::vector<Turtle*>& turtleStack,
				  std::vector<Turtle*>& /*retiredTurtles*/,
				  std::vector<Polygon*>& leafSegments){
  leafSegments.back()->addVertex(turtleStack.back()->position);
}

void Cordate::Dot::print(std::ostream& os) const {
  os << ".";
}


//Control the growth
Cordate::A::A(LeafConstructionInterface* constructor, double directionFactor) : LeafSystemInterface(constructor) { 
  m_directionFactor = directionFactor;
}

std::vector<LSysPtr> Cordate::A::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new LeftBracket(m_constructor) ));
  results.push_back(LSysPtr( new Slash(m_constructor, m_directionFactor*m_constructor->getDoubleParameter("curlAngle") ) ));
  results.push_back(LSysPtr( new Ampersand(m_constructor, m_constructor->getDoubleParameter("divergenceAngle") ) ));
  results.push_back(LSysPtr( new A(m_constructor, m_directionFactor) ));
  results.push_back(LSysPtr( new CurlyLeft(m_constructor) ));
  results.push_back(LSysPtr( new Dot(m_constructor) ));
  results.push_back(LSysPtr( new RightBracket(m_constructor) ));
  results.push_back(LSysPtr( new Dot(m_constructor) ));
  results.push_back(LSysPtr( new C(m_constructor) ));
  results.push_back(LSysPtr( new Dot(m_constructor) ));
  results.push_back(LSysPtr( new CurlyRight(m_constructor) ));
  return results;
}

void Cordate::A::processTurtles(std::vector<Turtle*>& /*turtleStack*/,
				std::vector<Turtle*>& /*retiredTurtles*/,
				std::vector<Polygon*>& /*leafSegments*/){
  //do nothing
}

void Cordate::A::print(std::ostream& os) const {
  os << "A(" << m_directionFactor << ")";
}


//Control the growth
Cordate::B::B(LeafConstructionInterface* constructor, double directionFactor) : LeafSystemInterface(constructor) { 
  m_directionFactor = directionFactor;
}

std::vector<LSysPtr> Cordate::B::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new LeftBracket(m_constructor) ));
  results.push_back(LSysPtr( new Slash(m_constructor,-m_directionFactor*m_constructor->getDoubleParameter("curlAngle") ) ));
  results.push_back(LSysPtr( new Ampersand(m_constructor,-m_constructor->getDoubleParameter("divergenceAngle") ) ));
  results.push_back(LSysPtr( new B(m_constructor, m_directionFactor) ));
  results.push_back(LSysPtr( new CurlyLeft(m_constructor) ));
  results.push_back(LSysPtr( new Dot(m_constructor) ));
  results.push_back(LSysPtr( new RightBracket(m_constructor) ));
  results.push_back(LSysPtr( new LeftBracket(m_constructor) ));
  results.push_back(LSysPtr( new C(m_constructor) ));
  results.push_back(LSysPtr( new Dot(m_constructor) ));
  results.push_back(LSysPtr( new RightBracket(m_constructor) ));
  results.push_back(LSysPtr( new Dot(m_constructor) ));
  results.push_back(LSysPtr( new C(m_constructor) ));
  results.push_back(LSysPtr( new CurlyRight(m_constructor) ));
  return results;
}

void Cordate::B::processTurtles(std::vector<Turtle*>& /*turtleStack*/,
				std::vector<Turtle*>& /*retiredTurtles*/,
				std::vector<Polygon*>& /*leafSegments*/){
  //do nothing
}

void Cordate::B::print(std::ostream& os) const {
  os << "B(" << m_directionFactor << ")";
}


//Control the growth
Cordate::C::C(LeafConstructionInterface* constructor) : LeafSystemInterface(constructor) {}

std::vector<LSysPtr> Cordate::C::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new G(m_constructor, m_constructor->getDoubleParameter("growthRate") ) ));
  results.push_back(LSysPtr( new C(m_constructor) ));
  return results;
}

void Cordate::C::processTurtles(std::vector<Turtle*>& /*turtleStack*/,
				std::vector<Turtle*>& /*retiredTurtles*/,
				std::vector<Polygon*>& /*leafSegments*/){
  //do nothing
}

void Cordate::C::print(std::ostream& os) const {
  os << "C";
}

