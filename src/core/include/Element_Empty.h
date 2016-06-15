/*                                              -*- mode:C++ -*-
  Element_Empty.h MFM representation of empty space
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
  \file Element_Empty.h MFM representation of empty space
  \author Trent R. Small.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ELEMENT_EMPTY_H
#define ELEMENT_EMPTY_H

#include "Element.h"
#include "itype.h"

namespace MFM
{

  template <class EC> class EventWindow; // FORWARD

  template <class EC>
  class Element_Empty : public Element<EC>
  {
    enum { EMPTY_VERSION = 1 };

    // Extract short names for parameter types
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    enum { EMPTY_ELEMENT_TYPE = T::ATOM_EMPTY_TYPE };

  public:
    static Element_Empty THE_INSTANCE;

    Element_Empty() : Element<EC>(MFM_UUID_FOR("Empty", EMPTY_VERSION))
    {
      Element<EC>::AllocateEmptyType(); // A special method just for Empty!
      Element<EC>::SetAtomicSymbol("E");
      Element<EC>::SetName("Empty");
    }

    virtual u32 GetEventWindowBoundary() const
    {
      return 0;
    }

    virtual u32 PercentMovable(const T& you,
                               const T& me, const SPoint& offset) const
    {
      return 100;
    }

    virtual const char* GetDescription() const
    {
      return "The Element which represents empty space. Events do not occur for this Element.";
    }

    virtual u32 GetElementColor() const
    {
      return 0x00000000;                    // Not black.. transparent
    }

    virtual void Behavior(EventWindow<EC>& window) const
    { }
  };

  template <class EC>
  Element_Empty<EC> Element_Empty<EC>::THE_INSTANCE;
}

#endif /* ELEMENT_EMPTY_H */
