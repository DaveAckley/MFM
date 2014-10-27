/*                                              -*- mode:C++ -*-
  Element_ForkBomb3.h Randomized light cone painting element
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
  \file Element_ForkBomb3.h Randomized light cone painter
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ELEMENT_FORKBOMB3_H
#define ELEMENT_FORKBOMB3_H

#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "AbstractElement_ForkBomb.h"
#include "itype.h"

namespace MFM
{

#define FORKBOMB3_VERSION 1

  template <class CC>
  class Element_ForkBomb3 : public AbstractElement_ForkBomb<CC>
  {
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum { R = P::EVENT_WINDOW_RADIUS };

  private:

    ElementParameterS32<CC> m_bombCount;

  public:

    static Element_ForkBomb3 THE_INSTANCE;

    virtual u32 GetBombRange() const
    {
      return (u32) m_bombCount.GetValue(); // Um, not really
    }

    Element_ForkBomb3() :
      AbstractElement_ForkBomb<CC>(MFM_UUID_FOR("BombYellow", FORKBOMB3_VERSION)),
      m_bombCount(this, "count", "Bomb Count",
                  "Number of bombs randomly dropped each bomb event",
                  0, 10, 30/*, 1*/)

    {
      Element<CC>::SetAtomicSymbol("By");
      Element<CC>::SetName("Yellow Fork Bomb");
    }

    virtual u32 LocalPhysicsColor(const T& atom, u32 selector) const
    {
      return 0xffcccc00;
    }

    virtual void Behavior(EventWindow<CC>& window) const
    {
      Random & random = window.GetRandom();
      const MDist<R> & md = MDist<R>::get();
      const u32 loIdx = md.GetFirstIndex(1);
      const u32 hiIdx = md.GetLastIndex(R);
      for (u32 i = 0; i < (u32) m_bombCount.GetValue(); ++i)
      {
        u32 idx = random.Between(loIdx,hiIdx);
        window.SetRelativeAtom(md.GetPoint(idx), window.GetCenterAtom());
      }
    }

  };

  template <class CC>
  Element_ForkBomb3<CC> Element_ForkBomb3<CC>::THE_INSTANCE;

}

#endif /* ELEMENT_FORKBOMB3_H */
