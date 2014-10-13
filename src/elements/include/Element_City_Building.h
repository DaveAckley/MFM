/*                                              -*- mode:C++ -*-
  Element_City_Building.h Vehicle spawn and consumpition point
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
  \file Element_City_Building.h Vehicle spawn and consumpition point
  \author Trent R. Small.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ELEMENT_CITY_BUILDING_H
#define ELEMENT_CITY_BUILDING_H

#include "CityConstants.h"
#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "itype.h"
#include "Dirs.h"
#include "itype.h"
#include "P3Atom.h"

namespace MFM
{

#define BUILDING_VERSION 1

  template<class CC>
  class Element_City_Building : public Element<CC>
  {
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum
    {
      R = P::EVENT_WINDOW_RADIUS,
      BITS = P::BITS_PER_ATOM,

      SUB_TYPE_POS = P3Atom<P>::P3_STATE_BITS_POS,
      SUB_TYPE_LEN = CityConstants::CITY_BUILDING_COUNT_LOG2,

      INITTED_POS = SUB_TYPE_POS + SUB_TYPE_LEN,
      INITTED_LEN = 1,

      AREA_INDEX_POS = INITTED_POS + INITTED_LEN,
      AREA_INDEX_LEN = 6,

      MAX_AREA_POS = AREA_INDEX_POS + AREA_INDEX_LEN,
      MAX_AREA_LEN = 6,

      MIN_AREA = 3
    };

    typedef BitField<BitVector<BITS>, SUB_TYPE_LEN, SUB_TYPE_POS> AFSubType;
    typedef BitField<BitVector<BITS>, INITTED_LEN, INITTED_POS> AFInitted;
    typedef BitField<BitVector<BITS>, MAX_AREA_LEN, MAX_AREA_POS> AFMaxArea;
    typedef BitField<BitVector<BITS>, AREA_INDEX_LEN, AREA_INDEX_POS> AFAreaIndex;

   public:
    static Element_City_Building THE_INSTANCE;

    void SetAreaIndex(T& us, u32 val) const
    {
      AFAreaIndex::Write(this->GetBits(us), val);
    }

    void GetAreaIndex(const T& us) const
    {
      return AFAreaIndex::Read(this->GetBits(us));
    }

    void SetMaxArea(T& us, u32 val) const
    {
      AFMaxArea::Write(this->GetBits(us), val);
    }

    void GetMaxArea(const T& us) const
    {
      return AFMaxArea::Read(this->GetBits(us));
    }

    void Initialize(T& us, Random& rand) const
    {
      SetInitted(us, true);
      SetAreaIndex(us, 0);
      u32 maxArea = rand.Create(1 << MAX_AREA_LEN);
      if(maxArea < MIN_AREA)
      {
        maxArea = MIN_AREA;
      }
      SetMaxArea(us, maxArea);
      SetSubType(us, rand.Create(CityConstants::CITY_BUILDING_COUNT));
    }

    bool IsInitted(const T& us) const
    {
      return AFInitted::Read(this->GetBits(us));
    }

    void SetInitted(T& us, bool val) const
    {
      AFInitted::Write(this->GetBits(us), val);
    }

    u32 GetSubType(const T& us) const
    {
      return AFSubType::Read(this->GetBits(us));
    }

    void SetSubType(T& us, u32 val) const
    {
      AFSubType::Write(this->GetBits(us), val);
    }

    void SetRandSubtype(T& us, Random& rand) const
    {
      SetSubType(us, rand.Create(CityConstants::CITY_BUILDING_COUNT));
    }

    static const u32 TYPE()
    {
      return THE_INSTANCE.GetType();
    }

    Element_City_Building() :
      Element<CC>(MFM_UUID_FOR("CityBuilding", BUILDING_VERSION))
    {
      Element<CC>::SetAtomicSymbol("Bd");
      Element<CC>::SetName("City Building");
    }

    virtual const T& GetDefaultAtom() const
    {
      static T defaultAtom(TYPE(), 0, 0, 0);

      return defaultAtom;
    }

    virtual u32 PercentMovable(const T& you, const T& me, const SPoint& offset) const
    {
      return 0;
    }

    virtual u32 DefaultPhysicsColor() const
    {
      return 0xff85C98B;
    }

    virtual u32 DefaultLowlightColor() const
    {
      return 0xff426344;
    }

    virtual u32 LocalPhysicsColor(const T& atom, u32 selector) const
    {
      switch(GetSubType(atom))
      {
      case 0x00: return 0xffff0000;
      default:   return 0xff800000;
      }
    }

    virtual const char* GetDescription() const
    {
      return "City filler for surrounded sidewalks";
    }

    virtual u32 Diffusability(EventWindow<CC> & ew, SPoint nowAt, SPoint maybeAt) const
    {
      return nowAt.Equals(maybeAt)?Element<CC>::COMPLETE_DIFFUSABILITY:0;
    }

   public:
    virtual void Behavior(EventWindow<CC>& window) const
    {
      if(!IsInitted(window.GetCenterAtom()))
      {
        T me = window.GetCenterAtom();
        Initialize(me, window.GetRandom());
        window.SetCenterAtom(me);
      }
      else
      {

      }
    }
  };

  template <class CC>
  Element_City_Building<CC> Element_City_Building<CC>::THE_INSTANCE;
}

#endif /* ELEMENT_CITY_BUILDING_H */
