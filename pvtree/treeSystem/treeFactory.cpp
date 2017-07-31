#include "pvtree/treeSystem/treeFactory.hpp"
#include <stdexcept>

TreeFactory::TreeFactory() {}

TreeFactory::TreeFactory(TreeFactory& /*treeFactory*/) {}

TreeFactory* TreeFactory::instance() {
  static TreeFactory treeFactory;
  return &treeFactory;
}

std::shared_ptr<TreeConstructionInterface> TreeFactory::getTree(
    std::string treeName) {
  // Check if the tree actually exists
  auto previousEntry = m_factoryFunctionRegistry.find(treeName);

  if (previousEntry == m_factoryFunctionRegistry.end()) {
    // Does not exist!
    throw std::invalid_argument("Cannot find tree with name = " + treeName);
  }

  return std::shared_ptr<TreeConstructionInterface>(
      ((*previousEntry).second)());
}

void TreeFactory::registerConstructor(
    std::string treeName,
    std::function<TreeConstructionInterface*(void)> constructorFunction) {
  // Check if there is already a tree with the given name
  auto previousEntry = m_factoryFunctionRegistry.find(treeName);

  if (previousEntry != m_factoryFunctionRegistry.end()) {
    throw std::invalid_argument(
        "Cannot record two tree constructors with the same name = " + treeName);
  }

  m_factoryFunctionRegistry[treeName] = constructorFunction;
}
