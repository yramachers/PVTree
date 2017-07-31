#ifndef LEAF_SYSTEM_INTERFACE
#define LEAF_SYSTEM_INTERFACE

#include <vector>
#include <ostream>
#include <memory>

class Turtle;
class Polygon;

class LeafConstructionInterface;

/*! \brief Common interface for defining Lindenmayer symbols.
 *
 * The common interface allows iteration of symbols and their
 * conversion into a 3D geometrical description. The difference
 * with the tree case is the presence of symbols which define
 * polygons (through defining the vertex positions).
 */
class LeafSystemInterface {
 public:
  /*! \brief Construction interface pointer which provides
   *         access to the shared parameters.
   */
  LeafConstructionInterface* m_constructor;

  /*! \brief To construct this base class it is necessary to pass a construction
   * interface.
   */
  explicit LeafSystemInterface(LeafConstructionInterface* constructor)
      : m_constructor(constructor){};

  virtual ~LeafSystemInterface(){};

  /*! \brief Function needed to provide the rule for how the symbol
   *         should be replaced.
   *  \returns a vector of symbols.
   */
  virtual std::vector<std::shared_ptr<LeafSystemInterface> > applyRule() = 0;

  /*! \brief Function needed to translate the symbol into a behviour in 3D
   * space.
   *  @param[in] turtleStack   The turtles still in production, with
   *                           the top of the stack active.
   *  @param[in] retiredTurtles  Turtles that are no longer active.
   *  @param[in] leafSegments  The list of polygons which are being constructed,
   *                           where the last polygon is being actively
   * constructed.
   */
  virtual void processTurtles(std::vector<Turtle*>& turtleStack,
                              std::vector<Turtle*>& retiredTurtles,
                              std::vector<Polygon*>& leafSegments) = 0;

  /*! \brief Function needed to convert the symbol to a string
   *  @param[in] os   Stream where the symbol should be printed.
   */
  virtual void print(std::ostream& os) const = 0;
};

#endif  // LEAF_SYSTEM_INTEFACE
