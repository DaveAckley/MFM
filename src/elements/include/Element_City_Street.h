/*                                              -*- mode:C++ -*-
  Element_City_Street.h Street element for city simulation
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
  \file Element_City_Street.h Street element for city simulation
  \author Trent R. Small.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ELEMENT_CITY_STREET_H
#define ELEMENT_CITY_STREET_H

#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "itype.h"
#include "Dirs.h"
#include "itype.h"
#include "P3Atom.h"

namespace MFM
{

#define STREET_VERSION 2

  template<class EC>
  class Element_City_Street : public Element<EC>
  {
    // Extract short names for parameter types
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    enum
    {
      R = EC::EVENT_WINDOW_RADIUS,
      BITS = AC::BITS_PER_ATOM,

      STREET_DIR_LEN = 4,
      STREET_DIR_POS = BITS - STREET_DIR_LEN - 1
    };

    typedef BitField<BitVector<BITS>, VD::U32, STREET_DIR_LEN, STREET_DIR_POS> AFStreetDirection;

   private:
    ElementParameterS32<EC> m_intersectionOdds;

   public:
    static Element_City_Street THE_INSTANCE;

    static const u32 TYPE()
    {
      return THE_INSTANCE.GetType();
    }

    Dir GetDirection(const T& us) const
    {
      return (Dir)AFStreetDirection::Read(this->GetBits(us));
    }

    void SetDirection(T& us, const Dir dir) const
    {
      if(dir & 1)
      {
        FAIL(ILLEGAL_ARGUMENT); /* No diagonals */
      }
      AFStreetDirection::Write(this->GetBits(us), (u32)dir);
    }

    Element_City_Street() :
      Element<EC>(MFM_UUID_FOR("CityStreet", STREET_VERSION)),
      m_intersectionOdds(this, "intersectionOdds",
                           "Intersection Odds",
                           "Odds of creating an intersection", 1, 10, 100)
    {
      Element<EC>::SetAtomicSymbol("St");
      Element<EC>::SetName("City Street");
    }

    virtual const T& GetDefaultAtom() const
    {
      static T defaultAtom(TYPE(), 0, 0, 0);
      this->SetDirection(defaultAtom, Dirs::NORTH);

      return defaultAtom;
    }

    virtual u32 PercentMovable(const T& you, const T& me, const SPoint& offset) const
    {
      return 0;
    }

    virtual u32 DefaultPhysicsColor() const
    {
      return 0xff202020;
    }

    virtual u32 DefaultLowlightColor() const
    {
      return 0xff101010;
    }

    virtual const char* GetDescription() const
    {
      return "Basic element for city simulation.";
    }

    virtual u32 Diffusability(EventWindow<EC> & ew, SPoint nowAt, SPoint maybeAt) const
    {
      return nowAt.Equals(maybeAt)?COMPLETE_DIFFUSABILITY:0;
    }

   private:

    void FillIfNotType(EventWindow<EC>& window, SPoint& offset, u32 type, const T& atom) const
    {
      if(window.GetRelativeAtomDirect(offset).GetType() != type)
      {
        window.SetRelativeAtomDirect(offset, atom);
      }
    }

    void FillIfType(EventWindow<EC>& window, SPoint& offset, u32 type, const T& atom) const
    {
      if(window.GetRelativeAtomDirect(offset).GetType() == type)
      {
        window.SetRelativeAtomDirect(offset, atom);
      }
    }

    bool MooreBorder(EventWindow<EC>& window, u32 type) const
    {
      SPoint pt;
      for(u32 i = 0; i < Dirs::DIR_COUNT; i++)
      {
        Dirs::FillDir(pt, (Dir)i);
        if(window.GetRelativeAtomDirect(pt).GetType() == type)
        {
          return true;
        }
      }
      return false;
    }

    bool CanSeeElementOfType(EventWindow<EC>& window, const u32 type) const
    {
      const MDist<R>& md = MDist<R>::get();
      for(u32 i = md.GetFirstIndex(1); i <= md.GetLastIndex(R); i++)
      {
        if(window.GetRelativeAtomDirect(md.GetPoint(i)).GetType() == type)
        {
          return true;
        }
      }
      return false;
    }

    void DoStreetAndSidewalk(EventWindow<EC>& window, Dir d) const;

    inline const T& GetIntersection() const;

    inline u32 IntersectionType() const;

    inline const T& GetSidewalk() const;

    inline u32 SidewalkType() const;

    inline u32 BuildingType() const;

   public:
    virtual void Behavior(EventWindow<EC>& window) const
    {
      SPoint offset;
      Dir d = this->GetDirection(window.GetCenterAtomDirect());

      DoStreetAndSidewalk(window, d);
      DoStreetAndSidewalk(window, Dirs::OppositeDir(d));
    }
  };

  template <class EC>
  Element_City_Street<EC> Element_City_Street<EC>::THE_INSTANCE;
}

#include "Element_City_Street.tcc"

#endif /* ELEMENT_CITY_STREET_H */
