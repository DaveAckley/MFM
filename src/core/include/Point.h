/*                                              -*- mode:C++ -*-
  Point.h Two-dimensional integral vector
  Copyright (C) 2014 The Regents of the University of New Mexico.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
  USA
*/

/**
  \file Point.h Two-dimensional integral vector
  \author Trent R. Small.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef POINT_H
#define POINT_H

#include <stdlib.h>
#include "itype.h"
#include "Random.h"
#include "Util.h"  /* For MAX and MIN */

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

  u32 GetManhattanLength() const;  // Would prefer 'unsigned T' return type but not using C++0x type_traits
  u32 GetMaximumLength() const;

  double GetEuclideanLength() const;

  void Clear() {
    m_x = m_y = 0;
  }

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

  bool Equals(const Point<T>& rhs) const;

  bool BoundedBelow(const Point<T>& bound) const;

  bool BoundedAbove(const Point<T>& bound) const;

  bool BoundedBy(const Point<T>& lowerBound,const Point<T>& upperBound) const;

  bool IsZero() const
  {
    return m_x == 0 && m_y == 0;
  }

  /**
   * Parses a char* for a Point of \emph{integral} type in the format
   * "(x,y)" with no whitespace. This treats any read values as
   * (T)atoi(x) and (T)atoi(y), so make sure that casting from a call
   * of atoi won't break anything.
   *
   * @param buffer The char* to parse a point directly from.
   */
  void Parse(char* buffer);

  friend Point<T> max(const Point<T> & lhs,const Point<T> & rhs) {
    T x = MAX(lhs.m_x, rhs.m_x);
    T y = MAX(lhs.m_y, rhs.m_y);
    return Point<T>(x,y);
  }

  friend Point<T> min(const Point<T> & lhs,const Point<T> & rhs) {
    T x = MIN(lhs.m_x, rhs.m_x);
    T y = MIN(lhs.m_y, rhs.m_y);
    return Point<T>(x,y);
  }


  ////// Operator overloads

  Point<T>& operator=(const Point<T> & rhs);

  Point<T>& operator+=(const Point<T> & rhs) {
    Add(rhs);
    return *this;
  }

  Point<T>& operator-=(const Point<T> & rhs) {
    Subtract(rhs);
    return *this;
  }

  Point<T>& operator*=(const s32 rhs) {
    Multiply(rhs);
    return *this;
  }

  Point<T>& operator/=(const s32 rhs) {
    Divide(rhs);
    return *this;
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
UPoint MakeUnsigned(const SPoint & spoint);

SPoint MakeSigned(const UPoint & upoint);

} /* namespace MFM */

#include "Point.tcc"

#endif /*POINT_H*/
