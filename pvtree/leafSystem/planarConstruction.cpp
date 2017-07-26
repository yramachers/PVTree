#include "pvtree/leafSystem/planarConstruction.hpp"
#include <iostream>

ClassImp(PlanarConstruction)

//! Register the Planar leaf with the leaf factory
static LeafFactoryRegistrar<PlanarConstruction> registrar("planar");

// Reduce the length of intial condition setup
using namespace Planar;
typedef std::shared_ptr<LeafSystemInterface> LSI;

/*! \brief Construct a Planar L-System constructor. Here the default parameters and their 
 *         ranges are set.
 */
PlanarConstruction::PlanarConstruction() : LeafConstructionInterface() {

  // Set default parameters
  applyConfigurationFile("leaves/defaults-planar.cfg");

}

/*! \brief Destructor.
 *         
 */
PlanarConstruction::~PlanarConstruction() {}

/*! \brief Print out the details about this constructor. This should show all the values 
 *         and also initial conditions.
 */
void PlanarConstruction::print(std::ostream& os/*= std::cout*/) {

  //Show base class information
  LeafConstructionInterface::print(os);

  std::vector<std::shared_ptr<LeafSystemInterface> > conditions = getInitialConditions();

  os << "Produced Planar Rules = ";
  for (unsigned int x=0; x<conditions.size(); x++) {
    conditions[x]->print(os);
  }
  os << std::endl;

}

/*! \brief Provide the initial conditions for the Planar L-System.
 *
 * I essentially just create a quad in a rather awkward way.
 */
std::vector<std::shared_ptr<LeafSystemInterface> > PlanarConstruction::getInitialConditions() {

  std::vector<LSI> leafConditions;

  leafConditions.push_back( LSI(new G(this, getDoubleParameter("offsetLength"), 1.0)) );
  leafConditions.push_back( LSI(new Slash(this, getDoubleParameter("initialAngle") )) );

  //Triangle 1
  leafConditions.push_back( LSI(new LeftBracket(this)) );
  leafConditions.push_back( LSI(new CurlyLeft(this)) );
  leafConditions.push_back( LSI(new Ampersand(this, 90.0)) );
  leafConditions.push_back( LSI(new G(this, 
				      getDoubleParameter("initialEdgeLength")/2.0, 
				      getDoubleParameter("mainGrowthRate") )) );
  leafConditions.push_back( LSI(new Slash(this, 90.0 )) );
  leafConditions.push_back( LSI(new Ampersand(this, 90.0)) );
  leafConditions.push_back( LSI(new G(this, 
				      getDoubleParameter("initialEdgeLength")/2.0, 
				      getDoubleParameter("mainGrowthRate") )) );
  leafConditions.push_back( LSI(new Dot(this)) );
  leafConditions.push_back( LSI(new Ampersand(this, 90.0)) );
  leafConditions.push_back( LSI(new G(this, 
				      getDoubleParameter("initialEdgeLength"), 
				      getDoubleParameter("mainGrowthRate") )) );
  leafConditions.push_back( LSI(new Dot(this)) );
  leafConditions.push_back( LSI(new Ampersand(this, 90.0)) );
  leafConditions.push_back( LSI(new G(this, 
				      getDoubleParameter("initialEdgeLength"), 
				      getDoubleParameter("mainGrowthRate") )) );
  leafConditions.push_back( LSI(new Dot(this)) );
  leafConditions.push_back( LSI(new CurlyRight(this)) );
  leafConditions.push_back( LSI(new RightBracket(this)) );

  //Triangle 2
  leafConditions.push_back( LSI(new LeftBracket(this)) );
  leafConditions.push_back( LSI(new Slash(this, 180.0 )) );
  leafConditions.push_back( LSI(new CurlyLeft(this)) );
  leafConditions.push_back( LSI(new Ampersand(this, 90.0)) );
  leafConditions.push_back( LSI(new G(this, 
				      getDoubleParameter("initialEdgeLength")/2.0, 
				      getDoubleParameter("mainGrowthRate") )) );
  leafConditions.push_back( LSI(new Slash(this, 90.0 )) );
  leafConditions.push_back( LSI(new Ampersand(this, 90.0)) );
  leafConditions.push_back( LSI(new G(this, 
				      getDoubleParameter("initialEdgeLength")/2.0, 
				      getDoubleParameter("mainGrowthRate") )) );
  leafConditions.push_back( LSI(new Dot(this)) );
  leafConditions.push_back( LSI(new Ampersand(this, 90.0)) );
  leafConditions.push_back( LSI(new G(this, 
				      getDoubleParameter("initialEdgeLength"), 
				      getDoubleParameter("mainGrowthRate") )) );
  leafConditions.push_back( LSI(new Dot(this)) );
  leafConditions.push_back( LSI(new Ampersand(this, 90.0)) );
  leafConditions.push_back( LSI(new G(this, 
				      getDoubleParameter("initialEdgeLength"), 
				      getDoubleParameter("mainGrowthRate") )) );
  leafConditions.push_back( LSI(new Dot(this)) );
  leafConditions.push_back( LSI(new CurlyRight(this)) );
  leafConditions.push_back( LSI(new RightBracket(this)) );


  return leafConditions;
}
