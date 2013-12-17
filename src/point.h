#ifndef POINT_H
#define POINT_H

#include <stdlib.h>

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

  Point(bool random);

  Point() { }

  ~Point() { }

  T GetX();

  T GetY();

  void Add(Point<T>* offset);

  void Add(T x, T y);

  void Multiply(T scalar);

  void SetX(T x);

  void SetY(T y);
};

#include "point.tcc"

#endif /*POINT_H*/
