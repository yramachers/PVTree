#ifndef PVTREE_TREE_SYSTEM_STUMP_CONSTRUCTION_HPP
#define PVTREE_TREE_SYSTEM_STUMP_CONSTRUCTION_HPP

#include "pvtree/treeSystem/treeConstructionInterface.hpp"
#include "pvtree/treeSystem/stump.hpp"
#include "pvtree/treeSystem/treeFactory.hpp"

/*! \brief Class to handle construction of Ternary tree type.
 * 
 * Initializes the default parameters for the Ternary tree type
 * and provides the initial conditions for the L-System.
 *
 * The base class provides the functionality to handle the parameters.
 */
class StumpConstruction : public TreeConstructionInterface  {
private:

public:
  StumpConstruction();
  virtual ~StumpConstruction();
  void print(std::ostream& os = std::cout);
  std::vector<std::shared_ptr<TreeSystemInterface> > getInitialConditions();

  ClassDef(StumpConstruction, 1);
};

#endif //PVTREE_TREE_SYSTEM_STUMP_CONSTRUCTION_HPP
