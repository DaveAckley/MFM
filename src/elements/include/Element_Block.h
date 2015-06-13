/*                                              -*- mode:C++ -*-
  Element_Block.h Basic non-diffusing non-behaving lump
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
  \file Element_Block.h A non-diffusing, non-behaving, lump of stuff
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ELEMENT_BLOCK_H
#define ELEMENT_BLOCK_H

#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "itype.h"
#include "Atom.h"

namespace MFM
{

  template <class EC>
  class Element_Block : public Element<EC>
  {
    // Short names for params
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    enum { R = EC::EVENT_WINDOW_RADIUS };

  public:
    enum {
      BLOCK_VERSION = 2
    };

    static Element_Block THE_INSTANCE;
    static const u32 TYPE()
    {
      return THE_INSTANCE.GetType();
    }

    Element_Block() : Element<EC>(MFM_UUID_FOR("Block", BLOCK_VERSION))
    {
      Element<EC>::SetAtomicSymbol("B");
      Element<EC>::SetName("Block");
    }

    virtual const T & GetDefaultAtom() const
    {
      static T defaultAtom(TYPE(),0,0,0);
      return defaultAtom;
    }

    virtual u32 GetElementColor() const
    {
      return 0xff00ff00;
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
    {}
  };

  template <class EC>
  Element_Block<EC> Element_Block<EC>::THE_INSTANCE;
}

#endif /* ELEMENT_BLOCK_H */
