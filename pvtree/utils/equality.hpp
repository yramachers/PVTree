#ifndef PV_TREE_UTILS_EQUALITY_HPP
#define PV_TREE_UTILS_EQUALITY_HPP

#include <algorithm>
#include <cmath>

/*! \brief Taken from http://en.cppreference.com/w/cpp/types/numeric_limits/epsilon
 *         to allow checking of floating point value equality.
 *
 */
template<class T>
typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type
almost_equal(T x, T y, int ulp){
  // Needs to be defined inline because it is a template!

  // the machine epsilon has to be scaled to the magnitude of the values used
  // and multiplied by the desired precision in ULPs (units in the last place)
  return std::abs(x-y) < std::numeric_limits<T>::epsilon() * std::abs(x+y) * ulp
    // unless the result is subnormal
    || std::abs(x-y) < std::numeric_limits<T>::min();
}


#endif //PV_TREE_UTILS_EQUALITY_HPP
