/* -*- C++ -*- */
#ifndef T2INFOPANEL_H
#define T2INFOPANEL_H

#include "TextPanel.h"
#include "TimeoutAble.h"
#include "SDLI.h"
#include "ITCStatus.h"
#include "T2ADCs.h"
#include "Menu.h"

namespace MFM {

  typedef TextPanel<12,12> StatusPanel;

  typedef TextPanel<32,3> StaticPanel;

  struct T2InfoPanel : public MenuMakerPanel, public TimeoutAble, public SDLIConfigurable {
    typedef MenuMakerPanel Super;
    virtual void PaintComponent(Drawing & config) ;

    T2InfoPanel()
      : mCornerLights{0}
      , mLogPanel(0)
      , mStatusPanel(0)
      , mStaticPanel(0)
    { }
    void configure(SDLI & sdli) ;

    virtual void onTimeout(TimeQueue& srcTQ) ;
    virtual const char * getName() const { return "T2InfoPanel"; }

    virtual ~T2InfoPanel() { }

    virtual void addItems(Menu& menu) {
      DIE_UNIMPLEMENTED();
    }

    void refreshStatusPanel() ;
  private:
    Panel * mCornerLights[4];
    Panel * mLogPanel;
    StatusPanel * mStatusPanel;
    StaticPanel * mStaticPanel;

    u32 mCPUMHz;
    double mTNV[7];
    double mLoadAvg[3];
    double mUptime[2];
    double mLastAER;
    double mLastAEPS;
    u32 mLastAEPSChangeCount;
    ITCStatus mITCs[ITC_COUNT];

    double mLastCDMTime;
    u32 mLastCDMChangeCount;

    T2ADCs mADCs;

  };
}

#endif /* T2INFOPANEL_H */
