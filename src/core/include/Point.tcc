/* -*- C++ -*- */
#include "Fail.h"
#include "Util.h"   /* For MAX */
#include <math.h>   /* For sqrt */

namespace MFM
{

  template <class T>
  Point<T>::Point() :
    m_x(0),
    m_y(0)
  { }

  template <class T>
  Point<T>::Point(T x, T y) :
    m_x(x),
    m_y(y)
  { }

  template <class T>
  Point<T>::Point(const Point<T> & p) :
    m_x(p.m_x),
    m_y(p.m_y)
  { }

  template <class T>
  Point<T>::Point(Random & random, T maxX, T maxY) :
    m_x((T)random.Create(maxX)),
    m_y((T)random.Create(maxY))
  { }

  template <class T>
  T Point<T>::GetX() const
  {
    return m_x;
  }

  template <class T>
  T Point<T>::GetY() const
  {
    return m_y;
  }

  template <class T>
  u32 Point<T>::GetManhattanLength() const
  {
    return (u32) (abs(m_x) + abs(m_y));
  }

  template <class T>
  u32 Point<T>::GetMaximumLength() const
  {
    return (u32) MAX(abs(m_x), abs(m_y));
  }

  template <class T>
  double Point<T>::GetEuclideanLength() const
  {
    return sqrt(m_x * m_x + m_y * m_y);
  }

  template <class T>
  void Point<T>::Add(const Point<T>& offset)
  {
    m_x += offset.m_x;
    m_y += offset.m_y;
  }

  template <class T>
  void Point<T>::Add(T x, T y)
  {
    m_x += x;
    m_y += y;
  }

  template <class T>
  void Point<T>::Subtract(const Point<T>& offset)
  {
    m_x -= offset.m_x;
    m_y -= offset.m_y;
  }

  template <class T>
  void Point<T>::Subtract(T x, T y)
  {
    m_x -= x;
    m_y -= y;
  }

  template <class T>
  void Point<T>::Multiply(T scalar)
  {
    m_x *= scalar;
    m_y *= scalar;
  }

  template <class T>
  void Point<T>::Divide(T scalar)
  {
    m_x /= scalar;
    m_y /= scalar;
  }

  template <class T>
  void Point<T>::Mod(T scalar)
  {
    m_x %= scalar;
    m_y %= scalar;
  }

  /**
   * Seeks a char* for a specified char. If this char is not found by
   * the time this function seeks to the end (specified by strlen), this
   * function calls FAIL(ARRAY_INDEX_OUT_OF_BOUNDS) . Therefore, this
   * function runs in O(n) time in terms of the length of the given
   * char* .
   *
   * @param start The start of the char* to scan.
   *
   * @param seekchar The char to look for in start.
   *
   * @returns start, seeked to the first occurrence of seekchar.
   */
  static char* SEEK(char* start, char seekchar)
  {
    MFM::u32 len = strlen(start);
    MFM::u32 curChar = 0;

    while(*start != seekchar)
    {
      curChar++;
      start++;

      if(curChar >= len)
      {
	FAIL(ARRAY_INDEX_OUT_OF_BOUNDS);
      }
    }
    return start;
  }


  template <class T>
  void Point<T>::Parse(char* buffer)
  {
    /*
     * SEEK FAILs if argument 2 is not found in argument 1 before
     * strlen(argument 1) is reached.
     */
    buffer = SEEK(buffer, '(') + 1;

    m_x = (T)atoi(buffer);

    buffer = SEEK(buffer, ',') + 1;

    m_y = (T)atoi(buffer);

    buffer = SEEK(buffer, ')');
  }


  template <class T>
  void Point<T>::SetX(T x)
  {
    m_x = x;
  }

  template <class T>
  void Point<T>::SetY(T y)
  {
    m_y = y;
  }

  template <class T>
  inline bool operator==(Point<T>& lhs, Point<T>& rhs)
  {
    return lhs.GetX() == rhs.GetX() &&
      lhs.GetY() == rhs.GetY();
  }

  template <class T>
  void Point<T>::Set(T x, T y)
  {
    m_x = x;
    m_y = y;
  }

  template <class T>
  void Point<T>::Set(const Point<T>& other)
  {
    m_x = other.m_x;
    m_y = other.m_y;
  }

  template <class T>
  Point<T>& Point<T>::operator=(const Point<T>& rhs)
  {
    m_x = rhs.m_x;
    m_y = rhs.m_y;
    return *this;
  }

  template <class T>
  bool Point<T>::Equals(const Point<T>& rhs) const
  {
    return GetX()==rhs.GetX() && GetY()==rhs.GetY();
  }

  template <class T>
  bool Point<T>::BoundedBelow(const Point<T>& bound) const
  {
    return GetX()>=bound.GetX() && GetY()>=bound.GetY();
  }

  template <class T>
  bool Point<T>::BoundedAbove(const Point<T>& bound) const
  {
    return GetX()<=bound.GetX() && GetY()<=bound.GetY();
  }

  template <class T>
  bool Point<T>::BoundedBy(const Point<T>& lowerBound, const Point<T>& upperBound) const
  {
    return BoundedBelow(lowerBound) && BoundedAbove(upperBound);
  }
} /* namespace MFM */
