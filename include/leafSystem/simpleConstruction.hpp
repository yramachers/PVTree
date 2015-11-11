#ifndef LEAF_SYSTEM_SIMPLE_CONSTRUCTION_HPP
#define LEAF_SYSTEM_SIMPLE_CONSTRUCTION_HPP

#include "leafSystem/leafConstructionInterface.hpp"
#include "leafSystem/simple.hpp"
#include "leafSystem/leafFactory.hpp"

/*! \brief Class to handle construction of Simple leaf type.
 * 
 * Initializes the default parameters for the Simple leaf type
 * and provides the initial conditions for the L-System.
 *
 * The base class provides the functionality to handle the parameters.
 */
class SimpleConstruction : public LeafConstructionInterface  {
private:

public:
  SimpleConstruction();
  virtual ~SimpleConstruction();
  void print(std::ostream& os = std::cout);
  std::vector<std::shared_ptr<LeafSystemInterface> > getInitialConditions();
  
  ClassDef(SimpleConstruction, 2);
};

#endif //LEAF_SYSTEM_SIMPLE_CONSTRUCTION_HPP
