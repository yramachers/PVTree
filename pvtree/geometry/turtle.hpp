#ifndef PV_TURTLE
#define PV_TURTLE

#include "TVector3.h"
#include <vector>

/*! \brief L-System 3D pen
 *
 * A class used to trace out an L-System in 3D.
 */
class Turtle {
 private:
 public:
  TVector3 position;    /*!< Starting position of turtle. */
  TVector3 orientation; /*!< Heading of turtle. */
  TVector3 lVector;
  double width;  /*!< Width of turtle at starting position. */
  double length; /*!< Distance turtle will travel along heading. */
  std::vector<Turtle*> children; /*!< List of child turtles starting at end
                                    position of this turtle. */
  Turtle* parent;                /*!< Turtle which created this. */
  bool complete;                 /*!< Has the turtle finished moving */

  Turtle();
  Turtle(Turtle& turtle);
  Turtle(TVector3 initialPosition, TVector3 initialOrientation,
         TVector3 initialLVector);
  ~Turtle();
  void move();
  void moveAlongVector(const TVector3& displacment);
};

#endif  // PV_TURTLE
