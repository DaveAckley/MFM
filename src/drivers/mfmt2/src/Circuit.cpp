#include "Circuit.h"
#include "T2EventWindow.h"
#include "T2ITC.h"
#include "T2Tile.h"
#include "Trace.h"

namespace MFM {

  const char * getCircuitStateName(CircuitState cs) {
    switch (cs) {
    default: return "?illegal";
#define XX(NM,ITC,LKH,D) case CS_##NM: return #NM;
     ALL_CIRCUIT_STATES_MACRO()
#undef XX
    }
  }

  const char * getCircuitStateDescription(CircuitState cs) {
    switch (cs) {
    default: return "?illegal";
#define XX(NM,ITC,LKH,D) case CS_##NM: return D;
     ALL_CIRCUIT_STATES_MACRO()
#undef XX
    }
  }

  bool isITCBoundInCircuitState(CircuitState cs) {
    switch (cs) {
    default: FAIL(ILLEGAL_ARGUMENT);
#define XX(NM,ITC,LKH,D) case CS_##NM: return ITC!=0;
     ALL_CIRCUIT_STATES_MACRO()
#undef XX
    }
  }

  bool isDropNeededInCircuitState(CircuitState cs) {
    return cs >= CS_RUNG && cs <= CS_TALKED;
  }
  bool isLockHeldInCircuitState(CircuitState cs) {
    switch (cs) {
    default: FAIL(ILLEGAL_ARGUMENT);
#define XX(NM,ITC,LKH,D) case CS_##NM: return LKH!=0;
     ALL_CIRCUIT_STATES_MACRO()
#undef XX
    }
  }

  const char * Circuit::getName() const {
    snprintf(mBuf,MBUF_SIZE,"C_%s(%sx%s)",
             getCircuitStateName(mState),
             mEventWindow.getName(),
             mITC ? mITC->getName() : "unbound");
    return mBuf;
  }

  Circuit::Circuit(T2EventWindow & ew)
    : mBuf(new char[MBUF_SIZE])
    , mEventWindow(ew)
  {
    reinit();
  }

  Circuit::~Circuit() {
    delete [] mBuf;
    mBuf = 0;
  }

  bool Circuit::isLockNeeded() const {
    return isBound() && getITC().isCacheUsable();
  }

  bool Circuit::isLockRequested() const {
    return isLockNeeded() && getYoink() >= 0;
  }

  bool Circuit::isLockHeld() const {
    return isLockHeldInCircuitState(mState);
  }

  bool Circuit::isDropNeeded() const {
    return isDropNeededInCircuitState(mState);
  }

  void Circuit::setCS(CircuitState cs) {
    MFM_API_ASSERT_ARG((u32) cs < CS_STATE_COUNT);
    if (mState == cs) return;
    T2EventWindow & ew = getEW();
    T2Tile & tile = ew.getTile();
    T2ITC * pitc = getITCIfAny();
    tile.tlog(Trace(ew,TTC_EW_CircuitStateChange,"%c%c%c",
                    pitc ? pitc->mDir6 : 0xff,
                    mState,
                    cs));
    mState = cs;
  }

  void Circuit::setITC(T2ITC& itc) {
    MFM_API_ASSERT_STATE(mEventWindow.isActiveEW());
    MFM_API_ASSERT_STATE(mITC == 0);
    mITC = &itc;
    mITC->registerActiveCircuitRaw(*this);
  }

  void Circuit::clearITC() {
    MFM_API_ASSERT_STATE(mEventWindow.isActiveEW());
    MFM_API_ASSERT_STATE(mITC != 0);
    mITC->unregisterActiveCircuitRaw(*this);
    mITC = 0;
  }

  void Circuit::abortCircuit() {
    if (mEventWindow.isActiveEW()) 
      clearITC();
    reinit();
  }

  T2ITC& Circuit::getITC() {
    MFM_API_ASSERT_STATE(mITC != 0);
    return *mITC;
  }

  const T2ITC& Circuit::getITC() const {
    MFM_API_ASSERT_STATE(mITC != 0);
    return *mITC;
  }

  void Circuit::bindCircuit(T2ITC& itc) {
    MFM_API_ASSERT_STATE(mITC == 0);
    reinit();
    setITC(itc);
    setCS(CS_BOUND);
  }

  void Circuit::bindCircuitForPassive(T2ITC& itc) {
    MFM_API_ASSERT_STATE(mITC == 0);
    reinit();
    mITC = &itc;
    mState = CS_BOUND;
  }

  void Circuit::resetCircuitForPassive() {
    mState = CS_BOUND;
    mMaxUnshippedSN = S8_MAX;
    mYoinkValue = -1;
  }

  void Circuit::unbindCircuit() {
    setCS(CS_UNUSED);
    clearITC();
    reinit();
  }

  void Circuit::reinit() {
    mState = CS_UNUSED;
    mITC = 0;
    mMaxUnshippedSN = S8_MAX;
    mYoinkValue = -1;
  }

  CircuitNum Circuit::getCircuitNum() const {
    return mEventWindow.getSlotNum();
  }

}
