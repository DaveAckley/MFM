/*                                              -*- mode:C++ -*-
  Element_AntiForkBomb.h Inflammation-based forkbomb suppression
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
  \file Element_AntiForkBomb.h Inflammation-based forkbomb suppression
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ELEMENT_ANTIFORKBOMB_H
#define ELEMENT_ANTIFORKBOMB_H

#include "Element.h"
#include "P3Atom.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "AbstractElement_ForkBomb.h"
#include "itype.h"

namespace MFM
{

#define ANTIFORKBOMB_VERSION 1

  template <class CC>
  class Element_AntiForkBomb : public Element<CC>
  {
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum {
      R = P::EVENT_WINDOW_RADIUS,
      BITS = P::BITS_PER_ATOM,

      //////
      // Element state fields

      INFLAMMATION_POS = T::ATOM_FIRST_STATE_BIT,
      INFLAMMATION_LEN = 2

    };

  private:
    /**
     * If fewer than this Af's in the event window (excluding self),
     * spawn
     */
    u32 m_minDensity;

    /**
     * If more than this Af's in the event window (excluding self),
     * suicide
     */
    u32 m_maxDensity;

  public:

    typedef BitField<BitVector<BITS>,VD::U32,INFLAMMATION_LEN,INFLAMMATION_POS> AFInflammationLevel;

    static Element_AntiForkBomb THE_INSTANCE;

    Element_AntiForkBomb() : Element<CC>(MFM_UUID_FOR("AntiFork", ANTIFORKBOMB_VERSION))
    {
      m_minDensity = 1;
      m_maxDensity = 3;
      Element<CC>::SetAtomicSymbol("Af");
      Element<CC>::SetName("Anti-Fork Bomb");
    }

    virtual u32 LocalPhysicsColor(const T& atom, u32 selector) const
    {
      u32 level = AFInflammationLevel::Read(atom);  // 0..3
      if (level == 0)
      {
        return 0xff333333; // Grey20
      }

      const u32 LEVEL_INCREMENT = 50;
      return ((level*LEVEL_INCREMENT + (255-3*LEVEL_INCREMENT))<<8) | 0xff000000;
    }

    virtual void Behavior(EventWindow<CC>& window) const
    {
      Random & random = window.GetRandom();
      const u32 ourType = THE_INSTANCE.GetType();
      const MDist<R> & md = MDist<R>::get();

      T self = window.GetCenterAtom();
      u32 myInflammationLevel = AFInflammationLevel::Read(self);

      const u32 loIdx = md.GetFirstIndex(1);
      const u32 hiIdx = md.GetLastIndex(R);

      u32 usCount = 0;
      u32 maxInflammation = 0;
      u32 emptyCount = 0;
      SPoint randomEmpty;
      SPoint randomSelf;

      for (u32 i = loIdx; i <= hiIdx; ++i)
      {
        const SPoint rel = md.GetPoint(i);
        if (!window.IsLiveSite(rel))
        {
          continue;
        }
        const T & atom = window.GetRelativeAtom(rel);
        const u32 type = atom.GetType();
        const Element<CC> * elt = window.GetTile().GetElement(type);
        if (elt == &Element_Empty<CC>::THE_INSTANCE)
        {
          if (random.OneIn(++emptyCount))
          {
            randomEmpty = rel;
          }
        } else if (type == ourType)
        {
          u32 level = AFInflammationLevel::Read(atom);
          if (level > maxInflammation)
          {
            maxInflammation = level;
          }
          if (random.OneIn(++usCount))
          {
            randomSelf = rel;
          }
        } else if (dynamic_cast<const AbstractElement_ForkBomb<CC>*>(elt))
        {
          // We see a pathogen!  Danger danger!  Red alert!
          maxInflammation = 4;
          break;
        }
      }

      if (maxInflammation >= 3)
      {
        u32 inflammationLevel = maxInflammation - 1;

        T inflamedUs = window.GetCenterAtom();
        AFInflammationLevel::Write(inflamedUs, inflammationLevel);
        window.SetCenterAtom(inflamedUs);

        //DEFEND AT ALL COSTS!
        for (u32 i = loIdx; i <= hiIdx; ++i)
        {
          const SPoint rel = md.GetPoint(i);
          const T & atom = window.GetRelativeAtom(rel);
          const u32 type = atom.GetType();
          if (type != ourType || AFInflammationLevel::Read(atom) < inflammationLevel)
          {
            window.SetRelativeAtom(rel, inflamedUs);
          }
        }
      }
      else if (myInflammationLevel > 0)
      {
        AFInflammationLevel::Write(self, myInflammationLevel - 1);
        window.SetCenterAtom(self);
      }
      else if (usCount < m_minDensity)
      {
        //It's calm around here, but need more of us on patrol
        if (emptyCount > 0)
        {
          window.SetRelativeAtom(randomEmpty, THE_INSTANCE.GetDefaultAtom());
        }
      } else if (usCount > m_maxDensity)
      {
        //It's calm and there's too many of us.  I sacrifice myself for the team.
        window.SetCenterAtom(Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom());
      } else
      {
        //It's calm and we have reasonable density.  I will patrol.
        window.Diffuse();
      }
    }

    virtual u32 PercentMovable(const T& you,
                               const T& me, const SPoint& offset) const
    {
      return 100;
    }

    virtual u32 DefaultPhysicsColor() const
    {
      T temp;
      return LocalPhysicsColor(temp, 0);
    }

  };

  template <class CC>
  Element_AntiForkBomb<CC> Element_AntiForkBomb<CC>::THE_INSTANCE;

}

#endif /* ELEMENT_ANTIFORKBOMB_H */
