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

    s32 m_bombRange;

  public:
    virtual u32 GetConfigurableCount() const
    {
      return 1;
    }

    virtual s32* GetConfigurableParameter(u32 index)
    {
      if(index)
      {
        FAIL(ILLEGAL_ARGUMENT);
      }
      return &m_bombRange;
    }

    virtual s32 GetConfigurableParameterValue(u32 index) const
    {
      if(index)
      {
        FAIL(ILLEGAL_ARGUMENT);
      }
      return m_bombRange;
    }

    virtual void SetConfigurableParameterValue(u32 index, s32 value)
    {
      if(index)
      {
        FAIL(ILLEGAL_ARGUMENT);
      }
      m_bombRange = value;
    }

    virtual s32 GetMinimumValue(u32 index) const
    {
      return 0;
    }

    virtual s32 GetMaximumValue(u32 index) const
    {
      return 4;
    }

    virtual const char* GetConfigurableName(u32 index) const
    {
      return "Bomb Radius";
    }

    static Element_ForkBomb3 THE_INSTANCE;

    s32 * GetBombRangePtr()
    {
      return (s32 *) &m_bombRange;
    }

    virtual u32 GetBombRange() const
    {
      return m_bombRange;
    }

    Element_ForkBomb3() : AbstractElement_ForkBomb<CC>(MFM_UUID_FOR("BombYellow", FORKBOMB3_VERSION))
    {
      m_bombRange = 10;
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
      const MDist<R> md = MDist<R>::get();
      const u32 loIdx = md.GetFirstIndex(1);
      const u32 hiIdx = md.GetLastIndex(R);
      for (u32 i = 0; i < m_bombRange; ++i)
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
