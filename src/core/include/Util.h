#ifndef UTIL_H          /* -*- C++ -*- */
#define UTIL_H

#include "Fail.h"
#include "itype.h"
#include "math.h"
#include <string.h>

namespace MFM {

#define MARK_USED(X) ((void)(&(X)))

  template <const bool mustBeTrue>
  inline void COMPILATION_REQUIREMENT()
  {
    typedef char errorIfFalse[mustBeTrue == 0 ? -1 : 1];
    errorIfFalse t;
    MARK_USED(t);
  }

  template <class T>
  inline T MAX(T x, T y) {
    return (x > y) ? x : y;
  }

  template <class T>
  inline T MIN(T x, T y) {
    return (x < y) ? x : y;
  }

  template <class T>
  inline T CLAMP(T min, T max, T val)
  {
    return val < min ? min : (val > max ? max : val);
  }

  template <class T>
  inline T ABS(T val)
  {
    return val > 0 ? val : (-val);
  }

  template <class T>
  inline double DISTANCE(T x1, T y1, T x2, T y2)
  {
    return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
  }
}

#endif /* UTIL_H */
