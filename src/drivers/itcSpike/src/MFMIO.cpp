#include "MFMIO.h"
#include "ITCSpikeDriver.h"

namespace MFM {

  MFMIO::MFMIO(ITCSpikeDriver & driver)
    : mDriver(driver)
    , mMfmPacketFD(-1)
    , mLastCommandIndex(-1)
  { }

  bool MFMIO::open() {
    if (mMfmPacketFD >= 0) abort();
    mMfmPacketFD = ::open(MFM_DEV_PATH, O_RDWR | O_NONBLOCK);
    if (mMfmPacketFD < 0) {
      fprintf(stderr,"Can't open %s: %s\n", MFM_DEV_PATH, strerror(errno));
      return false;
    }
    return true;
  }

  bool MFMIO::close() {
    if (mMfmPacketFD < 0) return false;
    if (::close(mMfmPacketFD) < 0) {
      fprintf(stderr,"Can't close %s: %s\n", MFM_DEV_PATH, strerror(errno));
      return false;
    }

    mMfmPacketFD = -1;
    return true;
  }

  bool MFMIO::flushPendingPackets() {
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

  void packetIO() {
    
  }


  bool MFMIO::tryReceivePacket(u32 dir8, PacketBuffer & dest) {
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

  bool MFMIO::trySendPacket(const unsigned char * buf, unsigned len) {
    ssize_t amt = write(mMfmPacketFD, buf, len);
    if (amt < 0) {
      if (errno == EHOSTUNREACH || errno == EAGAIN || errno == EWOULDBLOCK)
        return false;
      abort();
    }
    return true;
  }

  void MFMIO::handleFlashTraffic(unsigned char * buf, unsigned len) {
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
      FAIL(INCOMPLETE_CODE);
      //      mDriver.DoDriverOpLocally((DriverOp) ft->mCommand);  // BAM
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

  void MFMIO::processFlashTraffic() {

    while (!mFlashTrafficPackets.isEmpty())  {
      u8 buf[256];
      u8 len;
      bool ret = mFlashTrafficPackets.removePacket(buf, len);
      MFM_API_ASSERT_STATE(ret);
      handleFlashTraffic(buf, len);
    }
  }


  void MFMIO::packetIO() {  /* move packets in and out */
    // Nothing to do here?  tryReceive will do it in the loop?
  }

}

