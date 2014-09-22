/*                                              -*- mode:C++ -*-
  Element_CheckerForkRed.h Basic Element exhibiting a circular dependency
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
  \file   Element_CheckerForkRed.h Basic Element exhibiting a circular dependency
  \author Trent R. Small.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ELEMENT_CHECKERFORKRED_H
#define ELEMENT_CHECKERFORKRED_H

#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "itype.h"

namespace MFM
{

  #define CHECKERFORK_VERSION 1

  template <class CC>
  class Element_CheckerForkRed : public Element<CC>
  {
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum { R = P::EVENT_WINDOW_RADIUS };

  public:

    static Element_CheckerForkRed<CC> THE_INSTANCE;

    Element_CheckerForkRed()
      : Element<CC>(MFM_UUID_FOR("CheckerForkRed", CHECKERFORK_VERSION))
    {
      Element<CC>::SetAtomicSymbol("Cr");
      Element<CC>::SetName("CheckerForkRed");
    }

    virtual u32 PercentMovable(const T& you,
                               const T& me, const SPoint& offset) const
    {
      return 100;
    }

    virtual u32 DefaultPhysicsColor() const
    {
      return 0xffffa500;
    }

    virtual u32 DefaultLowlightColor() const
    {
      return 0xff7f5200;
    }

    virtual const char* GetDescription() const
    {
      return "Red Circular Dependency tutorial Element";
    }

    virtual void Behavior(EventWindow<CC>& window) const;
  };

  template <class CC>
  Element_CheckerForkRed<CC> Element_CheckerForkRed<CC>::THE_INSTANCE;
}

#include "Element_CheckerForkRed.tcc"

#endif /* ELEMENT_CHECKERFORKRED_H */
