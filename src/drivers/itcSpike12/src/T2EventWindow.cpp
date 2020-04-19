#include "T2EventWindow.h"
#include "T2Tile.h"
#include "Packet.h"

namespace MFM {

  void T2EventWindow::onTimeout(TimeQueue& srcTq) {
    if (mStateNum >= T2EWStateOps::mStateOpsArray.size())
      FAIL(ILLEGAL_STATE);
    T2EWStateOps * ops = T2EWStateOps::mStateOpsArray[mStateNum];
    if (!ops)
      FAIL(ILLEGAL_STATE);
    PacketBuffer pb;
    ops->timeout(*this, pb);
  }

  T2EventWindow::T2EventWindow(T2Tile& tile, EWSlotNum ewsn)
    : mTile(tile)
    , mSlotNum(ewsn)
    , mStateNum(EWSN_IDLE)
    , mCenter(0,0)
    , mRadius(0) // mRadius 0 means not in use
  { }

  /**** LATE EVENTWINDOW STATES HACKERY ****/

  /*** DEFINE STATEOPS SINGLETONS **/
#define XX(NAME,CUSTO,CUSRC,STUB,DESC) static T2EWStateOps_##NAME singletonT2EWStateOps_##NAME;
  ALL_EW_STATES_MACRO()
#undef XX

  /*** DEFINE EWSTATENUMBER -> STATEOPS MAPPING **/
  T2EWStateOps::T2EWStateArray T2EWStateOps::mStateOpsArray = {
#define XX(NAME,CUSTO,CUSRC,STUB,DESC) &singletonT2EWStateOps_##NAME,
  ALL_EW_STATES_MACRO()
#undef XX
    0
  };

  /*** DEFINE STUBBED-OUT STATE METHODS **/
#define YY00(NAME,FUNC) 
#define YY01(NAME,FUNC) 
#define YY10(NAME,FUNC) 
#define YY11(NAME,FUNC) void T2EWStateOps_##NAME::FUNC(T2EventWindow & ew, PacketBuffer & pb) { DIE_UNIMPLEMENTED(); }
#define XX(NAME,CUSTO,CUSRC,STUB,DESC) \
  YY##CUSTO##STUB(NAME,timeout)        \
  YY##CUSRC##STUB(NAME,receive)        \
  
  ALL_EW_STATES_MACRO()
#undef XX
#undef YY11
#undef YY10
#undef YY01
#undef YY00

  /*** STATE NAMES AS STRING **/
  const char * ewStateName[] = {
#define XX(NAME,CUSTO,CUSRC,STUB,DESC) #NAME,
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
#define XX(NAME,CUSTO,CUSRC,STUB,DESC) DESC,
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

  
  //// DEFAULT HANDLERS FOR T2EWStateOps
  void T2EWStateOps::timeout(T2EventWindow & ew, PacketBuffer & pb) {
    DIE_UNIMPLEMENTED();
  }

  void T2EWStateOps::receive(T2EventWindow & ew, PacketBuffer & pb) {
    DIE_UNIMPLEMENTED();
  }

  //// AINIT: CUSTOM STATE HANDLERS
  void T2EWStateOps_AINIT::timeout(T2EventWindow & ew, PacketBuffer & pb) {
    DIE_UNIMPLEMENTED();
  }

}
