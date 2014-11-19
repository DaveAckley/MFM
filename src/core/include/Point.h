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

namespace MFM
{

  /**
   * Two-dimension integral coordinates template class.  Primary uses
   * are UPoint -- two unsigned 32 bit numbers -- and SPoint -- two
   * signed 32 bit numbers.
   */
  template <class T>
  class Point
  {
  private:
    /**
     * The 2D X and Y coordinates of this Point .
     */
    T m_x, m_y;

  public:

    /**
     * Constructs a new Point whose coordinates are initially set to
     * \c 0 .
     */
    Point();

    /**
     * Constructs a new Point whose coordinates are set from specified
     * values.
     *
     * @param x The initial x value of this Point .
     *
     * @param y The initial y value of this Point .
     */
    Point(T x, T y);

    /**
     * Constructs a new Point from another Point, copying its
     * coordinates.
     *
     * @param p The Point to copy the coordinates of.
     */
    Point(const Point<T> & p);

    /**
     * Constructs a new Point with randomly chosen coordinates,
     * between \c 0 and a specified maximum value .
     *
     * @param random The PRNG used to choose the random numbers for
     *               the coordinates of the new Point .
     *
     * @param maxX The maximum X value that can be chosen by \c random .
     *
     * @param maxY The maximum Y value that can be chosen by \c random .
     */
    Point(Random& random, T maxX, T maxY);

    /**
     * Deconstructs this Point .
     */
    ~Point() { }

    /**
     * Gets the x coordinate of this Point .
     *
     * @returns The x coordinate of this Point .
     */
    T GetX() const;

    /**
     * Gets the y coordinate of this Point .
     *
     * @returns The y coordinate of this Point .
     */
    T GetY() const;

    /**
     * Gets the Manhattan Distance from the origin to the ends of the
     * vector represented by this Point .
     *
     * @returns The Manhattan Distance of this Point .
     *
     * @remarks Would prefer 'unsigned T' return type but not using
     *          C++0x type_traits
     */
    u32 GetManhattanLength() const;

    /**
     * Gets the maximum absolute value of the x and y coordinates;
     * i.e. \c MAX(abs(x),abs(y)) .  This value is also known as the
     * 'maximum metric', the 'L-infinity norm', and the 'Chebyshev
     * distance'
     *
     * @returns The maximum of the absolute values of the coordinates
     * in this Point .
     */
    u32 GetMaximumLength() const;

    /**
     * Gets the absolute distance of this Point from the origin using
     * the pythagorean theorem.
     *
     * @returns The length of the vector held by this Point .
     */
    double GetEuclideanLength() const;

    /**
     * Sets the held coordinates of this Point to \c 0 .
     */
    void Clear()
    {
      m_x = m_y = 0;
    }

    /**
     * Adds another Point to this Point , storing the result in this
     * Point .
     *
     * @param offset The Point to add to this Point .
     */
    void Add(const Point<T>& offset);

    /**
     * Adds a pair of given coordinates to the coordinates in this
     * Point .
     *
     * @param x The x coordinate to add to this Point .
     *
     * @param y The y coordinate to add to this Point .
     */
    void Add(T x, T y);

    /**
     * Subtracts another Point from this Point , storing the result in
     * this Point .
     *
     * @param offset The Point to subtract from this Point .
     */
    void Subtract(const Point<T>& offset);

    /**
     * Subtracts a pair of given coordinates from the coordinates in
     * this Point .
     *
     * @param x The x coordinate to subtract from this Point .
     *
     * @param y The y coordinate to subtract from this Point .
     */
    void Subtract(T x, T y);

    /**
     * Multiplies the held coordinates of this Point by a specified
     * scalar value.
     *
     * @param scalar The scalar value used to multiply the coordinates
     *               in this Point .
     */
    void Multiply(T scalar);

    /**
     * Divides the held coordinates of this Point by a specified
     * scalar value.
     *
     * @param scalar The scalar value used to divide the coordinates
     *               in this Point .
     */
    void Divide(T scalar);

    /**
     * Performs the modulus operation on the coordinates of this Point
     * by a specified scalar value .
     *
     * @param scalar The scalar value used to mod the coordinates of
     *               this Point .
     */
    void Mod(T scalar);

    /**
     * Sets the x value of this Point to a specified value .
     *
     * @param x The new x value of this Point .
     */
    void SetX(T x);

    /**
     * Sets the y value of this Point to a specified value .
     *
     * @param y The new y value of this Point .
     */
    void SetY(T y);

    /**
     * Sets the x and y values of this Point to specified coordinates .
     *
     * @param x The new x value of this Point .
     *
     * @param y The new y value of this Point .
     */
    void Set(T x, T y);

    /**
     * Sets the x and y values of this Point to the coordinates stored
     * in another Point .
     *
     * @param copyFrom The Point to copy the values of to this Point .
     */
    void Set(const Point<T>& copyFrom);

    /**
     * Checks for equality between this Point and a similarly typed Point .
     *
     * @param rhs The Point to check equality against this Point .
     *
     * @returns \c true if \c rhs is considered to be equal to this
     *          Point , else \c false.
     */
    bool Equals(const Point<T>& rhs) const;

    /**
     * Checks to see that the coordinates in this Point are both
     * greater than or equal to the coordinates in another Point .
     *
     * @param bound The Point to check boundedness against.
     *
     * @returns \c true if the coordinates in this Point are both
     *          greater than or equal to the respective coordinates in
     *          \c bound .
     */
    bool BoundedBelow(const Point<T>& bound) const;

    /**
     * Checks to see that the coordinates in this Point are both
     * less than or equal to the coordinates in another Point .
     *
     * @param bound The Point to check boundedness against.
     *
     * @returns \c true if the coordinates in this Point are both
     *          less than or equal to the respective coordinates in
     *          \c bound .
     */
    bool BoundedAbove(const Point<T>& bound) const;

    /**
     * Checks to see that the coordinates in this Point are bounded
     * both by a Point representing a lower bound and a Point
     * representing an upper bound; i.e.
     * \code{.cpp}

       BoundedBelow(lowerBound) && BoundedAbove(upperBound)

     * \endcode
     *
     * @param lowerBound The Point representing the lower bound of
     *                   this test.
     *
     * @param upperBound The Point representing the upper bound of
     *                   this test.
     *
     * @returns \c true if this Point is bounded by the two
     *          parameters, else \c false .
     */
    bool BoundedBy(const Point<T>& lowerBound,const Point<T>& upperBound) const;

    /**
     * Checks to see if the coordinates held by this Point are both \c
     * 0 .
     *
     * @returns \c true if both coordinates in this Point are equal to
     *          \c 0 .
     */
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

    /**
     * Creates a Point consisting of the maximum x coordinate of two
     * Points and the maximum y coordinate of two Points.
     *
     * @param lhs One of the Points used to construct this maximum
     *            Point .
     *
     * @param rhs The other Point used to construct this maximum Point .
     *
     * @returns A Point consisting of the maximum x coordinate and
     *          maximum y coordinate of \c lhs and \c rhs .
     */
    friend Point<T> max(const Point<T> & lhs,const Point<T> & rhs)
    {
      T x = MAX(lhs.m_x, rhs.m_x);
      T y = MAX(lhs.m_y, rhs.m_y);
      return Point<T>(x,y);
    }

    /**
     * Creates a Point consisting of the minimum x coordinate of two
     * Points and the minimum y coordinate of two Points.
     *
     * @param lhs One of the Points used to construct this minimum
     *            Point .
     *
     * @param rhs The other Point used to construct this minimum Point .
     *
     * @returns A Point consisting of the minimum x coordinate and
     *          minimum y coordinate of \c lhs and \c rhs .
     */
    friend Point<T> min(const Point<T> & lhs,const Point<T> & rhs)
    {
      T x = MIN(lhs.m_x, rhs.m_x);
      T y = MIN(lhs.m_y, rhs.m_y);
      return Point<T>(x,y);
    }


    ////// Operator overloads

    /**
     * Copies the coordinates from a specified Point to this Point .
     *
     * @param rhs The Point to copy coordinates from.
     *
     * @returns A refrence to this Point .
     */
    Point<T>& operator=(const Point<T> & rhs);

    /**
     * Adds the coordinates of a Point to the coordinates of this
     * Point .
     *
     * @param rhs The Point to add to this Point.
     *
     * @returns A reference to this Point .
     */
    Point<T>& operator+=(const Point<T> & rhs)
    {
      Add(rhs);
      return *this;
    }

    /**
     * Subtracts the coordinates of a Point from the coordinates of
     * this Point .
     *
     * @param rhs The Point to subtract from this Point.
     *
     * @returns A reference to this Point .
     */
    Point<T>& operator-=(const Point<T> & rhs)
    {
      Subtract(rhs);
      return *this;
    }

    /**
     * Multiplies the coordinates of this Point by a specified
     * integral scalar.
     *
     * @param rhs The scalar to multiply the coordinates of this Point
     *            by.
     *
     * @returns A reference to this Point.
     */
    Point<T>& operator*=(const s32 rhs)
    {
      Multiply(rhs);
      return *this;
    }

    /**
     * Divides the coordinates of this Point by a specified
     * integral scalar.
     *
     * @param rhs The scalar to divide the coordinates of this Point
     *            by.
     *
     * @returns A reference to this Point.
     */
    Point<T>& operator/=(const s32 rhs)
    {
      Divide(rhs);
      return *this;
    }

    /**
     * Creates a new Point consisting of the sum of the coordinates of
     * two given points.
     *
     * @param lhs The augend Point used to construct the summed Point .
     *
     * @param rhs The addend Point used to construct the summed Point .
     *
     * @returns The Point consisting of the sum of the coordinates in
     *          \c lhs and \c rhs .
     */
    friend Point<T> operator+(const Point<T> & lhs,const Point<T> & rhs)
    {
      Point<T> ret(lhs);
      ret.Add(rhs);
      return ret;
    }

    /**
     * Creates a new Point consisting of the difference of the
     * coordinates of two given points.
     *
     * @param lhs The minuend Point used to construct the differenced
     *            Point .
     *
     * @param rhs The subtrahend Point used to construct the summed
     *            Point .
     *
     * @returns The Point consisting of the difference of the
     *          coordinates in \c lhs and \c rhs .
     */
    friend Point<T> operator-(const Point<T> & lhs,const Point<T> & rhs)
    {
      Point<T> ret(lhs);
      ret.Subtract(rhs);
      return ret;
    }

    /**
     * Checks to see that a Point is considered to be equal to another
     * Point .
     *
     * @param lhs One Point to check for equality.
     *
     * @param rhs The other Point to check for equality.
     *
     * @returns \c true if \c lhs is considered to be equal to \c rhs
     *          , else \c false .
     */
    friend bool operator==(const Point<T> & lhs,const Point<T> & rhs)
    {
      return lhs.Equals(rhs);
    }

    /**
     * Checks to see that a Point is considered not to be equal to
     * another Point .
     *
     * @param lhs One Point to check for inequality.
     *
     * @param rhs The other Point to check for inequality.
     *
     * @returns \c true if \c lhs is considered to be inequal to \c
     *          rhs , else \c false .
     */
    friend bool operator!=(const Point<T> & lhs,const Point<T> & rhs)
    {
      return !(lhs==rhs);
    }

    /**
     * Constructs a new Point consisting of the coordinates of one
     * Point multiplied by an integral scalar.
     *
     * @param lhs The multiplicand Point used to construct the new
     *            Point .
     *
     * @param rhs The multiplier scalar used to construc the new Point
     * .
     *
     * @returns The Point which is the product of the coordinates of
     *          \c lhs and the scalar \c rhs .
     */
    friend Point<T> operator*(const Point<T> & lhs,const s32 rhs)
    {
      Point<T> ret(lhs);
      ret.Multiply(rhs);
      return ret;
    }

    /**
     * Constructs a new Point consisting of the coordinates of one
     * Point divided by an integral scalar.
     *
     * @param lhs The dividend Point used to construct the new Point .
     *
     * @param rhs The divisor scalar used to construct the new Point .
     *
     * @returns The Point which is the quotient of the coordinates of
     *          \c lhs and the scalar \c rhs .
     */
    friend Point<T> operator/(const Point<T> & lhs,const s32 rhs)
    {
      Point<T> ret(lhs);
      ret.Divide(rhs);
      return ret;
    }

    /**
     * Constructs a new Point consisting of the remainder of the
     * division between coordinates of one Point and an integral scalar.
     *
     * @param lhs The dividend Point used to construct the new Point .
     *
     * @param rhs The divisor scalar used to construct the new Point .
     *
     * @returns The Point which is the remainder of the division
     *          between the coordinates of \c lhs and the scalar \c
     *          rhs .
     */
    friend Point<T> operator%(const Point<T> & lhs,const s32 rhs)
    {
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

  /**
   * A Point which has coordinates of an unsigned 32-bit integral type.
   */
  typedef Point<u32> UPoint;

  /**
   * A point which has coordinates of a signed 16-bit integral type.
   */
  typedef Point <s16> SSPoint;

  /**
   * A point which has coordinates of an unsigned 16-bit integral type.
   */
  typedef Point <u16> USPoint;

  /**
   * Converts a signed Point to an unsigned Point .
   *
   * @param spoint The signed Point to convert.
   *
   * @returns An unsigned version of \c spoint .
   *
   * FAIL ILLEGAL_ARGUMENT if either coordinate of spoint is negative
   */
  UPoint MakeUnsigned(const SPoint & spoint);

  /**
   * Check if a signed Point can be passed to MakeUnsigned successfully
   *
   * @param spoint The signed Point to check
   *
   * @returns true iff neither coordinate of spoint is negative
   */
  inline bool CanMakeUnsigned(const SPoint & spoint)
  {
    return spoint.GetX() >= 0 && spoint.GetY() >= 0;
  }

  /**
   * Converts an unsigned Point to a signed Point .
   *
   * @param upoint The unsigned Point to convert.
   *
   * @returns A signed version of \c upoint .
   *
   * FAIL ILLEGAL_ARGUMENT if either coordinate of upoint is greater
   * than S32_MAX
   */
  SPoint MakeSigned(const UPoint & upoint);

  /**
   * Check if an unsigned Point can be passed to MakeSigned successfully
   *
   * @param upoint The unsigned Point to check
   *
   * @returns true iff neither coordinate of upoint is greater than S32_MAX
   */
  inline bool CanMakeSigned(const UPoint & upoint)
  {
    return upoint.GetX() <= (u32) S32_MAX && upoint.GetY() <= (u32) S32_MAX;
  }

} /* namespace MFM */

#include "Point.tcc"

#endif /*POINT_H*/
