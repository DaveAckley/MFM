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

  class Rect
  {
  private:
    SPoint m_position;
    UPoint m_size;

  public:
    Rect() { }

    Rect(const Rect & copy)
      : m_position(copy.m_position), m_size(copy.m_size)
    { }

    Rect(const SPoint & pos, const UPoint & size)
      : m_position(pos), m_size(size)
    { }

    void IntersectWith(const Rect & other) ;

    bool IsEmpty() const
    {
      return m_size.IsZero();
    }

    bool Contains(const SPoint & point) {
      return point.BoundedBy(m_position, m_position + MakeSigned(m_size));
    }

    const SPoint & GetPosition() const { return m_position; }
    s32 GetX() const { return m_position.GetX(); }
    s32 GetY() const { return m_position.GetY(); }

    void SetPosition(const SPoint & newPos) { m_position = newPos; }
    void SetX(s32 newX) { m_position.SetX(newX); }
    void SetY(s32 newY) { m_position.SetY(newY); }

    const UPoint & GetSize() const { return m_size; }
    u32 GetWidth() const { return m_size.GetX(); }
    u32 GetHeight() const { return m_size.GetY(); }

    void SetSize(const UPoint & newSize) { m_size = newSize; }
    void SetWidth(u32 newWidth) { m_size.SetX(newWidth); }
    void SetHeight(u32 newHeight) { m_size.SetY(newHeight); }

    ////// Operator overloads

    Rect& operator=(const Rect & rhs)
    {
      m_position = rhs.m_position;
      m_size = rhs.m_size;
      return *this;
    }

    Rect& operator&=(const Rect & rhs)
    {
      IntersectWith(rhs);
      return *this;
    }

  };
} /* namespace MFM */

#endif /*RECT_H*/
