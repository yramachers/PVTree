#ifndef TREE_SYSTEMS_MONOPODIAL_CONSTRUCTION_HPP
#define TREE_SYSTEMS_MONOPODIAL_CONSTRUCTION_HPP

#include "pvtree/treeSystem/treeConstructionInterface.hpp"
#include "pvtree/treeSystem/monopodial.hpp"
#include "pvtree/treeSystem/treeFactory.hpp"

/*! \brief Class to handle construction of Monopodial tree type.
 *
 * Initializes the default parameters for the Monopodial tree type
 * and provides the initial conditions for the L-System.
 *
 * The base class provides the functionality to handle the parameters.
 */
class MonopodialConstruction : public TreeConstructionInterface {
 private:
 public:
  MonopodialConstruction();
  ~MonopodialConstruction();
  void print(std::ostream& os = std::cout);
  std::vector<std::shared_ptr<TreeSystemInterface> > getInitialConditions();

  ClassDef(MonopodialConstruction, 1);
};

#endif  // TREE_SYSTEMS_MONOPODIAL_CONSTRUCTION_HPP
