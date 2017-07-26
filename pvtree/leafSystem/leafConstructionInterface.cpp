#include "pvtree/leafSystem/leafConstructionInterface.hpp"
#include "pvtree/utils/equality.hpp"
#include <stdexcept>
#include <random>
#include <iostream>
#include <fstream>
#include <cstring>
#include <iomanip>

/*! \brief Print all the parameters stored in the constructor and also their ranges.
 *
 * Useful for making quick checks that parameters are reasonable.
 */
void LeafConstructionInterface::print(std::ostream& os/*= std::cout*/) {

  // Get the maximum parameter name length
  unsigned int minimumParameterColumnSize = 17u;
  unsigned int valueColumnSize = 10u;

  for (auto& parameterName : m_doubleParameterNames){
    if ( parameterName.size() > minimumParameterColumnSize ){
      minimumParameterColumnSize = parameterName.size();
    }
  }
  for (auto& parameterName : m_integerParameterNames){
    if ( parameterName.size() > minimumParameterColumnSize ){
      minimumParameterColumnSize = parameterName.size();
    }
  }

  // Print header
  unsigned int totalTableWidth = minimumParameterColumnSize + 3*valueColumnSize + 13u;
  os << " " << std::setfill('-') << std::setw(totalTableWidth) << "" << std::setfill(' ') << std::endl;

  os << " | " << std::setw(minimumParameterColumnSize) << "Double Parameter" << " : "
     << std::setw(valueColumnSize) << "Value"   << " | "
     << std::setw(valueColumnSize) << "Minimum" << " | "
     << std::setw(valueColumnSize) << "Maximum" << " |"
     << std::endl;

  os << " " << std::setfill('-') << std::setw(totalTableWidth) << "" << std::setfill(' ') << std::endl;

  // Now print
  for (auto& parameterName : m_doubleParameterNames) {

    os << " | " << std::setw(minimumParameterColumnSize) << parameterName  << " : "
       << std::setw(valueColumnSize) << getDoubleParameter(parameterName)
       << " | "
       << std::setw(valueColumnSize) << getDoubleRange(parameterName).first
       << " | "
       << std::setw(valueColumnSize) << getDoubleRange(parameterName).second
       << " |"
       << std::endl;
  }

  os << " " << std::setfill('-') << std::setw(totalTableWidth) << "" << std::setfill(' ') << std::endl;

  os << " | " << std::setw(minimumParameterColumnSize) << "Integer Parameter" << " : "
     << std::setw(valueColumnSize) << "Value"   << " | "
     << std::setw(valueColumnSize) << "Minimum" << " | "
     << std::setw(valueColumnSize) << "Maximum" << " |"
     << std::endl;

  os << " " << std::setfill('-') << std::setw(totalTableWidth) << "" << std::setfill(' ') << std::endl;
  
  for (auto& parameterName : m_integerParameterNames) {

    os << " | " << std::setw(minimumParameterColumnSize) << parameterName  << " : "
       << std::setw(valueColumnSize) << getIntegerParameter(parameterName)
       << " | "
       << std::setw(valueColumnSize) << getIntegerRange(parameterName).first
       << " | "
       << std::setw(valueColumnSize) << getIntegerRange(parameterName).second
       << " |"
       << std::endl;
  }

  os << " " << std::setfill('-') << std::setw(totalTableWidth) << "" << std::setfill(' ') << std::endl;
}

/*! \brief Check that two LeafConstructionInterfaces have identical
 *         properties
 */
bool LeafConstructionInterface::operator==(const LeafConstructionInterface& right) {

  // Check that the underlying types are identical (uses ROOT reflection).
  if ( strcmp(this->ClassName(), right.ClassName()) != 0 ) {
    return false;
  }

  // Check the same parameter names are present and
  // in the same order. Require the same order for equality
  // because random parameter generation depends upon it.
  if ( this->getDoubleParameterNames() != right.getDoubleParameterNames() ){
    return false;
  }

  if ( this->getIntegerParameterNames() != right.getIntegerParameterNames() ){
    return false;
  }

  // Check all the parameter values and ranges are identical
  // where the double comparison needs to work within the
  // types precision.
  int equalityPrecisionFactor = 10;
  for ( auto leftName : this->getDoubleParameterNames() ) {
    if ( !almost_equal(this->getDoubleParameter(leftName), right.getDoubleParameter(leftName), equalityPrecisionFactor) ){
      return false;
    }
    
    if ( !almost_equal(this->getDoubleRange(leftName).first, right.getDoubleRange(leftName).first, equalityPrecisionFactor) ){
      return false;
    }
    
    if ( !almost_equal(this->getDoubleRange(leftName).second, right.getDoubleRange(leftName).second, equalityPrecisionFactor) ){
      return false;
    }
  } 

  for ( auto leftName : this->getIntegerParameterNames() ) {
    if ( this->getIntegerParameter(leftName) != right.getIntegerParameter(leftName) ){
      return false;
    }

    if ( this->getIntegerRange(leftName).first != right.getIntegerRange(leftName).first ){
      return false;
    }

    if ( this->getIntegerRange(leftName).second != right.getIntegerRange(leftName).second ){
      return false;
    }
  } 

  // Made it to the end, therefore probably identical
  return true;
}


/*! \brief Check that two LeafConstructionInterfaces do not have
 *         identical properties
 */
bool LeafConstructionInterface::operator!=(const LeafConstructionInterface& right) {
  // Simply reverses equality
  return !( (*this) == right );
}


/*! \brief For all the parameters specified in the leaf randomally choose 
 * new values within the range specified for each parameter. Use
 * a random seed each time to ensure that parameter choice can be
 * replicated.
 *
 * @param[in] seed  The seed value used in the random number generator.
 */
void LeafConstructionInterface::randomizeParameters(int seed) {

  std::default_random_engine randomEngine(seed);

  //Iterate over all double parameters
  for (unsigned int s=0u; s<m_doubleParameterNames.size(); s++) {

    std::uniform_real_distribution<double> distribution(m_doubleParameterRanges[m_doubleParameterNames[s]].first, 
							m_doubleParameterRanges[m_doubleParameterNames[s]].second);

    m_doubleParameters[m_doubleParameterNames[s]] = distribution(randomEngine);
  }

  //Iterate over all integer parameters
  for (unsigned int s=0u; s<m_integerParameterNames.size(); s++) {

    std::uniform_int_distribution<int> distribution(m_integerParameterRanges[m_integerParameterNames[s]].first, 
						    m_integerParameterRanges[m_integerParameterNames[s]].second);

    m_integerParameters[m_integerParameterNames[s]] = distribution(randomEngine);
  }

}

/*! \brief For a specific parameter choose a new random value in
 * the allowed range. Use a random seed each time to ensure that parameter 
 * choice can be replicated.
 *
 * @param[in] seed  The seed value used in the random number generator.
 * @param[in] name  The name of the parameter to randomly generate.
 * \throws std::invalid_argument   If a parameter with the given name and of any type doesn't exist.
 */
void LeafConstructionInterface::randomizeParameter(int seed, std::string name) {

  bool parameterExists = false;

  std::default_random_engine randomEngine(seed);

  //Check if there is a double parameter with this name
  if ( m_doubleParameterRanges.find(name) != m_doubleParameterRanges.end() ) {
    std::uniform_real_distribution<double> distribution(m_doubleParameterRanges[name].first, 
							m_doubleParameterRanges[name].second);
    m_doubleParameters[name] = distribution(randomEngine);
    
    parameterExists = true;
  }

  //Check if there is an integer parameter with this name
  if ( m_integerParameterRanges.find(name) != m_integerParameterRanges.end() ) {
    std::uniform_int_distribution<int> distribution(m_integerParameterRanges[name].first, 
						    m_integerParameterRanges[name].second);
    m_integerParameters[name] = distribution(randomEngine);

    parameterExists = true;
  }

  if (!parameterExists) {
    //Failed to find any parameter to randomize, throw an exception because this is not a good sign
    throw std::invalid_argument("Parameter \"" + name + "\" does not exist");
  }

}

/*! \brief Set the allowed range of a double parameter. If the current value lies
 * above the new maximum then it is set to the maximum and similarly if the current
 * value is below the minimum it is set to the minimum value.
 *
 * @param[in] name      The name of the parameter for which a range is defined.
 * @param[in] minValue  The minimum value allowed for the parameter.
 * @param[in] maxValue  The maximum value allowed for the parameter.
 */
void LeafConstructionInterface::setRandomParameterRange(std::string name, double minValue, double maxValue) {

  auto mapIterator = m_doubleParameterRanges.find(name);

  if ( mapIterator == m_doubleParameterRanges.end() ) {
    //Failed to find the parameter range therefore create a new one.
    //and set initial value to the minimum
    m_doubleParameterRanges[name] = std::pair<double, double>(minValue, maxValue);
    m_doubleParameters[name]      = minValue;
    m_doubleParameterNames.push_back(name);

  } else {

    //Already exists, so don't need to make a new range.
    m_doubleParameterRanges[name] = std::pair<double, double>(minValue, maxValue);

    //Check if initial value is still within the range
    if ( m_doubleParameters[name] < minValue ) {
      m_doubleParameters[name] = minValue;
    }

    if ( m_doubleParameters[name] > maxValue ) {
      m_doubleParameters[name] = maxValue;
    }
  }

}

/*! \brief Set the allowed range of an integer parameter. If the current value lies
 * above the new maximum then it is set to the maximum and similarly if the current
 * value is below the minimum it is set to the minimum value.
 *
 * @param[in] name      The name of the parameter for which a range is defined.
 * @param[in] minValue  The minimum value allowed for the parameter.
 * @param[in] maxValue  The maximum value allowed for the parameter.
 */
void LeafConstructionInterface::setRandomParameterRange(std::string name, int minValue, int maxValue) {

  auto mapIterator = m_integerParameterRanges.find(name);

  if ( mapIterator == m_integerParameterRanges.end() ) {
    //Failed to find the parameter range therefore create a new one.
    //and set initial value to the minimum
    m_integerParameterRanges[name] = std::pair<int, int>(minValue, maxValue);
    m_integerParameters[name]      = minValue;
    m_integerParameterNames.push_back(name);

  } else {

    //Already exists, so don't need to make a new range.
    m_integerParameterRanges[name] = std::pair<int, int>(minValue, maxValue);

    //Check if initial value is still within the range
    if ( m_integerParameters[name] < minValue ) {
      m_integerParameters[name] = minValue;
    }

    if ( m_integerParameters[name] > maxValue ) {
      m_integerParameters[name] = maxValue;
    }
  }

}

/*! \brief Set a double parameter value. If the parameter did not previously exist
 * it is added and a random range is defined. The initial random range has both the
 * minimum and maximum set to the initial value.
 *
 * @param[in] name      The name of the parameter for which a value is set.
 * @param[in] value     The new value for the parameter.
 */
void LeafConstructionInterface::setParameter(std::string name, double value) {

  auto mapIterator = m_doubleParameters.find(name);

  if ( mapIterator == m_doubleParameters.end() ) {
    //Failed to find the parameter therefore create a new one.
    m_doubleParameters[name]      = value;
    m_doubleParameterNames.push_back(name);
    m_doubleParameterRanges[name] = std::pair<double, double>(value, value);
  } else {
    //Already exists, so don't need to make a new range.
    m_doubleParameters[name]      = value;

    if ( value < m_doubleParameterRanges[name].first ) {
      m_doubleParameterRanges[name].first = value;
    }

    if ( value > m_doubleParameterRanges[name].second ) {
      m_doubleParameterRanges[name].second = value;
    }
  }

}

/*! \brief Set an integer parameter value. If the parameter did not previously exist
 * it is added and a random range is defined. The initial random range has both the
 * minimum and maximum set to the initial value.
 *
 * @param[in] name      The name of the parameter for which a value is set.
 * @param[in] value     The new value for the parameter.
 */
void LeafConstructionInterface::setParameter(std::string name, int value) {

  auto mapIterator = m_integerParameters.find(name);

  if ( mapIterator == m_integerParameters.end() ) {
    //Failed to find the parameter therefore create a new one.
    m_integerParameters[name]      = value;
    m_integerParameterNames.push_back(name);
    m_integerParameterRanges[name] = std::pair<double, double>(value, value);
  } else {
    //Already exists, so don't need to make a new range.
    m_integerParameters[name]      = value;

    if ( value < m_integerParameterRanges[name].first ) {
      m_integerParameterRanges[name].first = value;
    }

    if ( value > m_integerParameterRanges[name].second ) {
      m_integerParameterRanges[name].second = value;
    }
  }

}

/*! \brief Return the value of the named parameter.
 *
 * @param[in] name      The name of the parameter which should be returned.
 * \throws std::invalid_argument   If a parameter with the given name and type doesn't exist.
 */
double LeafConstructionInterface::getDoubleParameter(std::string name) const {

  auto mapIterator = m_doubleParameters.find(name);

  if ( mapIterator != m_doubleParameters.end() ) {
    return mapIterator->second;
  }

  //Failed to find parameter, throw an exception
  throw std::invalid_argument("Parameter \"" + name + "\" does not exist");
}

/*! \brief Return the value of the named parameter.
 *
 * @param[in] name      The name of the parameter which should be returned.
 * \throws std::invalid_argument   If a parameter with the given name and type doesn't exist.
 */
int LeafConstructionInterface::getIntegerParameter(std::string name) const {

  auto mapIterator = m_integerParameters.find(name);

  if ( mapIterator != m_integerParameters.end() ) {
    return mapIterator->second;
  }

  //Failed to find parameter, throw an exception
  throw std::invalid_argument("Parameter \"" + name + "\" does not exist");
}

/*! \brief Return the pair of values defining the range of the parameter allowed
 *
 * @param[in] name      The name of the parameter which should have its range returned.
 * \throws std::invalid_argument   If a parameter with the given name and type doesn't exist.
 */
std::pair<double, double> LeafConstructionInterface::getDoubleRange(std::string name) const {

  auto mapIterator = m_doubleParameterRanges.find(name);

  if ( mapIterator != m_doubleParameterRanges.end() ) {
    return mapIterator->second;
  }

  //Failed to find parameter, throw an exception
  throw std::invalid_argument("Parameter range \"" + name + "\" does not exist");
}

/*! \brief Return the pair of values defining the range of the parameter allowed
 *
 * @param[in] name      The name of the parameter which should have its range returned.
 * \throws std::invalid_argument   If a parameter with the given name and type doesn't exist.
 */
std::pair<int, int> LeafConstructionInterface::getIntegerRange(std::string name) const {

  auto mapIterator = m_integerParameterRanges.find(name);

  if ( mapIterator != m_integerParameterRanges.end() ) {
    return mapIterator->second;
  }

  //Failed to find parameter, throw an exception
  throw std::invalid_argument("Parameter range \"" + name + "\" does not exist");
}

/*! \brief Return the list of all parameter names with type double
 *
 */
std::vector<std::string>  LeafConstructionInterface::getDoubleParameterNames() const {
  return m_doubleParameterNames;
}

/*! \brief Return the list of all parameter names with type integer
 *
 */
std::vector<std::string>  LeafConstructionInterface::getIntegerParameterNames() const {
  return m_integerParameterNames;
}

/*! \brief Attempt to open the configuration file
 *
 */
bool LeafConstructionInterface::openConfigurationFile(std::string fileName, libconfig::Config& cfg) {

  // Read the file. If there is an error, report it and exit.
  try{
    cfg.readFile(fileName.c_str());
  } catch(const libconfig::FileIOException &fioex){
    std::cerr << "I/O error while reading file." << std::endl;
    return false;
  } catch(const libconfig::ParseException &pex) {
    std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
	      << " - " << pex.getError() << std::endl;
    return false;
  }

  return true;
}

/*! \brief Load leaf parameters from configuration file
 *
 */
void LeafConstructionInterface::applyConfigurationFile(std::string configurationFileName) {

  libconfig::Config cfg;

  // Need to start by finding the file
  // First try to find it w.r.t the local directory
  std::ifstream localTest(configurationFileName.c_str());

  bool tryShareDirectory = false;
  if ( localTest.is_open() ) {
    bool isFileOpen = openConfigurationFile(configurationFileName, cfg);
    if (!isFileOpen) {
      tryShareDirectory = true;
    }

  } else {
    
    tryShareDirectory = true;
  }

  if (tryShareDirectory) {
    // Not a local file or buggy so look in the installed share directory
    const char* environmentVariableContents = std::getenv("PVTREE_SHARE_PATH");

    if (environmentVariableContents != 0 ){
      std::string shareFilePath(std::string(environmentVariableContents) + "/config/" + configurationFileName);
      std::ifstream shareTest(shareFilePath.c_str());

      if (shareTest.is_open()) {
	bool isFileOpen = openConfigurationFile(shareFilePath, cfg);

	if (!isFileOpen) return;
      } else {

	// Not in either place so give up!
	std::cerr << "Unable to locate file " << configurationFileName << " locally or in the shared config." 
		  << std::endl;
	return;
      }
    } else {

      // Not in either place so give up!
      std::cerr << "Unable to locate file " << configurationFileName << " locally or in the shared config." 
		<< std::endl;
      return;
    }
  }

  // If reached here then try to extract the parameters
  try {
    libconfig::Setting& parameters = cfg.lookup("leaf.parameters");

    int numberOfParameters = parameters.getLength();

    for (int p=0; p<numberOfParameters; p++){
      
      std::string parameterName = parameters[p]["name"];

      // Get the type from value
      if (parameters[p]["value"].getType() == libconfig::Setting::TypeInt) {
	int value;

	parameters[p].lookupValue("value", value);
	setParameter(parameterName, value);

	if ( parameters[p].exists("minimum") && parameters[p].exists("maximum") ) {
	  int minimum, maximum;

	  parameters[p].lookupValue("minimum", minimum);
	  parameters[p].lookupValue("maximum", maximum);
	  setRandomParameterRange(parameterName, minimum, maximum);
	}
      }

      if (parameters[p]["value"].getType() == libconfig::Setting::TypeFloat) {
	double value;

	parameters[p].lookupValue("value", value);
	setParameter(parameterName, value);

	if ( parameters[p].exists("minimum") && parameters[p].exists("maximum") ) {
	  double minimum, maximum;

	  parameters[p].lookupValue("minimum", minimum);
	  parameters[p].lookupValue("maximum", maximum);
	  setRandomParameterRange(parameterName, minimum, maximum);
	}
      }
    }

  } catch(const libconfig::SettingNotFoundException &nfex) {
    std::cerr << "Missing setting in configuration file." << std::endl;
  }

}


ClassImp(LeafConstructionInterface)
