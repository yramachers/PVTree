#ifndef TREE_SYSTEMS_TREE_CONSTRUCTION_INTERFACE_HPP
#define TREE_SYSTEMS_TREE_CONSTRUCTION_INTERFACE_HPP

#include "pvtree/treeSystem/treeSystemInterface.hpp"
#include <vector>
#include <string>
#include <map>
#include <utility>
#include <iostream>
#include <libconfig.h++>

#include "TObject.h"

/*! \brief Common interface for initializing tree systems.
 * 
 * Common interface allows calls to control the random parameter
 * selection and initial conditions selection. It should also be
 * possible to set sepecific parameters to chosen values.
 *
 * In future it would be also good to control the function to be
 * used to create the intial conditions vector.
 *
 * Inherits from TObject to allow the class to be persisted to
 * ROOT files. Needs also an entry in Linkdef.h and a root
 * dictionary to be generated at build time.
 */
class TreeConstructionInterface : public TObject {
private:
  //Store parameter names to ensure same random number sequence
  std::vector<std::string> m_doubleParameterNames;//!< Names of double parameters
  std::vector<std::string> m_integerParameterNames;//!< Names of integer parameters

  //Store parameters
  std::map< std::string, double > m_doubleParameters;//!< Double parameter values
  std::map< std::string, int >    m_integerParameters;//!< Integer parameter values

  //! Minimimum and maximum values allowed for parameter
  std::map< std::string, std::pair<double, double> > m_doubleParameterRanges;
  //! Minimimum and maximum values allowed for parameter
  std::map< std::string, std::pair<int,    int> >    m_integerParameterRanges;

protected:
  // Configuration file handling
  bool openConfigurationFile(std::string fileName, libconfig::Config& cfg);
  void applyConfigurationFile(std::string configurationFileName);

public:
  virtual ~TreeConstructionInterface() {};
  virtual void print(std::ostream& os = std::cout);
  virtual std::vector<std::shared_ptr<TreeSystemInterface> > getInitialConditions() = 0;

  // Equality
  bool operator==(const TreeConstructionInterface& right);
  bool operator!=(const TreeConstructionInterface& right);

  // Common functionality
  virtual void                      randomizeParameters(int seed);
  virtual void                      randomizeParameter(int seed, std::string name);
  virtual void                      setRandomParameterRange(std::string name, double minValue, double maxValue);
  virtual void                      setRandomParameterRange(std::string name, int    minValue, int    maxValue);
  virtual void                      setParameter(std::string name, double value);
  virtual void                      setParameter(std::string name, int    value);

  virtual double                    getDoubleParameter(std::string name) const;
  virtual int                       getIntegerParameter(std::string name) const;
  virtual std::pair<double, double> getDoubleRange(std::string name) const;
  virtual std::pair<int, int>       getIntegerRange(std::string name) const;
  virtual std::vector<std::string>  getDoubleParameterNames() const;
  virtual std::vector<std::string>  getIntegerParameterNames() const;

  ClassDef(TreeConstructionInterface, 1);
};



#endif //TREE_SYSTEMS_TREE_CONSTRUCTION_INTERFACE_HPP
