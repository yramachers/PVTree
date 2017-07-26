#include "pvtree/leafSystem/leafFactory.hpp"
#include <stdexcept>
#include <iostream>

LeafFactory::LeafFactory() {}

LeafFactory::LeafFactory(LeafFactory& /*leafFactory*/) {}


LeafFactory* LeafFactory::instance(){
  static LeafFactory leafFactory;
  return &leafFactory;
}

std::shared_ptr<LeafConstructionInterface> LeafFactory::getLeaf(std::string leafName) {

  //Check if the leaf actually exists
  auto previousEntry = m_factoryFunctionRegistry.find(leafName);

  if (previousEntry == m_factoryFunctionRegistry.end()){
    //Does not exist!
    throw std::invalid_argument("Cannot find leaf with name = " + leafName);
  }

  return std::shared_ptr<LeafConstructionInterface>( ((*previousEntry).second)() );
}

void LeafFactory::registerConstructor(std::string leafName, 
				      std::function<LeafConstructionInterface*(void)> constructorFunction ){
  
  //Check if there is already a leaf with the given name
  auto previousEntry = m_factoryFunctionRegistry.find(leafName);

  if (previousEntry != m_factoryFunctionRegistry.end()) {
    throw std::invalid_argument("Cannot record two leaf constructors with the same name = " + leafName);
  }

  m_factoryFunctionRegistry[leafName] = constructorFunction;
}

