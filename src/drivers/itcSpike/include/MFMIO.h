/*  -*- mode:C++ -*- */
/*
  MFMIO.h Access to T2 InterTile Connection packet communications
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
  \file MFMIO.h Access to T2 InterTile Connection packet communications
  \author David H. Ackley.
  \date (C) 2019 All rights reserved.
  \lgpl
 */
#ifndef MFMIO_H
#define MFMIO_H

#include <stdlib.h>     /* For abort */
#include <sys/types.h>  /* For open */
#include <unistd.h>     /* For read */
#include <fcntl.h>      /* For O_RDWR */
#include <stdio.h>      /* For stderr */
#include <errno.h>      /* For errno */
#include <string.h>     /* For strerror */
#include "MFMT2Constants.h"
#include "Logger.h"
#include "FlashTraffic.h"
#include "PacketFIFO.h"
#include "ITCLocks.h"
#include "dirdatamacro.h" /* For DIR_ET etc, from /home/t2/T2-12/lkms/itc/*/

namespace MFM {
  struct ITCSpikeDriver; // FORWARD

  struct MFMIO {

    ITCSpikeDriver &mDriver;
    s32 mMfmPacketFD;
    s32 mLastCommandIndex;

    PacketFIFO mInboundPackets[6];
    PacketFIFO mFlashTrafficPackets;

    MFMIO(ITCSpikeDriver & driver) ;

    bool open() ;

    bool close() ;

    bool flushPendingPackets() ;

    bool tryReceivePacket(u32 dir8, PacketBuffer & dest) ;

    bool trySendPacket(const unsigned char * buf, unsigned len) ;

    void handleFlashTraffic(unsigned char * buf, unsigned len) ;

    void processFlashTraffic() ;

    void packetIO() ;  /* move packets in and out */
  };
}

#endif /* MFMIO_H */
