#include "ITCSpikeDriver.h"

namespace MFM {
  bool ITCSpikeDriver::getGlobalOpFlag() const { return mOpGlobal; }

  void ITCSpikeDriver::setDelayedDriverOp(DriverOp op, s32 delayUpdates) {
    if (delayUpdates <= 0) {
      mLocalOp = DRIVER_NO_OP;
      mOpCountdownTimer = 0;
    } else {
      mLocalOp = op;
      mOpCountdownTimer = (u32) delayUpdates;
    }
  }

  ITCSpikeDriver::ITCSpikeDriver(int argc, const char ** argv)
    : mRandom()
    , mDir8Iterator()
    , mTileModel(mRandom)
    , mLocksetTaken(0)
    , mLocksetFreed(0)
    , mStamper(*this)
    , mMFMIO(*this)
    , mOpCountdownTimer(0)
    , mLocalOp(DRIVER_NO_OP)
    , mOpGlobal(false)
  {
    mArgs.ProcessArguments(argc,argv);
      
    mDir8Iterator.Shuffle(mRandom);

    MFM::LOG.SetTimeStamper(&mStamper);
    if (!mITCLocks.open()) {
      MFM::LOG.Message("Can't open ITCLocks");
    }

    MFM::LOG.Message("Initting ITCs");
    for (u8 i = 0; i < DIR_COUNT; ++i) {
      mITC[i].setRandom(mRandom);
      mITC[i].setMFMIO(mMFMIO);
      mITC[i].setTileModel(mTileModel);
      mITC[i].setDir6(i);
    }

    MFM::LOG.Message("Freeing all locks");
    mITCLocks.freeLocks();
  }


  void ITCSpikeDriver::onceOnly()
  {
    // Set up intertile packet spike
    if (!mMFMIO.open()) {
      abort();
    }
    
    // Flush any existing packets
    mMFMIO.flushPendingPackets();
    
  }

  void ITCSpikeDriver::doTileProcessing() {
    mTileModel.update();
  }
  
  void ITCSpikeDriver::advanceITCStateMachines() {
    for (mDir8Iterator.ShuffleOrReset(mRandom); mDir8Iterator.HasNext(); )
    {
      u32 dir8 = mDir8Iterator.Next();
      u32 dir6 = mapDir8ToDir6(dir8);
      if (dir6 == DIR_COUNT) continue;

      ITC& itc = mITC[dir6];
      itc.update();
    }
  }

  int ITCSpikeDriver::run()
  {
    while (true) {
      mMFMIO.packetIO(); // simulating distributed packet io
      advanceITCStateMachines(); // sequence the levels
      doTileProcessing();    // simulating tile activity
    }
  }
}

