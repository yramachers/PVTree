#ifndef TREE_SYSTEM_TREE_FACTORY_HPP
#define TREE_SYSTEM_TREE_FACTORY_HPP

#include "pvtree/treeSystem/treeConstructionInterface.hpp"
#include <string>
#include <map>
#include <memory>
#include <functional>

/*! \brief Factory which will provide constructors for
 *         any defined tree Lindenmayer systems.
 *
 * Follows the singleton pattern so all access to methods
 * is made through a static instance.
 */
class TreeFactory {
private:
  TreeFactory();//!< Prevent construction of additional instances
  TreeFactory(TreeFactory& treeFactory);//!< Prevent copy construction of instances

  //! Store available tree constructors by a string
  std::map<std::string, std::function<TreeConstructionInterface*(void)>> m_factoryFunctionRegistry;
public:

  /*! \brief Retrieve the singleton reference to this factory.
   */
  static TreeFactory* instance();
  
  /*! \brief Retrieve a tree constructor using a tree name.
   * @param[in] treeName  Name of the tree as registered in the constructor definition
   * \returns A shared pointer to a tree constructor.
   */
  std::shared_ptr<TreeConstructionInterface> getTree(std::string treeName);
  
  /*! brief Register a tree constructor with the factory
   */
  void registerConstructor(std::string treeName, 
			   std::function<TreeConstructionInterface*(void)> constructorFunction);
};

/*! \brief Registration class for tree factory
 *
 * Need to create an instance per tree that should be available
 * in the global tree factory.
 */
template<class T>
class TreeFactoryRegistrar {
public:
  explicit TreeFactoryRegistrar(std::string treeName)
    {
      //register the class factory function
      TreeFactory::instance()->registerConstructor(treeName,
						   [](void) -> TreeConstructionInterface * { return new T();});
    }
};


#endif //TREE_SYSTEM_TREE_FACTORY_HPP
