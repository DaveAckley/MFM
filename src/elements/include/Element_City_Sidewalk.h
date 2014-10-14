/*                                              -*- mode:C++ -*-
  Element_City_Sidewalk.h Building border for City simulation
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
  \file Element_City_Sidewalk.h Building border for City simulation
  \author Trent R. Small.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ELEMENT_CITY_SIDEWALK_H
#define ELEMENT_CITY_SIDEWALK_H

#include "CityConstants.h"
#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "itype.h"
#include "Dirs.h"
#include "itype.h"
#include "P3Atom.h"
#include "Element_City_Park.h"

namespace MFM
{
#define CITY_VERSION 1

  template<class CC>
  class Element_City_Sidewalk : public Element<CC>
  {
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;

    enum
    {
      R = P::EVENT_WINDOW_RADIUS,
      BITS = P::BITS_PER_ATOM,

      BUILDING_FLAG_POS = P3Atom<P>::ATOM_FIRST_STATE_BIT,
      BUILDING_FLAG_LEN = 1,

      BUILDING_TIMER_POS = BUILDING_FLAG_POS + BUILDING_FLAG_LEN,
      BUILDING_TIMER_LEN = 10,

      MAX_TIMER_VALUE = (1 << BUILDING_TIMER_LEN) - 1,

      BUILDING_DIST_POS = BUILDING_FLAG_POS + BUILDING_FLAG_LEN,
      BUILDING_DIST_LEN = CityConstants::CITY_BUILDING_COUNT * 2
    };

    typedef BitField<BitVector<BITS>, BUILDING_FLAG_LEN, BUILDING_FLAG_POS> AFBuildingFlag;
    typedef BitField<BitVector<BITS>, BUILDING_TIMER_LEN, BUILDING_TIMER_POS> AFBuildingTimer;
    typedef BitField<BitVector<BITS>, BUILDING_DIST_LEN, BUILDING_DIST_POS> AFBuildingDist;

    void SetReadyToBuild(T& us) const
    {
      AFBuildingFlag::Write(this->GetBits(us), 1);
      AFBuildingDist::Write(this->GetBits(us), 0);
    }

    bool IsReadyToBuild(const T& us) const
    {
      return AFBuildingFlag::Read(this->GetBits(us)) != 0;
    }

    u32 GetBuildingTimer(const T& us) const
    {
      return AFBuildingTimer::Read(this->GetBits(us));
    }

    void SetBuildingTimer(T& us, const u32 val) const
    {
      AFBuildingTimer::Write(this->GetBits(us), val);
    }

    void TickBuildingTimer(T& us) const
    {
      u32 timerVal = GetBuildingTimer(us);
      if(timerVal != MAX_TIMER_VALUE)
      {
        SetBuildingTimer(us, GetBuildingTimer(us) + 1);
      }
      else
      {
        SetReadyToBuild(us);
      }
    }

   public:
    static Element_City_Sidewalk THE_INSTANCE;

    static const u32 TYPE()
    {
      return THE_INSTANCE.GetType();
    }

    Element_City_Sidewalk() :
      Element<CC>(MFM_UUID_FOR("CitySidewalk", CITY_VERSION))
    {
      Element<CC>::SetAtomicSymbol("Sw");
      Element<CC>::SetName("City Sidewalk");
    }

    virtual u32 PercentMovable(const T& you, const T& me, const SPoint& offset) const
    {
      return 0;
    }

    virtual u32 DefaultPhysicsColor() const
    {
      return 0xff808080;
    }

    virtual u32 DefaultLowlightColor() const
    {
      return 0xff404040;
    }

    virtual u32 LocalPhysicsColor(const T& atom, u32 selector) const
    {
      if(IsReadyToBuild(atom))
      {
        return 0xff707070;
      }
      else
      {
        return Element<CC>::PhysicsColor();
      }
    }

    virtual const char* GetDescription() const
    {
      return "Building border element for city simulation.";
    }

    virtual u32 Diffusability(EventWindow<CC> & ew, SPoint nowAt, SPoint maybeAt) const
    {
      return nowAt.Equals(maybeAt)?Element<CC>::COMPLETE_DIFFUSABILITY:0;
    }

    void DoTimerBehavior(EventWindow<CC>& window) const
    {
      T newMe = window.GetCenterAtom();
      TickBuildingTimer(newMe);

      window.SetCenterAtom(newMe);
    }

    void DoBuildingBehavior(EventWindow<CC>& window) const;

    void DoParkBehavior(EventWindow<CC>& window) const
    {
      const u32 types[] =
      {
        TYPE(),
        Element_City_Park<CC>::THE_INSTANCE.GetType()
      };
      if(IsMooreSurroundedBy(window, types, sizeof(types)/sizeof(u32)))
      {
        window.SetCenterAtom(Element_City_Park<CC>::THE_INSTANCE.GetDefaultAtom());
      }
    }

    bool CanSeeElementOfType(EventWindow<CC>& window, const u32 type, u32 radius) const
    {
      MDist<R>& md = MDist<R>::get();
      for(u32 i = md.GetFirstIndex(1); i <= md.GetLastIndex(radius); i++)
      {
        if(window.GetRelativeAtom(md.GetPoint(i)).GetType() == type)
        {
          return true;
        }
      }
      return false;
    }


    bool IsMooreSurroundedBy(EventWindow<CC>& window, const u32* types, u32 typeCount) const
    {
      MDist<R>& md = MDist<R>::get();

      for(u32 i = md.GetFirstIndex(1); i <= md.GetLastIndex(1); i++)
      {
        SPoint pt = md.GetPoint(i);
        bool hasOne = false;
        for(u32 j = 0; j < typeCount; j++)
        {
          if(window.GetRelativeAtom(pt).GetType() == types[j])
          {
            hasOne = true;
          }
        }
        if(!hasOne)
        {
          return false;
        }
      }
      return true;
    }

    virtual void Behavior(EventWindow<CC>& window) const
    {
      DoParkBehavior(window);

      if(IsReadyToBuild(window.GetCenterAtom()))
      {
        DoBuildingBehavior(window);
      }
      else
      {
        DoTimerBehavior(window);
      }
    }
  };

  template <class CC>
  Element_City_Sidewalk<CC> Element_City_Sidewalk<CC>::THE_INSTANCE;
}

#include "Element_City_Sidewalk.tcc"

#endif /* ELEMENT_CITY_SIDEWALK_H */
