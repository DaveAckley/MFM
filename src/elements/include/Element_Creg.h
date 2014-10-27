/*                                              -*- mode:C++ -*-
  Element_Creg.h Configurable density regulator
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
  \file   Element_Creg.h Tutorial template for a new Element
  \author Trent R. Small.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ELEMENT_CREG_H
#define ELEMENT_CREG_H

#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "itype.h"

namespace MFM
{

#define CREG_VERSION 1

  template <class CC>
  class Element_Creg : public Element<CC>
  {
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum { R = P::EVENT_WINDOW_RADIUS };

   private:
    ElementParameterS32<CC> m_targetDensity;

   public:
    static Element_Creg<CC> THE_INSTANCE;

    Element_Creg()
      : Element<CC>(MFM_UUID_FOR("Creg", CREG_VERSION)),
        m_targetDensity(this, "density", "Target Density",
                        "The Creg will try to fill this many spots in its event "
                        "window with other Creg.", 0, 3, 41/*, 1*/)

    {
      Element<CC>::SetAtomicSymbol("Cd");
      Element<CC>::SetName("Creg");
    }

    virtual u32 PercentMovable(const T& you,
                               const T& me, const SPoint& offset) const
    {
      return 100;
    }

    virtual u32 DefaultPhysicsColor() const
    {
      return 0xffff8300;
    }

    virtual u32 DefaultLowlightColor() const
    {
      return 0xff774100;
    }

    virtual const char* GetDescription() const
    {
      return "The Configurable Regulator regulates the density of the universe based "
             "on a configurable parameter.";
    }

    virtual void Behavior(EventWindow<CC>& window) const
    {
      const MDist<R> & md = MDist<R>::get();
      Random& rand = window.GetRandom();

      SPoint cregAtom;
      s32 cregCount = 0;
      SPoint nonCregAtom;
      s32 nonCregCount = 0;

      for(u32 i = md.GetFirstIndex(0); i <= md.GetLastIndex(R); i++)
      {
        const SPoint& rel = md.GetPoint(i);
        const T& atom = window.GetRelativeAtom(rel);

        if(Atom<CC>::IsType(atom, Element_Wall<CC>::THE_INSTANCE.GetType()))
        {
          continue; /* We're not going to destroy walls.*/
        }

        if(Atom<CC>::IsType(atom, Element<CC>::GetType()))
        {
          cregCount++;
          if(rand.OneIn(cregCount))
          {
            cregAtom = rel;
          }
        }
        else
        {
          nonCregCount++;
          if(rand.OneIn(nonCregCount))
          {
            nonCregAtom = rel;
          }
        }
      }

      if(cregCount > (u32) m_targetDensity.GetValue())
      {
        window.SetRelativeAtom(cregAtom, Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom());
      }
      else if(cregCount < m_targetDensity.GetValue())
      {
        window.SetRelativeAtom(nonCregAtom, Element_Creg<CC>::THE_INSTANCE.GetDefaultAtom());
      }
    }
  };

  template <class CC>
  Element_Creg<CC> Element_Creg<CC>::THE_INSTANCE;

}

#endif /* ELEMENT_CREG_H */
