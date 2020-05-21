/* -*- C++ -*- */
#ifndef TIMEOUTABLE_H
#define TIMEOUTABLE_H

#include "itype.h"
#include "Dirs.h"
#include "DateTimeStamp.h"
#include "Random.h"

//Spike files
#include "T2Constants.h"

#define time_after(a,b) ((long)((b) - (a)) < 0)
#define time_before(a,b) time_after(b,a)
#define time_after_eq(a,b) ((long)((a) - (b)) >= 0)
#define time_before_eq(a,b) time_after_eq(b,a)

#define LONG_SLEEP_MS (60*60*1000)   /* 1 hour */

namespace MFM {
  struct TimeQueue; // FORWARD

  typedef enum waitcode {
    WC_NOW,                     
    WC_HALF,   
    WC_FULL,   
    WC_LONG,   
    WC_RANDOM, 
    WC_RANDOM_SHORT, 
    MAX_WAIT_CODE
  } WaitCode;

  typedef enum waitms {
    WC_HALF_MS = 150,
    WC_FULL_MS = 300,

    WC_LONG_MIN_MS = 10000,
    WC_LONG_MAX_MS = 15000,
    WC_LONG_WIDTH = WC_LONG_MAX_MS - WC_LONG_MIN_MS+1,

    WC_RANDOM_MIN_MS = 30,
    WC_RANDOM_MAX_MS = 1500,

    WC_RANDOM_WIDTH = WC_RANDOM_MAX_MS - WC_RANDOM_MIN_MS+1,

    WC_RANDOM_SHORT_MIN_MS = 1,
    WC_RANDOM_SHORT_MAX_MS = 10,

    WC_RANDOM_SHORT_WIDTH = WC_RANDOM_SHORT_MAX_MS - WC_RANDOM_SHORT_MIN_MS+1

  } WaitMs;

  struct TimeoutAble {

    //// TimeoutAble API METHODS:
    virtual void onTimeout(TimeQueue& srcTq) = 0;
    virtual const char * getName() const = 0;

    TimeoutAble() ;

    virtual ~TimeoutAble() ;
    void insert(TimeQueue& onTQ, u32 thisFarInFutureMs, s32 fuzzbits=-1);
    void remove() ;

    void schedule(TimeQueue& tq, u32 now = 0, s32 fuzzbits=-1) { // Schedule for now (or then)
      if (isOnTQ()) remove();
      insert(tq,now,fuzzbits);
    }

    void scheduleWait(WaitCode wc) ;

    u32 getTimeout() const { return mTimeMS; }
    void printTimeout(ByteSink & bs) const ;

    bool isOnTQ() const { return mOnTQ != 0; }

    /** Reschedule this under the assumption that external
        circumstances relevant to this may have changed.  

        By default, bump() reschedules this to wake up immediately, if
        it is currently waiting on a TQ, or else does nothing.
    */
    virtual void bump() {
      if (mOnTQ !=0) {
        TimeQueue & tq = *mOnTQ;
        remove();
        insert(tq,0);
      }
    }

    void sleepOn(TimeQueue & onTQ) {
      insert(onTQ, LONG_SLEEP_MS);
    }

    u32 mTimeMS;
    u32 mNonce;
    TimeQueue * mOnTQ;

    bool operator<(const TimeoutAble & rhs) const {
      // Round 0: Shortcut identity
      if (this == &rhs) return false;

      // Round 1: Check real content
      if (time_before(mTimeMS, rhs.mTimeMS)) return true;
      if (time_after(mTimeMS, rhs.mTimeMS)) return false;

      // Round 2: Break content ties randomly
      if (mNonce < rhs.mNonce) return true;
      if (mNonce > rhs.mNonce) return false;

      // Round 3: Break random ties arbitrarily 
      return (intptr_t) this < (intptr_t) &rhs; // Reachable: Sure. Reached?
    }

  };
}
#endif /* TIMEOUTABLE_H */
