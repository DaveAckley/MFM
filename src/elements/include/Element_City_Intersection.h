/*                                              -*- mode:C++ -*-
  Element_City_Intersection.h Street hub for city simulation
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
  \file Element_City_Intersection.h Street hub for city simulation
  \author Trent R. Small.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ELEMENT_CITY_INTERSECTION_H
#define ELEMENT_CITY_INTERSECTION_H

#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "itype.h"
#include "Dirs.h"
#include "itype.h"
#include "P3Atom.h"

namespace MFM
{

#define CITY_VERSION 1

  template<class CC>
  class Element_City_Intersection : public Element<CC>
  {
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum
    {
      R = P::EVENT_WINDOW_RADIUS,
      BITS = P::BITS_PER_ATOM,

      INITIALIZED_LEN = 1,
      INITIALIZED_POS = BITS - INITIALIZED_LEN - 1
    };

    typedef BitField<BitVector<BITS>, INITIALIZED_LEN, INITIALIZED_POS> AFInitBits;

   private:
    ElementParameterS32<CC> m_streetCreateOdds;

   public:
    static Element_City_Intersection THE_INSTANCE;

    static const u32 TYPE()
    {
      return THE_INSTANCE.GetType();
    }

    bool IsInitialized(const T& us) const
    {
      return AFInitBits::Read(this->GetBits(us)) > 0;
    }

    void Uninitialize(T& us) const
    {
      AFInitBits::Write(this->GetBits(us), 0);
    }

    void SetInitialized(T& us) const
    {
      AFInitBits::Write(this->GetBits(us), 1);
    }

    Element_City_Intersection() :
      Element<CC>(MFM_UUID_FOR("CityIntersection", CITY_VERSION)),
      m_streetCreateOdds(this, "streetCreateOdds",
                         "Street Odds",
                         "Odds of creating a street", 1, 2, 10, 1)
    {
      Element<CC>::SetAtomicSymbol("In");
      Element<CC>::SetName("City Intersection");
    }

    virtual const T& GetDefaultAtom() const
    {

      static T defaultAtom(TYPE(), 0, 0, 0);
      Uninitialize(defaultAtom);

      return defaultAtom;

    }

    virtual u32 PercentMovable(const T& you, const T& me, const SPoint& offset) const
    {
      return 0;
    }

    virtual u32 DefaultPhysicsColor() const
    {
      return 0xff303030;
    }

    virtual u32 DefaultLowlightColor() const
    {
      return 0xff181818;
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

   private:
    void InitializeIntersection(T& atom, EventWindow<CC>& window) const;

   public:
    virtual void Behavior(EventWindow<CC>& window) const
    {
      if(!IsInitialized(window.GetCenterAtom()))
      {
        T newAtom = window.GetCenterAtom();
        InitializeIntersection(newAtom, window);
        SetInitialized(newAtom);

        window.SetCenterAtom(newAtom);
      }
    }
  };

  template <class CC>
  Element_City_Intersection<CC> Element_City_Intersection<CC>::THE_INSTANCE;
}

#include "Element_City_Intersection.tcc"

#endif /* ELEMENT_CITY_INTERSECTION_H */
