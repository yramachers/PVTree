#ifndef LEAF_SYSTEM_SIMPLE
#define LEAF_SYSTEM_SIMPLE

#include "pvtree/leafSystem/leafSystemInterface.hpp"
#include <vector>
#include <iostream>
#include <memory>
#include <cmath>

class Turtle;
class Polygon;
class LeafConstructionInterface;

/*! Fractal leaf generation in a similar fashion to the way the rest of the
/ tree structure is created. Only a leaf surface is actually produced, requires
/ a little extra work to make it solid. */
namespace Simple {

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

/*! Move turtle along the down direction */
class Down : public LeafSystemInterface {
 private:
  double m_distance;

 public:
  Down(LeafConstructionInterface* constructor, double distance);
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

/*! Rotate around vertical vector in the clockwise direction */
class Plus : public LeafSystemInterface {
 private:
  double m_angle;

 public:
  Plus(LeafConstructionInterface* constructor, double angle);
  std::vector<std::shared_ptr<LeafSystemInterface>> applyRule();
  void processTurtles(std::vector<Turtle*>& turtleStack,
                      std::vector<Turtle*>& retiredTurtles,
                      std::vector<Polygon*>& leafSegments);
  void print(std::ostream& os) const;
};

/*! Rotate around vertical vector in the anti-clockwise direction */
class Minus : public LeafSystemInterface {
 private:
  double m_angle;

 public:
  Minus(LeafConstructionInterface* constructor, double angle);
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

/*! Control the growth */
class A : public LeafSystemInterface {
 private:
  double m_timeIndex;

 public:
  A(LeafConstructionInterface* constructor, double timeIndex);
  std::vector<std::shared_ptr<LeafSystemInterface>> applyRule();
  void processTurtles(std::vector<Turtle*>& turtleStack,
                      std::vector<Turtle*>& retiredTurtles,
                      std::vector<Polygon*>& leafSegments);
  void print(std::ostream& os) const;
};

/*! Control the growth */
class B : public LeafSystemInterface {
 private:
  double m_timeIndex;

 public:
  B(LeafConstructionInterface* constructor, double timeIndex);
  std::vector<std::shared_ptr<LeafSystemInterface>> applyRule();
  void processTurtles(std::vector<Turtle*>& turtleStack,
                      std::vector<Turtle*>& retiredTurtles,
                      std::vector<Polygon*>& leafSegments);
  void print(std::ostream& os) const;
};
}

#endif  // LEAF_SYSTEM_SIMPLE
