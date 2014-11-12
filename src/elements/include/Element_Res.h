/*                                              -*- mode:C++ -*-
  Element_Res.h Basic resource element
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
  \file Element_Res.h Basic resource element
  \author Trent R. Small.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ELEMENT_RES_H
#define ELEMENT_RES_H

#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "itype.h"
#include "P1Atom.h"

namespace MFM
{

#define RES_VERSION 1

  /**
   * A basic 'Resource' element, created in controlled amounts by DReg
   * and available for use by other Elements that wish to have
   * controlled growth.
   */
  template <class CC>
  class Element_Res : public Element<CC>
  {
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;

  public:
    static Element_Res THE_INSTANCE;
    static const u32 TYPE() {
      return THE_INSTANCE.GetType();
    }

    Element_Res() : Element<CC>(MFM_UUID_FOR("Res", RES_VERSION))
    {
      Element<CC>::SetAtomicSymbol("R");
      Element<CC>::SetName("Res");
    }

    virtual u32 PercentMovable(const T& you,
                               const T& me, const SPoint& offset) const
    {
      return 100;
    }

    virtual u32 DefaultPhysicsColor() const
    {
      return 0xff676700;
    }

    virtual u32 DefaultLowlightColor() const
    {
      return 0xff333300;
    }

    virtual const char* GetDescription() const
    {
      return "The Element which represents a universal resource. Other atoms may freely "
             "convert these atoms into something else without disturbing the DREG's "
             "regulatory properties.";
    }

    virtual void Behavior(EventWindow<CC>& window) const
    {
      window.Diffuse();
    }
  };

  template <class CC>
  Element_Res<CC> Element_Res<CC>::THE_INSTANCE;

}

#endif /* ELEMENT_RES_H */
