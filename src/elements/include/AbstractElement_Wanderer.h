/*                                              -*- mode:C++ -*-
  AbstractElement_Wanderer.h Abstract Element tutorial base class
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
  \file AbstractElement_Wanderer.h Abstract Element tutorial base class
  \author Trent R. Small.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ABSTRACTELEMENT_WANDERER_H
#define ABSTRACTELEMENT_WANDERER_H

#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "itype.h"
#include "P1Atom.h"

namespace MFM
{
  template <class CC>
  class AbstractElement_Wanderer : public Element<CC>
  {
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum { R = P::EVENT_WINDOW_RADIUS };

   protected:
    virtual u32 GetWanderDistance() const = 0;

   public:

    AbstractElement_Wanderer(UUID u)
      : Element<CC>(u)
    { }

    virtual u32 PercentMovable(const T& you,
                               const T& me, const SPoint& offset) const
    {
      return 100;
    }

    virtual void Behavior(EventWindow<CC>& window) const
    {
      SPoint wanderPt;
      Random& rand = window.GetRandom();
      Dir d = (Dir)rand.Create(Dirs::DIR_COUNT);

      Dirs::FillDir(wanderPt, d);

      wanderPt *= Dirs::IsCorner(d) ? (GetWanderDistance() / 2) : GetWanderDistance();

      if(window.IsLiveSite(wanderPt))
      {
        if(window.GetRelativeAtom(wanderPt).GetType() ==
           Element_Empty<CC>::THE_INSTANCE.GetType())
        {
          window.SwapAtoms(wanderPt, SPoint(0, 0));
        }
      }
    }
  };
}

#endif /* ABSTRACTELEMENT_WANDERER_H */
