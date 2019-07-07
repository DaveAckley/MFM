
#include "T2Viz.h"
#include "SDLI.h"
#include "T2Utils.h"

namespace MFM {
  T2Viz::T2Viz(SDL_Surface * screen)
    : mUpdateCount(0)
    , mCPUMHz(0)
    , mLastAER(-1)
    , mLastAEPS(-1)
    , mLastAEPSChangeCount(0)
    , mLastCDMTime(-1)
    , mLastCDMChangeCount(0)
  {
    //// Init icons
    mITCIcons.init(screen);

    //// Init data management
    for (u32 i = 0; i < ITC_COUNT; ++i)
      mITCs[i].setDirIdx(i);

#define ZERO_ARRAY(NAME) do { for (u32 i = 0; i < sizeof(NAME)/sizeof(NAME[0]); ++i) NAME[i] = 0; } while (0)
    ZERO_ARRAY(mTNV);
    ZERO_ARRAY(mLoadAvg);
    ZERO_ARRAY(mUptime);
#undef ZERO_ARRAY

    //// Set up root window
    {
      mRootPanel.SetName("RootPanel");
      mRootPanel.SetDimensions(ROOT_WINDOW_WIDTH, ROOT_WINDOW_HEIGHT);
      mRootPanel.SetDesiredSize(ROOT_WINDOW_WIDTH, ROOT_WINDOW_HEIGHT);
      mRootPanel.SetForeground(Drawing::RED);
      mRootPanel.SetBackground(Drawing::BLACK);
      mRootPanel.SetBorder(Drawing::BLACK);
    }

    //// Corner lights show liveness
    {
      for (u32 i = 0; i < 4; ++i) {
        Panel & p = mCornerLights[i];
        OString16 name;
        name.Printf("Corner%d",i);
        p.SetName(name.GetZString());
        p.SetVisible(true);
        p.SetBackground(Drawing::BLACK);
        p.SetForeground(Drawing::BLACK);
        p.SetDimensions(CORNER_LIGHT_WINDOW_WIDTH, CORNER_LIGHT_WINDOW_HEIGHT);
        p.SetDesiredSize(CORNER_LIGHT_WINDOW_WIDTH, CORNER_LIGHT_WINDOW_HEIGHT);
        SPoint pos;
        switch (i) {
        case 0: pos.Set(0,0); break;
        case 1: pos.Set(ROOT_WINDOW_WIDTH - CORNER_LIGHT_WINDOW_WIDTH, 0); break;
        case 2: pos.Set(ROOT_WINDOW_WIDTH - CORNER_LIGHT_WINDOW_WIDTH, ROOT_WINDOW_HEIGHT - CORNER_LIGHT_WINDOW_HEIGHT); break;
        case 3: pos.Set(0, ROOT_WINDOW_HEIGHT - CORNER_LIGHT_WINDOW_HEIGHT); break;
        }
        p.SetRenderPoint(pos);
        mRootPanel.Insert(&p, NULL);
      }
    }

    //// Content window
    {
      mContentPanel.SetName("ContentPanel");
      mContentPanel.SetForeground(Drawing::WHITE);
      mContentPanel.SetBackground(Drawing::BLACK);
      mContentPanel.SetBorder(Drawing::BLACK);
      mContentPanel.SetVisible(true);
      mContentPanel.SetDimensions(CONTENT_WINDOW_WIDTH,CONTENT_WINDOW_HEIGHT);
      mContentPanel.SetDesiredSize(CONTENT_WINDOW_WIDTH,CONTENT_WINDOW_HEIGHT);
      const SPoint pos(CONTENT_WINDOW_XPOS,CONTENT_WINDOW_YPOS);
      mContentPanel.SetRenderPoint(pos);

      mRootPanel.Insert(&mContentPanel, NULL);
    }
    
    //// Insert the kids

    // packet deltas panel
    {
      mDeltasPanel.SetName("DeltasPanel");
      mDeltasPanel.SetForeground(Drawing::YELLOW);
      mDeltasPanel.SetBackground(T2_COLOR_BKGD_YELLOW);
      mDeltasPanel.SetBorder(T2_COLOR_BKGD_YELLOW);
      mDeltasPanel.SetVisible(true);
      mDeltasPanel.SetDimensions(DELTAS_WINDOW_WIDTH, DELTAS_WINDOW_HEIGHT);
      mDeltasPanel.SetDesiredSize(DELTAS_WINDOW_WIDTH, DELTAS_WINDOW_HEIGHT);
      mDeltasPanel.SetAnchor(ANCHOR_EAST);
      mDeltasPanel.SetFont(FONT_ASSET_ELEMENT);
      mDeltasPanel.SetFontHeightAdjust(-10);
      mDeltasPanel.SetElevatorWidth(0);

      mContentPanel.Insert(&mDeltasPanel, NULL);
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
      mLogPanel.setBytesToSkipPerLine(27);

      mRootPanel.Insert(&mLogPanel, NULL);
    }

    // Physical status
    {
      mStatusPanel.SetName("StatusPanel");
      mStatusPanel.SetForeground(T2_COLOR_LIGHT_BLUE);
      mStatusPanel.SetBackground(T2_COLOR_BKGD_BLUE);
      mStatusPanel.SetBorder(T2_COLOR_BKGD_BLUE);
      mStatusPanel.SetVisible(true);
      mStatusPanel.SetDimensions(STATUS_WINDOW_WIDTH, STATUS_WINDOW_HEIGHT);
      mStatusPanel.SetDesiredSize(STATUS_WINDOW_WIDTH, STATUS_WINDOW_HEIGHT);
      mStatusPanel.SetAnchor(ANCHOR_WEST);
      mStatusPanel.SetFont(FONT_ASSET_DEFAULT_FIXED);
      mStatusPanel.SetFontHeightAdjust(-7);
      mStatusPanel.SetElevatorWidth(0);
      mContentPanel.Insert(&mStatusPanel, NULL);
    }

    // The static data goes into the root window
    {
      mStaticPanel.SetName("StaticPanel");
      mStaticPanel.SetForeground(T2_COLOR_LIGHT_GREY);
      mStaticPanel.SetBackground(T2_COLOR_BKGD_RED);
      mStaticPanel.SetBorder(T2_COLOR_BKGD_RED);
      mStaticPanel.SetVisible(true);
      mStaticPanel.SetDimensions(STATIC_WINDOW_WIDTH, STATIC_WINDOW_HEIGHT);
      mStaticPanel.SetDesiredSize(STATIC_WINDOW_WIDTH,STATIC_WINDOW_HEIGHT);
      const SPoint pos(STATIC_WINDOW_XPOS,STATIC_WINDOW_YPOS);
      mStaticPanel.SetRenderPoint(pos);
      mStaticPanel.SetFont(FONT_ASSET_LOGGER);
      mStaticPanel.SetFontHeightAdjust(-7);
      mStaticPanel.SetElevatorWidth(0);

      mRootPanel.Insert(&mStaticPanel, NULL);
    }

    // The ITCs go into the root window
    for (u32 i = 0; i < ITC_COUNT; ++i) {
      mITCPanels[i].init(mITCs[i],mITCIcons);
      mRootPanel.Insert(&mITCPanels[i], NULL);
    }

    //// Repack
    mRootPanel.HandleResize(mRootPanel.GetDimensions());

    //// Get static data
    ByteSink & staticbs = mStaticPanel.GetByteSink();

    { // Serial number
      const char * PATH = "/sys/devices/platform/bone_capemgr/baseboard/serial-number";
      if (!readOneLinerFile(PATH,mSecretSerialNumber)) LOG.Error("Can't read '%s'", PATH);
      else staticbs.Printf("[%s]\n",mSecretSerialNumber.GetZString());
    }
    
    { // Build time tag
      staticbs.Printf("%08x%06x\n",MFM_BUILD_DATE,MFM_BUILD_TIME);
    }
    
  }
  
  void T2Viz::input(SDLI& sdli, Event& event) {
  }

  void T2Viz::update(SDLI& sdli) {
    ++mUpdateCount;
    //// Rotate blue
    for (u32 i = 0; i < 4; ++i) {
      u32 color = i==(mUpdateCount&3) ? Drawing::BLUE : Drawing::BLACK;
      mCornerLights[i].SetForeground(color);
      mCornerLights[i].SetBorder(color);
      mCornerLights[i].SetBackground(color);
    }

    //// Update log
    mLogPanel.update();

    //// Refresh cache values

    do { // Package tags
      StaticPanel::TextPanelByteSink & tagbs = mStaticPanel.GetByteSink();
      tagbs.Reset();

      s32 tmp;
      {
        const char * PATH = "/home/t2/cdmd-T2-12.mfz-cdm-install-tag.dat";
        if (!readOneDecimalNumberFile(PATH,tmp)) tagbs.Printf("T2-12: no tag\n");
        else tagbs.Printf("T2-12: %d\n",tmp);
      }
      {
        const char * PATH = "/home/t2/GITHUB/cdmd-MFM.mfz-cdm-install-tag.dat";
        if (!readOneDecimalNumberFile(PATH,tmp)) tagbs.Printf("MFMT2: no tag\n");
        else tagbs.Printf("MFMT2: %d\n",tmp);
      }
    } while(0);

    do { // CPU freq
      const char * PATH = "/sys/devices/system/cpu/cpufreq/policy0/scaling_cur_freq";
      s32 tmp;
      if (!readOneDecimalNumberFile(PATH, tmp)) LOG.Error("Can't read '%s'",PATH);
      else {
        tmp /= 1000;
        if ((u32) tmp != mCPUMHz) {
          if (mCPUMHz != 0) LOG.Message("CPU %dMHz -> %dMHz", mCPUMHz, tmp);
          mCPUMHz = tmp;
        }
      }
    } while(0);

    do { // TNV data
      const char * CMD = "/home/t2/T2-12/apps/tnv/tnv 1";
      OString128 buf;
      if (!readWholeProcessOutput(CMD, buf)) LOG.Error("Can't run '%s'",CMD);
      else {
        buf.Chomp();
        CharBufferByteSource cbs = buf.AsByteSource();
        for (u32 i = 0; i < sizeof(mTNV) / sizeof(mTNV[0]); ++i) {
          if (cbs.Scanf("%f",&mTNV[i]) != 1)
            mTNV[i] = -1;
        }
      }
    } while(0);

    do { // Load avg
      const char * PATH = "/proc/loadavg";
      if (!readFloatsFromFile(PATH, mLoadAvg, sizeof(mLoadAvg)/sizeof(mLoadAvg[0])))
          LOG.Error("Can't read '%s'",PATH);
    } while(0);

    do { // Uptime
      const char * PATH = "/proc/uptime";
      if (!readFloatsFromFile(PATH, mUptime, sizeof(mUptime)/sizeof(mUptime[0])))
          LOG.Error("Can't read '%s'",PATH);
    } while(0);

    do { // CDM liveness
      const char * PATH = "/run/cdm/status.dat";
      double a[9];
      // Assume all dead
      for (u32 i = 0; i < ITC_COUNT; ++i) mITCs[i].setIsAlive(false);
      if (readFloatsFromFile(PATH, a, sizeof(a)/sizeof(a[0]))) {
        if (a[8] != mLastCDMTime) {
          mLastCDMChangeCount = mUpdateCount;
          mLastCDMTime = a[8];
        }
        if (mLastCDMChangeCount + 50 <= mUpdateCount) mLastCDMTime = -1;
        else {
          u32 itc = 0;
          for (u32 i = 0; i < ROSE_DIR_COUNT; ++i) {
            if (i%4 == 0) continue;
            mITCs[itc++].setIsAlive(a[i] != 0);
          }
        }
      } else mLastCDMTime = -1; // Can't read file
    } while(0);

    do { // AER & AEPS
      const char * PATH = "/run/mfmt2/status.dat";
      double a[2];
      if (readFloatsFromFile(PATH, a, sizeof(a)/sizeof(a[0]))) {
        if (a[1] != mLastAEPS) {
          mLastAEPSChangeCount = mUpdateCount;
          mLastAEPS = a[1];
        }
        if (mLastAEPSChangeCount + 20 > mUpdateCount) mLastAER = a[0];
        else mLastAER = -1;
      } else mLastAER = -1; // Can't read file
    } while(0);

    do { // ITCStatus
      const char * PATH = "/sys/class/itc_pkt/status";
      OString16 status;
      if (!readOneLinerFile(PATH, status)) {
        LOG.Error("Can't read '%s'",PATH);
        break;
      }
      const char * p = status.GetZString();
      if (strlen(p) != ROSE_DIR_COUNT) LOG.Error("Bad status '%s'",p);
      else {
        u32 itc = 0;
        for (u32 i = 0; i < ROSE_DIR_COUNT; ++i) {
          if (i%4 == 0) continue;
          mITCs[itc++].setIsOpen(p[ROSE_DIR_COUNT - 1 - i] == '1');
        }
      }
    } while(0);

    do { // ITC stats
      const char * PATH = "/sys/class/itc_pkt/statistics";
      FileByteSource fbs(PATH);
      if (!fbs.IsOpen()) {
        LOG.Error("Can't read '%s'",PATH);
        break;
      }

      fbs.Scanf("%[^\n]\n",0);  // Toss first line
      u32 itc = 0;
      for (u32 i = 0; i < ROSE_DIR_COUNT; ++i) {
        if (i%4 == 0) fbs.Scanf("%[^\n]\n",0);  // Don't care about this line either
        else mITCs[itc++].updateStatsFromLine(fbs);
      }
      fbs.Close();
    } while(0);
  } 

  void T2Viz::output(SDLI& sdli) {
    mRootDrawing.Reset(sdli.getScreen(), FONT_ASSET_ELEMENT);

    { // Status panel
      const u32 MAX_SIZE = 1000;
      char buf[MAX_SIZE];
      snprintf(buf, MAX_SIZE,
               "%6.2fV \n"
               "%6.1fF \n"
               "%6.1fF \n"
               "%6dL \n"
               "%6dM \n"
               "%6.2fR \n"
               "%6.1fH \n"
               "%6.2fA \n"
               ,
               mTNV[0],
               mTNV[1],
               mTNV[2],
               (s32) mTNV[6],
               mCPUMHz,
               mLoadAvg[0],
               mUptime[0]/(60*60),
               mLastAER);

      mStatusPanel.GetByteSink().Reset();
      mStatusPanel.GetByteSink().Print(buf);
    }

    { // Deltas panel
      DeltasPanel::TextPanelByteSink & tbs = mDeltasPanel.GetByteSink();
      tbs.Reset();
      for (u32 i = 0; i < ITC_COUNT; ++i) {
        if (i == ITC_COUNT/2) tbs.Print("\n");
        mITCs[i].formatDeltasLine(tbs);
      }
      tbs.Print("\n");
    }

    mRootDrawing.Clear();
    mRootPanel.Paint(mRootDrawing);
  }
}
