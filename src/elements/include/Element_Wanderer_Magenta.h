/*                                              -*- mode:C++ -*-
  Element_Wanderer_Magenta.h Child Two in AbstractElement tutorial
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
  \file Element_Wanderer_Magenta.h Child Two in AbstractElement tutorial
  \author Trent R. Small.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ELEMENT_WANDERER_MAGENTA_H
#define ELEMENT_WANDERER_MAGENTA_H

#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "itype.h"
#include "P1Atom.h"
#include "AbstractElement_Wanderer.h"

namespace MFM
{

#define WANDERER_VERSION 1

  template <class CC>
  class Element_Wanderer_Magenta : public AbstractElement_Wanderer<CC>
  {
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum { R = P::EVENT_WINDOW_RADIUS };

   private:
    ElementParameterS32<CC> m_wanderDistance;

   public:
    static Element_Wanderer_Magenta THE_INSTANCE;

    Element_Wanderer_Magenta()
      : AbstractElement_Wanderer<CC>(MFM_UUID_FOR("WandererMagenta", WANDERER_VERSION)),
        m_wanderDistance(this, "magentaWanderDistance", "Wander Distance",
                         "Wander Distance", 0, 1, R)
    {
      Element<CC>::SetAtomicSymbol("Wm");
      Element<CC>::SetName("WandererMagenta");
    }

    virtual u32 PercentMovable(const T& you,
                               const T& me, const SPoint& offset) const
    {
      return 100;
    }

    virtual u32 DefaultPhysicsColor() const
    {
      return 0xffff00ff;
    }

    virtual u32 DefaultLowlightColor() const
    {
      return 0xff7f007f;
    }

   protected:
    virtual u32 GetWanderDistance() const
    {
      return (u32)m_wanderDistance.GetValue();
    }
  };

  template <class CC>
  Element_Wanderer_Magenta<CC> Element_Wanderer_Magenta<CC>::THE_INSTANCE;

}

#endif /* ELEMENT_WANDERER_MAGENTA_H */
