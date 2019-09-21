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

  template <class GC> class MFMT2Driver; //FORWARD

  template <class GC>
  struct MFMIO {

    MFMT2Driver<GC> &mDriver;
    s32 mMfmPacketFD;
    s32 mLastCommandIndex;

    PacketFIFO mInboundPackets[DIR_COUNT];
    PacketFIFO mFlashTrafficPackets;

    MFMIO(MFMT2Driver<GC> & driver)
      : mDriver(driver)
      , mMfmPacketFD(-1)
      , mLastCommandIndex(-1)
    { }

    bool open() {
      if (mMfmPacketFD >= 0) abort();
      mMfmPacketFD = ::open(MFM_DEV_PATH, O_RDWR | O_NONBLOCK);
      if (mMfmPacketFD < 0) {
        fprintf(stderr,"Can't open %s: %s\n", MFM_DEV_PATH, strerror(errno));
        return false;
      }
      return true;
    }

    bool close() {
      if (mMfmPacketFD < 0) return false;
      if (::close(mMfmPacketFD) < 0) {
        fprintf(stderr,"Can't close %s: %s\n", MFM_DEV_PATH, strerror(errno));
        return false;
      }

      mMfmPacketFD = -1;
      return true;
    }

    bool flushPendingPackets() {
      static unsigned char buf[256];

      u32 packetsHandled = 0;
      MFM_API_ASSERT_STATE(mMfmPacketFD >= 0);

      while (true) {
        ssize_t amt = read(mMfmPacketFD, buf, sizeof(buf));
        if (amt == 0) {
          fprintf(stderr,"EOF on %d\n", mMfmPacketFD);
          exit(5);
        }

        if (amt < 0) {
          if (errno == EAGAIN || errno == EWOULDBLOCK) {
            LOG.Message("Flushed %d stale packet(s)", packetsHandled);
            mLastCommandIndex = -1;
            break;
          } else {
            abort();
          }
        }

        ++packetsHandled;
      }
      return packetsHandled>0; /* nothing to read or flush finished */
    }


    bool tryReceivePacket(u32 dir8, PacketBuffer & dest) {
      static unsigned char buf[256];

      u32 packetsHandled = 0;
      u32 forDir6 = mapDir8ToDir6(dir8);
      MFM_API_ASSERT_ARG(forDir6 < DIR_COUNT);
      MFM_API_ASSERT_STATE(mMfmPacketFD >= 0);

      PacketFIFO & pf = mInboundPackets[forDir6];
      while (pf.isEmpty()) {
        ssize_t amt = read(mMfmPacketFD, buf, sizeof(buf));
        if (amt == 0) {
          fprintf(stderr,"EOF on %d\n", mMfmPacketFD);
          exit(5);
        }

        if (amt < 0) {
          if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0; /* nothing to read or flush finished */
          } else {
            abort();
          }
        }

        ++packetsHandled;

        if (amt > 1 && (buf[1] & 0x80)) // byte 1 MSB set is MFM SPECIAL, clear is cache update
          mFlashTrafficPackets.addPacket(buf, amt); // Stash flash traffic (with t2 hdr) for later
        else {
          /* Got cache update packet*/
          u32 pktDir8 = buf[0]&0x7;

          if (pktDir8 == dir8) {    // Lucky shortcut, bypass FIFO
            dest.Reset();
            dest.WriteBytes(&buf[1], amt - 1); // Strip t2 header
            return true;
          }
          
          u32 pktDir6 = mapDir8ToDir6(pktDir8);
          mInboundPackets[pktDir6].addPacket(&buf[1], amt - 1);
        }
      }
      // Here if we have a buffered packet waiting for this dir8
      bool ret = pf.removePacketBuffer(dest);
      MFM_API_ASSERT_STATE(ret);
      return true;
    }

    bool trySendPacket(const unsigned char * buf, unsigned len) {
      ssize_t amt = write(mMfmPacketFD, buf, len);
      if (amt < 0) {
        if (errno == EHOSTUNREACH || errno == EAGAIN || errno == EWOULDBLOCK)
          return false;
        abort();
      }
      return true;
    }

    void handleFlashTraffic(unsigned char * buf, unsigned len) {
      if (len == 0) {
        fprintf(stderr,"Zero length packet illegal\n");
        abort();
      }
      if ((buf[0]&0x80) == 0) {
        fprintf(stderr,"Cache update illegal here! %02x+%d\n",buf[0],len);
        abort();
      }
      if (len < sizeof(FlashTraffic)) {
        fprintf(stderr,"Packet length %d too small '%*s'\n", len, len, buf);
        return;
      }
      FlashTraffic *ft = (FlashTraffic*) buf;
      if (ft->mCommand < DRIVER_OP_MIN || ft->mCommand > DRIVER_OP_MAX) {
        fprintf(stderr,"MFM packet type '%c' unrecognized\n", ft->mCommand);
        return;
      }
      if (!ft->checksumValid()) {
        fprintf(stderr,"Invalid flash traffic checksum %02x in '%5s', expected %02x\n",
                ft->mChecksum,
                (char*) ft,
                ft->computeChecksum());
        return;
      }
      bool maybeForward = true;
      if (ft->executable(mLastCommandIndex)) {
        fprintf(stderr,"EXECUTING #%d:%c\n", mLastCommandIndex, ft->mCommand);
        mDriver.DoDriverOpLocally((DriverOp) ft->mCommand);  // BAM
      } else {
        fprintf(stderr,"OBSOLETE #%d:%02x\n", mLastCommandIndex, ft->mCommand);
        maybeForward = false;
      }
      if (maybeForward && ft->mTimeToLive > 0) {
        u8 fromDir = ft->mPktHdr&7;
        ft->mTimeToLive--;
        for (unsigned i = 0; i < 8; ++i) {
          if ((i&3) == 0 || i == fromDir) continue; // Skip 0, 4, and fromDir
          ft->mPktHdr = (ft->mPktHdr&~7) | i;
          ft->updateChecksum();
          trySendPacket(buf, len);
        }
      }
    }

    void processFlashTraffic() {

      while (!mFlashTrafficPackets.isEmpty())  {
        u8 buf[256];
        u8 len;
        bool ret = mFlashTrafficPackets.removePacket(buf, len);
        MFM_API_ASSERT_STATE(ret);
        handleFlashTraffic(buf, len);
      }
    }
  };
}

#endif /* MFMIO_H */
