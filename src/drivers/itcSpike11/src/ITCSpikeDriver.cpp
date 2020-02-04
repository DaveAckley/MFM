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
    , mSDLI()
    , mTileModel(mRandom,mSDLI)
    , mRootPanel()
    , mRootDrawing()
    , mLogPanel()
    , mTileStatusPanel()
    , mITCIcons()
    , mLocksetTaken(0)
    , mLocksetFreed(0)
    , mLastDisplayTick(0)
    , mLoopsPerDisplay(100000)
    , mLoopsRemaining(0)
    , mStamper(*this)
    , mMFMIO(*this)
    , mOpCountdownTimer(0)
    , mLocalOp(DRIVER_NO_OP)
    , mOpGlobal(false)
  {
    mITCIcons.init(mSDLI.getScreen());
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

    // Init display
    initDisplay();
  }

  void ITCSpikeDriver::initDisplay() {
    //// Set up root window
    {
      mRootPanel.SetName("RootPanel");
      mRootPanel.SetDimensions(ROOT_WINDOW_WIDTH, ROOT_WINDOW_HEIGHT);
      mRootPanel.SetDesiredSize(ROOT_WINDOW_WIDTH, ROOT_WINDOW_HEIGHT);
      mRootPanel.SetForeground(Drawing::RED);
      mRootPanel.SetBackground(Drawing::BLACK);
      mRootPanel.SetBorder(Drawing::BLACK);
    }

    // ITC status panels
    {
      for (u32 i = 0; i < ITC_COUNT; ++i) {
        mITCPanels[i].init(mITC[i],mITCIcons);
        mRootPanel.Insert(&mITCPanels[i], NULL);
      }
    }

    // Tile status
    {
      mTileStatusPanel.SetName("TileStatusPanel");
      mTileStatusPanel.SetDimensions(250, 80);
      mTileStatusPanel.SetDesiredSize(250, 80);
      const SPoint pos(32,32);
      mTileStatusPanel.SetRenderPoint(pos);
      mTileStatusPanel.SetFontHeightAdjust(-7);
      mTileStatusPanel.SetElevatorWidth(0);
      mTileStatusPanel.SetForeground(Drawing::RED);
      mTileStatusPanel.SetBackground(Drawing::BLACK);
      mTileStatusPanel.SetBorder(Drawing::BLACK);

      mRootPanel.Insert(&mTileStatusPanel, NULL);
    }

    // Logger
    {
      mLogPanel.SetName("LogPanel");
      mLogPanel.SetForeground(Drawing::GREEN);
      mLogPanel.SetBackground(T2_COLOR_BKGD_GREEN); // For that CRT glow
      mLogPanel.SetBorder(T2_COLOR_BKGD_GREEN); 
      mLogPanel.SetVisible(true);
      mLogPanel.SetDimensions(LOG_WINDOW_WIDTH, LOG_WINDOW_HEIGHT);
      mLogPanel.SetDesiredSize(LOG_WINDOW_WIDTH, LOG_WINDOW_HEIGHT);
      const SPoint pos(LOG_WINDOW_XPOS,LOG_WINDOW_YPOS);
      mLogPanel.SetRenderPoint(pos);
      mLogPanel.SetFont(FONT_ASSET_LOGGER);
      mLogPanel.SetFontHeightAdjust(-7);
      mLogPanel.SetElevatorWidth(0);

      mLogPanel.setPathToTrack("/var/log/syslog");
      mLogPanel.setBytesToSkipPerLine(27+8);

      mRootPanel.Insert(&mLogPanel, NULL);
    }

    //// Repack
    mRootPanel.HandleResize(mRootPanel.GetDimensions());

    MFM::LOG.Message("Init display DONE");
  }


  void ITCSpikeDriver::doDisplay() {
    //    MFM::LOG.Message("BUEHLER?");
    mLogPanel.update();
    {
      TileStatusPanel::TextPanelByteSink & bs = mTileStatusPanel.GetByteSink();
      bs.Reset();
      bs.Printf("%d ",mLoopsPerDisplay);
      mTileModel.output(bs);
    }
    mRootDrawing.Reset(mSDLI.getScreen(), FONT_ASSET_ELEMENT);
    mRootDrawing.Clear();
    mRootPanel.Paint(mRootDrawing);
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

  void ITCSpikeDriver::step()
  {
    mMFMIO.packetIO(); // simulating distributed packet io
    advanceITCStateMachines(); // sequence the levels
    doTileProcessing();    // simulating tile activity
  }

  void ITCSpikeDriver::run()
  {
    mSDLI.mainLoop(*this);
  }

  void ITCSpikeDriver::update()
  {
    const u32 NS_SHIFT_BITS = 28; // 1e9/(1<<27) == ~7.5Hz
    while (true) {
      if (mLoopsRemaining) --mLoopsRemaining;
      else {
        timespec now;
        doDisplay(); // Have to pay for display time too
        clock_gettime(CLOCK_MONOTONIC, &now);
        u32 tick = now.tv_nsec>>NS_SHIFT_BITS;
        if (tick == mLastDisplayTick)
          ++mLoopsPerDisplay;    // Too soon
        else if (tick == mLastDisplayTick+1 && mLoopsPerDisplay > 1)
          --mLoopsPerDisplay;    // On target or a little late
        else if (tick > mLastDisplayTick+1 && mLoopsPerDisplay > 0)
          mLoopsPerDisplay = 2*mLoopsPerDisplay/3 + 1;    // Way late
        mLastDisplayTick = tick;
        mLoopsRemaining = mLoopsPerDisplay;
        break;
      }
      step();
    }
  }
}

