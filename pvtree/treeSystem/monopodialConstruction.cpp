#include "pvtree/treeSystem/monopodialConstruction.hpp"
#include <iostream>

ClassImp(MonopodialConstruction)

    //! Register the Monopodial tree with the tree factory
    static TreeFactoryRegistrar<MonopodialConstruction> registrar("monopodial");

/*! \brief Construct a Monopodial L-System constructor. Here the default
 * parameters and their
 *         ranges are set.
 */
MonopodialConstruction::MonopodialConstruction() : TreeConstructionInterface() {
  // Set default parameters
  applyConfigurationFile("trees/defaults-monopodial.cfg");
}

/*! \brief Destructor.
 *
 */
MonopodialConstruction::~MonopodialConstruction() {}

/*! \brief Print out the details about this constructor. This should show all
 * the values
 *         and also initial conditions.
 */
void MonopodialConstruction::print(std::ostream& os /*= std::cout*/) {
  // Show base class information
  TreeConstructionInterface::print(os);

  std::vector<std::shared_ptr<TreeSystemInterface> > conditions =
      getInitialConditions();

  os << "Produced Monopodial Rules = ";
  for (unsigned int x = 0; x < conditions.size(); x++) {
    conditions[x]->print(os);
  }
  os << std::endl;
}

/*! \brief Provide the initial conditions for the Monopodial L-System.
 *
 */
std::vector<std::shared_ptr<TreeSystemInterface> >
MonopodialConstruction::getInitialConditions() {
  std::vector<std::shared_ptr<TreeSystemInterface> > treeConditions;

  treeConditions.push_back(
      std::shared_ptr<TreeSystemInterface>(new MonopodialBranching::Slash(
          this, getDoubleParameter("initialOrientation"))));
  treeConditions.push_back(std::shared_ptr<TreeSystemInterface>(
      new MonopodialBranching::A(this, getDoubleParameter("initialHeight"),
                                 getDoubleParameter("initialWidth"))));

  return treeConditions;
}
