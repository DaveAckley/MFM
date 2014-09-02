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

      SITES = EVENT_WINDOW_SITES(R),
      SITE_PART1_POS = P3Atom<P>::P3_STATE_BITS_POS,
      SITE_PART1_LEN = SITES / 2,
      SITE_PART2_POS = SITE_PART1_POS + SITE_PART1_LEN,
      SITE_PART2_LEN = SITES - SITE_PART1_LEN
    };

    typedef BitField<BitVector<BITS>, SITE_PART1_LEN, SITE_PART1_POS> AFSitePart1;
    typedef BitField<BitVector<BITS>, SITE_PART2_LEN, SITE_PART2_POS> AFSitePart2;

    typedef BitField<BitVector<SITES>, SITE_PART1_LEN, 0> XSSitePart1;
    typedef BitField<BitVector<SITES>, SITE_PART2_LEN, SITE_PART1_LEN> XSSitePart2;

  public:

    static Element_Xtal_General THE_INSTANCE;
    static const u32 TYPE()
    {
      return THE_INSTANCE.GetType();
    }

    Element_Xtal_General() : AbstractElement_Xtal<CC>(MFM_UUID_FOR("XtalGen", ELT_VERSION))
    {
      Element<CC>::SetAtomicSymbol("Xg");
      Element<CC>::SetName("General crystal");
    }

    virtual const T & GetDefaultAtom() const
    {
      static T defaultAtom(TYPE(),0,0,0);
      bool initted = false;
      if (!initted)
      {
        const SPoint ones[] = {
          SPoint(0,0),
          SPoint(4,0),
          SPoint(-4,0),
          SPoint(0,4),
          SPoint(0,-4)
        };
        const MDist<R> md = MDist<R>::get();
        for (u32 i = 0; i < sizeof(ones)/sizeof(ones[0]); ++i)
        {
          s32 idx = md.FromPoint(ones[i], R);
          if (idx < 0)
          {
            FAIL(ILLEGAL_STATE);
          }
          this->GetBits(defaultAtom).SetBit(idx + SITE_PART1_POS);
        }
        initted = true;
      }
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

    virtual u32 LocalPhysicsColor(const T& atom, u32 selector) const
    {
      return DefaultPhysicsColor();
    }

    typedef typename MFM::AbstractElement_Xtal<CC>::XtalSites XtalSites;

    static u32 mutate(Random & random, u32 val, u32 len)
    {
      /*
      while (random.OneIn(10))
      {
        val ^= 1<<random.Create(len);
      }
      */
      return val;
    }

    /**
     * XtalGens are only truly the same if their sites are identical.
     */
    virtual bool IsSameXtal(T & self, const T & otherAtom, EventWindow<CC>& window) const
    {
      return
        (AFSitePart1::Read(this->GetBits(self)) == AFSitePart1::Read(this->GetBits(otherAtom)))
        &&
        (AFSitePart2::Read(this->GetBits(self)) == AFSitePart2::Read(this->GetBits(otherAtom)));
    }

    virtual void GetSites(T & atom, XtalSites & sites, EventWindow<CC>& window) const
    {
      Random & random = window.GetRandom();
      u32 buf, mbuf;

      buf = AFSitePart1::Read(this->GetBits(atom));
      mbuf = mutate(random, buf, SITE_PART2_LEN);
      if (buf != mbuf)
      {
        buf = mbuf;
        AFSitePart1::Write(this->GetBits(atom), buf);
      }
      XSSitePart1::Write(sites, buf);

      buf = AFSitePart2::Read(this->GetBits(atom));
      mbuf = mutate(random, buf, SITE_PART2_LEN);
      if (buf != mbuf)
      {
        buf = mbuf;
        AFSitePart2::Write(this->GetBits(atom), buf);
      }

      XSSitePart2::Write(sites, buf);
    }

  };

  template <class CC>
  Element_Xtal_General<CC> Element_Xtal_General<CC>::THE_INSTANCE;

}

#endif /* ELEMENT_XTAL_GENERAL_H */
