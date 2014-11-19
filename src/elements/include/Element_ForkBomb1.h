/*                                              -*- mode:C++ -*-
  Element_ForkBomb1.h Light cone painting element
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
  \file Element_ForkBomb1.h Light cone painting element
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ELEMENT_FORKBOMB1_H
#define ELEMENT_FORKBOMB1_H

#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "AbstractElement_ForkBomb.h"
#include "itype.h"

namespace MFM
{

#define FORKBOMB1_VERSION 1

  template <class CC>
  class Element_ForkBomb1 : public AbstractElement_ForkBomb<CC>
  {
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum { R = P::EVENT_WINDOW_RADIUS };

  private:

    ElementParameterS32<CC> m_bombRange;

  public:

    static Element_ForkBomb1 THE_INSTANCE;

    virtual u32 GetBombRange() const
    {
      return (u32) m_bombRange.GetValue();
    }

    Element_ForkBomb1() :
      AbstractElement_ForkBomb<CC>(MFM_UUID_FOR("BombRed", FORKBOMB1_VERSION)),
      m_bombRange(this, "range", "Bomb Radius",
                  "Radius of copying during each bomb event",
                  0, 2, 4/*, 1*/)
    {
      Element<CC>::SetAtomicSymbol("Br");
      Element<CC>::SetName("Red Fork Bomb");
    }

    virtual u32 LocalPhysicsColor(const T& atom, u32 selector) const
    {
      return 0xffaa0000;
    }

  };

  template <class CC>
  Element_ForkBomb1<CC> Element_ForkBomb1<CC>::THE_INSTANCE;

}

#endif /* ELEMENT_FORKBOMB1_H */
