#ifndef PVTREE_TREE_SYSTEM_STUMP_HPP
#define PVTREE_TREE_SYSTEM_STUMP_HPP

#include "pvtree/treeSystem/treeSystemInterface.hpp"
#include <vector>
#include <iostream>
#include <memory>
#include <cmath>

class TreeConstructionInterface;

namespace StumpBranching {

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

}

#endif //PVTREE_TREE_SYSTEM_STUMP_HPP




