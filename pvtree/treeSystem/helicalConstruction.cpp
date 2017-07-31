#include "pvtree/treeSystem/helicalConstruction.hpp"
#include <iostream>

ClassImp(HelicalConstruction)

    //! Register the Helical tree with the tree factory
    static TreeFactoryRegistrar<HelicalConstruction> registrar("helical");

/*! \brief Construct a Helical L-System constructor. Here the default parameters
 * and their
 *         ranges are set.
 */
HelicalConstruction::HelicalConstruction() : TreeConstructionInterface() {
  // Set default parameters
  applyConfigurationFile("trees/defaults-helical.cfg");
}

/*! \brief Destructor.
 *
 */
HelicalConstruction::~HelicalConstruction() {}

/*! \brief Print out the details about this constructor. This should show all
 * the values
 *         and also initial conditions.
 */
void HelicalConstruction::print(std::ostream& os /*= std::cout*/) {
  // Show base class information
  TreeConstructionInterface::print(os);

  std::vector<std::shared_ptr<TreeSystemInterface> > conditions =
      getInitialConditions();

  os << "Produced Helical Rules = ";
  for (unsigned int x = 0; x < conditions.size(); x++) {
    conditions[x]->print(os);
  }
  os << std::endl;
}

/*! \brief Provide the initial conditions for the Helical L-System.
 *
 */
std::vector<std::shared_ptr<TreeSystemInterface> >
HelicalConstruction::getInitialConditions() {
  std::vector<std::shared_ptr<TreeSystemInterface> > treeConditions;

  treeConditions.push_back(
      std::shared_ptr<TreeSystemInterface>(new HelicalBranching::Slash(
          this, getDoubleParameter("initialOrientation"))));

  double angleBetweenStalks = 360.0 / getIntegerParameter("stalkPoints");

  for (int stalkPointIndex = 0;
       stalkPointIndex < getIntegerParameter("stalkPoints");
       stalkPointIndex++) {
    treeConditions.push_back(std::shared_ptr<TreeSystemInterface>(
        new HelicalBranching::LeftBracket(this)));
    treeConditions.push_back(
        std::shared_ptr<TreeSystemInterface>(new HelicalBranching::Slash(
            this, stalkPointIndex * angleBetweenStalks)));
    treeConditions.push_back(std::shared_ptr<TreeSystemInterface>(
        new HelicalBranching::Ampersand(this, 90.0)));
    treeConditions.push_back(std::shared_ptr<TreeSystemInterface>(
        new HelicalBranching::f(this, getDoubleParameter("initialRadius"))));
    treeConditions.push_back(std::shared_ptr<TreeSystemInterface>(
        new HelicalBranching::Plus(this, 90.0)));
    treeConditions.push_back(
        std::shared_ptr<TreeSystemInterface>(new HelicalBranching::Ampersand(
            this, -getDoubleParameter("inclinationAngle"))));

    if (stalkPointIndex % 2 == 0) {
      treeConditions.push_back(
          std::shared_ptr<TreeSystemInterface>(new HelicalBranching::A(
              this, getDoubleParameter("initialLength"),
              getDoubleParameter("initialWidthEven"), 0.0, 0)));
    } else {
      treeConditions.push_back(
          std::shared_ptr<TreeSystemInterface>(new HelicalBranching::A(
              this, getDoubleParameter("initialLength"),
              getDoubleParameter("initialWidthOdd"), 0.0, 0)));
    }

    treeConditions.push_back(std::shared_ptr<TreeSystemInterface>(
        new HelicalBranching::RightBracket(this)));
  }

  return treeConditions;
}
