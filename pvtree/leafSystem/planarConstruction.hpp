#ifndef PVTREE_LEAF_SYSTEM_PLANAR_CONSTRUCTION_HPP
#define PVTREE_LEAF_SYSTEM_PLANAR_CONSTRUCTION_HPP

#include "pvtree/leafSystem/leafConstructionInterface.hpp"
#include "pvtree/leafSystem/planar.hpp"
#include "pvtree/leafSystem/leafFactory.hpp"

/*! \brief Class to handle construction of Planar leaf type.
 * 
 * Initializes the default parameters for the Planar leaf type
 * and provides the initial conditions for the L-System.
 *
 * The base class provides the functionality to handle the parameters.
 */
class PlanarConstruction : public LeafConstructionInterface  {
private:

public:
  PlanarConstruction();
  virtual ~PlanarConstruction();
  void print(std::ostream& os = std::cout);
  std::vector<std::shared_ptr<LeafSystemInterface> > getInitialConditions();
  
  ClassDef(PlanarConstruction, 2);
};

#endif //PVTREE_LEAF_SYSTEM_PLANAR_CONSTRUCTION_HPP
