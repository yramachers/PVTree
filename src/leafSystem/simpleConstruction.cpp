#include "leafSystem/simpleConstruction.hpp"
#include <iostream>

ClassImp(SimpleConstruction)

//! Register the Simple leaf with the leaf factory
static LeafFactoryRegistrar<SimpleConstruction> registrar("simple");


/*! \brief Construct a Simple L-System constructor. Here the default parameters and their 
 *         ranges are set.
 */
SimpleConstruction::SimpleConstruction() : LeafConstructionInterface() {

  // Set default parameters
  applyConfigurationFile("leaves/defaults-simple.cfg");

}

/*! \brief Destructor.
 *         
 */
SimpleConstruction::~SimpleConstruction() {}

/*! \brief Print out the details about this constructor. This should show all the values 
 *         and also initial conditions.
 */
void SimpleConstruction::print(std::ostream& os/*= std::cout*/) {

  //Show base class information
  LeafConstructionInterface::print(os);

  std::vector<std::shared_ptr<LeafSystemInterface> > conditions = getInitialConditions();

  os << "Produced Simple Rules = ";
  for (unsigned int x=0; x<conditions.size(); x++) {
    conditions[x]->print(os);
  }
  os << std::endl;

}

/*! \brief Provide the initial conditions for the Simple L-System.
 *
 */
std::vector<std::shared_ptr<LeafSystemInterface> > SimpleConstruction::getInitialConditions() {

  std::vector<std::shared_ptr<LeafSystemInterface> > leafConditions;

  leafConditions.push_back( std::shared_ptr<LeafSystemInterface>(new Simple::Slash(this, getDoubleParameter("initialAngle") )) );
  leafConditions.push_back( std::shared_ptr<LeafSystemInterface>(new Simple::LeftBracket(this)) );
  leafConditions.push_back( std::shared_ptr<LeafSystemInterface>(new Simple::A(this, 0)) );
  leafConditions.push_back( std::shared_ptr<LeafSystemInterface>(new Simple::RightBracket(this)) );

  return leafConditions;
}
