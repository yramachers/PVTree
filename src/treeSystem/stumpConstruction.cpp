#include "treeSystem/stumpConstruction.hpp"
#include <iostream>

ClassImp(StumpConstruction)

//! Register the Stump tree with the tree factory
static TreeFactoryRegistrar<StumpConstruction> registrar("stump");

// Reduce the length of intial condition setup
using namespace StumpBranching;
typedef std::shared_ptr<TreeSystemInterface> TSI;

/*! \brief Construct a Stump L-System constructor. Here the default parameters and their 
 *         ranges are set.
 */
StumpConstruction::StumpConstruction() : TreeConstructionInterface() {

  // Set default parameters
  applyConfigurationFile("trees/defaults-stump.cfg");

}

/*! \brief Destructor.
 *         
 */
StumpConstruction::~StumpConstruction() {}

/*! \brief Print out the details about this constructor. This should show all the values 
 *         and also initial conditions.
 */
void StumpConstruction::print(std::ostream& os/*= std::cout*/) {

  //Show base class information
  TreeConstructionInterface::print(os);

  std::vector<std::shared_ptr<TreeSystemInterface> > conditions = getInitialConditions();

  os << "Produced Stump Rules = ";
  for (unsigned int x=0; x<conditions.size(); x++) {
    conditions[x]->print(os);
  }
  os << std::endl;

}

/*! \brief Provide the initial conditions for the Stump L-System.
 *
 */
std::vector<std::shared_ptr<TreeSystemInterface> > StumpConstruction::getInitialConditions() {

  std::vector<TSI> treeConditions;

  treeConditions.push_back( TSI(new Exclame(this, getDoubleParameter("initialWidth") )) );
  treeConditions.push_back( TSI(new F(this, getDoubleParameter("initialHeight") )) );
  treeConditions.push_back( TSI(new Slash(this, getDoubleParameter("initialAzimuth") )) );
  treeConditions.push_back( TSI(new Ampersand(this, 90.0-getDoubleParameter("initialElevation") )) );
  treeConditions.push_back( TSI(new F(this, getDoubleParameter("initialHeight") )) );

  return treeConditions;
}
