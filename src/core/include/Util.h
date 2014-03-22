#ifndef UTIL_H          /* -*- C++ -*- */
#define UTIL_H

template <class T>
inline T MAX(T x, T y) {
  return (x > y) ? x : y;
}

template <class T>
inline T MIN(T x, T y) {
  return (x < y) ? x : y;
}

#endif /* UTIL_H */
