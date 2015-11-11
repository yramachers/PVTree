#ifndef LEAF_SYSTEM_ROSE_CONSTRUCTION_HPP
#define LEAF_SYSTEM_ROSE_CONSTRUCTION_HPP

#include "leafSystem/leafConstructionInterface.hpp"
#include "leafSystem/rose.hpp"
#include "leafSystem/leafFactory.hpp"

/*! \brief Class to handle construction of Rose leaf type.
 * 
 * Initializes the default parameters for the Rose leaf type
 * and provides the initial conditions for the L-System.
 *
 * The base class provides the functionality to handle the parameters.
 */
class RoseConstruction : public LeafConstructionInterface  {
private:

public:
  RoseConstruction();
  virtual ~RoseConstruction();
  void print(std::ostream& os = std::cout);
  std::vector<std::shared_ptr<LeafSystemInterface> > getInitialConditions();

  ClassDef(RoseConstruction, 2);
};

#endif //LEAF_SYSTEM_ROSE_CONSTRUCTION_HPP
