#ifndef TREE_SYSTEMS_TERNARY_CONSTRUCTION_HPP
#define TREE_SYSTEMS_TERNARY_CONSTRUCTION_HPP

#include "pvtree/treeSystem/treeConstructionInterface.hpp"
#include "pvtree/treeSystem/ternary.hpp"
#include "pvtree/treeSystem/treeFactory.hpp"

/*! \brief Class to handle construction of Ternary tree type.
 *
 * Initializes the default parameters for the Ternary tree type
 * and provides the initial conditions for the L-System.
 *
 * The base class provides the functionality to handle the parameters.
 */
class TernaryConstruction : public TreeConstructionInterface {
 private:
 public:
  TernaryConstruction();
  virtual ~TernaryConstruction();
  void print(std::ostream& os = std::cout);
  std::vector<std::shared_ptr<TreeSystemInterface> > getInitialConditions();

  ClassDef(TernaryConstruction, 1);
};

#endif  // TREE_SYSTEMS_TERNARY_CONSTRUCTION_HPP
