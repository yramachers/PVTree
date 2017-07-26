#include "pvtree/treeSystem/ternaryConstruction.hpp"
#include <iostream>

ClassImp(TernaryConstruction)

//! Register the Ternary tree with the tree factory
static TreeFactoryRegistrar<TernaryConstruction> registrar("ternary");


/*! \brief Construct a Ternary L-System constructor. Here the default parameters and their 
 *         ranges are set.
 */
TernaryConstruction::TernaryConstruction() : TreeConstructionInterface() {

  // Set default parameters
  applyConfigurationFile("trees/defaults-ternary.cfg");

}

/*! \brief Destructor.
 *         
 */
TernaryConstruction::~TernaryConstruction() {}

/*! \brief Print out the details about this constructor. This should show all the values 
 *         and also initial conditions.
 */
void TernaryConstruction::print(std::ostream& os/*= std::cout*/) {

  //Show base class information
  TreeConstructionInterface::print(os);

  std::vector<std::shared_ptr<TreeSystemInterface> > conditions = getInitialConditions();

  os << "Produced Ternary Rules = ";
  for (unsigned int x=0; x<conditions.size(); x++) {
    conditions[x]->print(os);
  }
  os << std::endl;

}

/*! \brief Provide the initial conditions for the Ternary L-System.
 *
 */
std::vector<std::shared_ptr<TreeSystemInterface> > TernaryConstruction::getInitialConditions() {

  std::vector<std::shared_ptr<TreeSystemInterface> > treeConditions;

  treeConditions.push_back( std::shared_ptr<TreeSystemInterface>(new TernaryBranching::Exclame(this, getDoubleParameter("initialWidth") )) );
  treeConditions.push_back( std::shared_ptr<TreeSystemInterface>(new TernaryBranching::F(this, getDoubleParameter("initialHeight") )) );
  treeConditions.push_back( std::shared_ptr<TreeSystemInterface>(new TernaryBranching::Slash(this, getDoubleParameter("initialOrientation") )) );
  treeConditions.push_back( std::shared_ptr<TreeSystemInterface>(new TernaryBranching::A(this)) );

  return treeConditions;
}
