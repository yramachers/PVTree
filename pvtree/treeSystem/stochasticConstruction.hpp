#ifndef TREE_SYSTEMS_STOCHASTIC_CONSTRUCTION_HPP
#define TREE_SYSTEMS_STOCHASTIC_CONSTRUCTION_HPP

#include "pvtree/treeSystem/treeConstructionInterface.hpp"
#include "pvtree/treeSystem/stochastic.hpp"
#include "pvtree/treeSystem/treeFactory.hpp"

/*! \brief Class to handle construction of Stochastic tree type.
 * 
 * Initializes the default parameters for the Stochastic tree type
 * and provides the initial conditions for the L-System.
 *
 * The base class provides the functionality to handle the parameters.
 */
class StochasticConstruction : public TreeConstructionInterface  {
private:

public:
  StochasticConstruction();
  ~StochasticConstruction();
  void print(std::ostream& os = std::cout);
  std::vector<std::shared_ptr<TreeSystemInterface> > getInitialConditions();

  ClassDef(StochasticConstruction, 1);
};

#endif //TREE_SYSTEMS_STOCHASTIC_CONSTRUCTION_HPP
