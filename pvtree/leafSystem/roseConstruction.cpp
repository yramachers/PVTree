#include "pvtree/leafSystem/roseConstruction.hpp"
#include <iostream>

ClassImp(RoseConstruction)

//! Register the Rose leaf with the leaf factory
static LeafFactoryRegistrar<RoseConstruction> registrar("rose");


/*! \brief Construct a Rose L-System constructor. Here the default parameters and their 
 *         ranges are set.
 */
RoseConstruction::RoseConstruction() : LeafConstructionInterface() {

  // Set default parameters
  applyConfigurationFile("leaves/defaults-rose.cfg");

}

/*! \brief Destructor.
 *         
 */
RoseConstruction::~RoseConstruction() {}

/*! \brief Print out the details about this constructor. This should show all the values 
 *         and also initial conditions.
 */
void RoseConstruction::print(std::ostream& os/*= std::cout*/) {

  //Show base class information
  LeafConstructionInterface::print(os);

  std::vector<std::shared_ptr<LeafSystemInterface> > conditions = getInitialConditions();

  os << "Produced Rose Rules = ";
  for (unsigned int x=0; x<conditions.size(); x++) {
    conditions[x]->print(os);
  }
  os << std::endl;

}

/*! \brief Provide the initial conditions for the Rose L-System.
 *
 */
std::vector<std::shared_ptr<LeafSystemInterface> > RoseConstruction::getInitialConditions() {

  std::vector<std::shared_ptr<LeafSystemInterface> > leafConditions;

  leafConditions.push_back( std::shared_ptr<LeafSystemInterface>(new Rose::Slash(this, getDoubleParameter("initialAngle") )) );
  leafConditions.push_back( std::shared_ptr<LeafSystemInterface>(new Rose::LeftBracket(this)) );
  leafConditions.push_back( std::shared_ptr<LeafSystemInterface>(new Rose::A(this, 0)) );
  leafConditions.push_back( std::shared_ptr<LeafSystemInterface>(new Rose::RightBracket(this)) );

  return leafConditions;
}
