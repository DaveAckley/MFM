#include "T2InfoPanel.h"
#include <algorithm>

#include "TimeQueue.h"
#include "T2Utils.h"
#include "SimLogPanel.h"
#include "T2Tile.h"

#include "itcpktevent.h"

namespace MFM {
  T2InfoPanel::T2InfoPanel() 
    : mCornerLights{0}
    , mITCPanels{0}
    , mLogPanel(0)
    , mStatusPanel(0)
    , mHistoPanel(0)
    , mStaticPanel(0)
  {
    for (Dir6 dir6 = DIR6_MIN; dir6 <= DIR6_MAX; ++dir6) {
      mITCStatuses[dir6].setDir6Idx(dir6);
    }
  }

  void T2InfoPanel::configure(SDLI & sdli) {
    mITCIcons.init(sdli.getScreen());

    OString128 buf;
    Dir6 dirs[] = {DIR6_NW, DIR6_NE, DIR6_SE, DIR6_SW};
    for (u32 i = 0; i < 4; ++i) {
      buf.Reset();
      buf.Printf("Corner_%s",getDir6Name(dirs[i]));
      const char * pname = buf.GetZString();
      Panel * p = sdli.lookForPanel(pname);
      if (!p) fatal("Couldn't find panel '%s'",pname);
      mCornerLights[i] = p;
    }
    for (Dir6 dir6 = DIR6_MIN; dir6 <= DIR6_MAX; ++dir6) {
      buf.Reset();
      buf.Printf("ITC_%s",getDir6Name(dir6));
      const char * pname = buf.GetZString();
      ITCStatusPanel * p = dynamic_cast<ITCStatusPanel*>(sdli.lookForPanel(pname));
      if (!p) fatal("Couldn't find ITCStatusPanel '%s'",pname);
      mITCPanels[dir6] = p;
      p->init(mITCStatuses[dir6], mITCIcons);
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
    {
      const char * pname = "TypeHistogram";
      HistoPanel * p = dynamic_cast<HistoPanel*>(sdli.lookForPanel(pname));
      if (!p) fatal("Couldn't find HistoPanel '%s'",pname);
      mHistoPanel = p;
    }
  }

  void T2InfoPanel::PaintComponent(Drawing & config) {
#if 0
    static u32 bg = 0;
    bg += 0x010307;
    this->SetBackground(bg);
#endif
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
      T2Tile & tile = T2Tile::get();
      for (u32 dir6 = DIR6_MIN; dir6 <= DIR6_MAX; ++dir6) {
        T2ITC & itc = tile.getITC(dir6);
        ITCStateNumber itcsn = itc.getITCSN();
        mITCStatuses[dir6].setIsOpen(itcsn > ITCSN_SHUT);
        mITCStatuses[dir6].setIsAlive(itcsn >= ITCSN_OPEN);
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
      for (Dir6 dir8 = 0; dir8 < DIR8_COUNT; ++dir8) {
        Dir6 dir6 = mapDir8ToDir6(dir8);
        if (dir6 == DIR6_COUNT) 
          fbs.Scanf("%[^\n]\n",0);  // Don't care about this line either
        else mITCStatuses[dir6].updateStatsFromLine(fbs);
      }
      fbs.Close();
    } while(0);

    refreshStatusPanel();
    refreshHistoPanel();
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

  typedef std::pair<u32,u32> TypeAndCount;
  struct secondGtr {
    bool operator()(const TypeAndCount & left, const TypeAndCount & right) {
      return left.second > right.second;
    }
  };

  void T2InfoPanel::refreshHistoPanel() {
    MFM_API_ASSERT_NONNULL(mHistoPanel);
    ResettableByteSink & bs = mHistoPanel->GetByteSink();
    bs.Reset();

    T2Tile& tile = T2Tile::get();
    Sites & sites = tile.getSites();
    AtomTypeCountMap histo;
    for (u32 i = CACHE_LINES; i < T2TILE_WIDTH-CACHE_LINES; ++i) {
      for (u32 j = CACHE_LINES; j < T2TILE_HEIGHT-CACHE_LINES; ++j) {
        OurT2Site & site = sites.get(UPoint(i,j));
        const OurT2Atom & atom = site.GetAtom();
        histo[atom.GetType()]++;
      }
    }

    typedef std::vector<TypeAndCount> AtomTypeCountVector;
    AtomTypeCountVector vec(histo.begin(), histo.end());
    std::sort(vec.begin(), vec.end(), secondGtr());

    const u32 cSITES = T2TILE_OWNED_WIDTH * T2TILE_OWNED_HEIGHT;
    const u32 SIZE=32;
    char buf[SIZE+1];
    for (AtomTypeCountVector::iterator itr = vec.begin(); itr != vec.end(); ++itr) {
      u32 type = itr->first;
      u32 count = itr->second;
      double pct = 100.0 * count / cSITES;
      if (pct >= 10) snprintf(buf,SIZE,"%3d",(u32) pct);
      else if (pct >= 1) snprintf(buf,SIZE,"%3.1f",pct);
      else snprintf(buf,SIZE,".%02d",(u32) (100*pct));
      bs.Printf("%s%% %4d #%04x\n", buf, count, type);
    }
  }
}
