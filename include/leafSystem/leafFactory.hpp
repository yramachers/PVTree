#ifndef LEAF_SYSTEM_LEAF_FACTORY_HPP
#define LEAF_SYSTEM_LEAF_FACTORY_HPP

#include "leafSystem/leafConstructionInterface.hpp"
#include <string>
#include <map>
#include <memory>
#include <functional>

/*! \brief Factory which will provide constructors for
 *         any defined leaf Lindenmayer systems.
 *
 * Follows the singleton pattern so all access to methods
 * is made through a static instance.
 */
class LeafFactory {
private:
  LeafFactory();//!< Prevent construction of additional instances
  LeafFactory(LeafFactory& leafFactory);//!< Prevent copy construction of instances

  //! Store available leaf constructors by a string
  std::map<std::string, std::function<LeafConstructionInterface*(void)>> m_factoryFunctionRegistry;
public:

  /*! \brief Retrieve the singleton reference to this factory.
   */
  static LeafFactory* instance();
  
  /*! \brief Retrieve a leaf constructor using a leaf name.
   * @param[in] leafName  Name of the leaf as registered in the constructor definition
   * \returns A shared pointer to a leaf constructor.
   */
  std::shared_ptr<LeafConstructionInterface> getLeaf(std::string leafName);
  
  /*! brief Register a leaf constructor with the factory
   */
  void registerConstructor(std::string leafName, 
			   std::function<LeafConstructionInterface*(void)> constructorFunction);
};

/*! \brief Registration class for leaf factory
 *
 * Need to create an instance per leaf that should be available
 * in the global leaf factory.
 */
template<class T>
class LeafFactoryRegistrar {
public:
  explicit LeafFactoryRegistrar(std::string leafName)
    {
      //register the class factory function
      LeafFactory::instance()->registerConstructor(leafName,
						   [](void) -> LeafConstructionInterface * { return new T();});
    }
};


#endif //LEAF_SYSTEM_LEAF_FACTORY_HPP
