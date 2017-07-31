#ifndef PVTREE_LEAF_SYSTEM_PLANAR_HPP
#define PVTREE_LEAF_SYSTEM_PLANAR_HPP

#include "pvtree/leafSystem/leafSystemInterface.hpp"
#include <vector>
#include <iostream>
#include <memory>
#include <cmath>

class Turtle;
class Polygon;
class LeafConstructionInterface;

/*! Produce a very simple planar leaf according to a few parameters.
 *  Only a leaf surface is actually produced, requires a little extra
 *  work to make it solid. */
namespace Planar {

/*! Move but don't create any structure by itself */
class G : public LeafSystemInterface {
 private:
  double m_elongation;
  double m_growthRate;

 public:
  G(LeafConstructionInterface* constructor, double elongation,
    double growthRate);
  std::vector<std::shared_ptr<LeafSystemInterface>> applyRule();
  void processTurtles(std::vector<Turtle*>& turtleStack,
                      std::vector<Turtle*>& retiredTurtles,
                      std::vector<Polygon*>& leafSegments);
  void print(std::ostream& os) const;
};

/*! Store current state on the stack */
class LeftBracket : public LeafSystemInterface {
 private:
 public:
  explicit LeftBracket(LeafConstructionInterface* constructor);
  std::vector<std::shared_ptr<LeafSystemInterface>> applyRule();
  void processTurtles(std::vector<Turtle*>& turtleStack,
                      std::vector<Turtle*>& retiredTurtles,
                      std::vector<Polygon*>& leafSegments);
  void print(std::ostream& os) const;
};

/*! Retrieve state from the stack */
class RightBracket : public LeafSystemInterface {
 private:
 public:
  explicit RightBracket(LeafConstructionInterface* constructor);
  std::vector<std::shared_ptr<LeafSystemInterface>> applyRule();
  void processTurtles(std::vector<Turtle*>& turtleStack,
                      std::vector<Turtle*>& retiredTurtles,
                      std::vector<Polygon*>& leafSegments);
  void print(std::ostream& os) const;
};

/*! Rotate around vector H */
class Slash : public LeafSystemInterface {
 private:
  double m_angle;

 public:
  Slash(LeafConstructionInterface* constructor, double angle);
  std::vector<std::shared_ptr<LeafSystemInterface>> applyRule();
  void processTurtles(std::vector<Turtle*>& turtleStack,
                      std::vector<Turtle*>& retiredTurtles,
                      std::vector<Polygon*>& leafSegments);
  void print(std::ostream& os) const;
};

/*! Rotate around vector L */
class Ampersand : public LeafSystemInterface {
 private:
  double m_angle;

 public:
  Ampersand(LeafConstructionInterface* constructor, double angle);
  std::vector<std::shared_ptr<LeafSystemInterface>> applyRule();
  void processTurtles(std::vector<Turtle*>& turtleStack,
                      std::vector<Turtle*>& retiredTurtles,
                      std::vector<Polygon*>& leafSegments);
  void print(std::ostream& os) const;
};

/*! Start a new polygon */
class CurlyLeft : public LeafSystemInterface {
 private:
 public:
  explicit CurlyLeft(LeafConstructionInterface* constructor);
  std::vector<std::shared_ptr<LeafSystemInterface>> applyRule();
  void processTurtles(std::vector<Turtle*>& turtleStack,
                      std::vector<Turtle*>& retiredTurtles,
                      std::vector<Polygon*>& leafSegments);
  void print(std::ostream& os) const;
};

/*! Finish a polygon */
class CurlyRight : public LeafSystemInterface {
 private:
 public:
  explicit CurlyRight(LeafConstructionInterface* constructor);
  std::vector<std::shared_ptr<LeafSystemInterface>> applyRule();
  void processTurtles(std::vector<Turtle*>& turtleStack,
                      std::vector<Turtle*>& retiredTurtles,
                      std::vector<Polygon*>& leafSegments);
  void print(std::ostream& os) const;
};

/*! Create a vertex */
class Dot : public LeafSystemInterface {
 private:
 public:
  explicit Dot(LeafConstructionInterface* constructor);
  std::vector<std::shared_ptr<LeafSystemInterface>> applyRule();
  void processTurtles(std::vector<Turtle*>& turtleStack,
                      std::vector<Turtle*>& retiredTurtles,
                      std::vector<Polygon*>& leafSegments);
  void print(std::ostream& os) const;
};
}

#endif  // PVTREE_LEAF_SYSTEM_PLANAR_HPP
