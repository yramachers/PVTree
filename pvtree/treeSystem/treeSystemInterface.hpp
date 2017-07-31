#ifndef TREE_SYSTEM_INTERFACE
#define TREE_SYSTEM_INTERFACE

#include <vector>
#include <ostream>
#include <memory>
#include "pvtree/geometry/turtle.hpp"

class TreeConstructionInterface;

/*! \brief Common interface for defining Lindenmayer symbols.
 *
 * The common interface allows iteration of symbols and their
 * conversion into a 3D geometrical description.
 */
class TreeSystemInterface {
 public:
  /*! \brief Construction interface pointer which provides
   *         access to the shared parameters.
   */
  TreeConstructionInterface* m_constructor;

  /*! \brief To construct this base class it is necessary to pass a construction
   * interface.
   */
  explicit TreeSystemInterface(TreeConstructionInterface* constructor)
      : m_constructor(constructor){};

  virtual ~TreeSystemInterface(){};

  /*! \brief Function needed to provide the rule for how the symbol
   *         should be replaced.
   *  \returns a vector of symbols.
   */
  virtual std::vector<std::shared_ptr<TreeSystemInterface> > applyRule() = 0;

  /*! \brief Function needed to translate the symbol into a behviour in 3D
   * space.
   *  @param[in] turtleStack   The turtles still in production, with
   *                           the top of the stack active.
   *  @param[in] retiredTurtles  Turtles that are no longer active.
   */
  virtual void processTurtles(std::vector<Turtle*>& turtleStack,
                              std::vector<Turtle*>& retiredTurtles) = 0;

  /*! \brief Function needed to convert the symbol to a string
   *  @param[in] os   Stream where the symbol should be printed.
   */
  virtual void print(std::ostream& os) const = 0;
};

#endif  // TREE_SYSTEM_INTERFACE
