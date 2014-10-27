/*                                              -*- mode:C++ -*-
  Rect.h 2D rectangle with location, width, and height
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
  \file Rect.h 2D rectangle with location, width, and height
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef RECT_H
#define RECT_H

#include "itype.h"
#include "Point.h"
#include "Random.h"
#include <stdio.h>  /* For FILE */

namespace MFM
{
  /**
   * A 2D Rectangle with a location and dimensions.
   */
  class Rect
  {
  private:
    SPoint m_position;
    UPoint m_size;

  public:

    /**
     * Creates a new Rectangle with a position at the origin and no
     * size.
     */
    Rect() :
      m_position(SPoint(0, 0)),
      m_size(UPoint(0, 0))
    { }

    /**
     * Creates a new Rectangle with the properties of another given
     * rectangle.
     *
     * @param copy The Rectangle of which to copy the properties from.
     */
    Rect(const Rect & copy) :
      m_position(copy.m_position),
      m_size(copy.m_size)
    { }

    /**
     * Creates a new Rectangle with a given position and a given size.
     *
     * @param pos The initial position of this Rect .
     *
     * @param size The initial size of this Rect .
     */
    Rect(const SPoint & pos, const UPoint & size)
      : m_position(pos), m_size(size)
    { }

    /**
     * Creates a new Rectangle with a given position and a given size.
     *
     * @param x The initial x coordinate of the position of this Rect .
     *
     * @param y The initial y coordinate of the position of this Rect .
     *
     * @param w The initial width of this Rect .
     *
     * @param h The initial height of this Rect .
     */
    Rect(s32 x, s32 y, u32 w, u32 h) :
      m_position(x, y),
      m_size(w, h)
    { }

    /**
     * Transforms this Rect into one which represents the intersection
     * of this Rect and a specified Rect . This intersection will be
     * stored in this Rect .
     *
     * @param other The other Rect which will be used to calculate an
     *              intersection with this Rect .
     */
    void IntersectWith(const Rect & other) ;

    /**
     * Checks to see if this Rect has an empty size.
     *
     * @returns \c true if the size of this Rect is empty, i.e. that
     *          both of the coordinates in its size vector are equal
     *          to \c 0 .
     */
    bool IsEmpty() const
    {
      return m_size.IsZero();
    }

    /**
     * Checks to see that a given SPoint lies within the bounds of
     * this Rect .
     *
     * @param point The SPoint in question of existing within the
     *              bounds of this Rect .
     *
     * @returns \c true if \c point lies within this Rect , else \c
     *          false .
     */
    bool Contains(const SPoint & point)
    {
      return point.BoundedBy(m_position, m_position + MakeSigned(m_size));
    }

    /**
     * Gets the position of this Rect .
     *
     * @returns The position of this Rect .
     */
    const SPoint & GetPosition() const
    {
      return m_position;
    }

    /**
     * Gets the x coordinate of the position of this Rect .
     *
     * @returns The x coordinate of the position of this Rect .
     */
    s32 GetX() const
    {
      return m_position.GetX();
    }

    /**
     * Gets the y coordinate of the position of this Rect .
     *
     * @returns The y coordinate of the position of this Rect .
     */
    s32 GetY() const
    {
      return m_position.GetY();
    }

    /**
     * Sets the position of this Rect to a given SPoint .
     *
     * @param newPos The new position of this Rect .
     */
    void SetPosition(const SPoint & newPos)
    {
      m_position = newPos;
    }

    /**
     * Sets the x coordinate of the position of this Rect to a given
     * integral coordinate.
     *
     * @param newX The new x coordinate of this Rect .
     */
    void SetX(s32 newX)
    {
      m_position.SetX(newX);
    }

    /**
     * Sets the y coordinate of the position of this Rect to a given
     * integral coordinate.
     *
     * @param newY The new y coordinate of this Rect .
     */
    void SetY(s32 newY)
    {
      m_position.SetY(newY);
    }

    /**
     * Gets the current size of this Rect .
     *
     * @returns The current size of this Rect .
     */
    const UPoint & GetSize() const
    {
      return m_size;
    }

    /**
     * Gets the current width of this Rect .
     *
     * @returns The current width of this Rect .
     */
    u32 GetWidth() const
    {
      return m_size.GetX();
    }

    /**
     * Gets the current height of this Rect .
     *
     * @returns The current width of this Rect .
     */
    u32 GetHeight() const
    {
      return m_size.GetY();
    }

    /**
     * Sets the size of this Rect to a specified dimension.
     *
     * @param newSize The new size of this Rect .
     */
    void SetSize(const UPoint & newSize)
    {
      m_size = newSize;
    }

    /**
     * Sets the width of this Rect to a specified integral width.
     *
     * @param newWidth The new width of this Rect .
     */
    void SetWidth(u32 newWidth)
    {
      m_size.SetX(newWidth);
    }

    /**
     * Sets the height of this Rect to a specified integral height.
     *
     * @param newheight The new height of this Rect .
     */
    void SetHeight(u32 newHeight)
    {
      m_size.SetY(newHeight);
    }

    ////// Operator overloads

    /**
     * Copies the values of a given Rect to this Rect .
     *
     * @param rhs The Rect of which to copy the values from and into
     *            this Rect .
     *
     * @returns A reference to this Rect .
     */
    Rect& operator=(const Rect & rhs)
    {
      m_position = rhs.m_position;
      m_size = rhs.m_size;
      return *this;
    }

    /**
     * Calculates the intersection between this Rect and another Rect
     * , storing the result in this Rect .
     *
     * @param rhs The Rect used to calculate this intersection.
     *
     * @returns A reference to this Rect .
     */
    Rect& operator&=(const Rect & rhs)
    {
      IntersectWith(rhs);
      return *this;
    }
  };
} /* namespace MFM */

#endif /*RECT_H*/
