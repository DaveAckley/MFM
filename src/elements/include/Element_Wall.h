/*                                              -*- mode:C++ -*-
  Element_Wall.h Basic immovable element
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
  \file Element_Wall.h Basic immovable element
  \author Trent R. Small.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ELEMENT_WALL_H
#define ELEMENT_WALL_H

#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "itype.h"

namespace MFM
{

  template <class EC>
  class Element_Wall : public Element<EC>
  {
    enum {  WALL_VERSION = 2 };

    // Short names for params
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    enum { R = EC::EVENT_WINDOW_RADIUS };

  public:
    static Element_Wall THE_INSTANCE;
    static const u32 TYPE() {
      return THE_INSTANCE.GetType();
    }

    Element_Wall() : Element<EC>(MFM_UUID_FOR("Wall", WALL_VERSION))
    {
      Element<EC>::SetAtomicSymbol("W");
      Element<EC>::SetName("Wall");
    }

    virtual const T & GetDefaultAtom() const
    {
      static T defaultAtom(TYPE(),0,0,0);
      return defaultAtom;
    }

    virtual u32 DefaultPhysicsColor() const
    {
      return 0xffffffff;
    }

    /*
    virtual u32 DefaultLowlightColor() const
    {
      return 0xff202020;
    }
    */

    virtual const char* GetDescription() const
    {
      return "The Element which represents an immovable wall. Other Elements may "
             "modify the position or existance of this Wall, but most Elements like "
             "DREG will not.";
    }

    virtual u32 Diffusability(EventWindow<EC> & ew, SPoint nowAt, SPoint maybeAt) const
    {
      return nowAt.Equals(maybeAt)?COMPLETE_DIFFUSABILITY:0;
    }

    virtual u32 PercentMovable(const T& you,
                               const T& me, const SPoint& offset) const
    {
      return 0;
    }

    virtual void Behavior(EventWindow<EC>& window) const
    { }
  };

  template <class EC>
  Element_Wall<EC> Element_Wall<EC>::THE_INSTANCE;
}

#endif /* ELEMENT_WALL_H */
