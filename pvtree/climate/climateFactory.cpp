#include "pvtree/climate/climateFactory.hpp"
#include "pvtree/climate/climate.hpp"

#include <iostream>


ClimateFactory::ClimateFactory() : m_climateConfiguration(""),
				   m_climate(nullptr),
				   m_climateConfigurationChanged(true),
				   m_deviceLocation("location.cfg") {}

ClimateFactory::ClimateFactory(ClimateFactory& climateFactory) : m_climateConfiguration(climateFactory.m_climateConfiguration), m_deviceLocation(climateFactory.m_deviceLocation) {
  m_climate = nullptr;
  m_climateConfigurationChanged = true;
}

ClimateFactory::~ClimateFactory() {

  // Delete the climate if necessary
  if (m_climate != nullptr) {
    delete m_climate;
    m_climate = nullptr;
  }

}

ClimateFactory* ClimateFactory::instance() {
  static ClimateFactory climateFactory;
  return &climateFactory;
}

void ClimateFactory::setConfigurationFile(std::string configurationFileName) {

  if (configurationFileName != m_climateConfiguration){
    // If changed then prepare for climate creation
    m_climateConfiguration = configurationFileName;
    m_climateConfigurationChanged = true;
  }

}

void ClimateFactory::setDeviceLocation(LocationDetails deviceLocation) {

  m_deviceLocation = deviceLocation;
  m_climateConfigurationChanged = true;

}

const Climate* ClimateFactory::getClimate() {

  // May need to obtain a new climate
  if (m_climateConfigurationChanged) {

    if (m_climateConfiguration != "") {

      // Delete the old climate if necessary
      if (m_climate != nullptr) {
	delete m_climate;
	m_climate = nullptr;
      }

      m_climate = new Climate(m_climateConfiguration, m_deviceLocation);
      m_climateConfigurationChanged = false;
    } else {
      // Configuration not specified, time to fail ungracefully
      std::cerr << "ClimateFactory::getClimate : No configuration file specifed." << std::endl;
      throw;
    }
  }

  return m_climate;
}
