#include "climate/climateData.hpp"
#include <iostream>

ClimateData::ClimateData(std::shared_ptr<std::map<std::string,int>> nameToParameterID, time_t time) : m_time(time), m_nameToParameterID(nameToParameterID) {}


ClimateData::~ClimateData() {}


void ClimateData::setParameterMapping(std::shared_ptr<std::map<std::string,int>> nameToParameterID) {

  m_nameToParameterID = nameToParameterID;
}


double ClimateData::getValue(std::string parameterName) const {

  // Obtain the ID from the name if possible
  auto mapping = m_nameToParameterID->find(parameterName);

  if ( mapping != m_nameToParameterID->end() ){
    return getValue(mapping->second);
  }

  std::cerr << "Could not find parameter with name: " << parameterName << std::endl;
  throw;
}


double ClimateData::getValue(int parameterID) const {

  if ( m_parameterValues.find(parameterID) == m_parameterValues.end() ){
    std::cerr << "Could not find parameter with ID: " << parameterID << std::endl;
    throw;
  }

  return m_parameterValues.at(parameterID);
}


void ClimateData::setValue(int parameterID, double value) {
  m_parameterValues[parameterID] = value;
}


bool ClimateData::hasValue(std::string parameterName) const {

  // Obtain the ID from the name if possible
  auto mapping = m_nameToParameterID->find(parameterName);

  if ( mapping != m_nameToParameterID->end() ){
    return hasValue(mapping->second);
  }

  std::cerr << "For this Climate Data could not find parameter with name: " 
	    << parameterName << std::endl;
  throw;
}


bool ClimateData::hasValue(int parameterID) const {

  if ( m_parameterValues.find(parameterID) == m_parameterValues.end() ){
    return false;
  }

  return true;
}


time_t ClimateData::getTime() const {
  return m_time;
}


void ClimateData::setTime(time_t time) {
  m_time = time;
}

