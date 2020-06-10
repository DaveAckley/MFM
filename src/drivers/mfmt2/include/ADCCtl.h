/* -*- C++ -*- */
#ifndef ADCCTL_H
#define ADCCTL_H

#include <string.h>  /*for memset*/
#include "SDL.h"

#include "T2Constants.h"
#include "T2ADCs.h"
#include "TimeoutAble.h"

namespace MFM {

  struct T2Tile; // FORWARD

  struct ADCChannelWatcher : public TimeoutAble {
    const T2ADCs::ADCChannel mChannel;
    const u32 mRefreshRate;
    T2ADCs & mADCs;
    T2Tile & mTile;
    ADCChannelWatcher(T2ADCs::ADCChannel c, u32 refreshRate, T2ADCs & adcs, T2Tile & tile) ;

    virtual void onUpdate() = 0;

    virtual void onTimeout(TimeQueue& srcTq) {
      insert(srcTq, mRefreshRate);
      mADCs.updateChannel(mChannel);
      onUpdate();
    }

    virtual const char * getName() const {
      return mADCs.getChannelName(mChannel);
    }

    const char * getChannelDescription() const {
      return mADCs.getChannelDescription(mChannel);
    }

    unsigned getChannelRawData() const {
      return mADCs.getChannelRawData(mChannel);
    }
    
    double getChannelValue() {
      return mADCs.getChannelValue(mChannel);
    }
  };

  struct GridVoltageWatcher : ADCChannelWatcher {
    GridVoltageWatcher(T2ADCs& adcs, T2Tile &tile) ;

    virtual void onUpdate() ;
    ResourceLevel mLevel;
  };

  struct CoreTemperatureWatcher : ADCChannelWatcher {
    CoreTemperatureWatcher(T2ADCs& adcs, T2Tile &tile) ;
    virtual void onUpdate() ;
    ResourceLevel mLevel;
  };

  struct EdgeTemperatureWatcher : ADCChannelWatcher {
    EdgeTemperatureWatcher(T2ADCs& adcs, T2Tile &tile) ;
    virtual void onUpdate() ;
    ResourceLevel mLevel;
  };

  struct ButtonWatcher : ADCChannelWatcher {
    ButtonWatcher(T2ADCs& adcs, T2Tile &tile) ;
    virtual void onUpdate() ;
    bool mIsDown;
  };

  struct LightSensorWatcher : ADCChannelWatcher {
    LightSensorWatcher(T2ADCs& adcs, T2Tile &tile) ;
    virtual void onUpdate() ;
    float mFastAvg;
    float mSlowAvg;
  };

  struct ADCCtl {
    ADCCtl(T2Tile & tile) ;

    ~ADCCtl() { }
    T2ADCs mADCs;                  
    GridVoltageWatcher mGridVoltage;
    CoreTemperatureWatcher mCoreTemperature;
    EdgeTemperatureWatcher mEdgeTemperature;
    ButtonWatcher mButton;
    LightSensorWatcher mLight;
  };
}

#endif /* ADCCTL_H */
