#include "T2EventWindow.h"
#include "T2Tile.h"

namespace MFM {

  void T2EventWindow::onTimeout(TimeQueue& srcTq) {
    Random & r = mTile.getRandom();
    if (r.OneIn(10)) mTile.releaseEW(this);
    else insert(srcTq, r.Between(3,10));
  }

  T2EventWindow::T2EventWindow(T2Tile& tile, EWSlotNum ewsn)
    : mTile(tile)
    , mSlotNum(ewsn)
    , mStateNum(EWSN_IDLE)
    , mStatus(UNKNOWN)
    , mCenter(0,0)
    , mRadius(0) // mRadius 0 means not in use
  { }

  /**** LATE EVENTWINDOW STATES HACKERY ****/

  /*** STATE NAMES AS STRING **/
  const char * ewStateName[] = {
#define XX(NAME,CUSTO,CUSRC,DESC) #NAME,
  ALL_EW_STATES_MACRO()
#undef XX
  "?ILLEGAL"
  };

  const char * getEWStateName(EWStateNumber sn) {
    if (sn >= MAX_EW_STATE_NUMBER) return "illegal";
    return ewStateName[sn];
  }

  /*** STATE DESCRIPTIONS AS STRING **/
  const char * ewStateDesc[] = {
#define XX(NAME,CUSTO,CUSRC,DESC) DESC,
  ALL_EW_STATES_MACRO()
#undef XX
  "?ILLEGAL"
  };

  const char * getEWStateDescription(EWStateNumber sn) {
    if (sn >= MAX_EW_STATE_NUMBER) return "illegal";
    return ewStateDesc[sn];
  }

  const char * T2EventWindow::getName() const {
    static char buf[100];
    snprintf(buf,100,"EW%02d/%s",
             mSlotNum,
             getEWStateName(mStateNum));
    return buf;
  }
  /*
  void T2EventWindow::update() {
    debug("update: %p %s: %s\n",
          this,
          getEWStateName(mStateNum),
          getEWStateDescription(mStateNum));
    DIE_UNIMPLEMENTED();
  }
  */
}
