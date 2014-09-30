/*                                              -*- mode:C++ -*-
  Element_Rocket.h Demo high-speed payload-carrying device
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
  \file Element_Rocket.h Demo high-speed payload-carrying device
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ELEMENT_ROCKET_H
#define ELEMENT_ROCKET_H

#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "itype.h"
#include "P3Atom.h"

namespace MFM
{

  template <class CC>
  class Element_Rocket : public Element<CC>
  {
    // Short names for params
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum { R = P::EVENT_WINDOW_RADIUS };

  public:
    enum {
      ELT_VERSION = 1,

      //////
      // Element field sizes

      BITS_DIR = 3,
      BITS_DIST = 5,

      STATE_BITS_START = P3Atom<P>::P3_STATE_BITS_POS
    };

    typedef BitVector<P::BITS_PER_ATOM> BVA;

    typedef BitField<BVA, VD::U32, BITS_DIR, STATE_BITS_START> AFDir;
    typedef BitField<BVA, VD::U32, BITS_DIST, AFDir::END> AFDist;

    static const u32 STATE_BITS_END = AFDist::END;
    static const u32 STATE_BITS_COUNT = STATE_BITS_END - STATE_BITS_START + 1;
    static Element_Rocket THE_INSTANCE;

    static const u32 TYPE()
    {
      return THE_INSTANCE.GetType();
    }

    Element_Rocket() : Element<CC>(MFM_UUID_FOR("Rocket", ELT_VERSION))
    {
      Element<CC>::SetAtomicSymbol("Rk");
      Element<CC>::SetName("Rocket");
    }

    virtual const T & GetDefaultAtom() const
    {
      static T defaultAtom(TYPE(),0,0,0);
      return defaultAtom;
    }

    virtual u32 DefaultPhysicsColor() const
    {
      return 0xff00ff00;
    }

    virtual u32 DefaultLowlightColor() const
    {
      return 0xff00af00;
    }

    virtual u32 Diffusability(EventWindow<CC> & ew, SPoint nowAt, SPoint maybeAt) const
    {
      return nowAt.Equals(maybeAt)?Element<CC>::COMPLETE_DIFFUSABILITY:0;
    }

    virtual u32 PercentMovable(const T& you,
                               const T& me, const SPoint& offset) const
    {
      return 0;
    }

    virtual void Behavior(EventWindow<CC>& window) const
    {
      // Phase 0: Self check
      // Phase 1: See / Scan
      //  Scan goals:
      //   1.0: Count all rocket pieces going my way
      // Phase 2: Think
      //   2.0: If I'm alone, decay
      // Phase 3: Do
      //   3.0: Do special event window scan -- in opposite of heading
      //        direction order -- so we'll find the first guys to move first
      //   3.1: For each rocket piece going my way
      //        check in its heading direction for cases:
      //      3.1.1: Heading direction is out of event window or not alive: do nothing
      //      3.1.2: Empty in its heading direction: Move into it, decrement DIR
      //      3.1.3: Second heading direction is out of event window or not alive: do nothing
      //      3.1.4: Occupied in heading but empty in second heading: push occupant
      //             into second heading, move into first heading, decrement DIR
      //      3.1.5: Do nothing
      /*
        .  .  .  .  .  .
        . e. e.  .  .  .
        . e. b. b.  .  .
        .  . b. p.  .  .
        .  .  .  .  .  .
        .  .  .  .  .  .
        .  . b. e.  .  .
        .  . p. b. e.  .
        .  . b. e.  .  .
        .  .  .  .  .  .
        .  .  .  .  .  .

        .  .  .  .  .  .
        . e.  . e.  .  .
        .  . b.  . b.  .
        . e.  . p.  .  .
        .  . b.  .  .  .
        .  .  .  .  .  .
        .  .  .  .  .  .
        .  . b. e.  .  .
        .  .  .  .  .  .
        .  . p.  . b. e.
        .  .  .  .  .  .
        .  . b. e.  .  .
        .  .  .  .  .  .
        .  .  .  .  .  .


        .........
        ...bbb...
        ...bpb...
        ...bbb...
        .........

       */
      FAIL(INCOMPLETE_CODE);
    }
  };

  template <class CC>
  Element_Rocket<CC> Element_Rocket<CC>::THE_INSTANCE;
}

#endif /* ELEMENT_ROCKET_H */
