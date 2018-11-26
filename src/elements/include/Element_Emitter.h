/*                                              -*- mode:C++ -*-
  Element_Emitter.h Basic data emitting element
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
  \file Element_Emitter.h Basic data emitting element
  \author Trent R. Small.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ELEMENT_EMITTER_H
#define ELEMENT_EMITTER_H

#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "Element_Empty.h"
#include "Element_Data.h"
#include "AbstractElement_Reprovert.h"
#include "itype.h"

namespace MFM
{

#define DATA_CREATE_PER_1000 150

  template <class EC>
  class Element_Emitter : public AbstractElement_Reprovert<EC>
  {
  public:
    virtual u32 GetTypeFromThisElement() const {
      return 0xCE05;
    }
    
    enum { EMITTER_VERSION = 2 };

    // Extract short names for parameter types
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    enum { R = EC::EVENT_WINDOW_RADIUS };

  public:
    // Element Data Slot names
    enum {
      DATUMS_EMITTED_SLOT,
      DATUMS_REJECTED_SLOT,
      DATA_SLOT_COUNT
    };

    static Element_Emitter THE_INSTANCE;

    virtual T BuildDefaultAtom() const {
      T defaultAtom(this->GetType(), 0, 0, AbstractElement_Reprovert<EC>::STATE_BITS);
      this->SetGap(defaultAtom,2);
      return defaultAtom;
    }

    Element_Emitter() :
      AbstractElement_Reprovert<EC>(MFM_UUID_FOR("Emitter", EMITTER_VERSION))
    {
      Element<EC>::SetAtomicSymbol("Em");
      Element<EC>::SetName("Emitter");
    }

    virtual u32 PercentMovable(const T& you, const T& me, const SPoint& offset) const
    {
      return 0;
    }

    virtual u32 GetElementColor() const
    {
      return 0xff808080;
    }

    virtual const char* GetDescription() const
    {
      return "This vertically-reproducing Element emits DATA atoms with randomly generated "
             "payloads.";
    }

    virtual void Behavior(EventWindow<EC>& window) const
    {
      Random & random = window.GetRandom();

      this->ReproduceVertically(window);

      if(random.OddsOf(DATA_CREATE_PER_1000,1000))
      {
#if 0 //XXX Fri Jan 30 22:12:52 2015 EDS unreimplemented in v3
        Tile<EC> & tile = window.GetTile();
        ElementTable<EC> & et = tile.GetElementTable();

        u64 * datap = et.GetDataAndRegister(this->GetType(), DATA_SLOT_COUNT);
        ++datap[DATUMS_EMITTED_SLOT];                  // Count emission attempts
#endif

        // Pick random nearest empty, if any
        const MDist<R> & md = MDist<R>::get();
        for (u32 ring = 1; ring <= 2; ++ring)
        {
          u32 emptiesFound = 0;
          SPoint emptyPoint;
          for (u32 idx = md.GetFirstIndex(ring); idx <= md.GetLastIndex(ring); ++idx)
          {
            const SPoint sp = md.GetPoint(idx);
            if (!window.IsLiveSiteSym(sp))
            {
              continue;
            }
            const T other = window.GetRelativeAtomSym(sp);
            const u32 otherType = other.GetType();
            bool isEmpty = otherType == Element_Empty<EC>::THE_INSTANCE.GetType();
            if (isEmpty && random.OneIn(++emptiesFound))
              emptyPoint = sp;
          }
          if (emptiesFound > 0)
          {
            T atom = Element_Data<EC>::THE_INSTANCE.GetDefaultAtom();
            Element_Data<EC>::THE_INSTANCE.SetDatum(atom,random.Between(DATA_MINVAL, DATA_MAXVAL));
            window.SetRelativeAtomSym(emptyPoint, atom);
            return;
          }
        }

        //XXX        ++datap[DATUMS_REJECTED_SLOT];  // Opps, no room at the inn
      }
    }
  };

  template <class EC>
  Element_Emitter<EC> Element_Emitter<EC>::THE_INSTANCE;
}

#endif /* ELEMENT_EMITTER_H */
