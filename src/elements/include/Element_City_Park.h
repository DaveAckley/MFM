/*                                              -*- mode:C++ -*-
  Element_City_Park.h City filler for surrounded sidewalks
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
  \file Element_City_Park.h City filler for surrounded sidewalks
  \author Trent R. Small.
  \date (C) 2014 All rights reserved.
  \lgpl
*/
#ifndef ELEMENT_CITY_PARK_H
#define ELEMENT_CITY_PARK_H

#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "itype.h"
#include "Dirs.h"
#include "itype.h"
#include "P3Atom.h"

namespace MFM
{

#define PARK_VERSION 2

  template<class EC>
  class Element_City_Park : public Element<EC>
  {
    // Extract short names for parameter types
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    enum
    {
      R = EC::EVENT_WINDOW_RADIUS,
      BITS = AC::BITS_PER_ATOM
    };

  public:
    static Element_City_Park THE_INSTANCE;

    static const u32 TYPE()
    {
      return THE_INSTANCE.GetType();
    }

  Element_City_Park() :
    Element<EC>(MFM_UUID_FOR("CityPark", PARK_VERSION))
    {
      Element<EC>::SetAtomicSymbol("Pk");
      Element<EC>::SetName("City Park");
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

    virtual const char* GetDescription() const
    {
      return "City filler for surrounded sidewalks";
    }

    virtual u32 Diffusability(EventWindow<EC> & ew, SPoint nowAt, SPoint maybeAt) const
    {
      return nowAt.Equals(maybeAt)?COMPLETE_DIFFUSABILITY:0;
    }

  public:
    virtual void Behavior(EventWindow<EC>& window) const
    { }
  };

  template <class EC>
  Element_City_Park<EC> Element_City_Park<EC>::THE_INSTANCE;
}

#endif /* ELEMENT_CITY_PARK_H */
