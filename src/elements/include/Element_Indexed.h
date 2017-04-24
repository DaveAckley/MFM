/*                                              -*- mode:C++ -*-
  Element_Indexed.h An element with a 'unique' id
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
  \file Element_Indexed.h An element with a 'unique' id
  \author Trent R. Small.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ELEMENT_INDEXED_H
#define ELEMENT_INDEXED_H

#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "itype.h"
#include "P3Atom.h"
#include "ColorMap.h"

namespace MFM
{

  /**
   * An element with an index number to help in tracking.
   */
  template <class EC>
  class Element_Indexed : public Element<EC>
  {
    // Extract short names for parameter types
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;

  public:
    typedef BitVector<AC::BITS_PER_ATOM> BVA;
    enum {
      ELT_VERSION = 2,
      BITS_IN_INDEX = 24
    };
    typedef BitField<BVA, VD::U32, BITS_IN_INDEX, (AC::ATOM_FIRST_STATE_BIT+7)/8*8> AFIndex;

    static Element_Indexed THE_INSTANCE;
    static const u32 TYPE() {
      return THE_INSTANCE.GetType();
    }

    Element_Indexed() : Element<EC>(MFM_UUID_FOR("Indexed", ELT_VERSION))
    {
      Element<EC>::SetAtomicSymbol("Ix");
      Element<EC>::SetName("Indexed");
    }

    virtual u32 PercentMovable(const T& you,
                               const T& me, const SPoint& offset) const
    {
      return 100;
    }

    virtual u32 GetElementColor() const
    {
      return 0xffff00ff;
    }

    virtual u32 GetAtomColor(const ElementTable<EC> & et, const UlamClassRegistry<EC> & ucr, const T& atom, u32 selector) const
    {
      switch (selector) {
      case 1: {
        u32 idx = AFIndex::Read(this->GetBits(atom));
        return ColorMap_DIV6_RdYlGn::THE_INSTANCE.
          GetSelectedColor(idx%6, 0, 5, 0xffff0000);
      }
      default:
        return GetElementColor();
      }
      return 0x0;
    }

    virtual const char* GetDescription() const
    {
      return "An Element wih an index number.";
    }

    virtual void Behavior(EventWindow<EC>& window) const
    {
      T self = window.GetCenterAtomSym();

      LOG.Debug("IDX#%3d@%p(%2d,%2d)",
                AFIndex::Read(this->GetBits(self)),
                (void*) &window.GetTile(),
                window.GetCenterInTile().GetX(),
                window.GetCenterInTile().GetY());

      window.Diffuse();
    }

    virtual const T & GetDefaultAtom() const
    {
      static T defaultAtom(TYPE(), 0, 0, BITS_IN_INDEX);
      static u32 index = 0;
      AFIndex::Write(this->GetBits(defaultAtom), ++index);
      return defaultAtom;
    }

  };

  template <class EC>
  Element_Indexed<EC> Element_Indexed<EC>::THE_INSTANCE;

}

#endif /* ELEMENT_INDEXED_H */
