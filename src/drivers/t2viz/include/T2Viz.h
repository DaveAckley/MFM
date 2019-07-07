/* -*- C++ -*- */
#ifndef T2VIZ_H
#define T2VIZ_H

#include "SDL.h"

#include "Drawing.h"
#include "Panel.h"
#include "TextPanel.h"
#include "Label.h"
#include "TeeByteSink.h"

#include "Model.h"
#include "ITCStatus.h"
#include "ITCStatusPanel.h"
#include "LogPanel.h"
#include "ITCIcons.h"
#include "T2VizConstants.h"


namespace MFM {

  struct T2Viz : public Model {

    Panel mRootPanel;
    Panel mContentPanel; // Everything but the ITCs
    typedef TextPanel<50,9> DeltasPanel;
    DeltasPanel mDeltasPanel;  
    LogPanel mLogPanel;

    Panel mCornerLights[4];
    u32 mUpdateCount;

    typedef TextPanel<12,12> StatusPanel;
    StatusPanel mStatusPanel;

    typedef TextPanel<32,3> StaticPanel;
    StaticPanel mStaticPanel;

    Drawing mRootDrawing;

    ITCIcons mITCIcons;

    //// Static data
    OString64 mSecretSerialNumber;
    OString64 mMFMInstallTag;

    //// Cache data
    u32 mCPUMHz;
    double mTNV[7];
    double mLoadAvg[3];
    double mUptime[2];
    double mLastAER;
    double mLastAEPS;
    u32 mLastAEPSChangeCount;
    ITCStatus mITCs[ITC_COUNT];
    ITCStatusPanel mITCPanels[ITC_COUNT];

    double mLastCDMTime;
    u32 mLastCDMChangeCount;

    T2Viz(SDL_Surface *) ;
    virtual void input(SDLI&,Event&) ;
    virtual void update(SDLI&) ;
    virtual void output(SDLI&) ;
    virtual ~T2Viz() { }
  };
}

#endif /* T2VIZ_H */
