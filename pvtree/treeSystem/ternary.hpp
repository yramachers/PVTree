#ifndef LSYSTEM_ALGO_BOTANY_TERNARY
#define LSYSTEM_ALGO_BOTANY_TERNARY

#include "pvtree/treeSystem/treeSystemInterface.hpp"
#include <vector>
#include <iostream>
#include <memory>
#include <cmath>

class TreeConstructionInterface;

/*! Simple example test using the L-System defined in Algorithmic botany.
/ See chapter 2 figure 2.8 in http://algorithmicbotany.org/papers/abop/abop.pdf
*/
namespace TernaryBranching {

/*! Trunk formation */
class F : public TreeSystemInterface {
 private:
  double m_elongation;

 public:
  F(TreeConstructionInterface* constructor, double elongation);
  std::vector<std::shared_ptr<TreeSystemInterface>> applyRule();
  void processTurtles(std::vector<Turtle*>& turtleStack,
                      std::vector<Turtle*>& retiredTurtles);
  void print(std::ostream& os) const;
};

/*! Width */
class Exclame : public TreeSystemInterface {
 private:
  double m_width;

 public:
  Exclame(TreeConstructionInterface* constructor, double width);
  std::vector<std::shared_ptr<TreeSystemInterface>> applyRule();
  void processTurtles(std::vector<Turtle*>& turtleStack,
                      std::vector<Turtle*>& retiredTurtles);
  void print(std::ostream& os) const;
};

/*! Store the current state on the stack */
class LeftBracket : public TreeSystemInterface {
 private:
 public:
  explicit LeftBracket(TreeConstructionInterface* constructor);
  std::vector<std::shared_ptr<TreeSystemInterface>> applyRule();
  void processTurtles(std::vector<Turtle*>& turtleStack,
                      std::vector<Turtle*>& retiredTurtles);
  void print(std::ostream& os) const;
};

/*! Pull the last state onto the stack */
class RightBracket : public TreeSystemInterface {
 private:
 public:
  explicit RightBracket(TreeConstructionInterface* constructor);
  std::vector<std::shared_ptr<TreeSystemInterface>> applyRule();
  void processTurtles(std::vector<Turtle*>& turtleStack,
                      std::vector<Turtle*>& retiredTurtles);
  void print(std::ostream& os) const;
};

/*! Rotate around vector H by an angle in degrees. */
class Slash : public TreeSystemInterface {
 private:
  double m_angle;

 public:
  Slash(TreeConstructionInterface* constructor, double angle);
  std::vector<std::shared_ptr<TreeSystemInterface>> applyRule();
  void processTurtles(std::vector<Turtle*>& turtleStack,
                      std::vector<Turtle*>& retiredTurtles);
  void print(std::ostream& os) const;
};

/*! Rotate around vector L by an angle in degrees. */
class Ampersand : public TreeSystemInterface {
 private:
  double m_angle;

 public:
  Ampersand(TreeConstructionInterface* constructor, double angle);
  std::vector<std::shared_ptr<TreeSystemInterface>> applyRule();
  void processTurtles(std::vector<Turtle*>& turtleStack,
                      std::vector<Turtle*>& retiredTurtles);
  void print(std::ostream& os) const;
};

/*! Controls the growth (Doesn't draw anything!) */
class A : public TreeSystemInterface {
 private:
 public:
  explicit A(TreeConstructionInterface* constructor);
  std::vector<std::shared_ptr<TreeSystemInterface>> applyRule();
  void processTurtles(std::vector<Turtle*>& turtleStack,
                      std::vector<Turtle*>& retiredTurtles);
  void print(std::ostream& os) const;
};
}

#endif  // LSYSTEM_ALGO_BOTANY_TERNARY
