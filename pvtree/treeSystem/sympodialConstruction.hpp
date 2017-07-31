#ifndef TREE_SYSTEMS_SYMPODIAL_CONSTRUCTION_HPP
#define TREE_SYSTEMS_SYMPODIAL_CONSTRUCTION_HPP

#include "pvtree/treeSystem/treeConstructionInterface.hpp"
#include "pvtree/treeSystem/sympodial.hpp"
#include "pvtree/treeSystem/treeFactory.hpp"

/*! \brief Class to handle construction of Sympodial tree type.
 *
 * Initializes the default parameters for the Sympodial tree type
 * and provides the initial conditions for the L-System.
 *
 * The base class provides the functionality to handle the parameters.
 */
class SympodialConstruction : public TreeConstructionInterface {
 private:
 public:
  SympodialConstruction();
  ~SympodialConstruction();
  void print(std::ostream& os = std::cout);
  std::vector<std::shared_ptr<TreeSystemInterface> > getInitialConditions();

  ClassDef(SympodialConstruction, 1);
};

#endif  // TREE_SYSTEMS_SYMPODIAL_CONSTRUCTION_HPP
