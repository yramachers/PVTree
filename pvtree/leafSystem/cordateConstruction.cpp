#include "pvtree/leafSystem/cordateConstruction.hpp"
#include <iostream>

ClassImp(CordateConstruction)

    //! Register the Cordate leaf with the leaf factory
    static LeafFactoryRegistrar<CordateConstruction> registrar("cordate");

/*! \brief Construct a Cordate L-System constructor. Here the default parameters
 * and their
 *         ranges are set.
 */
CordateConstruction::CordateConstruction() : LeafConstructionInterface() {
  // Set default parameters
  applyConfigurationFile("leaves/defaults-cordate.cfg");
}

/*! \brief Destructor.
 *
 */
CordateConstruction::~CordateConstruction() {}

/*! \brief Print out the details about this constructor. This should show all
 * the values
 *         and also initial conditions.
 */
void CordateConstruction::print(std::ostream& os /*= std::cout*/) {
  // Show base class information
  LeafConstructionInterface::print(os);

  std::vector<std::shared_ptr<LeafSystemInterface> > conditions =
      getInitialConditions();

  os << "Produced Cordate Rules = ";
  for (unsigned int x = 0; x < conditions.size(); x++) {
    conditions[x]->print(os);
  }
  os << std::endl;
}

/*! \brief Provide the initial conditions for the Cordate L-System.
 *
 */
std::vector<std::shared_ptr<LeafSystemInterface> >
CordateConstruction::getInitialConditions() {
  std::vector<std::shared_ptr<LeafSystemInterface> > leafConditions;

  leafConditions.push_back(std::shared_ptr<LeafSystemInterface>(
      new Cordate::Slash(this, getDoubleParameter("initialAngle"))));
  leafConditions.push_back(std::shared_ptr<LeafSystemInterface>(
      new Cordate::G(this, getDoubleParameter("stemLength"))));
  leafConditions.push_back(
      std::shared_ptr<LeafSystemInterface>(new Cordate::LeftBracket(this)));
  leafConditions.push_back(
      std::shared_ptr<LeafSystemInterface>(new Cordate::A(this, 1.0)));
  leafConditions.push_back(
      std::shared_ptr<LeafSystemInterface>(new Cordate::RightBracket(this)));
  leafConditions.push_back(
      std::shared_ptr<LeafSystemInterface>(new Cordate::LeftBracket(this)));
  leafConditions.push_back(
      std::shared_ptr<LeafSystemInterface>(new Cordate::B(this, 1.0)));
  leafConditions.push_back(
      std::shared_ptr<LeafSystemInterface>(new Cordate::RightBracket(this)));

  return leafConditions;
}
