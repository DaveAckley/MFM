/*                                              -*- mode:C++ -*-
  Element_Xtal_General.h General-purpose configurable evolvable crystal
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
  \file Element_Xtal_General.h Right-tipped, knight's-J-move crystal
  \author David H. Ackley.
  \author Trent R. Small.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ELEMENT_XTAL_GENERAL_H
#define ELEMENT_XTAL_GENERAL_H

#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "AbstractElement_Xtal.h"
#include "itype.h"

namespace MFM
{

#define ELT_VERSION 1

  template <class CC>
  class Element_Xtal_General : public AbstractElement_Xtal<CC>
  {
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;

    enum {
      R = P::EVENT_WINDOW_RADIUS,
      BITS = P::BITS_PER_ATOM,

      SITES = EVENT_WINDOW_SITES(R)
    };

    typedef BitField<BitVector<BITS>, VD::BITS, SITES, BITS - SITES> AFSites;

    ElementParameterNeighborhood<CC,SITES> m_neighborhood;

  public:

    static Element_Xtal_General THE_INSTANCE;
    static const u32 TYPE()
    {
      return THE_INSTANCE.GetType();
    }

    Element_Xtal_General() :
      AbstractElement_Xtal<CC>(MFM_UUID_FOR("XtalGen", ELT_VERSION)),
      m_neighborhood(this, "neighborhood", "Neighborhood",
                     "Newly-drawn Xg's will have this neighborhood.",0)
    {
      Element<CC>::SetAtomicSymbol("Xg");
      Element<CC>::SetName("General crystal");
    }

    virtual const T & GetDefaultAtom() const
    {
      static T defaultAtom(TYPE(),0,0,0);
      AFSites::WriteLong(this->GetBits(defaultAtom), m_neighborhood.GetValue());

      return defaultAtom;
    }

    virtual u32 GetSymI(T &atom, EventWindow<CC>& window) const
    {
      return (u32) PSYM_NORMAL;
    }

    virtual u32 DefaultPhysicsColor() const
    {
      return 0xff11bb33;
    }

    virtual u32 DefaultLowlightColor() const
    {
      return 0xff0c8023;
    }

    /*
    virtual u32 LocalPhysicsColor(const T& atom, u32 selector) const
    {
      return DefaultPhysicsColor();
    }
    */

    typedef typename MFM::AbstractElement_Xtal<CC>::XtalSites XtalSites;

    /**
     * XtalGens are only truly the same if their sites are identical.
     */
    virtual bool IsSameXtal(T & self, const T & otherAtom, EventWindow<CC>& window) const
    {
      return
        AFSites::ReadLong(this->GetBits(self)) == AFSites::ReadLong(this->GetBits(otherAtom));
    }

    virtual void GetSites(T & atom, XtalSites & sites, EventWindow<CC>& window) const
    {
      u64 bits = AFSites::ReadLong(this->GetBits(atom));
      sites.WriteLong(0, SITES, bits);
    }

  };

  template <class CC>
  Element_Xtal_General<CC> Element_Xtal_General<CC>::THE_INSTANCE;

}

#endif /* ELEMENT_XTAL_GENERAL_H */
