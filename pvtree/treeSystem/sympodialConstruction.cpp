#include "pvtree/treeSystem/sympodialConstruction.hpp"
#include <iostream>

ClassImp(SympodialConstruction)

//! Register the tree
static TreeFactoryRegistrar<SympodialConstruction> registrar("sympodial");


/*! \brief Construct a Sympodial L-System constructor. Here the default parameters and their 
 *         ranges are set.
 */
SympodialConstruction::SympodialConstruction() : TreeConstructionInterface() {

  // Set default parameters
  applyConfigurationFile("trees/defaults-sympodial.cfg");

}

/*! \brief Destructor.
 *         
 */
SympodialConstruction::~SympodialConstruction() {}

/*! \brief Print out the details about this constructor. This should show all the values 
 *         and also initial conditions.
 */
void SympodialConstruction::print(std::ostream& os/*= std::cout*/) {

  //Show base class information
  TreeConstructionInterface::print(os);

  std::vector<std::shared_ptr<TreeSystemInterface> > conditions = getInitialConditions();

  os << "Produced Sympodial Rules = ";
  for (unsigned int x=0; x<conditions.size(); x++) {
    conditions[x]->print(os);
  }
  os << std::endl;

}

/*! \brief Provide the initial conditions for the Sympodial L-System.
 *
 */
std::vector<std::shared_ptr<TreeSystemInterface> > SympodialConstruction::getInitialConditions() {

  std::vector<std::shared_ptr<TreeSystemInterface> > treeConditions;

  treeConditions.push_back( std::shared_ptr<TreeSystemInterface>(new SympodialBranching::Slash(this, getDoubleParameter("initialOrientation") )) );
  treeConditions.push_back( std::shared_ptr<TreeSystemInterface>(new SympodialBranching::A(this, getDoubleParameter("initialHeight"), getDoubleParameter("initialWidth") )) );

  return treeConditions;
}
