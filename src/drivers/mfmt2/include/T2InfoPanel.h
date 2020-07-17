/* -*- C++ -*- */
#ifndef T2INFOPANEL_H
#define T2INFOPANEL_H

#include "TextPanel.h"
#include "TimeoutAble.h"
#include "SDLI.h"
#include "ITCStatus.h"
#include "ITCStatusPanel.h"
#include "ITCIcons.h"
#include "T2ADCs.h"
#include "Menu.h"
#include "dirdatamacro.h"  // For DIR6*

namespace MFM {

  typedef TextPanel<12,12> StatusPanel;

  typedef TextPanel<32,3> StaticPanel;

  typedef TextPanel<17,7> HistoPanel;

  struct T2InfoPanel : public MenuMakerPanel, public TimeoutAble, public SDLIConfigurable {
    typedef MenuMakerPanel Super;
    virtual void PaintComponent(Drawing & config) ;

    T2InfoPanel() ;

    void configure(SDLI & sdli) ;

    virtual void onTimeout(TimeQueue& srcTQ) ;
    virtual const char * getName() const { return "T2InfoPanel"; }

    virtual ~T2InfoPanel() { }

    virtual void addItems(Menu& menu) {
      DIE_UNIMPLEMENTED();
    }

    void refreshStatusPanel() ;

    void refreshHistoPanel() ;
  private:
    Panel * mCornerLights[4];
    Panel * mITCPanels[DIR6_COUNT];
    Panel * mLogPanel;
    StatusPanel * mStatusPanel;
    HistoPanel * mHistoPanel;
    StaticPanel * mStaticPanel;

    u32 mCPUMHz;
    double mTNV[7];
    double mLoadAvg[3];
    double mUptime[2];
    double mLastAER;
    double mLastAEPS;
    u32 mLastAEPSChangeCount;
    u32 mLastT2StatsAge;
    T2TileStats mLastT2Stats;
    ITCStatus mITCStatuses[DIR6_COUNT];
    ITCIcons mITCIcons;

    double mLastCDMTime;
    u32 mLastCDMChangeCount;

    T2ADCs mADCs;

  };
}

#endif /* T2INFOPANEL_H */
