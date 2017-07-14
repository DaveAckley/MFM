/*                                              -*- mode:C++ -*-
  Element_City_Car.h Vehicle for city simulation
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
  \file Element_City_Car.h Vehicle for city simulation
  \author Trent R. Small.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ELEMENT_CITY_CAR_H
#define ELEMENT_CITY_CAR_H

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

#define CAR_VERSION 2

  template<class EC>
  class Element_City_Car : public Element<EC>
  {
   private:
    // Extract short names for parameter types
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    enum
    {
      R = EC::EVENT_WINDOW_RADIUS,
      BITS = AC::BITS_PER_ATOM,

      DEST_TYPE_POS = T::ATOM_FIRST_STATE_BIT,
      DEST_TYPE_LEN = CityConstants::CITY_BUILDING_COUNT_LOG2,

      GAS_POS = DEST_TYPE_POS + DEST_TYPE_LEN,
      GAS_LEN = 8,

      MAX_GAS = 1 << GAS_LEN - 1,

      DIRECTION_POS = GAS_POS + GAS_LEN,
      DIRECTION_LEN = 4
    };

    typedef BitField<BitVector<BITS>, VD::U32, DEST_TYPE_LEN, DEST_TYPE_POS> AFDestType;
    typedef BitField<BitVector<BITS>, VD::U32, GAS_LEN, GAS_POS> AFGas;
    typedef BitField<BitVector<BITS>, VD::U32, DIRECTION_LEN, DIRECTION_POS> AFDirection;

   public:
    static Element_City_Car THE_INSTANCE;

    void SetDestType(T& us, u32 val) const
    {
      AFDestType::Write(this->GetBits(us), val);
    }

    u32 GetDestType(const T& us) const
    {
      return AFDestType::Read(this->GetBits(us));
    }

    void SetDirection(T& us, Dir val) const
    {
      AFDirection::Write(this->GetBits(us), (u32)val);
    }

    Dir GetDirection(const T& us) const
    {
      return (Dir)AFDirection::Read(this->GetBits(us));
    }

    void FillTank(T& us) const
    {
      AFGas::Write(this->GetBits(us), MAX_GAS);
    }

    u32 GetGas(const T& us) const
    {
      return AFGas::Read(this->GetBits(us));
    }

    /**
     * Decrements gas, returns true if out of gas.
     */
    bool UseGas(T& us) const
    {
      AFGas::Write(this->GetBits(us), GetGas(us) - 1);
      return GetGas(us) == 0;
    }

    static const u32 TYPE()
    {
      return THE_INSTANCE.GetType();
    }

    Element_City_Car() :
      Element<EC>(MFM_UUID_FOR("CityCar", CAR_VERSION))
    {
      Element<EC>::SetAtomicSymbol("Cr");
      Element<EC>::SetName("City Car");
    }

    virtual const T& GetDefaultAtom() const
    {
      static T defaultAtom(TYPE(), 0, 0, 0);

      FillTank(defaultAtom);

      return defaultAtom;
    }

    virtual u32 PercentMovable(const T& you, const T& me, const SPoint& offset) const
    {
      return 0;
    }

    virtual u32 GetElementColor() const
    {
      return 0xffffff00;
    }

    virtual u32 GetAtomColor(const ElementTable<EC> & et, const UlamClassRegistry<EC> & ucr, const T& atom, u32 selector) const
    {
      switch(GetDestType(atom))
      {
      case 0x0: return 0xffff0000;
      case 0x1: return 0xff00ff00;
      case 0x2: return 0xff0000ff;
      case 0x3: return 0xff00ffff;
      case 0x4: return 0xffff00ff;
      case 0x5: return 0xffffff00;
      case 0x6: return 0xff7f0000;
      case 0x7: return 0xff007f00;
      case 0x8: return 0xff00007f;
      case 0x9: return 0xff007f7f;
      case 0xa: return 0xff7f007f;
      case 0xb: return 0xff7f7f00;
      case 0xc: return 0xffffffff;
      case 0xd: return 0xff400040;
      case 0xe: return 0xff7f7f7f;
      case 0xf: return 0xff404040;
      default:  return 0xff800000;
      }
    }

    virtual const char* GetDescription() const
    {
      return "City vehicle";
    }

    virtual u32 Diffusability(EventWindow<EC> & ew, SPoint nowAt, SPoint maybeAt) const
    {
      return nowAt.Equals(maybeAt)?COMPLETE_DIFFUSABILITY:0;
    }

    u32 GetSidewalkType() const;

    u32 GetIntersectionType() const;

    u32 GetStreetType() const;

    u32 GetBuildingType() const;

    void ReplaceCenterWithStreet(EventWindow<EC>& window) const;

   public:
    virtual void Behavior(EventWindow<EC>& window) const;

  };

  template <class EC>
  Element_City_Car<EC> Element_City_Car<EC>::THE_INSTANCE;
}

#include "Element_City_Car.tcc"

#endif /* ELEMENT_CITY_CAR_H */
