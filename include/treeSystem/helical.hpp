#ifndef LSYSTEM_HELICAL
#define LSYSTEM_HELICAL

#include "treeSystem/treeSystemInterface.hpp"
#include <vector>
#include <iostream>
#include <memory>
#include <cmath>

class TreeConstructionInterface;

/*! System that can produce helical like structures (and probably many other weird
/ and wonderful structures). It has optional sympodial like branching. */
namespace HelicalBranching {

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


  /*! Trunk movement without formation */
  class f : public TreeSystemInterface {
  private:
    double m_elongation;

  public:
    f(TreeConstructionInterface* constructor, double elongation);
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


  /*! Altering length of current active turtle */
  class Woosh : public TreeSystemInterface {
  private:
    double m_length;
    double m_elongation;

  public:
    Woosh(TreeConstructionInterface* constructor, double length, double elongation);
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


  /*! Rotate around the vertical vector in the clockwise direction */
  class Plus : public TreeSystemInterface {
  private:
    double m_angle;

  public:
    Plus(TreeConstructionInterface* constructor, double angle);
    std::vector<std::shared_ptr<TreeSystemInterface>> applyRule();
    void processTurtles(std::vector<Turtle*>& turtleStack,
			std::vector<Turtle*>& retiredTurtles);
    void print(std::ostream& os) const;
  };


  /*! Rotate around the vertical vector in the anti-clockwise direction */
  class Minus : public TreeSystemInterface {
  private:
    double m_angle;

  public:
    Minus(TreeConstructionInterface* constructor, double angle);
    std::vector<std::shared_ptr<TreeSystemInterface>> applyRule();
    void processTurtles(std::vector<Turtle*>& turtleStack,
			std::vector<Turtle*>& retiredTurtles);
    void print(std::ostream& os) const;
  };


  /*! Controls the growth of the trunk */
  class A : public TreeSystemInterface {
  private:
    double m_length;
    double m_width;
    double m_angle;
    int    m_count;

  public:
    A(TreeConstructionInterface* constructor, double length, double width, double angle, int count);
    std::vector<std::shared_ptr<TreeSystemInterface>> applyRule();
    void processTurtles(std::vector<Turtle*>& turtleStack,
			std::vector<Turtle*>& retiredTurtles);
    void print(std::ostream& os) const;
  };


  /*! Controls the growth of the leaf points */
  class B : public TreeSystemInterface {
  private:
    double m_length;
    double m_width;

  public:
    B(TreeConstructionInterface* constructor, double length, double width);
    std::vector<std::shared_ptr<TreeSystemInterface>> applyRule();
    void processTurtles(std::vector<Turtle*>& turtleStack,
			std::vector<Turtle*>& retiredTurtles);
    void print(std::ostream& os) const;
  };

}

#endif //LSYSTEM_HELICAL




