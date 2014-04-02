#ifndef UTIL_H          /* -*- C++ -*- */
#define UTIL_H

#include "Fail.h"
#include "itype.h"
#include <string.h>

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

#endif /* UTIL_H */
