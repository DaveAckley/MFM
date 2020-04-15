#include "ADCCtl.h"

#include "T2Tile.h"

namespace MFM {
  ADCChannelWatcher::ADCChannelWatcher(T2ADCs::ADCChannel c, u32 refreshRate, T2ADCs & adcs, T2Tile & tile)
    : mChannel(c)
    , mRefreshRate(refreshRate)
    , mADCs(adcs)
    , mTile(tile)
  {
    tile.insertOnMasterTimeQueue(*this,refreshRate);
  }

  GridVoltageWatcher::GridVoltageWatcher(T2ADCs& adcs, T2Tile &tile)
    : ADCChannelWatcher(T2ADCs::ADC_CHNL_GRDVLT_A,1000,adcs,tile)
    , mLevel(RL_ACCEPTABLE)
  { }

  void GridVoltageWatcher::onUpdate() {
    double voltage = getChannelValue();
    ResourceLevel level = RL_ACCEPTABLE;
    if (0) { }
    else if (voltage < 8.0) level = RL_VERY_LOW;
    else if (voltage <10.0) level = RL_LOW;
    else if (voltage >20.0) level = RL_VERY_HIGH;
    else if (voltage >14.0) level = RL_HIGH;
    if (level != mLevel) {
      mLevel = level;
      mTile.resourceAlert(RT_GRID_VOLTAGE, mLevel);
    }
  }

  CoreTemperatureWatcher::CoreTemperatureWatcher(T2ADCs& adcs, T2Tile &tile)
    : ADCChannelWatcher(T2ADCs::ADC_CHNL_CTRTMP_A,1000,adcs,tile)
    , mLevel(RL_ACCEPTABLE)
  { }

  void CoreTemperatureWatcher::onUpdate() {
    double degf = getChannelValue();
    ResourceLevel level = RL_ACCEPTABLE;
    if (0) { }
    else if (degf < 40.0) level = RL_VERY_LOW;
    else if (degf < 60.0) level = RL_LOW;
    else if (degf >112.0) level = RL_VERY_HIGH;
    else if (degf >102.0) level = RL_HIGH;
    if (level != mLevel) {
      mLevel = level;
      mTile.resourceAlert(RT_CORE_TEMPERATURE, mLevel);
    }
  }

  EdgeTemperatureWatcher::EdgeTemperatureWatcher(T2ADCs& adcs, T2Tile &tile)
    : ADCChannelWatcher(T2ADCs::ADC_CHNL_EDGTMP_A,1000,adcs,tile)
    , mLevel(RL_ACCEPTABLE)
  { }

  void EdgeTemperatureWatcher::onUpdate() {
    double degf = getChannelValue();
    ResourceLevel level = RL_ACCEPTABLE;
    if (0) { }
    else if (degf < 40.0) level = RL_VERY_LOW;
    else if (degf < 60.0) level = RL_LOW;
    else if (degf >105.0) level = RL_VERY_HIGH;
    else if (degf > 99.0) level = RL_HIGH;
    if (level != mLevel) {
      mLevel = level;
      mTile.resourceAlert(RT_CORE_TEMPERATURE, mLevel);
    }
  }

  ButtonWatcher::ButtonWatcher(T2ADCs& adcs, T2Tile &tile)
    : ADCChannelWatcher(T2ADCs::ADC_CHNL_USER_ACT,100,adcs,tile)
    , mIsDown(false)
  { }
  
  void ButtonWatcher::onUpdate() {
    double val = getChannelValue();
    bool pressed = getChannelValue() > 0;
    if (pressed != mIsDown) {
      LOG.Message("Pushing fake isDown %d pressed %d val %f",mIsDown, pressed, val);
      mIsDown = pressed;
      SDL_Event fake;
      memset(&fake,0,sizeof(fake));
      fake.type = mIsDown ? SDL_KEYDOWN : SDL_KEYUP;
      fake.key.state = mIsDown ? SDL_PRESSED : SDL_RELEASED;
      fake.key.keysym.sym = SDLK_MENU;
      SDL_PushEvent(&fake);
    }
  }

  LightSensorWatcher::LightSensorWatcher(T2ADCs& adcs, T2Tile &tile)
    : ADCChannelWatcher(T2ADCs::ADC_CHNL_ADCLIGHT,200,adcs,tile)
    , mFastAvg(-1)
    , mSlowAvg(-1)
  { }
  
  void LightSensorWatcher::onUpdate() {
#define FASTAVG_RATE 0.75
#define SLOWAVG_RATE 0.95    
#define ACTION_THRESHOLD_FRAC 3    /* tripling/thirding */
    float val = getChannelValue() + 1; // Gtee non-zero
    if (mFastAvg < 0) mFastAvg = val;
    else mFastAvg = FASTAVG_RATE * mFastAvg + (1.0-FASTAVG_RATE) * val;

    if (mSlowAvg < 0) mSlowAvg = val;
    else mSlowAvg = SLOWAVG_RATE * mSlowAvg + (1.0-SLOWAVG_RATE) * val;

    s32 pressed = -1;
    double ratio = mFastAvg / mSlowAvg;
    if (ratio > ACTION_THRESHOLD_FRAC) pressed = 1; // Upward edge
    else if (ratio < 1.0/ACTION_THRESHOLD_FRAC) pressed = 0; // Downward edge

    if (pressed >= 0) {
      LOG.Message("Active light %d fast %f slow %f",
                  pressed, mFastAvg, mSlowAvg);

      // Going to signal a change.  Reset avgs
      mSlowAvg = mFastAvg = val;
      SDL_Event fake;
      memset(&fake,0,sizeof(fake));
      fake.type = SDL_KEYDOWN;
      fake.key.state = SDL_PRESSED;
      fake.key.keysym.sym = pressed > 0 ? SDLK_PAGEDOWN : SDLK_PAGEUP;
      SDL_PushEvent(&fake);
    }
  }

  ADCCtl::ADCCtl(T2Tile & tile)
    : mADCs()
    , mGridVoltage(mADCs,tile)
    , mCoreTemperature(mADCs,tile)
    , mEdgeTemperature(mADCs,tile)
    , mButton(mADCs,tile)
    , mLight(mADCs,tile)
  { }
}
