#ifndef LEAF_SYSTEM_CORDATE_CONSTRUCTION_HPP
#define LEAF_SYSTEM_CORDATE_CONSTRUCTION_HPP

#include "leafSystem/leafConstructionInterface.hpp"
#include "leafSystem/cordate.hpp"
#include "leafSystem/leafFactory.hpp"

/*! \brief Class to handle construction of Cordate leaf type.
 * 
 * Initializes the default parameters for the Cordate leaf type
 * and provides the initial conditions for the L-System.
 *
 * The base class provides the functionality to handle the parameters.
 */
class CordateConstruction : public LeafConstructionInterface  {
private:

public:
  CordateConstruction();
  virtual ~CordateConstruction();
  void print(std::ostream& os = std::cout);
  std::vector<std::shared_ptr<LeafSystemInterface> > getInitialConditions();

  ClassDef(CordateConstruction, 2);
};

#endif //LEAF_SYSTEM_CORDATE_CONSTRUCTION_HPP
