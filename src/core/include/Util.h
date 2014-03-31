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

#endif /* UTIL_H */
