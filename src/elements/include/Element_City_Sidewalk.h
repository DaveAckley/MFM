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
  class Element_City_Sidewalk : public Element<CC>
  {
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum
    {
      R = P::EVENT_WINDOW_RADIUS,
      BITS = P::BITS_PER_ATOM
    };

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

    virtual const char* GetDescription() const
    {
      return "Building border element for city simulation.";
    }

    virtual u32 Diffusability(EventWindow<CC> & ew, SPoint nowAt, SPoint maybeAt) const
    {
      return nowAt.Equals(maybeAt)?Element<CC>::COMPLETE_DIFFUSABILITY:0;
    }

    virtual void Behavior(EventWindow<CC>& window) const
    {
    }
  };

  template <class CC>
  Element_City_Sidewalk<CC> Element_City_Sidewalk<CC>::THE_INSTANCE;
}

#endif /* ELEMENT_CITY_SIDEWALK_H */
