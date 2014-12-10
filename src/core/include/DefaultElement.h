/*                                              -*- mode:C++ -*-
  DefaultElement.h An extensible but concrete element
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
  \file DefaultElement.h An extensible but concrete element
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef DEFAULTELEMENT_H
#define DEFAULTELEMENT_H

#include "Element.h"

namespace MFM
{
  /**
   * A DefaultElement is a concrete element, primarily for use by culam.
   */
  template <class CC>
  class DefaultElement : public Element<CC>
  {
    typedef typename CC::ATOM_TYPE T;

  public:
    DefaultElement(const UUID & uuid) : Element<CC>(uuid)
    { }

    /**
     * Destroys this DefaultElement.
     */
    virtual ~DefaultElement()
    { }

    virtual void Behavior(EventWindow<CC>& window) const
    {
      // No behavior by default
    }

    virtual u32 DefaultPhysicsColor() const
    {
      // Colored white by default
      return 0xffffffff;
    }

    virtual u32 PercentMovable(const T& you,
                               const T& me, const SPoint& offset) const
    {
      return 0;
    }


  };
}

#endif /* DEFAULTELEMENT_H */
