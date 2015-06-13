/*                                              -*- mode:C++ -*-
  Element_Dmover.h Directed mover Element
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
  \file   Element_Dmover.h Directed mover Element
  \author Dave Ackley
  \author Ezra Stallings
  \author Andres Ruiz Cardozo
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ELEMENT_DMOVER_H
#define ELEMENT_DMOVER_H

#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "Atom.h"
#include "itype.h"

namespace MFM
{

  template <class EC>
  class Element_Dmover : public Element<EC>
  {
    // Extract short names for parameter types
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    enum {
      R = EC::EVENT_WINDOW_RADIUS,
      ELT_VERSION = 3
    };

  private:

    ElementParameterS32<EC> m_searchRadius;

  public:

    static Element_Dmover<EC> THE_INSTANCE;

    Element_Dmover()
      : Element<EC>(MFM_UUID_FOR("Dmover", ELT_VERSION)),
        m_searchRadius(this, "radius", "Scan radius",
                       "Max radius to search for empty and non-empty sites.", 0, 1, R/*, 1*/)
    {
      Element<EC>::SetAtomicSymbol("Dm");
      Element<EC>::SetName("Dmover");
    }

    /*
       Set how likely your element is to be moved by another
       element. See Element.h for details.
    */
    virtual u32 PercentMovable(const T& you,
                               const T& me, const SPoint& offset) const
    {
      return 0;
    }

    /* This color will be the default rendering color for your element. */
    virtual u32 GetElementColor() const
    {
      return 0xffd2b486;
    }

    /*
      This is a short description of your element.
     */
    virtual const char* GetDescription() const
    {
      return "A Directed Mover moves move southern atoms to northern empty sites";
    }

    /*
      This method is executed every time an atom of your element is
      chosen for an event. See the tutorial in the wiki for further
      information.
     */
    virtual void Behavior(EventWindow<EC>& window) const
    {
      Random & random = window.GetRandom();
      const MDist<R> & md = MDist<R>::get();

      SPoint emptyRel;
      u32 emptyCount = 0;

      SPoint occupiedRel;
      u32 occupiedCount = 0;

      for (u32 idx = md.GetFirstIndex(1);
           idx <= md.GetLastIndex(m_searchRadius.GetValue());
           ++idx)
      {
        const SPoint rel = md.GetPoint(idx);
        if (!window.IsLiveSiteSym(rel))
        {
          continue;
        }
        T other = window.GetRelativeAtomSym(rel);
        u32 otherType = other.GetType();

        bool isOtherEmpty = Element_Empty<EC>::THE_INSTANCE.IsType(otherType);

        if(rel.GetY() < 0 && isOtherEmpty)  // negative Y is up
        {
          if (random.OneIn(++emptyCount))
          {
            emptyRel = rel;
          }
        }

        if (rel.GetY() > 0 && !isOtherEmpty)
        {
          if (random.OneIn(++occupiedCount))
          {
            occupiedRel = rel;
          }
        }
      }

      if (emptyCount > 0 && occupiedCount > 0)  // We have a move
      {
        window.SwapAtomsSym(emptyRel, occupiedRel);
      }
    }
  };

  /*
    The singleton instance of the Dmover element.
  */
  template <class EC>
  Element_Dmover<EC> Element_Dmover<EC>::THE_INSTANCE;

}

#endif /* ELEMENT_DMOVER_H */
