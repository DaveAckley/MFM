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

#define STREET_VERSION 1

  template<class CC>
  class Element_City_Street : public Element<CC>
  {
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum
    {
      R = P::EVENT_WINDOW_RADIUS,
      BITS = P::BITS_PER_ATOM,

      STREET_DIR_LEN = 4,
      STREET_DIR_POS = BITS - STREET_DIR_LEN - 1
    };

    typedef BitField<BitVector<BITS>, STREET_DIR_LEN, STREET_DIR_POS> AFStreetDirection;

   private:
    ElementParameterS32<CC> m_intersectionOdds;

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
      AFStreetDirection::Write(this->GetBits(us), (u32)dir);
    }

    Element_City_Street() :
      Element<CC>(MFM_UUID_FOR("CityStreet", STREET_VERSION)),
      m_intersectionOdds(this, "intersectionOdds",
                           "Intersection Odds",
                           "Odds of creating an intersection", 1, 50, 500, 10)
    {
      Element<CC>::SetAtomicSymbol("St");
      Element<CC>::SetName("City Street");
    }

    virtual const T& GetDefaultAtom() const
    {
      static T defaultAtom(TYPE(), 0, 0, 0);
      this->SetDirection(defaultAtom, Dirs::NORTHEAST);

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

    virtual u32 Diffusability(EventWindow<CC> & ew, SPoint nowAt, SPoint maybeAt) const
    {
      return nowAt.Equals(maybeAt)?Element<CC>::COMPLETE_DIFFUSABILITY:0;
    }

   private:

    void FillIfNotType(EventWindow<CC>& window, SPoint& offset, u32 type, const T& atom) const
    {
      if(window.GetRelativeAtom(offset).GetType() != type)
      {
        window.SetRelativeAtom(offset, atom);
      }
    }

    void FillIfType(EventWindow<CC>& window, SPoint& offset, u32 type, const T& atom) const
    {
      if(window.GetRelativeAtom(offset).GetType() == type)
      {
        window.SetRelativeAtom(offset, atom);
      }
    }

    bool MooreBorder(EventWindow<CC>& window, u32 type) const
    {
      SPoint pt;
      for(u32 i = 0; i < Dirs::DIR_COUNT; i++)
      {
        Dirs::FillDir(pt, (Dir)i);
        if(window.GetRelativeAtom(pt).GetType() == type)
        {
          return true;
        }
      }
      return false;
    }

    void DoStreetAndSidewalk(EventWindow<CC>& window, Dir d) const;

   public:
    virtual void Behavior(EventWindow<CC>& window) const
    {
      SPoint offset;
      Dir d = this->GetDirection(window.GetCenterAtom());

      DoStreetAndSidewalk(window, d);
      DoStreetAndSidewalk(window, Dirs::OppositeDir(d));
    }
  };

  template <class CC>
  Element_City_Street<CC> Element_City_Street<CC>::THE_INSTANCE;
}

#include "Element_City_Street.tcc"

#endif /* ELEMENT_CITY_STREET_H */
