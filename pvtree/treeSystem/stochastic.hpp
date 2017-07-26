#ifndef LSYSTEM_STOCHASTIC
#define LSYSTEM_STOCHASTIC

#include "pvtree/treeSystem/treeSystemInterface.hpp"
#include <vector>
#include <iostream>
#include <memory>
#include <cmath>
#include <random>

class TreeConstructionInterface;

/*! Example using the Ternary L-System defined in Algorithmic botany. 
/ See chapter 2 figure 2.8 in http://algorithmicbotany.org/papers/abop/abop.pdf 
/ Extended to include probabilistic branching. */
namespace StochasticBranching {

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
    double m_increaseRate;

  public:
    Exclame(TreeConstructionInterface* constructor, double width, double increaseRate);
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

  /*! Attempt to rotate towards the vertical. */
  class Verticate : public TreeSystemInterface {
  private:
    double m_angle;

  public:
    Verticate(TreeConstructionInterface* constructor, double angle);
    std::vector<std::shared_ptr<TreeSystemInterface>> applyRule();
    void processTurtles(std::vector<Turtle*>& turtleStack,
			std::vector<Turtle*>& retiredTurtles);
    void print(std::ostream& os) const;
  };


  /*! Controls the primary growth (Doesn't draw anything!) */
  class A : public TreeSystemInterface {
  private:
    double m_branchProbabilityThreshold;
    int    m_iterationCount;

  public:
    A(TreeConstructionInterface* constructor, double branchProbabilityThreshold, int iterationCount);
    std::vector<std::shared_ptr<TreeSystemInterface>> applyRule();
    void processTurtles(std::vector<Turtle*>& turtleStack,
			std::vector<Turtle*>& retiredTurtles);
    void print(std::ostream& os) const;

    static std::default_random_engine randomEngine; // Probably should make this private
  };

  /*! Controls end of branch growth (Doesn't draw anything!) */
  class B : public TreeSystemInterface {
  private:
    int m_iterationCount;

  public:
    B(TreeConstructionInterface* constructor, int iterationCount);
    std::vector<std::shared_ptr<TreeSystemInterface>> applyRule();
    void processTurtles(std::vector<Turtle*>& turtleStack,
			std::vector<Turtle*>& retiredTurtles);
    void print(std::ostream& os) const;
  };


  /*! Prepare the random number generator at the appropriate time */
  class Rand : public TreeSystemInterface {
  private:

  public:
    explicit Rand(TreeConstructionInterface* constructor);
    std::vector<std::shared_ptr<TreeSystemInterface>> applyRule();
    void processTurtles(std::vector<Turtle*>& turtleStack,
			std::vector<Turtle*>& retiredTurtles);
    void print(std::ostream& os) const;
  };

}

#endif //LSYSTEM_STOCHASTIC




