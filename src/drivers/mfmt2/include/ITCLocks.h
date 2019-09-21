/*  -*- mode:C++ -*- */
/*
  ITCLocks.h Access to the T2 InterTile Connection hardware locks
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
  \file ITCLocks.h Access to the T2 InterTile Connection hardware locks
  \author David H. Ackley.
  \date (C) 2019 All rights reserved.
  \lgpl
 */
#ifndef ITCLOCKS_H
#define ITCLOCKS_H

#include <time.h>   /* for time_t */
#include "itype.h"
#include "Dirs.h"
#include "Fail.h"
#include "Logger.h"
#include "Point.h"
#include "dirdatamacro.h" /*for DIR_ET, etc, from /home/t2/T2-12/lkms/itc */

namespace MFM
{
  /**
   * ITCLocks manages and provides access to the T2 locking hardware
   */
  class ITCLocks
  {
  private:
    static const char * LOCK_DEVICE_PATH;
    static const char * LOCK_STATUS_PATH;
    static const char * ITC_STATUS_PATH;

    s32 mLockDeviceFD;
    s32 mLockStatusFD;
    s32 mITCStatusFD;

    u32 mLocksetAttempts;
    u32 mLocksetAcquisitions;
    u32 mUnreadyErrors;
    u32 mContestedErrors;
    u32 mTimeoutErrors;
    u32 mFailedErrors;

    u8 mConnectedLockset;
    time_t mConnectedLocksetRefreshTime;
    
  public:

    ITCLocks() ;

    bool open() ;

    bool close() ;

    bool tryLock(u8 lockset) ;

    /**
       Access the set of currently connected ITCs.  Updated once per
       second, on calls where allowRefresh is true
     */
    u8 connected(bool allowRefresh) ;

    /**
       return all possible locks that could be needed to have an event
       at \c site in a \c tileSize tile given an event window boundary
       of \c ewRadius.  
     */
    u8 maxLocksetFor(const u32 EVENT_WINDOW_RADIUS, UPoint site, UPoint tileSize, u32 ewRadius) ;

    bool tryLocksFor(const u32 EVENT_WINDOW_RADIUS, UPoint site, UPoint tileSize, u32 ewRadius, u8 & locksetTaken) ;

    bool freeLocks() ;

    void resetCounters() ;

    void fakeEvent() ;

    /**
     * Destroys ITCLocks.
     */
    ~ITCLocks()
    {
      close();
    }

  };

  inline u8 mapDir8ToDir6(u8 dir8) {
    switch (dir8) {
    case Dirs::NORTH: 
    case Dirs::SOUTH: 
    default: return DIR_COUNT;
      
    case Dirs::NORTHEAST: return DIR_NE;
    case Dirs::EAST:      return DIR_ET;
    case Dirs::SOUTHEAST: return DIR_SE;
    case Dirs::SOUTHWEST: return DIR_SW;
    case Dirs::WEST:      return DIR_WT;
    case Dirs::NORTHWEST: return DIR_NW;
    }
  }

}

#endif /* ITCLOCKS_H */
