/*                                              -*- mode:C++ -*-
  AbstractElement_ForkBomb.h Light cone painting element
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
  \file AbstractElement_ForkBomb.h Light cone painting element
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ABSTRACTELEMENT_FORKBOMB_H
#define ABSTRACTELEMENT_FORKBOMB_H

#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "itype.h"

namespace MFM
{
  template <class EC>
  class AbstractElement_ForkBomb : public Element<EC>
  {
    enum { FORKBOMBBASE_VERSION = 2 };

    // Extract short names for parameter types
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    enum { R = EC::EVENT_WINDOW_RADIUS };

  private:

  public:

    /**
     * From 0 (no forking) up to EVENT_WINDOW_RADIUS (maximum speed forking)
     */
    virtual u32 GetBombRange() const = 0;

    AbstractElement_ForkBomb(const UUID & uuid) : Element<EC>(uuid)
    {
    }

    virtual u32 PercentMovable(const T& you,
                               const T& me, const SPoint& offset) const
    {
      return 100;
    }

    virtual u32 GetElementColor() const = 0;

    virtual void Behavior(EventWindow<EC>& window) const
    {
      const MDist<R> & md = MDist<R>::get();
      u32 range = GetBombRange();
      if (range > R)
        range = R;
      for (u32 idx = md.GetFirstIndex(1); idx <= md.GetLastIndex(range); ++idx)
      {
        const SPoint rel = md.GetPoint(idx);
        window.SetRelativeAtomSym(rel, window.GetCenterAtomSym());
      }
    }
  };
}

#endif /* ABSTRACTELEMENT_FORKBOMB_H */
