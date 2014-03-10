#ifndef POINT_H      /* -*- C++ -*- */
#define POINT_H

#include <stdlib.h>
#include "itype.h"
#include "random.h"

namespace MFM {

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

  Point(Random& random, T maxX, T maxY);

  Point();

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

  void Divide(T scalar);

  void Mod(T scalar);

  void SetX(T x);

  void SetY(T y);

  void Set(T x, T y);

  void Set(const Point<T>& copyFrom);

  bool Equals(const Point<T>& rhs);

  ////// Operator overloads

  Point<T>& operator=(const Point<T> & rhs);

  Point<T>& operator+=(const Point<T> & rhs) {
    Add(rhs);
  }

  Point<T>& operator-=(const Point<T> & rhs) {
    Subtract(rhs);
  }

  Point<T>& operator*=(const s32 rhs) {
    Multiply(rhs);
  }

  Point<T>& operator/=(const s32 rhs) {
    Divide(rhs);
  }

  friend Point<T> operator+(const Point<T> & lhs,const Point<T> & rhs) {
    Point<T> ret(lhs);
    ret.Add(rhs);
    return ret;
  }

  friend Point<T> operator-(const Point<T> & lhs,const Point<T> & rhs) {
    Point<T> ret(lhs);
    ret.Subtract(rhs);
    return ret;
  }

  friend bool operator==(const Point<T> & lhs,const Point<T> & rhs) {
    return lhs.Equals(rhs);
  }

  friend bool operator!=(const Point<T> & lhs,const Point<T> & rhs) {
    return !(lhs==rhs);
  }

  friend Point<T> operator*(const Point<T> & lhs,const s32 rhs) {
    Point<T> ret(lhs);
    ret.Multiply(rhs);
    return ret;
  }

  friend Point<T> operator/(const Point<T> & lhs,const s32 rhs) {
    Point<T> ret(lhs);
    ret.Divide(rhs);
    return ret;
  }

  friend Point<T> operator%(const Point<T> & lhs,const s32 rhs) {
    Point<T> ret(lhs);
    ret.Mod(rhs);
    return ret;
  }


};

/**
 * A Signed Point, which we will use for both absolute and relative
 * indexing, even though absolute indexing should more properly use a
 * Point<u32>, to ease abs = abs+rel type arithmetic without needing
 * to screw with signed/unsigned template traits and such.  It's on
 * the absolute users of SPoint to ensure their stuff is first
 * quadrant when they use it.
 */
typedef Point<s32> SPoint;

typedef Point<u32> UPoint;

/** 
 * Non-template conversions, just for our common case.
 */
UPoint makeUnsigned(const SPoint & spoint);

SPoint makeSigned(const UPoint & upoint);

} /* namespace MFM */

#include "point.tcc"

#endif /*POINT_H*/

