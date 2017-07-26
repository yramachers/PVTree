#ifndef TREE_SYSTEMS_HELICAL_CONSTRUCTION_HPP
#define TREE_SYSTEMS_HELICAL_CONSTRUCTION_HPP

#include "pvtree/treeSystem/treeConstructionInterface.hpp"
#include "pvtree/treeSystem/helical.hpp"
#include "pvtree/treeSystem/treeFactory.hpp"

/*! \brief Class to handle construction of Helical tree type.
 * 
 * Initializes the default parameters for the Helical tree type
 * and provides the initial conditions for the L-System.
 *
 * The base class provides the functionality to handle the parameters.
 */
class HelicalConstruction : public TreeConstructionInterface  {
private:

public:
  HelicalConstruction();
  ~HelicalConstruction();
  void print(std::ostream& os = std::cout);
  std::vector<std::shared_ptr<TreeSystemInterface> > getInitialConditions();

  ClassDef(HelicalConstruction, 1);
};

#endif //TREE_SYSTEMS_HELICAL_CONSTRUCTION_HPP
