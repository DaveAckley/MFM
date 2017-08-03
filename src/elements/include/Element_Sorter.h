/*                                              -*- mode:C++ -*-
  Element_Sorter.h DHS Specific data sorting element
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
  \file Element_Sorter.h DHS Specific data sorting element
  \author Trent R. Small.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ELEMENT_SORTER_H
#define ELEMENT_SORTER_H

#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "ElementUtils.h"
#include "Element_Res.h"  /* For Element_Res::TYPE */
#include "Element_Data.h"  /* For Element_Data::TYPE */
#include "ColorMap.h"
#include "itype.h"
#include "WindowScanner.h"

namespace MFM
{

  /** Sorter element from the Demon Horde Sort (C++ implementation) */
  template <class EC>
  class Element_Sorter : public Element<EC>
  {
    enum {  SORTER_VERSION = 2 };

    // Extract short names for parameter types
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    enum { R = EC::EVENT_WINDOW_RADIUS };

   public:
    static Element_Sorter THE_INSTANCE;
    static const u32 TYPE()
    {
      return THE_INSTANCE.GetType();
    }

    static const u32 STATE_THRESHOLD_IDX = 0;    // First bit in state
    static const u32 STATE_THRESHOLD_LEN = 32;
    static const u32 STATE_BITS = STATE_THRESHOLD_LEN;

    static const SPoint m_southeastSubWindow[4];
    static const SPoint m_northeastSubWindow[4];
    static const SPoint m_southwestSubWindow[4];
    static const SPoint m_northwestSubWindow[4];

    inline u32 GetDataType() const
    {
      return Element_Data<EC>::THE_INSTANCE.GetType();
    }

    inline u32 GetEmptyType() const
    {
      return Element_Empty<EC>::THE_INSTANCE.GetType();
    }

    Element_Sorter() :
      Element<EC>(MFM_UUID_FOR("Sorter", SORTER_VERSION))
    {
      Element<EC>::SetAtomicSymbol("Sr");
      Element<EC>::SetName("Sorter");
    }

    u32 GetThreshold(const T &atom, u32 badType) const
    {
      if (!Atom<AC>::IsType(atom,TYPE()))
      {
        return badType;
      }
      return atom.GetStateField(STATE_THRESHOLD_IDX,STATE_THRESHOLD_LEN);
    }

    bool SetThreshold(T &atom, u32 value) const
    {
      if (!Atom<AC>::IsType(atom,TYPE()))
      {
        return false;
      }
      atom.SetStateField(STATE_THRESHOLD_IDX,STATE_THRESHOLD_LEN,value);
      return true;
    }

    virtual const T & GetDefaultAtom() const
    {
      static T defaultAtom(TYPE(),0,0,STATE_BITS);
      return defaultAtom;
    }

    virtual u32 PercentMovable(const T& you,
                               const T& me, const SPoint& offset) const
    {
      return 100;
    }

    virtual u32 GetElementColor() const
    {
      return 0xffff0000;
    }

    virtual const char* GetDescription() const
    {
      return "A sorting \"demon\", which sorts DATA atoms based on their values and "
             "a value held within this Sorter as well. See the \"Demon Hoard Sort\" "
             "algorithm for details.";
    }

    virtual void AppendDescription(const T& atom, ByteSink& desc) const
    {
      u32 threshold = GetThreshold(atom, 0);

      if(threshold)
      {
        desc.Printf("Threshold: %d", threshold);
      }
      else
      {
        desc.Printf("Threshold: INVALID");
      }
    }

    virtual u32 GetAtomColor(const ElementTable<EC> & et, const UlamClassRegistry<EC> & ucr, const T& atom, u32 selector) const
    {
      switch (selector)
      {
      case 1:
        return ColorMap_CubeHelixRev::THE_INSTANCE.
          GetInterpolatedColor(GetThreshold(atom,0),DATA_MINVAL,DATA_MAXVAL,0xffff0000);

      default:
        return this->GetElementColor();
      }
    }

    virtual void Behavior(EventWindow<EC>& window) const
    {
      Random & random = window.GetRandom();
      SPoint reproducePt;
      T self = window.GetCenterAtomSym();
      WindowScanner<EC> scanner(window);
      if(scanner.FindRandomInVonNeumann(Element_Res<EC>::THE_INSTANCE.GetType(),
                                        reproducePt) > 0)
      {
        window.SetRelativeAtomSym(reproducePt, self);
      }

      bool movingUp = random.CreateBool();

      SPoint src, dst;
      bool foundPts;

      for(s32 i = 0; i < 2; i++)
      {
        foundPts = false;
        if(movingUp)
        {
          if(scanner.FindRandomAtomsInSubWindows(2,
                     &src, GetDataType(), m_southeastSubWindow, 4,
                     &dst, GetEmptyType(), m_northwestSubWindow, 4))
          {
            foundPts = true;
          }
        }
        else
        {
          if(scanner.FindRandomAtomsInSubWindows(2,
                     &src, GetDataType(), m_northeastSubWindow, 4,
                     &dst, GetEmptyType(), m_southwestSubWindow, 4))
          {
            foundPts = true;
          }
        }

        if(foundPts)
        {
          u32 threshold = GetThreshold(window.GetCenterAtomSym(),0);
          u32 datum = Element_Data<EC>::THE_INSTANCE.GetDatum(window.GetRelativeAtomSym(src),0);
          bool cmp = (movingUp && (datum < threshold)) || (!movingUp && (datum > threshold));

          if(cmp)
          {
            SetThreshold(self, datum);
            window.SetCenterAtomSym(self);
            window.SwapAtomsSym(src, dst);
            break;
          }
        }

        movingUp = !movingUp;

      }
      window.Diffuse();
    }
  };

  template <class EC>
  Element_Sorter<EC> Element_Sorter<EC>::THE_INSTANCE;

  template <class EC>
  const SPoint Element_Sorter<EC>::m_southeastSubWindow[4] =
  {
    SPoint(1,1),SPoint(1,2),SPoint(2,1),SPoint(2,2)
  };

  template <class EC>
  const SPoint Element_Sorter<EC>::m_northeastSubWindow[4] =
  {
    SPoint(1,-1),SPoint(1,-2),SPoint(2,-1),SPoint(2,-2)
  };

  template <class EC>
  const SPoint Element_Sorter<EC>::m_northwestSubWindow[4] =
  {
    SPoint(-1,-1),SPoint(-1,-2),SPoint(-2,-1),SPoint(-2,-2)
  };

  template <class EC>
  const SPoint Element_Sorter<EC>::m_southwestSubWindow[4] =
  {
    SPoint(-1,1),SPoint(-1,2),SPoint(-2,1),SPoint(-2,2)
  };
}
#endif /* ELEMENT_SORTER_H */
