#include "treeSystem/stochasticConstruction.hpp"
#include <iostream>

ClassImp(StochasticConstruction)

//! Register the Stochastic tree with the tree factory
static TreeFactoryRegistrar<StochasticConstruction> registrar("stochastic");


/*! \brief Construct a Stochastic L-System constructor. Here the default parameters and their 
 *         ranges are set.
 */
StochasticConstruction::StochasticConstruction() : TreeConstructionInterface() {

  // Set default parameters
  applyConfigurationFile("trees/defaults-stochastic.cfg");

}

/*! \brief Destructor.
 *         
 */
StochasticConstruction::~StochasticConstruction() {}

/*! \brief Print out the details about this constructor. This should show all the values 
 *         and also initial conditions.
 */
void StochasticConstruction::print(std::ostream& os/*= std::cout*/) {

  //Show base class information
  TreeConstructionInterface::print(os);

  std::vector<std::shared_ptr<TreeSystemInterface> > conditions = getInitialConditions();

  os << "Produced Stochastic Rules = ";
  for (unsigned int x=0; x<conditions.size(); x++) {
    conditions[x]->print(os);
  }
  os << std::endl;

}

/*! \brief Provide the initial conditions for the Stochastic L-System.
 *
 */
std::vector<std::shared_ptr<TreeSystemInterface> > StochasticConstruction::getInitialConditions() {

  std::vector<std::shared_ptr<TreeSystemInterface> > treeConditions;

  treeConditions.push_back( std::shared_ptr<TreeSystemInterface>(new StochasticBranching::Rand(this)) );
  treeConditions.push_back( std::shared_ptr<TreeSystemInterface>(new StochasticBranching::Exclame(this, getDoubleParameter("initialWidth"), getDoubleParameter("widthIncreaseRate") )) );
  treeConditions.push_back( std::shared_ptr<TreeSystemInterface>(new StochasticBranching::F(this, getDoubleParameter("initialHeight") )) );
  treeConditions.push_back( std::shared_ptr<TreeSystemInterface>(new StochasticBranching::Slash(this, getDoubleParameter("initialOrientation") )) );
  treeConditions.push_back( std::shared_ptr<TreeSystemInterface>(new StochasticBranching::A(this, getDoubleParameter("initialBranchProb"), 0)) );

  return treeConditions;
}
