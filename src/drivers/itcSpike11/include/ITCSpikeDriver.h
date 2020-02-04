/*  -*- mode:C++ -*- */
/*
  ITCSpikeDriver.h Bottom-up intertile event design support
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
  \file ITCSpikeDriver.h Definitions of 'immediate mode' inter-T2 packets.
  \author David H. Ackley.
  \date (C) 2019 All rights reserved.
  \lgpl
 */
#ifndef ITCSPIKEDRIVER_H
#define ITCSPIKEDRIVER_H

#include "Dirs.h"
#include "DateTimeStamp.h"
#include "MFMIO.h"
#include "ITC.h"
#include "ITCIcons.h"
#include "ITCStatusPanel.h"
#include "TileModel.h"
#include "VArguments.h"
#include "FileByteSink.h"  /* for STDERR */
#include "SDLI.h"
#include "RootPanel.h"
#include "LogPanel.h"

namespace MFM
{
  struct ITCSpikeDriver
  {
  private:

    Random mRandom;
    RandomDirIterator mDir8Iterator;
    SDLI mSDLI;
    TileModel mTileModel;
    RootPanel mRootPanel;
    Drawing mRootDrawing;
    LogPanel mLogPanel;

    typedef TextPanel<32,3> TileStatusPanel;
    TileStatusPanel mTileStatusPanel;

    ITCIcons mITCIcons;
    ITC mITC[ITC_COUNT];
    ITCStatusPanel mITCPanels[ITC_COUNT];
    u8 mLocksetTaken;
    u8 mLocksetFreed;

    u32 mLastDisplayTick;
    u32 mLoopsPerDisplay;
    u32 mLoopsRemaining;

    struct ThreadStamper : public DateTimeStamp
    {
      typedef DateTimeStamp Super;
      ITCSpikeDriver &driver;
      ThreadStamper(ITCSpikeDriver &driver) : driver(driver) { }
      virtual Result PrintTo(ByteSink & byteSink, s32 argument = 0)
      {
        Super::PrintTo(byteSink, argument);
        byteSink.Printf("ITCSPIKE: ");
        return SUCCESS;
      }
    };

    VArguments mArgs;
    ThreadStamper mStamper;
    MFMIO mMFMIO;
    ITCLocks mITCLocks;

  public:
    TileModel& GetTileModel() { return mTileModel; }

    MFMIO& GetMFMIO() { return mMFMIO; }
    ITCLocks& GetITCLocks() { return mITCLocks; }

    u32 mOpCountdownTimer;
    DriverOp mLocalOp;
    bool mOpGlobal; /*next op applies to whole grid*/

  public:
    bool getGlobalOpFlag() const ;
    void setDelayedDriverOp(DriverOp op, s32 delayUpdates) ;

    ITCSpikeDriver(int argc, const char ** argv);

    void onceOnly() ;

    void initDisplay() ;

    void doDisplay() ;

    void run() ; // Whole program execution

    void update() ;  // All work per display

    void step() ; // Advance IO and state machines once 

    void advanceITCStateMachines() ;  // Advance state machines once

    void doTileProcessing() ;

  };

} // namespace MFM

#endif /* ITCSPIKEDRIVER_H */
