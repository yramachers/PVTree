#include "pvtree/leafSystem/planar.hpp"
#include "pvtree/geometry/turtle.hpp"
#include "pvtree/geometry/polygon.hpp"
#include "pvtree/leafSystem/leafConstructionInterface.hpp"

// To shorten the following statements
using LSysPtr = std::shared_ptr<LeafSystemInterface>;

// Move but don't create a structure by itself (reserved for vertex creation
// rule)
Planar::G::G(LeafConstructionInterface* constructor, double elongation,
             double growthRate)
    : LeafSystemInterface(constructor) {
  m_elongation = elongation;
  m_growthRate = growthRate;
}

std::vector<LSysPtr> Planar::G::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(
      LSysPtr(new G(m_constructor, m_elongation * m_growthRate, m_growthRate)));

  return results;
}

void Planar::G::processTurtles(std::vector<Turtle*>& turtleStack,
                               std::vector<Turtle*>& /*retiredTurtles*/,
                               std::vector<Polygon*>& /*leafSegments*/) {
  Turtle* activeTurtle = turtleStack.back();

  // Move the turtle
  activeTurtle->length += m_elongation;
  turtleStack.back()->move();
  turtleStack.back()->length = 0.0;
}

void Planar::G::print(std::ostream& os) const {
  os << "G(" << m_elongation << "," << m_growthRate << ")";
}

// Store the current state on the stack
Planar::LeftBracket::LeftBracket(LeafConstructionInterface* constructor)
    : LeafSystemInterface(constructor) {}

std::vector<LSysPtr> Planar::LeftBracket::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr(new LeftBracket(m_constructor)));
  return results;
}

void Planar::LeftBracket::processTurtles(
    std::vector<Turtle*>& turtleStack, std::vector<Turtle*>& /*retiredTurtles*/,
    std::vector<Polygon*>& /*leafSegments*/) {
  turtleStack.push_back(new Turtle(*(turtleStack.back())));
}

void Planar::LeftBracket::print(std::ostream& os) const { os << "["; }

// Retrieve the state from the stack
Planar::RightBracket::RightBracket(LeafConstructionInterface* constructor)
    : LeafSystemInterface(constructor) {}

std::vector<LSysPtr> Planar::RightBracket::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr(new RightBracket(m_constructor)));
  return results;
}

void Planar::RightBracket::processTurtles(
    std::vector<Turtle*>& turtleStack, std::vector<Turtle*>& /*retiredTurtles*/,
    std::vector<Polygon*>& /*leafSegments*/) {
  delete turtleStack.back();
  turtleStack.pop_back();
}

void Planar::RightBracket::print(std::ostream& os) const { os << "]"; }

// Rotate around vector H by an angle in degrees.
Planar::Slash::Slash(LeafConstructionInterface* constructor, double angle)
    : LeafSystemInterface(constructor) {
  m_angle = angle;
}

std::vector<LSysPtr> Planar::Slash::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr(new Slash(m_constructor, m_angle)));
  return results;
}

void Planar::Slash::processTurtles(std::vector<Turtle*>& turtleStack,
                                   std::vector<Turtle*>& /*retiredTurtles*/,
                                   std::vector<Polygon*>& /*leafSegments*/) {
  // Get rotation angle in radians
  double angleToRotate = m_angle * (M_PI / 180.0);

  // Rotate lVector around orientation
  turtleStack.back()->lVector.Rotate(angleToRotate,
                                     turtleStack.back()->orientation);
}

void Planar::Slash::print(std::ostream& os) const {
  os << "/(" << m_angle << ")";
}

// Rotate around vector L by an angle in degrees.
Planar::Ampersand::Ampersand(LeafConstructionInterface* constructor,
                             double angle)
    : LeafSystemInterface(constructor) {
  m_angle = angle;
}

std::vector<LSysPtr> Planar::Ampersand::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr(new Ampersand(m_constructor, m_angle)));
  return results;
}

void Planar::Ampersand::processTurtles(
    std::vector<Turtle*>& turtleStack, std::vector<Turtle*>& /*retiredTurtles*/,
    std::vector<Polygon*>& /*leafSegments*/) {
  // Get rotation angle in radians
  double angleToRotate = m_angle * (M_PI / 180.0);

  // Rotate orientation around L vector
  turtleStack.back()->orientation.Rotate(angleToRotate,
                                         turtleStack.back()->lVector);
}

void Planar::Ampersand::print(std::ostream& os) const {
  os << "&(" << m_angle << ")";
}

// Start a new polygon
Planar::CurlyLeft::CurlyLeft(LeafConstructionInterface* constructor)
    : LeafSystemInterface(constructor) {}

std::vector<LSysPtr> Planar::CurlyLeft::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr(new CurlyLeft(m_constructor)));
  return results;
}

void Planar::CurlyLeft::processTurtles(std::vector<Turtle*>& /*turtleStack*/,
                                       std::vector<Turtle*>& /*retiredTurtles*/,
                                       std::vector<Polygon*>& leafSegments) {
  leafSegments.push_back(new Polygon());
}

void Planar::CurlyLeft::print(std::ostream& os) const { os << "{"; }

// Finish a polygon (doesn't do anything!)
Planar::CurlyRight::CurlyRight(LeafConstructionInterface* constructor)
    : LeafSystemInterface(constructor) {}

std::vector<LSysPtr> Planar::CurlyRight::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr(new CurlyRight(m_constructor)));
  return results;
}

void Planar::CurlyRight::processTurtles(
    std::vector<Turtle*>& /*turtleStack*/,
    std::vector<Turtle*>& /*retiredTurtles*/,
    std::vector<Polygon*>& /*leafSegments*/) {
  // Doesn't need to do anything just yet
}

void Planar::CurlyRight::print(std::ostream& os) const { os << "}"; }

// Create a vertex
Planar::Dot::Dot(LeafConstructionInterface* constructor)
    : LeafSystemInterface(constructor) {}

std::vector<LSysPtr> Planar::Dot::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr(new Dot(m_constructor)));
  return results;
}

void Planar::Dot::processTurtles(std::vector<Turtle*>& turtleStack,
                                 std::vector<Turtle*>& /*retiredTurtles*/,
                                 std::vector<Polygon*>& leafSegments) {
  leafSegments.back()->addVertex(turtleStack.back()->position);
}

void Planar::Dot::print(std::ostream& os) const { os << "."; }
