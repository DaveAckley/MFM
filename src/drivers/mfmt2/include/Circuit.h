/* -*- C++ -*- */
#ifndef CIRCUIT_H
#define CIRCUIT_H

#include "T2Types.h"

namespace MFM {

  struct T2EventWindow; // FORWARD
  struct T2ITC; // FORWARD
  
#define ALL_CIRCUIT_STATES_MACRO()                             \
  /*  NM     ITC LKH  D */                                     \
  XX(UNUSED,   0, 0, "other fields are invalid")               \
  XX(BOUND,    1, 0, "bound to an ITC")                        \
  XX(RUNG,     1, 0, "RING sent (active), recvd (passive)")    \
  XX(ANSWERED, 1, 1, "ANSWER recvd (active), sent (passive)")  \
  XX(DECLINED, 1, 0, "BUSY recvd (active), sent (passive)")    \
  XX(DROPPED,  1, 0, "DROP send (active), recv (passive)")     \
  XX(TALKED,   1, 1, "Cache upds sent/in progress (active)")   \
  XX(HUNGUP,   0, 0, "Active side, done, free -> UNUSED")      \
  /*ALL_CIRCUIT_STATES_MACRO*/  
  
  enum CircuitState {
#define XX(NM,ITC,LKH,D) CS_##NM,
    ALL_CIRCUIT_STATES_MACRO()                        
#undef XX
    CS_STATE_COUNT
  };

  const char * getCircuitStateName(CircuitState cs) ;
  const char * getCircuitStateDescription(CircuitState cs) ;
  bool isITCBoundInCircuitState(CircuitState cs) ;
  bool isLockHeldInCircuitState(CircuitState cs) ;
  bool isDropNeededInCircuitState(CircuitState cs) ;

  struct Circuit {

    Circuit(T2EventWindow &) ;
    ~Circuit() ;

    //// PREDICATES
    bool isBound() const { return getITCIfAny() != 0; }
    bool isLockNeeded() const ;
    bool isLockRequested() const ;
    bool isLockHeld() const ;
    bool isDropNeeded() const ;

    void setCS(CircuitState cs) ;
    CircuitState getCS() const { return mState; }

    T2EventWindow & getEW() { return mEventWindow; }
    const T2EventWindow & getEW() const { return mEventWindow; }

    const T2ITC * getITCIfAny() const { return mITC; }
    T2ITC * getITCIfAny() { return mITC; }
    const T2ITC & getITC() const ;
    T2ITC & getITC() ;

    CircuitNum getCircuitNum() const ;
    s8 getMaxUnshippedSN() const { return mMaxUnshippedSN; }
    void setMaxUnshippedSN(u8 sn) { mMaxUnshippedSN = sn; }
        
    void setITC(T2ITC& itc) ;
    void clearITC() ;

    void setYoink(u8 zeroOrOne) {
      MFM_API_ASSERT_ARG(zeroOrOne < 2);
      MFM_API_ASSERT_STATE(getYoink() < 0);
      mYoinkValue = (s8) zeroOrOne;
    }
    void clearYoink() {
      MFM_API_ASSERT_STATE(getYoink() >= 0);
      mYoinkValue = -1;
    }
    s8 getYoink() const { return mYoinkValue; }

    void bindCircuit(T2ITC& itc) ;
    void unbindCircuit() ;

    void bindCircuitForPassive(T2ITC& itc) ;
    void resetCircuitForPassive() ;

    void abortCircuit() ;
    const char * getName() const ;
  private:
    static const u32 MBUF_SIZE = 100;
    char * mBuf;
    void reinit() ;
    T2EventWindow & mEventWindow;
    CircuitState mState;
    T2ITC * mITC;
    s8 mMaxUnshippedSN;
    s8 mYoinkValue;
  };

}

#endif /* CIRCUIT_H */
