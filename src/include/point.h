#ifndef POINT_H      /* -*- C++ -*- */
#define POINT_H

#include <stdlib.h>
#include "itype.h"

/* 
 * Going to go ahead and use templates here
 * so we can store any kind of number in these.
 */
template <class T>
class Point
{
private:
  T m_x, m_y;

public:

  Point(T x, T y);

  Point(const Point<T> &);

  Point(bool random);

  Point(bool random, T maxX, T maxY);

  Point() { }

  ~Point() { }

  T GetX() const;

  T GetY() const;

  u32 GetManhattanDistance() const;  // Would prefer 'unsigned T' return type but not using C++0x type_traits

  double GetLength() const;

  void Add(const Point<T>& offset);

  void Add(T x, T y);

  void Subtract(const Point<T>& offset);

  void Subtract(T x, T y);

  void Multiply(T scalar);

  void SetX(T x);

  void SetY(T y);

  void Set(T x, T y);

  Point<T>& operator=(const Point<T> & rhs);
};


#include "point.tcc"

#endif /*POINT_H*/
