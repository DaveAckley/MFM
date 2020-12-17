/*  -*- mode:C++ -*- */
/*
  FlashTraffic.h Definitions of 'immediate mode' inter-T2 packets.
  Copyright (C) 2019 The T2 Tile Project.  All rights reserved.

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
  \file FlashTraffic.h Definitions of 'immediate mode' inter-T2 packets.
  \author David H. Ackley.
  \date (C) 2019 All rights reserved.
  \lgpl
 */
#ifndef FLASHTRAFFIC_H
#define FLASHTRAFFIC_H

#include "T2Types.h"
#include "itype.h"
#include "Point.h"
#include "Dirs.h"

#include "FlashCommands.h"

namespace MFM
{
  struct ArgBytes {
    ArgBytes(u32 arg) {
      u32 i = 0;
      while (i < sizeof(mBytes)) {
        mBytes[i++] = arg&0xff;
        arg >>= 8;
      }
    }
    u32 get() const {
      u32 ret = 0;
      u32 i = sizeof(mBytes);
      while (i > 0) {
        ret <<= 8;
        ret |= mBytes[--i];
      }
      return ret;
    }
    u8 mBytes[sizeof(u32)];
  };
  
  struct FlashTraffic {
    static const BPoint dir8Offsets[Dirs::DIR_COUNT];
    static const u32 MAX_FLASH_DISTANCE = 120;

    static bool isInRange(const BPoint & bp) {
      return bp.GetMaximumLength() <= MAX_FLASH_DISTANCE;
    }

    static FlashTraffic make(T2FlashCmd cmd) ;

    static FlashTraffic make(T2FlashCmd cmd, s32 index, u32 range, u32 tag)
    {
      return FlashTraffic(0x80, cmd, index, range, tag);
    }

    FlashTraffic(const FlashTraffic ft, Dir8 dir8)
      : mPktHdr(0x80|dir8)
      , mCommand(ft.mCommand)
      , mIndex(ft.mIndex)
      , mRange(ft.mRange)
      , mArg(ft.mArg)
      , mTimeToLive(ft.mTimeToLive-1)
      , mOrigin(ft.mOrigin-dir8Offsets[dir8])
      , mChecksum(computeChecksum())
    {
      MFM_API_ASSERT_ARG(ft.mTimeToLive > 0);
      MFM_API_ASSERT_ARG(isInRange(mOrigin));
    }

    FlashTraffic(u8 pkthdr = 0x80, u8 cmd = U8_MAX, u8 index = 0, u8 ttl = 0, u32 tag = 0)
      : mPktHdr(pkthdr)
      , mCommand(cmd)
      , mIndex(index)
      , mRange(ttl)
      , mArg(ArgBytes(tag))
      , mTimeToLive(ttl)
      , mOrigin(0,0)
      , mChecksum(computeChecksum())
    { }

    bool canPropagateTo(Dir8 inDir8) const {
      return mTimeToLive > 0 && isInRange(mOrigin+dir8Offsets[inDir8]);
    }

    bool matchesCIRO(const FlashTraffic oth) const {
      return
        matchesCIR(oth) &&
        mOrigin.Equals(oth.mOrigin);
    }

    bool matchesCIR(const FlashTraffic oth) const {
      return
        mCommand == oth.mCommand &&
        mIndex == oth.mIndex &&
        mRange == oth.mRange;
    }

    void updateChecksum() {
      mChecksum = computeChecksum();
    }

    bool checksumValid() const {
      return mChecksum == computeChecksum();
    }

    bool executable(s32 & lastCommandIndex) {
      if (lastCommandIndex >= 0) {
        u8 advance = mIndex - (u8) lastCommandIndex;
        if (advance == 0 || advance >= U8_MAX/3) return false;
      }
      lastCommandIndex = mIndex;
      return true;
    }

    u8 mPktHdr;
    u8 mCommand;
    u8 mIndex;
    u8 mRange;
    ArgBytes mArg; 
    u8 mTimeToLive;
    BPoint mOrigin;
    u8 mChecksum;

    // Actually do it.  static to avoid a vptr
    static bool execute(const FlashTraffic & ft) ; 

  private:
    u8 computeChecksum() const {
      u32 val = 0;
      const u8 * p = &mCommand; // Skipping the header -- it's altered in transit 
      while (p != &mChecksum) val = (val<<1) ^ *p++;
      return (u8) (val ^ (val>>8) ^ (val>>16));
    }

  };
}

#endif /* FLASHTRAFFIC_H */
