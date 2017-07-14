/*                                              -*- mode:C++ -*-
  Element_Data.h Basic integer holding element
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
  \file Element_Data.h Basic integer holding element
  \author Trent R. Small.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ELEMENT_DATA_H
#define ELEMENT_DATA_H

#include "Element.h"
#include "ColorMap.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "itype.h"

#define DATA_MAXVAL 1000000
#define DATA_MINVAL 1

namespace MFM
{

  template <class EC>
  class Element_Data : public Element<EC>
  {
    enum {  DATA_VERSION = 2 };

    // Extract short names for parameter types
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;

   public:
    Element_Data() : Element<EC>(MFM_UUID_FOR("Data", DATA_VERSION))
    {
      Element<EC>::SetAtomicSymbol("Dt");
      Element<EC>::SetName("Data");
    }

    static Element_Data THE_INSTANCE;
    static const u32 TYPE()
    {
      return THE_INSTANCE.GetType();
    }

    static const u32 STATE_DATA_IDX = 0;
    static const u32 STATE_DATA_LEN = 32;

    u32 GetDatum(const T &atom, u32 badType) const
    {
      if (!Atom<AC>::IsType(atom,TYPE()))
      {
        return badType;
      }
      return atom.GetStateField(STATE_DATA_IDX,STATE_DATA_LEN);
    }

    bool SetDatum(T &atom, u32 value) const
    {
      if (!Atom<AC>::IsType(atom,TYPE()))
      {
        return false;
      }
      atom.SetStateField(STATE_DATA_IDX,STATE_DATA_LEN,value);
      return true;
    }

    virtual void AppendDescription(const T& atom, ByteSink& desc) const
    {
      u32 datum = GetDatum(atom, 0);
      if(datum)
      {
        desc.Printf("Datum: %d", datum);
      }
      else
      {
        desc.Printf("Datum: INVALID");
      }
    }

    virtual const T & GetDefaultAtom() const
    {
      static T defaultAtom(TYPE(),0,0,STATE_DATA_LEN);
      static bool initted = false;
      if (!initted)
      {
        SetDatum(defaultAtom, (DATA_MAXVAL + DATA_MINVAL) / 2);
        initted = true;
      }
      return defaultAtom;
    }

    virtual u32 GetElementColor() const
    {
      return 0xff0000ff;
    }

    virtual u32 PercentMovable(const T& you, const T& me, const SPoint& offset) const
    {
      return 100;
    }

    virtual const char* GetDescription() const
    {
      return "An element designed simply to hold a 32-bit value, usually generated "
             "at random when created by an emitter atom.";
    }

    virtual u32 GetAtomColor(const ElementTable<EC> & et, const UlamClassRegistry<EC> & ucr, const T& atom, u32 selector) const
    {
      switch (selector)
      {
      case 1:
        return ColorMap_CubeHelixRev::THE_INSTANCE.
          GetInterpolatedColor(GetDatum(atom,0),DATA_MINVAL,DATA_MAXVAL,0xffff0000);
      default:
        return this->GetElementColor();
      }
    }

    virtual void Behavior(EventWindow<EC>& window) const
    {
      u32 val = GetDatum(window.GetCenterAtomSym(),-1);
      if (val < DATA_MINVAL || val > DATA_MAXVAL)
      {
        FAIL(ILLEGAL_STATE);
      }

      window.Diffuse();
    }
  };

  template <class EC>
  Element_Data<EC> Element_Data<EC>::THE_INSTANCE;

}

#endif /* ELEMENT_DATA_H */
