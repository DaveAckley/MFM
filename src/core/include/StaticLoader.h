/*                                              -*- mode:C++ -*-
  StaticLoader.h Element type assignment system
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
  \file StaticLoader.h Element type assignment system
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef STATICLOADER_H
#define STATICLOADER_H

#include "itype.h"
#include "Fail.h"
#include "UUID.h"

namespace MFM
{
  /**
     Horrible globally-centralized all-static hack to assign type
     codes without having an interTile type-map exchange protocol to
     allow us to assign types on a per-Tile basis.  This code will not
     work at all on truly distributed hardware.

     StaticLoader attempts to assign reasonably well-spaced typecodes
     to minimize the chance of bit flips converting one type into
     another legal type (assuming it escapes detection by the ECC).
     It increments a counter but, rather than using the counter value
     directly, it then 'expands' the count across the bits as widely
     as possible, based on the current position of the leftmost 1 bit
     in the counter.  This causes collisions, so the array of assigned
     types is used to reject them.  A last-ditch pass of 'unexpanded'
     bits ensures all 1<<BITS types are eventually assigned; the
     counter will reach 2<<BITS if that happens.

     On sixteen bits, the assigned numbers sequence starts out like
     this: 0x0000, 0xffff, 0x00ff, 0xff00, 0x001f, 0x03e0, 0x03ff,
     0x7c00, 0x7c1f, ...
   */
  template <class CC, u32 BITS>
  class StaticLoader {
    static u32 m_counter;
    static const u32 SLOTS = 1<<BITS;
    static const UUID *(m_uuids[SLOTS]);
    static u32 NextType() ;

  public:
    static u32 AllocateType(const UUID & forUUID) ;

    /**
     * Return the type assigned to \a forUUID, or -1 if the UUID is
     * not found.  Note this is O(#types)!  Not for inner loop use!
     */
    static s32 TypeFromUUID(const UUID & forUUID) ;

    /**
     * Return a 'compatible type' assigned to \a forUUID, or -1 if no
     * such UUID is not found.  Note this is O(#types)!  Not for inner
     * loop use!
     */
    static s32 TypeFromCompatibleUUID(const UUID & forUUID) ;

    /**
     * Return a pointer to the UUID associated with a given type or 0
     * if the type has not been assigned.  O(1).
     */
    static const UUID * UUIDOfType(u32 type) {
      if (type < 0 || type >= SLOTS)
        return 0;
      return m_uuids[type];
    }

  };
  //typedef StaticLoader<16> U16StaticLoader;
}

#include "StaticLoader.tcc"

#endif /* STATICLOADER_H */
