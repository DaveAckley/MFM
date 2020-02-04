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

#include "itype.h"

namespace MFM
{
  struct FlashTraffic {
    FlashTraffic(u8 pkthdr, u8 cmd, u8 index, u8 ttl)
      : mPktHdr(pkthdr)
      , mCommand(cmd)
      , mIndex(index)
      , mTimeToLive(ttl)
      , mChecksum(computeChecksum())
    { }

    u8 computeChecksum() {
      u32 num = 0;
      num = (num << 5) ^ mCommand;
      num = (num << 5) ^ mIndex;
      num = (num << 5) ^ mTimeToLive;
      return (u8) (num ^ (num>>7) ^ (num>>14));
    }
    void updateChecksum() {
      mChecksum = computeChecksum();
    }
    bool checksumValid() {
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
    u8 mTimeToLive;
    u8 mChecksum;
  };
}

#endif /* FLASHTRAFFIC_H */
