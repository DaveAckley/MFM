#include "T2InfoPanel.h"

#include "TimeQueue.h"
#include "T2Utils.h"
#include "SimLogPanel.h"
#include "T2Tile.h"

namespace MFM {
  void T2InfoPanel::configure(SDLI & sdli) {
    OString128 buf;
    const char * dirs[] = {"NW","NE","SE","SW"};
    for (u32 i = 0; i < 4; ++i) {
      buf.Reset();
      buf.Printf("Corner_%s",dirs[i]);
      const char * pname = buf.GetZString();
      Panel * p = sdli.lookForPanel(pname);
      if (!p) fatal("Couldn't find panel '%s'",pname);
      mCornerLights[i] = p;
    }
    {
      const char * pname = "SimLog";
      SimLogPanel * p = dynamic_cast<SimLogPanel*>(sdli.lookForPanel(pname));
      if (!p) fatal("Couldn't find SimLogPanel '%s'",pname);
      mLogPanel = p;
    }
    {
      const char * pname = "StaticInfo";
      StaticPanel * p = dynamic_cast<StaticPanel*>(sdli.lookForPanel(pname));
      if (!p) fatal("Couldn't find StaticPanel '%s'",pname);
      mStaticPanel = p;
    }
    {
      const char * pname = "StatusInfo";
      StatusPanel * p = dynamic_cast<StatusPanel*>(sdli.lookForPanel(pname));
      if (!p) fatal("Couldn't find StatusPanel '%s'",pname);
      mStatusPanel = p;
    }
  }

  void T2InfoPanel::PaintComponent(Drawing & config) {
    static u32 bg = 0;
    bg += 0x010307;
    this->SetBackground(bg);
    Super::PaintComponent(config);
  }

  void T2InfoPanel::onTimeout(TimeQueue& srcTQ) {
#if 0 /*Not sleeping since we have no good wake to rewake when remote visibility returns.. */
    if (!IsVisibleFrom(0)) {
      sleepOn(srcTQ);           // Long timeout if not visible
      return;
    }
#endif

    insert(srcTQ,1024);         // Otherwise update at about ~1HZ

    u32 binsec = srcTQ.now() >> 10; // Tick every 1024 ms

    //// Rotate corner lights
    for (u32 i = 0; i < 4; ++i) {
      u32 color;
      if (0) { }
      else if (i==((binsec+3)&3)) color = Drawing::BLACK;
      else if (i==((binsec+2)&3)) color = Drawing::RED;
      else if (i==((binsec+1)&3)) color = Drawing::GREEN;
      else /*if (i==((binsec+0)&3))*/ color = Drawing::BLUE;
      mCornerLights[i]->SetForeground(color);
      mCornerLights[i]->SetBorder(color);
      mCornerLights[i]->SetBackground(color);
    }

    //// Update log
    //LOG TAKES CARE OF ITSELF    mLogPanel.update();

    //// Refresh cache values

    do { // Package tags
      ResettableByteSink & tagbs = mStaticPanel->GetByteSink();
      tagbs.Reset();

      s32 tmp;
      {
        const char * PATH = "/home/t2/cdmd-T2-12.mfz-cdm-install-tag.dat";
        if (!readOneDecimalNumberFile(PATH,tmp)) tagbs.Printf("T2-12: no tag\n");
        else tagbs.Printf("T2-12: %d\n",tmp);
      }
      {
        const char * PATH = "/home/t2/cdmd-MFM.mfz-cdm-install-tag.dat";
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
      mADCs.update();
      for (u32 i = 0; i < sizeof(mTNV) / sizeof(mTNV[0]); ++i) {
        mTNV[i] = mADCs.getChannelValue((T2ADCs::ADCChannel) i);
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
      u32 mUpdateCount = srcTQ.now();
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

#if 0
    do { // AER & AEPS
      const char * PATH = "/run/mfmt2/status.dat";
      double a[2];
      if (readFloatsFromFile(PATH, a, sizeof(a)/sizeof(a[0]))) {
        if (a[1] != mLastAEPS) {
          mLastAEPSChangeCount = mUpdateCount;
          mLastAEPS = a[1];
        }
        if (mLastAEPSChangeCount + 100 > mUpdateCount) mLastAER = a[0];
        else mLastAER = -1;
      } else mLastAER = -1; // Can't read file
    } while(0);
#endif

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

    refreshStatusPanel();
  }

  void T2InfoPanel::refreshStatusPanel() {
    MFM_API_ASSERT_NONNULL(mStatusPanel);
    ResettableByteSink & bs = mStatusPanel->GetByteSink();
    T2Tile& tile = T2Tile::get();
    ADCCtl & adc = tile.getADCCtl();
    bs.Reset();
    const u32 SIZE=32;
    char buf[SIZE+1];
    {
      double val = adc.mGridVoltage.getChannelValue();
      snprintf(buf,SIZE," %5.2fV\n",val);
      bs.Print(buf);
    }
    {
      double val = adc.mCoreTemperature.getChannelValue();
      snprintf(buf,SIZE," %5.1fF\n",val);
      bs.Print(buf);
    }
    {
      double val = adc.mEdgeTemperature.getChannelValue();
      snprintf(buf,SIZE," %5.1fF\n",val);
      bs.Print(buf);
    }
    {
      double val = adc.mLight.getChannelValue();
      snprintf(buf,SIZE," %5dL\n",(int)(val));
      bs.Print(buf);
    }
    {
      bs.Printf(" %5dM\n",mCPUMHz);
    }
    {
      snprintf(buf,SIZE,"%6.1fH\n",mUptime[0]/(60*60));
      bs.Print(buf);
    }
  }
}
