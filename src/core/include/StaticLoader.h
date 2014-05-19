/* -*- C++ -*- */
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
  template <u32 BITS>
  class StaticLoader {
    static u32 m_counter;
    static const u32 SLOTS = 1<<BITS;
    static const UUID *(m_uuids[SLOTS]);
    static u32 NextType() ;

  public:
    static u32 AllocateType(const UUID & forUUID) ;

    static const UUID * UUIDOfType(u32 type) {
      if (type < 0 || type >= SLOTS)
        return 0;
      return m_uuids[type];
    }

  };
  typedef StaticLoader<16> U16StaticLoader;
}

#include "StaticLoader.tcc"

#endif /* STATICLOADER_H */
