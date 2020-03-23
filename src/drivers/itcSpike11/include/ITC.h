/*  -*- mode:C++ -*- */
/*
  ITC.h InterTile Connection model for packet handling
  Copyright (C) 2020 The T2 Tile Project.  All rights reserved.

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
  \file ITC.h InterTile Connection model for packet handling
  \author David H. Ackley.
  \date (C) 2020 All rights reserved.
  \lgpl
 */
#ifndef ITC_H
#define ITC_H

#include <time.h>   /* for time_t */
#include <sys/types.h>  /* For open */
#include <unistd.h>     /* For read */
#include <fcntl.h>      /* For O_RDWR */
#include <stdio.h>      /* For stderr */
#include <errno.h>      /* For errno */
#include <string.h>     /* For strerror */
#include "itype.h"
#include "Dirs.h"
#include "Fail.h"
#include "TileModel.h"
#include "PacketFIFO.h"
#include <stdarg.h>

namespace MFM
{
  /**
   * An ITC manages the packet processing state machine for an intertile connection
   */
  class ITC
  {
  private:
    Random * mRandom;
    TileModel * mTileModel;
    s32 mMfmPacketFD;
    u8 mDir6;
    u8 mLevel;
    u8 mStage;
    u32 mUpdateTimeout;
    bool mKnownIncompatible;
    u32 mBackoffInterval;
    u32 mTileGeneration;
    u32 mPacketsSent;
    u32 mPacketsDropped;
    u32 mPacketsReceived;
    bool mEnabled;
    
    static const u32 STARTING_BACKOFF_INTERVAL = 1000;

  public:
    bool open() ;

    bool close() ;

    bool flushPendingPackets() ;

    bool tryReceivePacket(PacketBuffer & dest) ;

    bool trySendPacket(const unsigned char * buf, unsigned len) ;

    inline Random & getRandom() {
      MFM_API_ASSERT_NONNULL(mRandom);
      return *mRandom;
    }

    inline TileModel & getTileModel() {
      MFM_API_ASSERT_NONNULL(mTileModel);
      return *mTileModel;
    }

    inline u8 getDir6() const {
      MFM_API_ASSERT(mDir6 < 6, ILLEGAL_STATE);
      return mDir6;
    }

    ITC() ;

    ~ITC() ;

    void setEnabled(bool enabled) ;

    bool getEnabled() const ;

    void setRandom(Random& random) ;

    void setTileModel(TileModel& tileModel) ;

    void setDir6(u8 dir6) ;

    void update() ;

    void handleInboundPacket(PacketBuffer & pb) ;

    void handleLevelPacket(u8 lvl, u8 stage, const char * args, u32 arglen) ;

    void handleTileChange() ;

    bool sendLevelPacket() ;

    u32 getLevel() const ;

    u32 getStage() const ;

    bool getKnownIncompatible() const ;

    void configureLevel(u32 newLevel) ;

    void incrementLevel(s32 amt) ;

    void incrementStageAndAnnounce(u32 amt) ;

    void resetBackoff() ;

    void bumpBackoff() ;

    void message(const char * format, ... ) ;

    void warn(const char * format, ... ) ;

    void error(const char * format, ... ) ;

    void vreport(u32 msglevel, const char * format, va_list & ap) ;

  };

}

#endif /* ITC_H */
