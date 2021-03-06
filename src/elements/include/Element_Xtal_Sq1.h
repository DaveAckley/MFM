/*                                              -*- mode:C++ -*-
  Element_Xtal_Sq1.h Square, spacing 1 crystal
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
  \file Element_Xtal_Sq1.h Square, spacing 1 crystal
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ELEMENT_XTAL_SQ1_H
#define ELEMENT_XTAL_SQ1_H

#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "AbstractElement_Xtal.h"
#include "itype.h"

namespace MFM
{

  template <class EC>
  class Element_Xtal_Sq1 : public AbstractElement_Xtal<EC>
  {
  public:
    virtual u32 GetTypeFromThisElement() const {
      return 0xCE11;
    }
    
    enum {  ELT_VERSION = 2 };

    // Extract short names for parameter types
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    enum { R = EC::EVENT_WINDOW_RADIUS };

  private:

  public:

    static Element_Xtal_Sq1 THE_INSTANCE;

    Element_Xtal_Sq1() : AbstractElement_Xtal<EC>(MFM_UUID_FOR("XtalSq1", ELT_VERSION))
    {
      Element<EC>::SetAtomicSymbol("Xs");
      Element<EC>::SetName("Square crystal spacing 1");
    }

    virtual u32 GetElementColor() const
    {
      return 0xff11bb33;
    }

    typedef typename MFM::AbstractElement_Xtal<EC>::XtalSites XtalSites;

    virtual u32 GetSymI(T &atom, EventWindow<EC>& window) const
    {
      return (u32) PSYM_NORMAL;
    }

    virtual void GetSites(T & atom, XtalSites & sites, EventWindow<EC>& window) const
    {
      static XtalSites isites;
      static bool initted = false;
      if (!initted)
      {
        for (s32 x = -R; x <= R; ++x)
        {
          for (s32 y = -R; y <= R; ++y)
          {
            if (!(x&1) && !(y&1))
            {
              SPoint c(x,y);
              if (c.GetManhattanLength() <= R)
              {
                this->WriteBit(isites, c, true);
              }
            }
          }
        }
        initted = true;
      }
      sites = isites;
    }

  };

  template <class EC>
  Element_Xtal_Sq1<EC> Element_Xtal_Sq1<EC>::THE_INSTANCE;

}

#endif /* ELEMENT_XTAL_SQ1_H */
